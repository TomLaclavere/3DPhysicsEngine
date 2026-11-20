/**
 * @file main_viewer.cpp
 * @brief Real-time 3D replay of the CSV files written by PhysicsWorld.
 *
 * Usage: 3DPhysicsViewer <csv_directory>
 *
 * The directory must contain objects.csv and one motion_object_N.csv per object.
 */
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
struct SceneObject
{
    std::string          type;
    Vector3              size {};
    Vector3              normal {};
    bool                 fixed = false;
    std::vector<Vector3> path;
};

/// Same palette as python/utilities/object_utilities.py.
const std::array<Color, 8> PALETTE = { Color { 255, 121, 198, 255 }, Color { 139, 233, 253, 255 },
                                       Color { 80, 250, 123, 255 },  Color { 255, 184, 108, 255 },
                                       Color { 189, 147, 249, 255 }, Color { 255, 85, 85, 255 },
                                       Color { 241, 250, 140, 255 }, Color { 98, 114, 164, 255 } };

const char* const VERTEX_SHADER = R"(#version 330
in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 mvp;
uniform mat4 matNormal;
out vec3 normal;
void main()
{
    normal = normalize(vec3(matNormal * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
})";

const char* const FRAGMENT_SHADER = R"(#version 330
in vec3 normal;
uniform vec4 colDiffuse;
out vec4 finalColor;
void main()
{
    float light = 0.35 + 0.65 * max(dot(normalize(normal), normalize(vec3(0.4, -0.6, 0.7))), 0.0);
    finalColor = vec4(colDiffuse.rgb * light, colDiffuse.a);
})";

std::vector<SceneObject> loadObjects(const std::filesystem::path& file)
{
    std::ifstream input(file);
    if (!input)
    {
        std::cerr << "Cannot open " << file << '\n';
        return {};
    }

    std::vector<SceneObject> objects;
    std::string              line;
    std::getline(input, line); // header
    while (std::getline(input, line))
    {
        std::vector<std::string> fields;
        std::stringstream        stream(line);
        std::string              field;
        while (std::getline(stream, field, ','))
            fields.push_back(field);
        if (fields.size() < 14)
            continue;

        SceneObject object;
        object.type   = fields[2];
        object.size   = { std::stof(fields[7]), std::stof(fields[8]), std::stof(fields[9]) };
        object.normal = { std::stof(fields[10]), std::stof(fields[11]), std::stof(fields[12]) };
        object.fixed  = fields[13] != "0";
        objects.push_back(object);
    }
    return objects;
}

/// Read a motion file. Only the time and position columns are used.
bool loadPath(const std::filesystem::path& file, std::vector<float>& times, std::vector<Vector3>& path)
{
    std::ifstream input(file);
    if (!input)
    {
        std::cerr << "Cannot open " << file << '\n';
        return false;
    }

    times.clear();
    std::string line;
    std::getline(input, line); // header
    while (std::getline(input, line))
    {
        const char* cursor = line.c_str();
        char*       end    = nullptr;
        std::array<float, 4> values {};
        for (float& value : values)
        {
            value  = std::strtof(cursor, &end);
            cursor = (*end == ',') ? end + 1 : end;
        }
        times.push_back(values[0]);
        path.push_back({ values[1], values[2], values[3] });
    }
    return true;
}

/// Corners of a plane, using the same tangent axes and extents as Plane in the engine.
std::array<Vector3, 4> planeCorners(const SceneObject& plane, Vector3 centre)
{
    const Vector3 n       = Vector3Normalize(plane.normal);
    const Vector3 tangent = (std::abs(n.x) < 0.9f) ? Vector3 { 1.f, 0.f, 0.f } : Vector3 { 0.f, 1.f, 0.f };
    const Vector3 u = Vector3Normalize(Vector3Subtract(tangent, Vector3Scale(n, Vector3DotProduct(tangent, n))));
    const Vector3 v = Vector3Normalize(Vector3CrossProduct(n, u));

    const Vector3 alongU = Vector3Scale(u, plane.size.y * 0.5f); // halfWidth
    const Vector3 alongV = Vector3Scale(v, plane.size.x * 0.5f); // halfHeight
    return { Vector3Subtract(Vector3Subtract(centre, alongU), alongV),
             Vector3Add(Vector3Subtract(centre, alongU), alongV),
             Vector3Add(Vector3Add(centre, alongU), alongV),
             Vector3Subtract(Vector3Add(centre, alongU), alongV) };
}

void drawPlane(const SceneObject& plane, Vector3 centre, Color color)
{
    const auto  corners = planeCorners(plane, centre);
    const Color fill    = Fade(color, 0.25f);
    DrawTriangle3D(corners[0], corners[1], corners[2], fill);
    DrawTriangle3D(corners[0], corners[2], corners[3], fill);
    for (size_t i = 0; i < corners.size(); ++i)
        DrawLine3D(corners[i], corners[(i + 1) % corners.size()], color);
}
} // namespace

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <csv_directory>\n";
        return 1;
    }
    const std::filesystem::path directory = argv[1];

    std::vector<SceneObject> objects = loadObjects(directory / "objects.csv");
    if (objects.empty())
    {
        std::cerr << "No object found in " << directory << '\n';
        return 1;
    }

    std::vector<float> times;
    size_t             frameCount = 0;
    for (size_t i = 0; i < objects.size(); ++i)
    {
        if (!loadPath(directory / ("motion_object_" + std::to_string(i) + ".csv"), times, objects[i].path))
            return 1;
        frameCount = (i == 0) ? objects[i].path.size() : std::min(frameCount, objects[i].path.size());
    }
    if (frameCount == 0)
    {
        std::cerr << "The motion files in " << directory << " are empty\n";
        return 1;
    }

    const float duration = (frameCount > 1) ? times[frameCount - 1] - times[0] : 0.f;
    const float step     = (frameCount > 1) ? duration / static_cast<float>(frameCount - 1) : 0.f;

    // Frame the objects and the plane centres: fixed planes can be huge
    Vector3 low { 1e30f, 1e30f, 1e30f };
    Vector3 high = Vector3Negate(low);
    for (const SceneObject& object : objects)
    {
        const bool isFixedPlane = object.fixed && object.type == "Plane";
        for (size_t i = 0; i < (isFixedPlane ? 1 : frameCount); ++i)
        {
            low  = Vector3Min(low, object.path[i]);
            high = Vector3Max(high, object.path[i]);
        }
    }
    const Vector3 sceneCentre = Vector3Scale(Vector3Add(low, high), 0.5f);
    const float   sceneRadius = std::max(0.5f * Vector3Distance(low, high), 1.f);

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "3D Physics Viewer");
    SetTargetFPS(60);

    const Shader shader = LoadShaderFromMemory(VERTEX_SHADER, FRAGMENT_SHADER);
    Model        sphere = LoadModelFromMesh(GenMeshSphere(1.f, 16, 24));
    Model        cube   = LoadModelFromMesh(GenMeshCube(1.f, 1.f, 1.f));
    sphere.materials[0].shader = shader;
    cube.materials[0].shader   = shader;

    // Orbit camera, z up, initially looking along +y so that x-z scenes face the viewer
    Vector3 target   = sceneCentre;
    float   yaw      = -PI / 2.f;
    float   pitch    = 0.25f;
    float   distance = 2.6f * sceneRadius;

    Camera3D camera {};
    camera.up         = { 0.f, 0.f, 1.f };
    camera.fovy       = 45.f;
    camera.projection = CAMERA_PERSPECTIVE;

    float simTime = 0.f;
    float speed   = 1.f;
    bool  playing = true;

    while (!WindowShouldClose())
    {
        const float dt          = GetFrameTime();
        const float barTop      = static_cast<float>(GetScreenHeight()) - 14.f;
        const bool  mouseOnBar  = GetMousePosition().y >= barTop;
        const float screenWidth = static_cast<float>(GetScreenWidth());

        // Playback controls
        if (IsKeyPressed(KEY_SPACE))
            playing = !playing;
        if (IsKeyPressed(KEY_R))
            simTime = 0.f;
        if (IsKeyPressed(KEY_UP))
            speed = std::min(speed * 2.f, 16.f);
        if (IsKeyPressed(KEY_DOWN))
            speed = std::max(speed / 2.f, 1.f / 16.f);
        if (IsKeyDown(KEY_RIGHT))
            simTime += 0.25f * duration * dt;
        if (IsKeyDown(KEY_LEFT))
            simTime -= 0.25f * duration * dt;
        if (mouseOnBar && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            simTime = duration * GetMousePosition().x / screenWidth;

        if (playing)
            simTime += speed * dt;
        if (simTime > duration)
            simTime = 0.f;
        simTime = std::clamp(simTime, 0.f, duration);

        // Camera controls
        const Vector2 mouseDelta = GetMouseDelta();
        if (!mouseOnBar && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            yaw -= mouseDelta.x * 0.005f;
            pitch = std::clamp(pitch + mouseDelta.y * 0.005f, -1.5f, 1.5f);
        }
        distance = std::max(distance * (1.f - 0.1f * GetMouseWheelMove()), 0.01f * sceneRadius);

        const Vector3 offset { distance * std::cos(pitch) * std::cos(yaw),
                               distance * std::cos(pitch) * std::sin(yaw), distance * std::sin(pitch) };
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            const Vector3 right = Vector3Normalize(Vector3CrossProduct(Vector3Negate(offset), camera.up));
            const Vector3 up    = Vector3Normalize(Vector3CrossProduct(right, Vector3Negate(offset)));
            const float   scale = distance * 0.0015f;
            target = Vector3Add(target, Vector3Scale(right, -mouseDelta.x * scale));
            target = Vector3Add(target, Vector3Scale(up, mouseDelta.y * scale));
        }
        camera.target   = target;
        camera.position = Vector3Add(target, offset);

        // Interpolate between the two closest saved steps
        const float  frame  = (step > 0.f) ? simTime / step : 0.f;
        const size_t first  = std::min(static_cast<size_t>(frame), frameCount - 1);
        const size_t second = std::min(first + 1, frameCount - 1);
        const float  blend  = frame - static_cast<float>(first);
        const auto   position = [&](const SceneObject& object)
        { return Vector3Lerp(object.path[first], object.path[second], blend); };

        BeginDrawing();
        ClearBackground(Color { 40, 42, 54, 255 });

        rlSetClipPlanes(0.005 * sceneRadius, 200.0 * sceneRadius);
        BeginMode3D(camera);

        for (size_t i = 0; i < objects.size(); ++i)
        {
            const SceneObject& object = objects[i];
            const Color        color  = PALETTE[i % PALETTE.size()];
            if (object.type == "Sphere")
            {
                const float radius = 0.5f * object.size.x;
                DrawModelEx(sphere, position(object), { 0.f, 0.f, 1.f }, 0.f, { radius, radius, radius }, color);
            }
            else if (object.type == "AABB")
            {
                DrawModelEx(cube, position(object), { 0.f, 0.f, 1.f }, 0.f, object.size, color);
            }
        }

        // Translucent planes last, without writing depth, so that walls do not hide the objects
        rlDrawRenderBatchActive();
        rlDisableDepthMask();
        rlDisableBackfaceCulling();
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (objects[i].type == "Plane")
                drawPlane(objects[i], position(objects[i]), PALETTE[i % PALETTE.size()]);
        }
        rlDrawRenderBatchActive();
        rlEnableBackfaceCulling();
        rlEnableDepthMask();

        EndMode3D();

        DrawText(TextFormat("t = %.2f / %.2f s   speed x%.3g   %s   %d FPS", static_cast<double>(simTime),
                            static_cast<double>(duration), static_cast<double>(speed),
                            playing ? "playing" : "paused", GetFPS()),
                 12, 12, 20, RAYWHITE);
        DrawText("Space: play/pause   Up/Down: speed   Left/Right: seek   R: restart", 12, 38, 16, LIGHTGRAY);
        DrawText("Left drag: orbit   Right drag: pan   Wheel: zoom", 12, 58, 16, LIGHTGRAY);
        DrawRectangleRec({ 0.f, barTop, screenWidth, 14.f }, Color { 68, 71, 90, 255 });
        DrawRectangleRec({ 0.f, barTop, (duration > 0.f) ? screenWidth * simTime / duration : 0.f, 14.f },
                         Color { 139, 233, 253, 255 });
        EndDrawing();
    }

    UnloadModel(sphere);
    UnloadModel(cube);
    UnloadShader(shader);
    CloseWindow();
    return 0;
}

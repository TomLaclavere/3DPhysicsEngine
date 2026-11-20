import numpy as np
import pandas as pd
import plotly.graph_objects as go

from python.utilities.motion_utilities import MotionCSV

_PALETTE = [
    "#ff79c6",
    "#8be9fd",
    "#50fa7b",
    "#ffb86c",
    "#bd93f9",
    "#ff5555",
    "#f1fa8c",
    "#6272a4",
]


def _get_color(idx: int) -> str:
    return _PALETTE[idx % len(_PALETTE)]


class ObjectPlotting:
    """Visualise and animate 3-D physics objects whose trajectories are stored
    in MotionCSV files.

    Parameters
    ----------
    objectpath : str
        Path to a CSV file describing every object (name, type, size…).
    motionpaths : list[str]
        One motion-CSV file per object, in the same order as *objectpath*.
    """

    # ------------------------------------------------------------------
    # Construction
    # ------------------------------------------------------------------

    def __init__(self, objectpath: str, motionpaths: list[str]):
        self.object_df = pd.read_csv(objectpath)

        if len(motionpaths) != len(self.object_df):
            raise ValueError("Number of motion files must match number of objects")

        self.motions: list[MotionCSV] = []
        for i, fp in enumerate(motionpaths):
            object_name = (
                self.object_df.iloc[i]["name"] if i < len(self.object_df) else None
            )
            self.motions.append(MotionCSV(fp, object_name))

        self.time = self.motions[0].time
        for m in self.motions[1:]:
            if len(m.time) != len(self.time):
                raise ValueError("Motion files have different time lengths")

        self.objectpath = objectpath
        self.motionpaths = motionpaths

    # ------------------------------------------------------------------
    # Public plotting helpers
    # ------------------------------------------------------------------

    def plot_objects_init(self):
        """Plot every object at its initial position (t = 0)."""
        self._plot_at_time_index(0, title="Initial state")

    def plot_objects_final(self):
        """Plot every object at its final position (t = -1)."""
        self._plot_at_time_index(-1, title="Final state")

    def plot_objects_time(self, time: float):
        """Plot every object at the requested simulation time."""
        index = self.motions[0].time_to_index(time)
        self._plot_at_time_index(index, title=f"State at t = {time:.3g} s")

    def plot_objects_pos(self, pos: list | None = None):
        """Plot every object at *pos* (list of [x,y,z] per object) or at the
        default position stored in the object CSV if *pos* is None."""
        fig = go.Figure()
        for idx, obj in self.object_df.iterrows():
            if pos is None:
                obj_pos = (obj["pos(x)"], obj["pos(y)"], obj["pos(z)"])
            else:
                obj_pos = pos[idx]
            self._add_object(
                fig, obj_pos, self._size(obj), self._normal(obj), obj["type"], obj["name"], idx
            )
        x_range, y_range, z_range = self._compute_axis_ranges()
        self._apply_scene_layout(fig, "Objects – custom positions", x_range=x_range,
            y_range=y_range,
            z_range=z_range,)
        fig.show()

    def animate_objects(
        self,
        stride: int | None = None,
        target_fps: int = 30,
        target_speed: float = 1.0,
    ):
        """Build a Plotly animation with Play / Pause buttons and a time slider.

        Parameters
        ----------
        stride : int or None
            Number of simulation steps between successive animation frames.
            If None (default), computed automatically from *dt*, *target_fps*,
            and *target_speed* so the animation always plays at a consistent speed.
        target_fps : int
            Visual frame rate of the animation (frames per second).
        target_speed : float
            Playback speed multiplier relative to real simulation time.
            5.0 means the animation runs 5× faster than real time.
        """
        dt = float(self.time[1] - self.time[0]) if len(self.time) > 1 else 0.01

        if stride is None:
            # stride * dt * target_fps = target_speed  →  consistent visual speed
            stride = max(1, round(target_speed / (dt * target_fps)))

        frame_duration_ms = max(10, round(1000 / target_fps))

        fig = go.Figure()
        n_frames = len(self.time)

        # Compute stable axis ranges once so the camera never jumps
        x_range, y_range, z_range = self._compute_axis_ranges()

        # Plotly rebuilds every updated trace at each frame, so all the moving
        # spheres are merged into one low-poly mesh instead of one surface each.
        df = self.object_df
        is_moving_sphere = (df["type"] == "Sphere") & (df["fixed"] == 0)
        spheres = df.index[is_moving_sphere]
        others = df.index[~is_moving_sphere]
        moving_others = [idx for idx in others if df.loc[idx, "fixed"] == 0]
        radii = 0.5 * df.loc[spheres, "size(x)"].to_numpy(float)
        unit_vertices, unit_triangles = self._unit_sphere_mesh()

        def spheres_xyz(t: int):
            centres = np.array([self.motions[i].pos[t] for i in spheres])
            vertices = centres[:, None, :] + radii[:, None, None] * unit_vertices
            return vertices.reshape(-1, 3).T

        # ---- initial traces (t = 0) ----------------------------------------
        if len(spheres):
            offsets = np.arange(len(spheres))[:, None, None] * len(unit_vertices)
            triangles = (unit_triangles + offsets).reshape(-1, 3).T
            x, y, z = spheres_xyz(0)
            fig.add_trace(
                go.Mesh3d(
                    x=x, y=y, z=z,
                    i=triangles[0], j=triangles[1], k=triangles[2],
                    facecolor=[_get_color(i) for i in spheres for _ in unit_triangles],
                )
            )
        first_other = 1 if len(spheres) else 0
        for idx in others:
            fig.add_trace(self._build_trace(df.loc[idx], self.motions[idx].pos[0], idx))

        # ---- frames -----------------------------------------------------------
        # Only moving objects are sent: fixed ones keep their initial trace
        updated = ([0] if len(spheres) else []) + [
            first_other + list(others).index(idx) for idx in moving_others
        ]
        frames = []
        for t in range(0, n_frames, stride):
            frame_data = []
            if len(spheres):
                x, y, z = spheres_xyz(t)
                frame_data.append(go.Mesh3d(x=x, y=y, z=z))
            for idx in moving_others:
                frame_data.append(self._build_trace(df.loc[idx], self.motions[idx].pos[t], idx))
            frames.append(go.Frame(data=frame_data, traces=updated, name=str(t)))
        fig.frames = frames

        # ---- layout -----------------------------------------------------------
        self._apply_scene_layout(
            fig,
            "Physics Simulation",
            x_range=x_range,
            y_range=y_range,
            z_range=z_range,
        )
        fig.update_layout(
            updatemenus=[self._play_pause_menu(frame_duration_ms)],
            sliders=[self._time_slider(n_frames, stride)],
        )
        fig.show()

    def animate_objects_2d(
        self,
        target_fps: int = 30,
        target_speed: float = 1.0,
    ):
        """Build a lightweight, smooth 2-D animation in the x-z plane (y is ignored).

        Objects are drawn to scale, as their projected outline: only moving
        objects are sent in each frame, fixed planes are drawn once.

        Parameters
        ----------
        target_fps : int
            Visual frame rate of the animation (frames per second).
        target_speed : float
            Playback speed multiplier relative to real simulation time.
        """
        dt = float(self.time[1] - self.time[0]) if len(self.time) > 1 else 0.01
        stride = max(1, round(target_speed / (dt * target_fps)))

        df = self.object_df
        is_fixed_plane = (df["type"] == "Plane") & (df["fixed"] != 0)
        moving = df.index[~is_fixed_plane]
        fixed = df.index[is_fixed_plane]
        outlines = {i: self._outline_2d(df.loc[i]) for i in df.index}
        paths = {i: self.motions[i].pos[::stride] for i in moving}

        def outline_trace(idx, pos):
            return go.Scatter(
                x=pos[0] + outlines[idx][0],
                y=pos[2] + outlines[idx][1],
                mode="lines",
                fill="toself",
                fillcolor=_get_color(idx),
                line=dict(color=_get_color(idx), width=1),
                hoverinfo="skip",
                showlegend=False,
            )

        # Frame on the objects and plane centres: fixed planes can be huge
        xs = np.concatenate(
            [paths[i][:, 0] + d for i in moving for d in (outlines[i][0].min(), outlines[i][0].max())]
            + [df.loc[fixed, "pos(x)"].to_numpy()]
        )
        zs = np.concatenate(
            [paths[i][:, 2] + d for i in moving for d in (outlines[i][1].min(), outlines[i][1].max())]
            + [df.loc[fixed, "pos(z)"].to_numpy()]
        )
        margin = max(0.1 * max(np.ptp(xs), np.ptp(zs)), 1.0)

        n_steps = len(paths[moving[0]])
        fig = go.Figure(
            data=[outline_trace(i, paths[i][0]) for i in moving]
            + [outline_trace(i, df.loc[i, ["pos(x)", "pos(y)", "pos(z)"]].to_numpy(float)) for i in fixed],
            frames=[
                go.Frame(
                    data=[outline_trace(i, paths[i][k]) for i in moving],
                    traces=list(range(len(moving))),
                    name=str(k * stride),
                )
                for k in range(n_steps)
            ],
        )
        fig.update_layout(
            title=dict(text="Physics Simulation (x-z)", font=dict(size=20, color="#f8f8f2")),
            template="plotly_dark",
            paper_bgcolor="#282a36",
            plot_bgcolor="#282a36",
            height=700,
            xaxis=dict(title="X", range=[xs.min() - margin, xs.max() + margin], constrain="domain"),
            yaxis=dict(title="Z", range=[zs.min() - margin, zs.max() + margin], scaleanchor="x"),
            updatemenus=[self._play_pause_menu(round(1000 / target_fps), redraw=False)],
            sliders=[self._time_slider(len(self.time), stride)],
        )
        fig.show()

    # ------------------------------------------------------------------
    # Internal helpers – geometry builders
    # ------------------------------------------------------------------

    @staticmethod
    def _unit_sphere_mesh(n_u: int = 12, n_v: int = 8) -> tuple[np.ndarray, np.ndarray]:
        """Return the vertices (N, 3) and triangles (M, 3) of a low-poly unit sphere."""
        u = np.linspace(0, 2 * np.pi, n_u, endpoint=False)
        v = np.linspace(0, np.pi, n_v)
        vertices = np.column_stack(
            (
                np.outer(np.cos(u), np.sin(v)).ravel(),
                np.outer(np.sin(u), np.sin(v)).ravel(),
                np.outer(np.ones(n_u), np.cos(v)).ravel(),
            )
        )
        triangles = []
        for a in range(n_u):
            b = (a + 1) % n_u
            for c in range(n_v - 1):
                p, q, r, s = a * n_v + c, b * n_v + c, b * n_v + c + 1, a * n_v + c + 1
                triangles += [(p, q, r), (p, r, s)]
        return vertices, np.array(triangles)

    def _outline_2d(self, obj: pd.Series) -> tuple[np.ndarray, np.ndarray]:
        """Return the closed x-z outline of *obj*, relative to its centre, to scale."""
        size = self._size(obj)
        if obj["type"] == "Sphere":
            angle = np.linspace(0, 2 * np.pi, 33)
            radius = 0.5 * size[0]
            return radius * np.cos(angle), radius * np.sin(angle)
        if obj["type"] == "AABB":
            hx, hz = 0.5 * size[0], 0.5 * size[2]
            return np.array([-hx, hx, hx, -hx, -hx]), np.array([-hz, -hz, hz, hz, -hz])

        # Same corners as the 3-D plane, in perimeter order
        plane = self._plane_trace(np.zeros(3), size, self._normal(obj), "", "#000000")
        corners = [(0, 0), (0, 1), (1, 1), (1, 0), (0, 0)]
        x, z = np.asarray(plane.x), np.asarray(plane.z)
        return np.array([x[c] for c in corners]), np.array([z[c] for c in corners])

    def _build_trace(self, obj: pd.Series, pos: np.ndarray, idx: int):
        """Return the correct Plotly trace for *obj* centred at *pos*."""
        obj_type = obj["type"]
        size = self._size(obj)
        name = obj["name"]
        color = _get_color(idx)
        normal = self._normal(obj)

        if obj_type == "Sphere":
            return self._sphere_trace(pos, size[0], name, color)
        elif obj_type == "AABB":
            return self._box_trace(pos, size, name, color)
        elif obj_type == "Plane":
            return self._plane_trace(pos, size, normal, name, color)
        else:
            raise ValueError(f"Unknown object type: {obj_type!r}")

    def _add_object(
        self, fig: go.Figure, pos, size, normal, obj_type: str, name: str, idx: int
    ):
        """Convenience wrapper used by the static plot methods."""
        # Build a minimal Series-like object so _build_trace can be reused
        obj = pd.Series(
            {
                "type": obj_type,
                "name": name,
                "size(x)": size[0],
                "size(y)": size[1],
                "size(z)": size[2],
                "rota(x)": normal[0],
                "rota(y)": normal[1],
                "rota(z)": normal[2]
            }
        )
        fig.add_trace(self._build_trace(obj, np.asarray(pos), idx))

    # ------------------------------------------------------------------
    # Sphere
    # ------------------------------------------------------------------

    @staticmethod
    def _sphere_trace(
        pos: np.ndarray, size: float, name: str, color: str
    ) -> go.Surface:

        radius = 0.5 * size

        u = np.linspace(0, 2*np.pi, 60)
        v = np.linspace(0, np.pi, 60)

        x = pos[0] + radius * np.outer(np.cos(u), np.sin(v))
        y = pos[1] + radius * np.outer(np.sin(u), np.sin(v))
        z = pos[2] + radius * np.outer(np.ones_like(u), np.cos(v))

        return go.Surface(
            x=x,
            y=y,
            z=z,
            name=name,
            opacity=0.8,
            showscale=False,
            colorscale=[[0, color], [1, color]],
        )

    # ------------------------------------------------------------------
    # Box / Cube
    # ------------------------------------------------------------------

    @staticmethod
    def _box_trace(pos: np.ndarray, size: tuple, name: str, color: str) -> go.Mesh3d:
        sx, sy, sz = size
        cx, cy, cz = pos

        x = np.array([cx-sx/2, cx+sx/2, cx+sx/2, cx-sx/2,
                    cx-sx/2, cx+sx/2, cx+sx/2, cx-sx/2])

        y = np.array([cy-sy/2, cy-sy/2, cy+sy/2, cy+sy/2,
                    cy-sy/2, cy-sy/2, cy+sy/2, cy+sy/2])

        z = np.array([cz-sz/2, cz-sz/2, cz-sz/2, cz-sz/2,
                    cz+sz/2, cz+sz/2, cz+sz/2, cz+sz/2])

        i = [0,0,4,4,0,0,2,2,1,1,0,0]
        j = [1,2,5,6,1,5,3,7,2,6,3,7]
        k = [2,3,6,7,5,4,7,6,6,5,7,4]

        return go.Mesh3d(
            x=x, y=y, z=z,
            i=i, j=j, k=k,
            name=name,
            opacity=0.7,
            color=color,
        )

    # ------------------------------------------------------------------
    # Plane
    # ------------------------------------------------------------------

    @staticmethod
    def _plane_trace(pos: np.ndarray, size: tuple, normal : tuple, name: str, color: str) -> go.Surface:
       
        sx, sy, _ = size
        cx, cy, cz = pos

        # normalisation
        n = np.asarray(normal, dtype=float)
        n = n / np.linalg.norm(n)

        if abs(n[0]) < 0.9:
            tangent = np.array([1.0, 0.0, 0.0])
        else:
            tangent = np.array([0.0, 1.0, 0.0])

        u = tangent - n * np.dot(n, tangent)  
        u /= np.linalg.norm(u)
        v = np.cross(n, u)
        v /= np.linalg.norm(v)

        grid_x = np.array([[-sx/2, sx/2],
                        [-sx/2, sx/2]])

        grid_y = np.array([[-sy/2, -sy/2],
                        [ sy/2,  sy/2]])

        X = cx + grid_x*u[0] + grid_y*v[0]
        Y = cy + grid_x*u[1] + grid_y*v[1]
        Z = cz + grid_x*u[2] + grid_y*v[2]

        return go.Surface(
            x=X,
            y=Y,
            z=Z,
            name=name,
            opacity=0.6,
            showscale=False,
            colorscale=[[0, color], [1, color]],
        )

    # ------------------------------------------------------------------
    # Layout helpers
    # ------------------------------------------------------------------

    @staticmethod
    def _apply_scene_layout(
        fig: go.Figure,
        title: str,
        x_range=None,
        y_range=None,
        z_range=None,
    ):
        scene = dict(
            xaxis_title="X",
            yaxis_title="Y",
            zaxis_title="Z",
            aspectmode="cube",
            bgcolor="#282a36",
            xaxis=dict(gridcolor="#44475a", zerolinecolor="#6272a4"),
            yaxis=dict(gridcolor="#44475a", zerolinecolor="#6272a4"),
            zaxis=dict(gridcolor="#44475a", zerolinecolor="#6272a4"),
        )
        if x_range is not None:
            scene["xaxis"]["range"] = x_range
            scene["yaxis"]["range"] = y_range
            scene["zaxis"]["range"] = z_range

        fig.update_layout(
            title=dict(text=title, font=dict(size=20, color="#f8f8f2")),
            paper_bgcolor="#282a36",
            plot_bgcolor="#282a36",
            font=dict(color="#f8f8f2"),
            width=700,
            height=600,
            scene=scene,
            margin=dict(l=0, r=0, t=50, b=0),
            legend=dict(
                bgcolor="#44475a",
                bordercolor="#6272a4",
                borderwidth=1,
            ),
        )

    # ------------------------------------------------------------------
    # Animation controls
    # ------------------------------------------------------------------

    @staticmethod
    def _play_pause_menu(frame_duration_ms: int = 30, redraw: bool = True) -> dict:
        return {
            "type": "buttons",
            "direction": "left",
            "pad": {"r": 10, "t": 10},
            "x": 0.1,
            "y": 0,
            "xanchor": "right",
            "yanchor": "top",
            "font": {"color": "#f8f8f2", "size": 13},
            "bgcolor": "#44475a",
            "showactive": False,
            "bordercolor": "#6272a4",
            "buttons": [
                {
                    "label": "▶ Play",
                    "method": "animate",
                    "args": [
                        None,
                        {
                            "frame": {"duration": frame_duration_ms, "redraw": redraw},
                            "fromcurrent": True,
                            "mode": "immediate",
                        },
                    ],
                },
                {
                    "label": "⏸ Pause",
                    "method": "animate",
                    "args": [
                        [None],
                        {
                            "frame": {"duration": 0, "redraw": False},
                            "mode": "immediate",
                        },
                    ],
                },
            ],
        }

    def _time_slider(self, n_frames: int, stride: int) -> dict:
        steps = [
            {
                "method": "animate",
                "label": f"{self.time[t]:.2f}",
                "args": [
                    [str(t)],
                    {
                        "mode": "immediate",
                        "frame": {"duration": 0, "redraw": True},
                        "transition": {"duration": 0},
                    },
                ],
            }
            for t in range(0, n_frames, stride)
        ]
        return {
            "yanchor": "top",
            "font": {"color": "#f8f8f2", "size": 11},
            "xanchor": "left",
            "currentvalue": {
                "font": {"size": 14, "color": "white"},
                "prefix": "t = ",
                "suffix": " s",
                "visible": True,
                "xanchor": "right",
            },
            "pad": {"b": 10, "t": 50},
            "len": 0.9,
            "x": 0.1,
            "y": 0,
            "steps": steps,
            "bgcolor": "rgba(255,255,255,0.1)",
            "activebgcolor": "rgba(255,255,255,0.4)",
            "bordercolor": "rgba(255,255,255,0.3)",
        }

    # ------------------------------------------------------------------
    # Misc utilities
    # ------------------------------------------------------------------

    def _plot_at_time_index(self, index: int, title: str):
        fig = go.Figure()
        for idx, obj in self.object_df.iterrows():
            pos = self.motions[idx].pos[index]
            trace = self._build_trace(obj, pos, idx)
            fig.add_trace(trace)
            
        x_range, y_range, z_range = self._compute_axis_ranges()
        self._apply_scene_layout(fig, title, x_range=x_range,
            y_range=y_range,
            z_range=z_range,)
        fig.show()

    def _compute_axis_ranges(self) -> tuple[list, list, list]:
        all_pos = []
        for idx, obj in self.object_df.iterrows():
            if obj["fixed"] == 0:
                all_pos.append(self.motions[idx].pos)
        all_pos = np.vstack(all_pos)

        x_min, x_max = all_pos[:,0].min(), all_pos[:,0].max()
        y_min, y_max = all_pos[:,1].min(), all_pos[:,1].max()
        z_min, z_max = all_pos[:,2].min(), all_pos[:,2].max()

        max_size = max(self.object_df[["size(x)", "size(y)", "size(z)"]].max().max(), 1.0)
        x_margin = max(0.15*(x_max - x_min), max_size/4)
        y_margin = max(0.15*(y_max - y_min), max_size/4)
        z_margin = max(0.15*(z_max - z_min), max_size/4)

        x_range = [x_min - x_margin, x_max + x_margin]
        y_range = [y_min - y_margin, y_max + y_margin]
        z_range = [z_min - z_margin, z_max + z_margin]

        lengths = [x_range[1]-x_range[0], y_range[1]-y_range[0], z_range[1]-z_range[0]]
        max_len = max(lengths)

        def center_range(r):
            c = (r[0] + r[1])/2
            return [c - max_len/2, c + max_len/2]

        return center_range(x_range), center_range(y_range), center_range(z_range)

    @staticmethod
    def _size(obj: pd.Series) -> tuple:
        return (obj["size(x)"], obj["size(y)"], obj["size(z)"])
    
    @staticmethod
    def _normal(obj: pd.Series) -> tuple:
        return (obj["rota(x)"], obj["rota(y)"], obj["rota(z)"])

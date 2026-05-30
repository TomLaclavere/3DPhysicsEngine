#include "objects/material.hpp"
#include "test_functions.hpp"

#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

// --------------------------------------------------------------------------
//  Constructors
// --------------------------------------------------------------------------

TEST(Material_Test, DefaultConstructor)
{
    Material mat;

    EXPECT_EQ(mat.getName(), "default");
    EXPECT_DECIMAL_EQ(mat.getDensity(), 1000_d);
    EXPECT_DECIMAL_EQ(mat.getStiffness(), 0_d);
    EXPECT_DECIMAL_EQ(mat.getDamping(), 0_d);
    EXPECT_DECIMAL_EQ(mat.getFriction(), 0_d);
    EXPECT_DECIMAL_EQ(mat.getRestitution(), 1_d);
}

TEST(Material_Test, NamedConstructor)
{
    Material mat("steel", 7800_d, 200000_d, 1000_d, 0.3_d, 0.5_d);

    EXPECT_EQ(mat.getName(), "steel");
    EXPECT_DECIMAL_EQ(mat.getDensity(), 7800_d);
    EXPECT_DECIMAL_EQ(mat.getStiffness(), 200000_d);
    EXPECT_DECIMAL_EQ(mat.getDamping(), 1000_d);
    EXPECT_DECIMAL_EQ(mat.getFriction(), 0.3_d);
    EXPECT_DECIMAL_EQ(mat.getRestitution(), 0.5_d);
}

// --------------------------------------------------------------------------
//  Setters
// --------------------------------------------------------------------------

TEST(Material_Test, Setters)
{
    Material mat;

    mat.setName("rubber");
    EXPECT_EQ(mat.getName(), "rubber");

    mat.setDensity(1200_d);
    EXPECT_DECIMAL_EQ(mat.getDensity(), 1200_d);

    mat.setStiffness(500_d);
    EXPECT_DECIMAL_EQ(mat.getStiffness(), 500_d);

    mat.setDamping(10_d);
    EXPECT_DECIMAL_EQ(mat.getDamping(), 10_d);

    mat.setFriction(0.6_d);
    EXPECT_DECIMAL_EQ(mat.getFriction(), 0.6_d);

    mat.setRestitution(0.8_d);
    EXPECT_DECIMAL_EQ(mat.getRestitution(), 0.8_d);
}

// --------------------------------------------------------------------------
//  Negative-value validation
// --------------------------------------------------------------------------

TEST(Material_Test, NegativeValueThrows)
{
    Material mat;

    EXPECT_THROW(mat.setStiffness(-1_d), std::invalid_argument);
    EXPECT_THROW(mat.setDamping(-0.1_d), std::invalid_argument);
    EXPECT_THROW(mat.setFriction(-5_d), std::invalid_argument);
    EXPECT_THROW(mat.setRestitution(-0.5_d), std::invalid_argument);
}

// --------------------------------------------------------------------------
//  loadFromFile
// --------------------------------------------------------------------------

TEST(Material_Test, LoadFromFile)
{
    const std::string path = "temp_material_test.yaml";

    {
        std::ofstream f(path);
        f << "name: wood\n";
        f << "density: 600.0\n";
        f << "stiffness: 10000.0\n";
        f << "damping: 50.0\n";
        f << "friction: 0.4\n";
        f << "restitution: 0.3\n";
    }

    Material mat;
    mat.loadFromFile(path);

    EXPECT_EQ(mat.getName(), "wood");
    EXPECT_DECIMAL_EQ(mat.getDensity(), 600_d);
    EXPECT_DECIMAL_EQ(mat.getStiffness(), 10000_d);
    EXPECT_DECIMAL_EQ(mat.getDamping(), 50_d);
    EXPECT_DECIMAL_EQ(mat.getFriction(), 0.4_d);
    EXPECT_DECIMAL_EQ(mat.getRestitution(), 0.3_d);

    std::remove(path.c_str());
}

TEST(Material_Test, Singleton)
{
    // Material::get() is a non-static member that returns a shared singleton instance
    Material  m;
    Material& a = m.get();
    Material& b = m.get();
    EXPECT_EQ(&a, &b);
}

TEST(Material_Test, LoadFromFile_InvalidPath)
{
    Material mat;
    EXPECT_THROW(mat.loadFromFile("nonexistent_file.yaml"), std::exception);
}

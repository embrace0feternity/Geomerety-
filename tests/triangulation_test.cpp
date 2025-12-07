#include <gtest/gtest.h>
#include "triangulation.hpp"

namespace g = geometry;
namespace gt = geometry::triangulation;

using namespace std::literals;

TEST(TriangulationTest, expectedConvexHullError) { 
    std::vector<g::Point2D> triangulationPoints = { {0, 0}, {10, 0} };

    auto rt = gt::DelaunayTriangulation(triangulationPoints);
    EXPECT_FALSE(rt.has_value());
    EXPECT_TRUE(rt.error() == "At least three points are required for triangulation."s);
}

///
///
///

TEST(TriangulationTest, expectedConvexHullValue) { 
    std::vector<g::Point2D> triangulationPoints = { {0, 0}, {10, 0}, {5, 10} };

    auto rt = gt::DelaunayTriangulation(triangulationPoints);
    EXPECT_TRUE(rt.has_value());
}

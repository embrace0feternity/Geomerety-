#include <gtest/gtest.h>
#include "convex_hull.hpp"

namespace g = geometry;
namespace gc = geometry::convex_hull;

using namespace std::literals;

TEST(ConvexHullTest, expectedConvexHullError) { 
    std::vector<g::Point2D> convexHullPoints = { {0, 0}, {10, 0} };

    auto rt = gc::GrahamScan(convexHullPoints);
    EXPECT_FALSE(rt.has_value());
    EXPECT_TRUE(rt.error() == "At least three points are required for convex hull."s);
}

///
///
///

TEST(ConvexHullTest, expectedConvexHullValue) { 
    std::vector<g::Point2D> convexHullPoints = { {0, 0}, {10, 0}, {5, 10} };

    auto rt = gc::GrahamScan(convexHullPoints);
    EXPECT_TRUE(rt.has_value());
}

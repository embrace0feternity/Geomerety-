#include <gtest/gtest.h>
#include "intersections.hpp"

namespace g = geometry;
namespace gi = geometry::intersections;

TEST(IntersectionsTest, lineToLine) { 
    g::Line l1 { { 0, 0 }, { 10, 10 } };

    {
        g::Line l2 { { 0, 5 }, { 15, 5 } };

        auto intersection = gi::GetIntersectPoint(l1, l2);
        EXPECT_TRUE(intersection.has_value());
        
        auto &points = intersection.value();
        EXPECT_EQ(1, points.size());

        g::Point2D p { 5, 5 };
        EXPECT_TRUE(points.front() == p);
    }
    {
        g::Line l2 { { 0, 30 }, { 15, 30 } };

        auto intersection = gi::GetIntersectPoint(l1, l2);
        EXPECT_FALSE(intersection.has_value());
    }
}

///
///
///

TEST(IntersectionsTest, lineToCircle) { 
    g::Line l1 { { 0, 0 }, { 10, 0 } };

    {
        g::Circle c1 { { 15, 0 }, 10 };

        auto intersection = gi::GetIntersectPoint(l1, c1);
        EXPECT_TRUE(intersection.has_value());
        
        auto &points = intersection.value();
        EXPECT_EQ(1, points.size());

        g::Point2D p { 5, 0 };
        EXPECT_TRUE(points.front() == p);
    }
    {
        g::Circle c1 { { 5, 0 }, 2 };

        auto intersection = gi::GetIntersectPoint(l1, c1);
        EXPECT_TRUE(intersection.has_value());
        
        auto &points = intersection.value();
        EXPECT_EQ(2, points.size());

        g::Point2D p1 { 3, 0 };
        EXPECT_TRUE(points.front() == p1);
        g::Point2D p2 { 7, 0 };
        EXPECT_TRUE(points.back() == p2);
    }
    {
        g::Circle c1 { { 15, 0 }, 2 };

        auto intersection = gi::GetIntersectPoint(l1, c1);
        EXPECT_FALSE(intersection.has_value());
    }
}

///
///
///

TEST(IntersectionsTest, circleToCircle) { 
    g::Circle c1 { { 0, 0 }, 5 };

    {
        g::Circle c2 { { 15, 0 }, 12 };

        auto intersection = gi::GetIntersectPoint(c1, c2);
        EXPECT_TRUE(intersection.has_value());
        
        auto &points = intersection.value();
        EXPECT_EQ(2, points.size());
    }
    {
        g::Circle c2 { { 15, 0 }, 2 };

        auto intersection = gi::GetIntersectPoint(c1, c2);
        EXPECT_FALSE(intersection.has_value());
    }
}

///
///
///

TEST(IntersectionsTest, nonSupported) { 
    g::Circle c1 { { 0, 0 }, 5 };
    g::Rectangle r1 { { 15, 0 }, 12, 10 };

    EXPECT_THROW(gi::GetIntersectPoint(c1, r1), std::logic_error);
}
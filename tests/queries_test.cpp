#include <gtest/gtest.h>
#include "queries.hpp"

namespace g = geometry;
namespace gq = geometry::queries;

class QueriesGroup : public testing::Test {
protected:
    g::Line l { { -10, 0 }, { 30, 0 } };
    g::Triangle t  { { -10, 0 }, { 10, 0 }, { 0, 15 } };
    g::Rectangle r { { -10, 0 }, 20, 20 };
    g::RegularPolygon rp { { 0, 10 }, 10, 5 };
    g::Circle c { { 0, 5 }, 10 };
    g::Polygon p { std::vector<g::Point2D> {{ { -10, 0 }, { 10, 0 }, { 0, 15 } }}};
};

///
///
///

TEST_F(QueriesGroup, DistanceToPointTest) { 
    g::Point2D x { 0, 10 };
    
    EXPECT_DOUBLE_EQ(10, gq::DistanceToPoint(l, x));
    EXPECT_TRUE(gq::DistanceToPoint(t, x) > 2);
    EXPECT_DOUBLE_EQ(10, gq::DistanceToPoint(r, x));
    EXPECT_TRUE(gq::DistanceToPoint(rp, x) > 8);
    EXPECT_DOUBLE_EQ(5, gq::DistanceToPoint(c, x));
    EXPECT_TRUE(gq::DistanceToPoint(p, x) > 2);
}

///
///
///

TEST_F(QueriesGroup, BoundingBox) {
    g::BoundingBox bl { -10, 0, 30, 0 };
    g::BoundingBox bt { -10, 0, 10, 15 };
    g::BoundingBox br { -10, 0, 10, 20 };
    g::BoundingBox brp { -10, 0, 10, 20 };
    g::BoundingBox bc { -10, -5, 10, 15 };
    g::BoundingBox bp = bt;

    auto compareBoxes = [](const g::BoundingBox &box, const g::BoundingBox &sbox){
        return (sbox.min_x == box.min_x) && (sbox.min_y == box.min_y)
            && (sbox.max_x == box.max_x) && (sbox.max_y == box.max_y);
    };

    EXPECT_TRUE(compareBoxes(bl, gq::GetBoundBox(l)));
    EXPECT_TRUE(compareBoxes(bt, gq::GetBoundBox(t)));
    EXPECT_TRUE(compareBoxes(br, gq::GetBoundBox(r)));
    EXPECT_TRUE(compareBoxes(brp, gq::GetBoundBox(rp)));
    EXPECT_TRUE(compareBoxes(bc, gq::GetBoundBox(c)));
    EXPECT_TRUE(compareBoxes(bp, gq::GetBoundBox(p)));
}

///
///
///

TEST_F(QueriesGroup, GetHeight) {     
    EXPECT_EQ(0, gq::GetHeight(l));
    EXPECT_EQ(15, gq::GetHeight(t));
    EXPECT_EQ(20, gq::GetHeight(r));
    EXPECT_EQ(20, gq::GetHeight(rp));
    EXPECT_EQ(15, gq::GetHeight(c));
    EXPECT_EQ(15, gq::GetHeight(p));
}

///
///
///

TEST_F(QueriesGroup, BoundingBoxOverlap) {
    g::Rectangle rb { { 18, -10 }, 12, 30 };

    EXPECT_TRUE(gq::BoundingBoxesOverlap(l, rb));
    EXPECT_FALSE(gq::BoundingBoxesOverlap(t, rb));
    EXPECT_FALSE(gq::BoundingBoxesOverlap(r, rb));
    EXPECT_FALSE(gq::BoundingBoxesOverlap(rp, rb));
    EXPECT_FALSE(gq::BoundingBoxesOverlap(c, rb));
    EXPECT_FALSE(gq::BoundingBoxesOverlap(p, rb));
}

///
///
///

TEST_F(QueriesGroup, DistanceBetweenShapes) {
    /// Not supported
    auto rt = gq::DistanceBetweenShapes(r, rp);
    EXPECT_FALSE(rt);
    rt = gq::DistanceBetweenShapes(l, r);
    EXPECT_FALSE(rt);

    g::Line l2 { {0, 10}, {30, 10} };
    rt = gq::DistanceBetweenShapes(l, l2);
    EXPECT_TRUE(rt);
    EXPECT_EQ(10, rt.value());

    g::Circle c2 { {0, 40}, 15 };
    rt = gq::DistanceBetweenShapes(c, c2);
    EXPECT_TRUE(rt);
    EXPECT_EQ(10, rt.value());
}

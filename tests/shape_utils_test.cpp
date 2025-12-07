#include <gtest/gtest.h>
#include "shape_utils.hpp"

///
///
///

namespace g = geometry;
namespace gu = geometry::utils;
namespace gq = geometry::queries;

TEST(ShapeUtilsTest, findAllCollisions) { 
    /// Collisions: 5
    ///     l1 x r1
    ///     l1 x c1
    ///     c1 x t1
    ///     t1 x p1
    ///     c1 x rp1
    g::Line l1 { { 1.0, 1.0 }, { 3.0, 4.0 }};
    g::Rectangle r1 { { 1.0, 3.0 }, 1, 1 };
    g::Circle c1 { { 3.6, 1.0 }, 1.2 };
    g::Triangle t1 { { 4.0, 2.0 }, { 6.0, 2.0 }, { 5.0, 3.0 }};
    g::Polygon p1 { {{ 7.0, 2.0 }, { 7.0, 4.0 }, { 5.0, 4.0 }} };
    g::RegularPolygon rp1 { { 3.6, -1.0 }, 1, 4};

    std::vector<g::Shape> v { l1, r1, c1, t1, p1, rp1 };

    auto collisions = gu::FindAllCollisions(v);
    EXPECT_EQ(5, collisions.size());
    
    bool match = true;
    for (auto [i, j] : collisions) { 
        std::visit(gq::Multilambda{
            [&match](const auto &s1, const auto &s2){ match = false; },
            [](const g::Line& s1, const g::Rectangle &s2){ /* do nothing */ },
            [](const g::Line& s1, const g::Circle &s2){ /* do nothing */ },
            [](const g::Circle& s1, const g::Triangle&s2){ /* do nothing */ },
            [](const g::Triangle& s1, const g::Polygon &s2){ /* do nothing */ },
            [](const g::Circle& s1, const g::RegularPolygon&s2){ /* do nothing */ }
        }, i, j);
    }

    EXPECT_TRUE(match);
}

///
///
///

TEST(ShapeUtilsTest, highestShape) { 
    g::Line l1 { { 0.0, 0.0 }, { 0.0, 4.0 }};
    g::Rectangle r1 { { 0.0, 0.0 }, 1, 1 };
    g::Circle c1 { { 0.0, 0.0 }, 1.2 };
    g::Triangle t1 { { 0.0, 0.0 }, { 4.0, 2.0 }, { 8.0, 0.0 }};
    g::Polygon p1 { {{ 0.0, 0.0 }, { 4.0, 0.0 }, { 4.0, 1.5 }} };
    g::RegularPolygon rp1 { { 0.0, 0.0 }, 1.8, 4};

    std::vector<g::Shape> v { l1, r1, c1, t1, p1, rp1 };

    EXPECT_EQ(l1.Height(), gu::FindHighestShape(v));

    std::erase_if(v, [h = l1.Height()](const g::Shape &s){
        return std::visit([h](const auto &s){ return s.Height() == h; }, s);
    });

    EXPECT_EQ(t1.Height(), gu::FindHighestShape(v));
}

///
///
///

TEST(ShapeUtilsTest, parseLine) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        line 30 45 -60 -20;
        line;
        line 10 20 -30;
        line 10 20 -30 -40;
        line 20 15 20 80 30
    )");

    EXPECT_EQ(2, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::Line &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<0>(shapes[0]);
    auto &s2 = std::get<0>(shapes[1]);

    auto st = (s1.start == g::Point2D { 30, 45 }) 
        && (s1.end == g::Point2D { -60, -20 })
        && (s2.start == g::Point2D { 10, 20 }) 
        && (s2.end == g::Point2D { -30, -40 });
    
    EXPECT_TRUE(st);
}

///
///
///

TEST(ShapeUtilsTest, parseTriangle) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        triangle 30 45 30 60 45 45;
        triangle;
        triangle 10 20 -30;
        triangle 10 20 -30 -40;
    )");

    EXPECT_EQ(1, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::Triangle &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<1>(shapes[0]);

    auto st = (s1.a == g::Point2D { 30, 45 }) 
        && (s1.b == g::Point2D { 30, 60 })
        && (s1.c == g::Point2D { 45, 45 });
    EXPECT_TRUE(st);
    
    auto v1 = s1.Vertices();
    st = (v1[0] == g::Point2D { 30, 45 }) 
        && (v1[1] == g::Point2D { 30, 60 })
        && (v1[2] == g::Point2D { 45, 45 });
    EXPECT_TRUE(st);
}

///
///
///

TEST(ShapeUtilsTest, parseRectangle) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        rectangle 30 45 10 20;
        rectangle;
        rectangle 30 45 -10 10;
        rectangle 10 20 0 0;
        rectangle 30 20 -10;
    )");

    EXPECT_EQ(1, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::Rectangle &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<2>(shapes[0]);

    auto st = (s1.bottom_left == g::Point2D { 30, 45 }) 
        && (s1.width == 10)
        && (s1.height == 20);
    EXPECT_TRUE(st);
    
    auto v1 = s1.Vertices();
    st = (v1[0] == g::Point2D { 30, 45 }) 
        && (v1[1] == (g::Point2D { 30, 45 } + g::Point2D { 10, 0 }))
        && (v1[2] == (g::Point2D { 30, 45 } + g::Point2D { 10, 20 }))
        && (v1[3] == (g::Point2D { 30, 45 } + g::Point2D { 0, 20 }));
    EXPECT_TRUE(st);
}

///
///
///

TEST(ShapeUtilsTest, parseRegularPolygon) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        regularpolygon 30 45 10 6;
        regularpolygon;
        regularpolygon 30 45 10;
        regularpolygon 10 20 -5 6;
        regularpolygon 30 20 10 2;
    )");

    EXPECT_EQ(1, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::RegularPolygon &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<3>(shapes[0]);

    auto st = (s1.center_p == g::Point2D { 30, 45 }) 
        && (s1.radius == 10)
        && (s1.sides == 6);
    EXPECT_TRUE(st);
}

///
///
///

TEST(ShapeUtilsTest, parseCircle) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        circle 20 0 10;
        circle;
        circle 20 20 10 0;
        circle 20 20;
    )");

    EXPECT_EQ(1, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::Circle &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<4>(shapes[0]);

    auto st = (s1.center_p == g::Point2D { 20, 0 }) 
        && (s1.radius == 10);
    EXPECT_TRUE(st);
}

///
///
///

TEST(ShapeUtilsTest, parsePolygon) {
    std::vector<g::Shape> shapes = gu::ParseShapes(R"(
        polygon 20 0 10 0 15;
        polygon;
        polygon 20 0 10 0 10 10;
        polygon 20 20;
    )");

    EXPECT_EQ(1, shapes.size());

    auto areShapesLines = std::ranges::all_of(shapes, [](const g::Shape &s){ 
        return std::visit(gq::Multilambda{
            [](const g::Polygon &){ return true; },
            [](const auto &){ return false; },
        }, s);
    });

    EXPECT_TRUE(areShapesLines);
    auto &s1 = std::get<5>(shapes[0]);


    auto v = s1.Vertices();
    auto st = (v[0] == g::Point2D { 20, 0 }) 
        && (v[1] == g::Point2D { 10, 0 }) 
        && (v[2] == g::Point2D { 10, 10 });
    EXPECT_TRUE(st);
}
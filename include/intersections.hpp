#pragma once
#include "geometry.hpp"
#include <cmath>
#include <optional>

namespace geometry::intersections {

/*
* Класс для поиска пересечений между двумя фигурами
*
* Требуется организовать возможность нахождения пересечений только для следующих комбинаций фигур:
*    - Line   & Line
*    - Line   & Circle
*    - Circle & Circle
*
* Для всех остальных требуется выбросить исключение std::logic_error
*/

using IntersectPoints = std::optional<std::vector<Point2D>>;

///
///
///

class IntersectionVisitor {
    using RadVector = Point2D;

public:
    /// Invariant - lines are non-collinear.
    [[nodiscard]] IntersectPoints operator()(const Line& line1, const Line& line2) const noexcept {
        RadVector A = line1.start;
        RadVector B = line1.end;
        RadVector C = line2.start;
        RadVector D = line2.end;

        RadVector AB = B - A;
        RadVector CD = D - C;
        RadVector AC = C - A;

        double det = AB.Cross(CD);
        
        double t = AC.Cross(CD) / det;
        double u = AC.Cross(AB) / det;

        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            RadVector intersection = A + AB * t;
            return std::vector<Point2D>{intersection};
        }

        return std::nullopt;
    }

    [[nodiscard]] IntersectPoints operator()(const Circle& circle, const Line& line) const noexcept {
        constexpr double EPS = 1e-9;
        
        Point2D O = circle.Center();
        double R = circle.radius;
        Point2D P1 = line.start;
        Point2D P2 = line.end;
        
        // Если точки совпадают (вырожденный отрезок)
        if (P1.DistanceTo(P2) < EPS) {
            return std::abs(P1.DistanceTo(O) - R) < EPS 
                ? IntersectPoints {std::vector<Point2D>{P1}} 
                : std::nullopt;
        }
        
        // Смещаем систему координат в центр окружности
        Point2D A = P1 - O;
        Point2D B = P2 - O;
        Point2D D = B - A;
        
        // Решаем квадратное уравнение |A + tD|² = R²
        double a = D.Dot(D);
        double b = 2.0 * A.Dot(D);
        double c = A.Dot(A) - R * R;
        
        double discr = b * b - 4.0 * a * c;
        
        if (discr < -EPS) return std::nullopt;
        
        std::vector<Point2D> points;
        
        if (discr <= EPS) {  // discriminant ≈ 0
            double t = -b / (2.0 * a);
            if (t >= -EPS && t <= 1.0 + EPS) {
                points.push_back(P1 + D * t);
            }
        } else {
            double sqrt_discr = std::sqrt(discr);
            double t1 = (-b - sqrt_discr) / (2.0 * a);
            double t2 = (-b + sqrt_discr) / (2.0 * a);
            
            if (t1 >= -EPS && t1 <= 1.0 + EPS) {
                points.push_back(P1 + D * t1);
            }
            if (t2 >= -EPS && t2 <= 1.0 + EPS) {
                points.push_back(P1 + D * t2);
            }
        }
        
        // Сортируем по расстоянию от начала отрезка (опционально)
        std::sort(points.begin(), points.end(),
            [&P1](const Point2D& a, const Point2D& b) {
                return P1.DistanceTo(a) < P1.DistanceTo(b);
            });
        
        return points.empty() ? std::nullopt : IntersectPoints{points};
    }       

    [[nodiscard]] IntersectPoints operator()(const Line& line, const Circle& circle) const noexcept {
        return (*this)(circle, line);
    }

    [[nodiscard]] IntersectPoints operator()(const Circle& circle1, const Circle& circle2) const noexcept {
        auto c1 = circle1.Center();
        auto c2 = circle2.Center();
        auto r1 = circle1.radius;
        auto r2 = circle2.radius;

        double dx = c2.x - c1.x;
        double dy = c2.y - c1.y;
        double d = std::sqrt(dx * dx + dy * dy);

        if (d > r1 + r2 || d < std::abs(r1 - r2)) {
            return std::nullopt;
        }

        double a = (r1 * r1 - r2 * r2 + d * d) / (2.0 * d);
        
        double h = std::sqrt(r1 * r1 - a * a);

        Point2D p;
        p.x = c1.x + (a * dx) / d;
        p.y = c1.y + (a * dy) / d;

        double perp_dx = -dy / d;
        double perp_dy = dx / d;
        Point2D intersection1, intersection2;
        intersection1.x = p.x + h * perp_dx;
        intersection1.y = p.y + h * perp_dy;
        
        intersection2.x = p.x - h * perp_dx;
        intersection2.y = p.y - h * perp_dy;

        return std::vector { intersection1, intersection2 };
    }

    template <typename S1, typename S2>
    [[nodiscard]] IntersectPoints operator()(const S1&, const S2&) const {
        throw std::logic_error("Unsupported figures");
        return std::nullopt;
    }
};

///
///
///

inline IntersectPoints GetIntersectPoint(const Shape &shape1, const Shape &shape2) {
    return std::visit(IntersectionVisitor{}, shape1, shape2);
}

}  // namespace geometry::intersections
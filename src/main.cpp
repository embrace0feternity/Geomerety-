#include "convex_hull.hpp"
#include "geometry.hpp"
#include "geometery_formatter.hpp"
#include "intersections.hpp"
#include "queries.hpp"
#include "shape_utils.hpp"
#include "triangulation.hpp"
#include "visualization.hpp"

#include <algorithm>
#include <print>
#include <ranges>
#include <random>

using namespace geometry;

namespace rng = std::ranges;
namespace views = std::ranges::views;

void PrintAllHeights(std::span<const Shape> shapes) noexcept {
    std::println("\n=== Heights ===");

    std::size_t ith = 0;
    for (const auto &s : shapes) {
        std::visit([ith](auto &s){
            std::println("Shape {}: {}, height = {}", ith, s, s.Height());
        }, s);
        ++ith;
    }
}

///
///
///

void PrintAllIntersections(const Shape &shape, std::span<const Shape> others) {
    std::println("\n=== Intersections ===");

    /*
     * Используйте ranges чтобы оставить только фигуры,
     * поддерживающие возможность находить пересечения между собой
     *
     * Затем примените монадический интерфейс для обработки результатов:
     *     - Пересечение найдено в точке A между фигурами B и C
     *     - Фигуры B и C не пересекаются
     */
    auto supportedShapes = others | std::views::filter([&shape](const Shape &other){
        return std::visit(queries::Multilambda{
            [](const Line &line, const Circle &circle){ return true; },
            [](const Circle &circle, const Line &line){ return true; },
            [](const Circle &circle1, const Circle &circle2){ return true; },
            [](const Line &line1, const Line &line2){ return true; },
            [](const auto &circle, const auto &line){ return false; }
        }, shape, other
        );
    });

    /// TODO монады
    /// проще без монад
    for (const auto &ss : supportedShapes) {
        auto rt = intersections::GetIntersectPoint(shape, ss);
        if (rt) {
            std::visit([&rt](const auto &s1, const auto &s2){
                std::println("{} and {} have intersection points {:new_line}", s1, s2, rt.value());
            }, shape, ss);
        }
        else 
        {
            std::visit([](const auto &s1, const auto &s2){
                std::println("No intersection between {} and {}", s1, s2);
            }, shape, ss);
        }
    } 
}

///
///
///

void PrintDistancesFromPointToShapes(Point2D p, std::span<const Shape> shapes) {
    std::println("\n=== Distance from Point Test ===");

    /*
     * Используйте ranges чтобы выбрать любые 5 фигур из списка.
     * Затем найдите расстояния от заданной точки до всех выбранных фигур.
     * Выведите результат в формате "Расстояние от точки P до фигуры S равно D"
     */
    std::random_device rd;
    std::mt19937 gen {rd()};

    std::vector<Shape> any { shapes.begin(), shapes.end() };
    std::shuffle(any.begin(), any.end(), gen);
    for (const auto i : any | std::views::take(5)) {
        std::visit([&p](const auto &s){
            std::println("The distance from {} to {} is {}", p, s, geometry::queries::DistanceToPoint(s, p));
        }, i);
    }
}

///
///
///

void PerformShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Analysis ===");
    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Найти все пересечения между фигурами используя метод Bounding Box
     *     - Найти самую высокую фигуру (чья высота наибольшая)expected
     *     - Вывести расстояние между любыми двумя фигурами, которые поддерживают данную функциональность
     */

    auto collisions = geometry::utils::FindAllCollisions(shapes);
    std::println("There are {} collisions between shapes:", collisions.size());
    for (const auto &c : collisions) {
        std::visit([](const auto &s1, const auto &s2){
            std::println("\t{} and {};", s1, s2);
        }, c.first, c.second);
    }

    auto height = geometry::utils::FindHighestShape(shapes);
    if (!height.has_value()) {
        std::println("No shape for height calculation");
    } else {
        std::println("Maximum height - {}", height.value());
    }
    
    auto idxPairs = std::views::cartesian_product(
        std::views::iota(0zu, shapes.size()),
        std::views::iota(0zu, shapes.size())
    )
    | std::views::filter([](const auto &pair){ 
        auto [i, j] = pair;
        return i < j; 
    });

    auto it = std::ranges::find_if(idxPairs, [&shapes](const auto &pair){
        auto [i, j] = pair;
        return geometry::queries::DistanceBetweenShapes(shapes[i], shapes[j]).has_value();
    });

    if (it == idxPairs.end()) {
        std::println("Cannot calculate distance");
    } else {
        auto [i, j] = *it;
        auto d = geometry::queries::DistanceBetweenShapes(shapes[i], shapes[j]);
        std::visit([&d](const auto &s1, const auto &s2){
            std::println("Distance between {} and {} - {}", s1, s2, d.value());
        }, shapes[0], shapes[1]);
    }
}

///
///
///

void PerformExtraShapeAnalysis(std::span<const Shape> shapes) {
    std::println("\n=== Shape Extra Analysis ===");

    /*
     * Используйте ranges и созданные классы чтобы:
     *     - Вывести 3 любые фигуры, которые находятся выше 50.0
     *     - Вывести фигуры с наименьшей и с наибольшей высотами
     */
    auto any3above50 = shapes 
    | std::views::filter([](const Shape &s){
        return std::visit([](const auto &s){ return s.Height() > 50.0; }, s);
    })
    | std::views::take(3);

    std::println("Shapes higher that 50.0:");
    for (const auto &i : any3above50) {
        std::visit([](const auto &s){ std::println("{}", s); }, i);
    }
    
    auto maxHeight = geometry::utils::FindHighestShape(shapes);
    if (!maxHeight.has_value()) {
        std::println("No shape for height calculation");
    } else {
        std::println("Maximum height - {}", maxHeight.value());
    }

    auto minHeight = std::ranges::min(std::views::transform(shapes, [](const auto &s){
        return std::visit([](const auto &s){ return s.Height(); }, s);
    }));
    std::println("Minimum height - {}", minHeight);
}

///
///
///

int main() {
    using namespace std::literals;

    std::vector<Shape> shapes =  utils::ParseShapes(R"(
        circle 40 45 15;
        circle -5 40 15;
        regularpolygon -25 20 20 5;
        regularpolygon 10 -30 15 10;
        triangle 35 35 35 55 50 35;
        triangle 20 -50 40 -50 30 -30;
        polygon -50 -40 -70 5 -50 10 -25 5;
        rectangle 45 -30 15 10;
        line 30 45 -40 -20;
        badshape;
        circle 0 0 -1;
        polygon 1;
        rectangle 30 40 -5 -10;
    )");
    std::println("Parsed {} shapes", shapes.size());

    // Выведите индекс каждой фигуры и её высоту
    PrintAllHeights(shapes);

    // Вызываем разработанные функции
    PrintAllIntersections(shapes[0], std::span<const geometry::Shape>{ ++shapes.begin(), shapes.end()});
    PrintDistancesFromPointToShapes(Point2D{10.0, 10.0}, shapes);
    PerformShapeAnalysis(shapes);
    PerformExtraShapeAnalysis(shapes);

    // Рисуем все фигуры
    // Важно: после изучения графика - нажмите Enter чтобы продолжить выполнение и построить 2ой график
    //
    geometry::visualization::Draw(shapes, std::string { "/tmp/shapeplot.png" });

    //
    // Формируем список из вершин всех фигур
    //
    { 
        std::vector<Point2D> points;

        //
        // Находим список точек, для построения выпуклой оболочки - convex hull - алгоритмом Грэхема 
        // Создаём из них объект класса `Polygon` и добавляем его в список shapes
        // Рисуем все фигуры
        //
        for (const auto &i : shapes) {
            std::visit(
                [&points](const auto &s){ std::ranges::copy(s.Vertices(), std::back_inserter(points)); }, 
                i);
        }

        auto grahamExtract = geometry::convex_hull::GrahamScan(points);
        if (grahamExtract.has_value()) {
            shapes.push_back(geometry::Polygon { grahamExtract.value() });
            geometry::visualization::Draw(shapes, "/tmp/grahamplot.png"s );
        } else {
            std::println("{}", grahamExtract.error());
        }
    }

    {
        std::vector<Point2D> points = {{0, 0}, {10, 0}, {5, 8}, {15, 5}, {2, 12}};

        //
        // Используйте список точек points или свой, чтобы
        // выполнить алгоритм триангуляции Делоне алгоритмом Боуэра-Ватсона
        //
        // После успешного завершения алгоритма - выведите результат для проверки
        // используя geometry::visualization::Draw
        //
        auto triangulationResult = geometry::triangulation::DelaunayTriangulation(points);
        if (triangulationResult.has_value()) {
            geometry::visualization::Draw(triangulationResult.value(), "/tmp/triangulation.png"s);
        }
        else {
            std::println("{}", triangulationResult.error());
        }
    }
    return 0;
}
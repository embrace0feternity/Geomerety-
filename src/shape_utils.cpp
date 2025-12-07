#include "shape_utils.hpp"
#include <functional>
#include <print>
#include <iostream>

namespace geometry::utils {

// Разбивает строку на слова (по пробелам), игнорируя лишние пробелы
std::vector<std::string_view> SplitIntoWords(std::string_view s) {
    std::vector<std::string_view> words;
    size_t start = 0;
    size_t end = 0;

    while (start < s.size()) {
        // Пропускаем пробелы
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        if (start >= s.size()) break;
        end = start;
        while (end < s.size() && !std::isspace(static_cast<unsigned char>(s[end]))) ++end;
        words.push_back(s.substr(start, end - start));
        start = end;
    }
    return words;
}

// Безопасный парсинг строки в double (без исключений)
std::optional<double> ParseDouble(std::string_view s) {
    double value = 0.0;
    auto result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec == std::errc{} && result.ptr == s.data() + s.size()) {
        return value;
    }
    return std::nullopt;
}

// Парсит строку в вектор double
std::optional<std::vector<double>> ParseDoubles(std::string_view s) {
    if (s.empty()) return std::nullopt;
    auto tokens = SplitIntoWords(s);
    if (tokens.empty()) return std::nullopt;

    std::vector<double> result;
    result.reserve(tokens.size());

    for (auto token : tokens) {
        auto num = ParseDouble(token);
        if (!num.has_value()) {
            return std::nullopt;
        }
        result.push_back(*num);
    }
    return result;
}

// Проверяет размер вектора и возвращает его, если совпадает
std::optional<std::vector<double>> RequireSize(const std::vector<double>& v, size_t expected) {
    return (v.size() == expected) ? std::make_optional(v) : std::nullopt;
}

std::optional<std::vector<double>> ExpectedPoints(const std::vector<double>& v, int expected = -1) {
    if (v.size() % 2 == 1) return std::nullopt;
    if (expected == -1) {
        return std::make_optional(v);
    }
    return ((v.size() / 2) == expected) ? std::make_optional(v) : std::nullopt;
}

// Проверяет, что значение > 0
std::optional<double> RequirePositive(double x) {
    return (x > 0) ? std::make_optional(x) : std::nullopt;
}

// Проверяет, что double представляет целое число >= min_value
std::optional<int> RequireIntegerAtLeast(double d, int min_value) {
    int i = static_cast<int>(d);
    if (static_cast<double>(i) == d && i >= min_value) {
        return i;
    }
    return std::nullopt;
}

// Конструкторы фигур

/// Creates a circle using it's center position (v[0]; v[1]) and a radius(v[2])
std::optional<Shape> MakeCircle(const std::vector<double>& v) {
    return RequireSize(v, 3)
        .and_then([](const std::vector<double> &v){
            return RequirePositive(v[2]);
        })
        .transform([&v](double radius){
            return Circle { Point2D{ v[0], v[1] }, radius };
        });
}

/// Creates a line from 2 points ((v[0]; v[1]); (v[2], v[3]))
std::optional<Shape> MakeLine(const std::vector<double>& v) {
    return RequireSize(v, 4)
        .transform([](const std::vector<double> &v){
            return Line { Point2D{ v[0], v[1] }, Point2D{ v[2], v[3] } };
        });
}

/// Creates a line from 3 points ((v[0]; v[1]); (v[2], v[3]), (v[4], v[5]))
std::optional<Shape> MakeTriangle(const std::vector<double>& v) {
    return RequireSize(v, 6)
        .transform([](const std::vector<double> &v){
            return Triangle { Point2D{ v[0], v[1] }, Point2D{ v[2], v[3] }, Point2D{ v[4], v[5] } };
        });
}

/// Creates a rectagle with a left bottom point (v[0], v[1]),
///  a width v[2] and a height v[3].
std::optional<Shape> MakeRectangle(const std::vector<double>& v) {
    return RequireSize(v, 4)
        .and_then([](const std::vector<double> &v){
            return RequirePositive(v[2]);
        })
        .and_then([&v](double){
            return RequirePositive(v[3]);
        })
        .transform([&v](double){
            return Rectangle { Point2D{ v[0], v[1] }, v[2], v[3] };
        });
}

/// Creates a polygon with the center position (v[0]; v[1]), radius v[2]
///  and number of sides v[3]
std::optional<Shape> MakeRegularPolygon(const std::vector<double>& v) {
    return RequireSize(v, 4)
        .and_then([](const std::vector<double> &v){
            return RequirePositive(v[2]);
        })
        .and_then([&v](double){
            return RequireIntegerAtLeast(v[3], 3);
        })
        .transform([&v](double){
            return RegularPolygon { Point2D{ v[0], v[1] }, v[2], v[3] };
        });
}

/// Creates an any form polygon
std::optional<Shape> MakePolygon(const std::vector<double>& v) {
    /// createPolygon will be called if and only if v has even number of points
    auto createPolygon = [](const std::vector<double> &v){
        std::vector<Point2D> rt;
        rt.reserve(v.size() / 2);
        for (int i = 0; i < v.size(); i += 2){
            rt.emplace_back(v[i], v[i+1]);
        }
        return rt;
    };

    if (v.size() < 6) {
        return std::nullopt;
    }

    return ExpectedPoints(v, -1)
        .transform([&createPolygon](const std::vector<double> &v){
            return Polygon { createPolygon(v) };
        });
}

// Парсинг одной фигуры
std::optional<Shape> ParseSingleShape(std::string_view token) {
    auto parts = SplitIntoWords(token);
    if (parts.empty()) return std::nullopt;

    std::string_view type = parts[0];
    std::string param_str;
    for (auto i : std::views::iota(1u, parts.size())) {
        if (!param_str.empty()) param_str += ' ';
        param_str += std::string(parts[i]);
    }

    // Выбираем конструктор по имени
    auto get_maker = [](std::string_view t) 
        -> std::optional<std::function<std::optional<Shape>(const std::vector<double>&)>> {
        if (t == "circle")   return MakeCircle;
        if (t == "line")     return MakeLine;
        if (t == "triangle") return MakeTriangle;
        if (t == "rectangle")return MakeRectangle;
        if (t == "polygon")  return MakePolygon;
        if (t == "regularpolygon")  return MakeRegularPolygon;
        return std::nullopt;
    };

    //Обратите внимание на код ниже
    return get_maker(type)
        .and_then([&](auto maker) {
            return ParseDoubles(param_str)
                .and_then(maker);
        });
}

std::vector<Shape> ParseShapes(std::string_view input) {
    std::vector<Shape> result;

    // Разделяем по ';'
    size_t start = 0;
    size_t end = 0;
    while (start < input.size()) {
        end = input.find(';', start);
        if (end == std::string_view::npos) end = input.size();

        std::string_view token = input.substr(start, end - start);
        // Убираем пробелы по краям
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.front()))) token.remove_prefix(1);
        while (!token.empty() && std::isspace(static_cast<unsigned char>(token.back()))) token.remove_suffix(1);

        if (!token.empty()) {
            auto shape_opt = ParseSingleShape(token);
            if (shape_opt.has_value()) {
                result.push_back(*shape_opt);
            }
        }

        start = end + 1;
    }

    return result;
}

std::vector<std::pair<Shape, Shape>> FindAllCollisions(std::span<const Shape> shapes) {
    /*
     * Используйте библиотеку ranges, чтобы найти все коллизии между фигурами методом BoundingBoxesOverlap
     *
     * Также используйте наиболее эффективный метод добавления объектов в collisions
     */
    std::vector<std::pair<Shape, Shape>> collisions;
    collisions.reserve(shapes.size());

    /// Create a view of pairs(i-th, j-th), where indexes lay in ranges [0; shapes.size())
    auto idxPairs = std::views::cartesian_product(
        std::views::iota(0zu, shapes.size()),
        std::views::iota(0zu, shapes.size()) 
    );

    /// Get rid of all the duplicates ({1,0} and {0, 1} are equal) and
    ///  those pairs where i == j. 
    auto idxUnique = idxPairs 
        | std::views::filter([](const auto &pair){ 
            auto [i, j] = pair;
            return i < j; 
        });

    /// Look for collisions between remaining pairs.
    auto collisionPairs = idxUnique
        | std::views::filter([&shapes](const auto &pair) { 
            auto [i, j] = pair; 
            return queries::BoundingBoxesOverlap(shapes[i], shapes[j]);}) 
        | std::views::transform([&shapes](const auto &pair){
            auto [i, j] = pair; 
            return std::make_pair(shapes[i], shapes[j]);
        });

    std::ranges::copy(collisionPairs, std::back_inserter(collisions));

    return collisions;
}

std::optional<size_t> FindHighestShape(std::span<const Shape> shapes) {
    /*
     * Используйте библиотеку ranges, чтобы найти самую высокую фигуру
     *
     * Важно: использование ручной итерации по фигурам не разрешается
     */
    if (shapes.size() == 0) {
        return std::nullopt;
    }
    
    auto heights = std::views::transform(shapes, [](const auto &s){
        return std::visit([](const auto &s){ return s.Height(); }, s);
    });
    
    return std::ranges::max(heights);
}

}
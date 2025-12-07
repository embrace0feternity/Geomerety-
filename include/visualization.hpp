#include "geometry.hpp"
#include "triangulation.hpp"
#include <span>

namespace geometry::visualization {

void Draw(std::span<geometry::Shape> shapes, const std::string &resultFilePath);

void Draw(std::span<const geometry::triangulation::DelaunayTriangle> triangles, const std::string &resultFilePath);

}  // namespace geometry::visualization

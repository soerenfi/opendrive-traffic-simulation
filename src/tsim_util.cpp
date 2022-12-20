
#include <cmath>
#include <cstdint>
#include <limits>

namespace tsim {
namespace util {

bool almostEqual(double a, double b) {
  return std::abs(a - b) < std::numeric_limits<double>::epsilon();
}

} // namespace util

} // namespace tsim

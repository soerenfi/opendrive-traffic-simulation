#ifndef __TSIM_GEOMETRIC_HPP__
#define __TSIM_GEOMETRIC_HPP__

#include <glm/fwd.hpp>

#include <cmath>
#include <cstdint>
#include <limits>

namespace tsim {

using Point = glm::vec3;

namespace util {

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

bool almostEqual(double a, double b);

} // namespace util

} // namespace tsim

#endif // __TSIM_GEOMETRIC_HPP__

#include <cmath>
#include <numbers>
#include <array>

#include "fmt/format.h"

inline double compute_tracking_error0(double pivot_spindle, double pivot_stylus, double radius)
{
    auto d2 = radius * radius - pivot_spindle * pivot_spindle + pivot_stylus * pivot_stylus;
    auto d = std::sqrt(d2);
    fmt::println("d2 = {}, d = {}", d2, d);
    return 360.0 * std::asin(d / radius) / (2.0 * std::numbers::pi);
}

inline double compute_tracking_error(double pivot_spindle, double pivot_stylus, double radius)
{
    auto d = radius * radius - pivot_spindle * pivot_spindle + pivot_stylus * pivot_stylus;
    d /= (2.0 * pivot_stylus);
    return 360.0 * std::asin(d / radius) / (2.0 * std::numbers::pi);
}

struct geometry_t
{
    char const* name_;
    double pivot_spindle_;
    double pivot_stylus_;
    double offset_;
};

constexpr geometry_t rega{ "Rega", 222.0, 237.0, 22.0 };
constexpr geometry_t linn{ "Linn", 211.0, 229.0, 24.0 };
constexpr geometry_t SME { "SME", 232.32, 215.35, 23.204 };
constexpr geometry_t SME12 { "SME12", 308.19, 295.60, 17.278 };

int main()
{
    std::array dd {57.2, 60.0, 70.0, 80.0, 90.0, 100.0, 110.0, 120.4};
    auto& g = rega;
    for(auto d: dd)
        fmt::println("{} at {}, error = {}", g.name_, d, compute_tracking_error(g.pivot_spindle_, g.pivot_stylus_, d) - g.offset_);
}
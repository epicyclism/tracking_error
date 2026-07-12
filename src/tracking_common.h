//
// Copyright (c) 2026 Paul Ranson, paul@epicyclism.com
//
//
#pragma once

#include <array>
#include <vector>
#include <cmath>
#include <numbers>

constexpr double inner_min  = 54.0;
constexpr double outer_max = 150.0;
constexpr double scan_increment = 0.01;
constexpr size_t step_count = (outer_max - inner_min) / scan_increment;
constexpr double rpm = 100.0 / 3;
constexpr double pk_vel = 100.0; // mm/s
// radius and velocity at rpm for interesting region
std::array<double, step_count> x_axis;         // mm
std::array<double, step_count> x_axis_velocity;// mm/s

inline constexpr double to_velocity(double radius)
{
    return radius * 2 * std::numbers::pi * rpm / 60.0;
}

inline void init_x_axis()
{
    size_t i = 0;
    for(auto x = inner_min; x < outer_max; x += scan_increment)
    {
        x_axis[i] = x;
        x_axis_velocity[i] = to_velocity(x);
        ++i;
    }
}

constexpr inline double from_degrees(double d)
{
    return d * 2.0 * std::numbers::pi / 360.0;
}

constexpr inline double to_degrees(double r)
{
    return r * 360.0 / (2.0 * std::numbers::pi);
}

// assumes values cross 0, good assumption for this application.
//
template<typename I> I find_next_opposite_sign(I b, I e)
{
    if(*b < 0)
    {
        while(b != e)
        {
            if( *b > 0)
                break;
            ++b;
        }
    }
    else
    {
        while(b != e)
        {
            if( *b < 0)
                break;
            ++b;
        }
    }
    return b;
}

// tracking error in radians
inline double compute_tracking_error(double pivot_spindle, double pivot_stylus, double radius)
{
    auto d = radius * radius - pivot_spindle * pivot_spindle + pivot_stylus * pivot_stylus;
    d /= (2.0 * pivot_stylus);
    return std::asin(d / radius);
}

// 2nd harmonic percent assuming pk recording velocity.
// te tracking error radians, gs rotational groove vel mm/s
// After Baerwald then Stevenson
inline double compute_tracking_distortion( double te, double gs)
{
    return 100.0 * pk_vel * std::tan(std::abs(te)) / gs;
}

struct geometry_t
{
    char const* name_;
    double pivot_spindle_;
    double pivot_stylus_;
    double offset_;
    double offset_rad_cache_;
    bool modified_;
};

constexpr geometry_t rega{ "Rega", 222.0, 237.0, 22.0, from_degrees(22.0), false };
constexpr geometry_t linn{ "Linn", 211.0, 229.0, 24.0, from_degrees(24.0), false };
constexpr geometry_t SME { "SME", 215.35, 232.32, 23.204, from_degrees(23.204), false };
constexpr geometry_t SME12 { "SME12", 295.60, 308.19, 17.278, from_degrees(17.278), false };

struct geometry_data_t
{
    std::array<double, step_count> tracking_error_;
    std::array<double, step_count> tracking_distortion_;
    std::array<double, step_count> skating_force_;
    std::vector<double> zeroes_;
};

inline void update_offset_rad_cache(geometry_t& g)
{
    g.offset_rad_cache_ = from_degrees(g.offset_);
}

inline void recompute(geometry_t const& g, geometry_data_t& data)
{
    double rad = inner_min;
    for(auto i = 0; i < data.tracking_error_.size(); ++i)
    {
        auto e = compute_tracking_error(g.pivot_spindle_, g.pivot_stylus_, rad);
        e -= g.offset_rad_cache_;
        auto d = compute_tracking_distortion(e, x_axis_velocity[i]);
        data.tracking_error_[i] = to_degrees(e);
        data.tracking_distortion_[i] = d;
    }
    data.zeroes_.clear();
    for(auto v = find_next_opposite_sign(data.tracking_error_.begin(), data.tracking_error_.end()); v != data.tracking_error_.end(); v = find_next_opposite_sign(v, data.tracking_error_.end()))
        data.zeroes_.emplace_back(inner_min + double(std::distance(data.tracking_error_.begin(), v)) * 0.01);
}

inline void init_geometries(auto& geometries, auto& geometries_data)
{
    for(size_t i = 0; i < geometries.size(); ++i)
    {
        update_offset_rad_cache(geometries[i]);
        recompute(geometries[i], geometries_data[i]);
    }
}
//
// Copyright (c) 2026 Paul Ranson, paul@epicyclism.com
//
//

#include <algorithm>
#include <numeric>

#include "tracking_common.h"

#include "fmt/format.h"
#include "fmt/ranges.h"

#include "timer.h"

void test_a(geometry_t const& g)
{
    fmt::println("Spot testing {}", g.name_);
    for(auto r = inner_min; r < outer_max + 10.0; r += 10.0)
    {
        auto te = compute_tracking_error(g.pivot_spindle_, g.pivot_stylus_, r) - g.offset_rad_cache_;
        fmt::println("radius {} mm, error {} deg, distortion {} %", r, to_degrees(te), compute_tracking_distortion(te, to_velocity(r)));
    }
}

void test_b(geometry_t const& g)
{
    std::vector<double> te;
    std::vector<double> td;
    te.reserve(step_count);
    td.reserve(step_count);
    {
        timer t("computing tracking error and distortion");
        for(auto r = inner_min; r < outer_max; r += scan_increment)
        {
            auto e = compute_tracking_error(g.pivot_spindle_, g.pivot_stylus_, r) - g.offset_rad_cache_;
            te.emplace_back(e);
            td.emplace_back(compute_tracking_distortion(std::abs(e), to_velocity(r)));
        }
    }
    std::vector<double> nulls;
    for(auto v = find_next_opposite_sign(te.begin(), te.end()); v != te.end(); v = find_next_opposite_sign(v,te.end()))
        nulls.emplace_back(inner_min + double(std::distance(te.begin(), v)) * 0.01);
    fmt::println("{} nulls at {}", g.name_, nulls);
	fmt::println("{} max error {}, max distortion {}%", g.name_, to_degrees(*std::ranges::max_element(te)), *std::ranges::max_element(td));
}

double evaluate_e(geometry_data_t const& data)
{
    double err = 0.0;
    for(auto e : data.tracking_error_)
        err += std::abs(e);
    return err / double(data.tracking_error_.size());
}

double evaluate_d(geometry_data_t const& data)
{
    double distortion = 0.0;
    for(auto d : data.tracking_distortion_)
        distortion += std::abs(d);
    return distortion / double(data.tracking_distortion_.size());
}

void test_c(geometry_t const& g)
{
    timer t("brute force lowest distortion");
    geometry_data_t data;
    geometry_t g2 = g;
    double distortion = 100.0;
    for(double off = from_degrees(-5.0); off <= from_degrees(5.0); off += from_degrees(0.1))
    {
        for(double oh = -5.0; oh < 5.0; oh += 0.1)
        {
            g2.offset_rad_cache_ = g.offset_rad_cache_ + off;
            g2.pivot_stylus_ = g.pivot_stylus_ + oh;
            recompute(g2, data);
            auto dt = evaluate_e(data);
            if(dt < distortion)
            {
                distortion = dt;
                fmt::println("offset {} deg, overhang {} mm, avg error {} %", to_degrees(g2.offset_rad_cache_), g2.pivot_stylus_ - g2.pivot_spindle_, distortion);
            }
        }
    }
}

void test_d(geometry_t const& g)
{
    timer t("brute force lowest distortion, overhang only");
    geometry_data_t data;
    geometry_t g2 = g;
    double distortion = 100.0;
        for (double oh = -15.0; oh < 15.0; oh += 0.1)
        {
            g2.pivot_stylus_ = g.pivot_stylus_ + oh;
            recompute(g2, data);
            auto dt = evaluate_d(data);
            if (dt < distortion)
            {
                distortion = dt;
                fmt::println("offset {} deg, overhang {} mm, avg error {} %", to_degrees(g2.offset_rad_cache_), g2.pivot_stylus_ - g2.pivot_spindle_, distortion);
            }
        }
}

constexpr double centre = 140.0;
constexpr double plusminus = 2.5;

void test_plus(geometry_t const& g, int additional_distance)
{
	geometry_data_t data;
	geometry_t g2 = g;
	auto b = data.tracking_distortion_.begin() + (centre - plusminus - inner_min) / scan_increment;
	auto e = b + (2 * plusminus) / scan_increment;
	recompute(g2, data);
	auto avg1 = std::accumulate(b, e, 0.0) / ((2 * plusminus) / scan_increment);
	g2.pivot_stylus_ += additional_distance;
	recompute(g2, data);
	auto avg2 = std::accumulate(b, e, 0.0) / ((2 * plusminus) / scan_increment);
	fmt::println("Avg distortion base: {} %", avg1);
	fmt::println("Avg distortion with additional distance: {} %", avg2);
}

int main()
{
//    constexpr geometry_t under{ "under", 215.0, 206.4, 0.0, from_degrees(0.0), false };
    constexpr geometry_t under{ "under", 215.0, 206.40, 0.0, from_degrees(0.0), false };

//    test_a(SME12);
//    test_a(SME);
//    test_a(rega);
//    test_a(linn);
//    test_a(under);

//    test_b(SME12);
//    test_b(SME);
//    test_b(rega);
//    test_b(linn);   

    init_x_axis();
//    test_c(rega);

//     test_b(under);
//    test_d(under);
    test_plus(rega, 7);
}
#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <array>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

constexpr double inner_min  = 54.0;
constexpr double outer_max = 150.0;
constexpr double scan_increment = 0.01;
constexpr double rpm = 100.0 / 3;
constexpr double pk_vel = 100.0; // mm/s
// radius and velocity at rpm for interesting region
std::vector<double> x_axis;         // mm
std::vector<double> x_axis_velocity;// mm/s

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

//  2nd harmonic percent assuming pk recording velocity.
// te tracking error radians, gs rotational groove vel mm/s
// After Baerwald then Stevenson
inline double compute_tracking_distortion( double te, double gs)
{
    return 100.0 * pk_vel * std::tan(te) / gs;
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

constexpr geometry_t rega{ "Rega", 222.0, 237.0, 22.0, false };
constexpr geometry_t linn{ "Linn", 211.0, 229.0, 24.0, false };
constexpr geometry_t SME { "SME", 215.35, 232.32, 23.204, false };
constexpr geometry_t SME12 { "SME12", 295.60, 308.19, 17.278, false };

struct geometry_data_t
{
    std::vector<double> tracking_error_;
    std::vector<double> tracking_distortion_;
    std::vector<double> skating_force_;
    std::vector<double> zeroes_;
};

inline double from_degrees(double d)
{
    return d * 2.0 * std::numbers::pi / 360.0;
}

inline double to_degrees(double r)
{
    return r * 360.0 / (2.0 * std::numbers::pi);
}

inline void update_offset_rad_cache(geometry_t& g)
{
    g.offset_rad_cache_ = from_degrees(g.offset_);
}

inline void recompute(geometry_t const& g, geometry_data_t& data)
{
    data.tracking_error_.resize(size_t((outer_max - inner_min) / scan_increment));
    data.tracking_distortion_.resize(size_t((outer_max - inner_min) / scan_increment));
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

void draw(std::array<geometry_t, 6>& g, size_t current_geometry, std::array<geometry_data_t, 6>& data)
{
	auto [ww, wh] = ImGui::GetWindowSize();
	auto ww3 = ww / 3;
	auto io{ ImGui::GetIO() };
	ImGui::Begin("Tracking Error", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
	// Set window size and position
	ImGui::SetWindowSize(ImVec2(ww - 5, wh - 5));
	ImGui::SetWindowPos(ImVec2(2, 2));
	ImGui::SetNextWindowPos(ImVec2(0.0F, 0.0F));
	ImGui::SetNextWindowSize(ImVec2(0.0F, 0.0F));
    if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Option 1"))
        {
            current_geometry = 0;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Option 2"))
        {
            current_geometry = 1;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

	ImGui::Text("Parameters");
    bool modded = false;
    if (ImGui::BeginCombo("Load from...", 0, ImGuiComboFlags_NoPreview))
    {
        for (int n = 2; n < g.size(); n++)
        {
            if (ImGui::Selectable(g[n].name_))
            {
                g[current_geometry] = g[n];
                modded = true;
            }
        }
        ImGui::EndCombo();
    }

    double tmp = g[current_geometry].pivot_spindle_;
    if(ImGui::InputDouble("Pivot - Spindle", &tmp, 0.1, 0.5, "%.2f"))
    {
        g[current_geometry].pivot_spindle_ = tmp;
        modded = true;
    }
    tmp = g[current_geometry].pivot_stylus_;
    if(ImGui::InputDouble("Pivot - Stylus", &tmp, 0.1, 0.5, "%.2f"))
    {
        g[current_geometry].pivot_stylus_ = tmp;
        modded = true;
    }
	tmp = g[current_geometry].offset_;
    if (ImGui::InputDouble("Headshell offset", &tmp, 0.1, 30.0, "%.2f"))
    {
        g[current_geometry].offset_ = tmp;
        update_offset_rad_cache(g[current_geometry]);
        modded = true;
    }
    if (modded)
    {
        recompute(g[current_geometry], data[current_geometry]);
    }
    ImGui::Separator();
	ImGui::TextUnformatted("Graphs");
    if (ImPlot::BeginPlot("Tracking"))
    {
        ImPlot::SetupAxes("radius (mm)","error (deg)");
		ImPlot::PlotLine("Tracking Error", x_axis.data(), data[current_geometry].tracking_error_.data(), static_cast<int>(data[current_geometry].tracking_error_.size()));
		ImPlot::PlotLine("Tracking Distortuib", x_axis.data(), data[current_geometry].tracking_distortion_.data(), static_cast<int>(data[current_geometry].tracking_error_.size()));
        ImPlot::EndPlot();
    }
    ImGui::TextUnformatted("Zeroes at ");
    for(auto z : data[current_geometry].zeroes_)
    {
        ImGui::SameLine();
        ImGui::Text("%f ", z);
    }
    ImGui::Text("first val %f, zero count %ld", data[current_geometry].tracking_error_[0], data[current_geometry].zeroes_.size());
    ImGui::End();
}

int main()
{
	std::array<geometry_t, 6> geometries{ rega, linn, rega, linn, SME, SME12};
	std::array<geometry_data_t, 6> geometries_data;
    size_t current_geometry = 0;
    auto x = inner_min;
    // mm and cm/s ? 
    std::ranges::generate(x_axis, [&](){ auto xr = x; x += scan_increment; return xr;});    
    std::ranges::generate(x_axis_velocity, [&](){ auto xr = x; x += scan_increment; return xr * 2 * std::numbers::pi * rpm / 60.0;});    
    
    for(size_t i = 0; i < 6; ++i)
    {
        update_offset_rad_cache(geometries[i]);
        recompute(geometries[i], geometries_data[i]);
    }
    x_axis.resize(geometries_data[0].tracking_error_.size());
    x_axis_velocity.resize(geometries_data[0].tracking_error_.size());
 
    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "Tracking Error Evaluator";
        runnerParams.callbacks.ShowGui = [&]() {
        draw(geometries, current_geometry, geometries_data);
        };
    runnerParams.imGuiWindowParams.showMenuBar = false;
        // Status bar:
    runnerParams.imGuiWindowParams.showStatusBar = false;
    runnerParams.imGuiWindowParams.showStatus_Fps = false;
    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
        HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;
    runnerParams.callbacks.PostInit = [] { ImPlot::CreateContext(); };
	runnerParams.callbacks.BeforeExit = [] { ImPlot::DestroyContext(); };
    //runnerParams.callbacks.SetupImGuiStyle = [&cfg_cache]() {ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_(cfg_cache.theme_)); };
    // ini
    runnerParams.iniFolderType = HelloImGui::IniFolderType::AppUserConfigFolder;

    HelloImGui::Run(runnerParams);
    return 0;
}

#include <algorithm>
#include <cmath>
#include <numbers>
#include <string>
#include <array>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

constexpr double inner_min  = 44.0;
constexpr double outer_max = 150.0;
constexpr double scan_increment = 0.1;

inline double compute_tracking_error(double pivot_spindle, double pivot_stylus, double radius)
{
    auto d = pivot_stylus - std::sqrt(pivot_spindle * pivot_spindle - radius * radius);
    return 360.0 * std::atan(d / radius) / (2.0 * std::numbers::pi);
}

struct geometry_t
{
    char const* name_;
    double pivot_spindle_;
    double pivot_stylus_;
    double offset_;
    double min_radius_;
    double max_radius_;
};

constexpr geometry_t rega{ "Rega", 222.0, 237.0, 22.0, 48.0, 145.0 };
constexpr geometry_t linn{ "Linn", 211.0, 229.0, 24.0, 48.0, 145.0 };
constexpr geometry_t SME { "SME", 232.32, 215.35, 23.204, 48.0, 145.0 };
constexpr geometry_t Custom{ "Custom", 231.2, 213.25, 23.84, 48.0, 145.0 };
constexpr size_t custom_geometry = 3;

struct geometry_data_t
{
    std::vector<double> tracking_error_;
    std::vector<double> tracking_distortion_;
    std::vector<double> skating_force_;
};

void recompute(geometry_t const& g, geometry_data_t& data)
{
    data.tracking_error_.resize(size_t((outer_max - inner_min) / scan_increment));
    double rad = outer_max;
	std::ranges::generate(data.tracking_error_, [&]() { auto e = compute_tracking_error(g.pivot_spindle_, g.pivot_stylus_, rad); rad -= scan_increment; return e - g.offset_; });
}

void draw(std::array<geometry_t, 4>& g, size_t current_geometry, geometry_data_t& data)
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
	ImGui::Text("Parameters for %s geometry", g[current_geometry].name_);

    bool modded = false;
    double tmp = g[current_geometry].pivot_spindle_;
    if(ImGui::InputDouble("Pivot - Spindle", &tmp, 0.1, 0.5, "%.2f"))
    {
        g[current_geometry].pivot_spindle_ = tmp;
		modded = true;
    }
    tmp = g[current_geometry].pivot_stylus_;
    if(ImGui::InputDouble("Pivot - Stylus", &tmp, 0.1, 0.5, "%.2f"))
    {
        if(current_geometry != custom_geometry)
        {
            g[custom_geometry] = g[current_geometry];
            current_geometry = custom_geometry;
        }
        g[current_geometry].pivot_stylus_ = tmp;
		modded = true;
    }
	tmp = g[current_geometry].offset_;
    if (ImGui::InputDouble("Headshell offset", &tmp, 0.1, 30.0, "%.2f"))
    {
        if (current_geometry != custom_geometry)
        {
            g[custom_geometry] = g[current_geometry];
            current_geometry = custom_geometry;
        }
        g[custom_geometry].offset_ = tmp;
		modded = true;
    }
    tmp = g[current_geometry].min_radius_;
    if(ImGui::InputDouble("Min radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        if (current_geometry != custom_geometry)
        {
            g[custom_geometry] = g[current_geometry];
            current_geometry = custom_geometry;
        }
        g[custom_geometry].min_radius_ = tmp;
		modded = true;
    }
    tmp = g[current_geometry].max_radius_;
    if(ImGui::InputDouble("Max radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        if (current_geometry != custom_geometry)
        {
            g[custom_geometry] = g[current_geometry];
            current_geometry = custom_geometry;
        }
        g[custom_geometry].max_radius_ = tmp;
		modded = true;
    }
    if (modded)
    {
        recompute(g[current_geometry], data);
    }
    ImGui::Separator();
	ImGui::TextUnformatted("Graph");
    if (ImPlot::BeginPlot("Tracking Error"))
    {
		ImPlot::PlotLine("Tracking Error", data.tracking_error_.data(), static_cast<int>(data.tracking_error_.size()), 0.01);
        ImPlot::EndPlot();
    }
    ImGui::End();
}

int main()
{
	std::array<geometry_t, 4> geometries{ rega, linn, SME, Custom };
	size_t current_geometry = 0;

	geometry_data_t data;
    recompute(geometries[current_geometry], data);

    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "Tracking Error Evaluator";
        runnerParams.callbacks.ShowGui = [&]() {
        draw(geometries, current_geometry, data);
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

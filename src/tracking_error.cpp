#include <algorithm>
#include <cmath>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

constexpr double inner_min  = 44.0;
constexpr double outer_max = 150.0;
constexpr double scan_increment = 0.1;

double compute_tracking_error(double pivot_spindle, double pivot_stylus, double radius)
{
    auto e = std::sqrt(pivot_spindle * pivot_spindle - radius * radius) - pivot_stylus;
    return 360.0 * (90.0 - std::acos(e / radius)) /  M_2_PI;
}

struct geometry_t
{
    double pivot_spindle_;
    double pivot_stylus_;
    double offset_;
    double min_radius_;
    double max_radius_;
	void recompute()
	{
        tracking_error_.resize(size_t((outer_max - inner_min) / scan_increment));
        double rad = outer_max;
		std::ranges::generate(tracking_error_, [&]() { auto e = compute_tracking_error(pivot_spindle_, pivot_stylus_, rad); rad -= scan_increment; return e + offset_; });
	}
    std::vector<double> tracking_error_;
};

void draw(geometry_t& g)
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
	ImGui::TextUnformatted("Parameters");
    bool modded = false;
    double tmp = g.pivot_spindle_;
    if(ImGui::InputDouble("Pivot - Spindle", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.pivot_spindle_ = tmp;
		modded = true;
    }
    tmp = g.pivot_stylus_;
    if(ImGui::InputDouble("Pivot - Stylus", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.pivot_stylus_ = tmp;
		modded = true;
    }
	tmp = g.offset_;
    if (ImGui::InputDouble("Headshell offset", &tmp, 0.1, 30.0, "%.2f"))
    {
        g.offset_ = tmp;
		modded = true;
    }
    tmp = g.min_radius_;
    if(ImGui::InputDouble("Min radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.min_radius_ = tmp;
		modded = true;
    }
    tmp = g.max_radius_;
    if(ImGui::InputDouble("Max radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.max_radius_ = tmp;
		modded = true;
    }
    if (modded)
    {
        g.recompute();
    }
    ImGui::Separator();
	ImGui::TextUnformatted("Graph");
    if (ImPlot::BeginPlot("Tracking Error"))
    {
		ImPlot::PlotLine("Tracking Error", g.tracking_error_.data(), static_cast<int>(g.tracking_error_.size()), 0.01);
        ImPlot::EndPlot();
    }
    ImGui::End();
}

int main()
{
    geometry_t g{222.0, 237.0, 22.0, 48.0, 145.0};
    g.recompute();

    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "Tracking Error Evaluator";
        runnerParams.callbacks.ShowGui = [&]() {
        draw(g);
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

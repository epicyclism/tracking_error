//
// Copyright (c) 2026 Paul Ranson, paul@epicyclism.com
//
//

#include <string>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

#include "tracking_common.h"

std::array<geometry_t, 6> geometries{ rega, linn, rega, linn, SME, SME12 };
std::array<geometry_data_t, 6> geometries_data;

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
    if (ImPlot::BeginPlot("TE"))
    {
        ImPlot::SetupAxes("radius (mm)","error (deg)");
		ImPlot::PlotLine("Tracking Error", x_axis.data(), data[current_geometry].tracking_error_.data(), static_cast<int>(data[current_geometry].tracking_error_.size()));
        ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("TED"))
    {
        ImPlot::SetupAxes("radius (mm)","distortion (%)");
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
    size_t current_geometry = 0;
    init_x_axis();
    init_geometries(geometries, geometries_data);
 
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

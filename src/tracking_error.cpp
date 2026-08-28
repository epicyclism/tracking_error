//
// Copyright (c) 2026 Paul Ranson, paul@epicyclism.com
//
//

#include <string>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

#include "tracking_common.h"

void draw(int current_custom_geometry, geometry_t* gp, geometry_data_t* datap)
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
        if (ImGui::BeginTabItem("Custom 1"))
        {
            current_custom_geometry = 0;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Custom 2"))
        {
            current_custom_geometry = 1;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
	ImGui::Text("Parameters");
    bool modded = false;
	geometry_t& g = gp[current_custom_geometry];
	geometry_data_t& data = datap[current_custom_geometry];
    if (ImGui::BeginCombo("Load from...", 0, ImGuiComboFlags_NoPreview))
    {
        for (int n = 0; n < std_geometries.size(); ++n)
        {
            if (ImGui::Selectable(std_geometries[n].name_))
            {
                g = std_geometries[n];
                modded = true;
            }
        }
        ImGui::EndCombo();
    }

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
        update_offset_rad_cache(g);
        modded = true;
    }
    if (modded)
    {
        recompute(g, data   );
    }
    ImGui::Separator();
	ImGui::TextUnformatted("Graphs");
    if (ImPlot::BeginPlot("TE"))
    {
        ImPlot::SetupAxes("radius (mm)","error (deg)");
        if(gp[0].display_)
		    ImPlot::PlotLine("Tracking Error", x_axis.data(), datap[0].tracking_error_.data(), static_cast<int>(datap[0].tracking_error_.size()));
        if(gp[1].display_)
		    ImPlot::PlotLine("Tracking Error", x_axis.data(), datap[1].tracking_error_.data(), static_cast<int>(datap[1].tracking_error_.size()));
        ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("TED"))
    {
        ImPlot::SetupAxes("radius (mm)","distortion (%)");
        if(gp[0].display_)
		    ImPlot::PlotLine("Tracking Distortion", x_axis.data(), datap[0].tracking_distortion_.data(), static_cast<int>(datap[0].tracking_error_.size()));
        if(gp[1].display_)
		    ImPlot::PlotLine("Tracking Distortion", x_axis.data(), datap[1].tracking_distortion_.data(), static_cast<int>(datap[1].tracking_error_.size()));
        ImPlot::EndPlot();
    }
    if (ImPlot::BeginPlot("Anti-skate"))
    {
        ImPlot::SetupAxes("radius (mm)", "skating force Nmm");
        if (gp[0].display_)
            ImPlot::PlotLine("Skating force", x_axis.data(), datap[0].skating_force_.data(), static_cast<int>(datap[0].skating_force_.size()));
        if (gp[1].display_)
            ImPlot::PlotLine("Skating force", x_axis.data(), datap[1].skating_force_.data(), static_cast<int>(datap[1].skating_force_.size()));
        ImPlot::EndPlot();
    }
    ImGui::TextUnformatted("Zeroes at ");
    if (gp[0].display_)
    {
        ImGui::Text("%s - ", gp[0].name_);
        for (auto z : datap[0].zeroes_)
        {
            ImGui::SameLine();
            ImGui::Text("%f ", z);
        }
    }
	if (gp[1].display_)
	{
		ImGui::Text("%s - ", gp[1].name_);
		for (auto z : datap[1].zeroes_)
		{
			ImGui::SameLine();
			ImGui::Text("%f ", z);
		}
	}
    ImGui::End();
}

int main()
{
    init_x_axis();
    geometry_t g[2] = { rega, linn };
    g[0].name_ = "Custom 1";
	g[1].name_ = "Custom 2";
    geometry_data_t data[2];
	recompute(g[0], data[0]);
	recompute(g[1], data[1]);
	int current_custom_geometry = 0;

    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "Tracking Error Evaluator";
        runnerParams.callbacks.ShowGui = [&]() {
        draw(current_custom_geometry, g, data);
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

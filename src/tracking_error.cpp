//
// Copyright (c) 2026 Paul Ranson, paul@epicyclism.com
//
//

#include <fstream>
#include <string>

#include "fmt/core.h"
#include <fmt/ostream.h>

#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"
#include "misc/cpp/imgui_stdlib.h"
#include "portable-file-dialogs.h"

#include "implot.h"

#include "tracking_common.h"

enum graph_type_t
{
    graph_type_tracking_error,
    graph_type_tracking_distortion,
    graph_type_skating_force,
};

struct uistate_t
{
    int current_custom_geometry;
    graph_type_t graph_type;
    std::string  fn_;
};

void export_data(uistate_t& uistate, geometry_t* gp, geometry_data_t* datap)
{
	auto path = pfd::save_file("Export Data", uistate.fn_, { "DAT Files", "*.dat", "All Files", "*" }, pfd::opt::force_overwrite).result();
    if (path.empty())
        return;
    uistate.fn_ = path;
    std::ofstream ofs(uistate.fn_);
    if(!ofs)
	{
		pfd::message("Error", "Could not open file for writing", pfd::choice::ok, pfd::icon::error);
		return;
	}
    fmt::println(ofs, "# Tracking Error Data Export");
    for (int i = 0; i < 2; ++i)
    {
        fmt::println(ofs, "# Geometry {}: {}", i, gp[i].name_);
        fmt::println(ofs, "#   Pivot - Spindle: {}", gp[i].pivot_spindle_);
        fmt::println(ofs, "#   Pivot - Stylus: {}", gp[i].pivot_stylus_);
        fmt::println(ofs, "#   Headshell Offset: {}", gp[i].offset_);
    }
	fmt::println(ofs, "# plotting reminder, starting off point...");
    fmt::println(ofs, "# set style data lines");
    fmt::println(ofs, "# to plot tracking error");
    fmt::println(ofs, "# plot <filename> index 0:1");
    fmt::println(ofs, "# to plot tracking distortion");
    fmt::println(ofs, "# plot <filename> index 1:2");
    fmt::println(ofs, "# to plot skating torque");
    fmt::println(ofs, "# plot <filename> index 3:4");
	fmt::println(ofs, "#   Columns: radius (mm), geometry 1, geometry 2");
    auto r = gp[0].inner_radius_;
    fmt::println(ofs, "#   Tracking error");
	for (auto p = 0; p < datap[0].tracking_error_.size(); ++p)
	{
		fmt::println(ofs, "{} {} {}", r, datap[0].tracking_error_[p], datap[1].tracking_error_[p]);
		r += scan_increment;
	}
    fmt::print(ofs, "\n\n");
    r = gp[0].inner_radius_;
    fmt::println(ofs, "#   Tracking distortion");
    for (auto p = 0; p < datap[0].tracking_distortion_.size(); ++p)
    {
        fmt::println(ofs, "{} {} {}", r, datap[0].tracking_distortion_[p], datap[1].tracking_distortion_[p]);
        r += scan_increment;
    }
    fmt::print(ofs, "\n\n");
    r = gp[0].inner_radius_;
    fmt::println(ofs, "#   Skating torque");
    for (auto p = 0; p < datap[0].skating_force_.size(); ++p)
    {
        fmt::println(ofs, "{} {} {}", r, datap[0].skating_force_[p], datap[1].skating_force_[p]);
        r += scan_increment;
    }
}

void draw(uistate_t& uistate, geometry_t* gp, geometry_data_t* datap)
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
            uistate.current_custom_geometry = 0;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Custom 2"))
        {
            uistate.current_custom_geometry = 1;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
	ImGui::Text("Parameters");
    bool modded = false;
	geometry_t& g = gp[uistate.current_custom_geometry];
	geometry_data_t& data = datap[uistate.current_custom_geometry];
    if (ImGui::BeginCombo("Load from...", 0, ImGuiComboFlags_NoPreview))
    {
        for (int n = 0; n < std_geometries.size(); ++n)
        {
            if (ImGui::Selectable(std_geometries[n].name_))
            {
				g.pivot_spindle_ = std_geometries[n].pivot_spindle_;
				g.pivot_stylus_ = std_geometries[n].pivot_stylus_;
				g.offset_ = std_geometries[n].offset_;
				g.inner_radius_ = std_geometries[n].inner_radius_;
				g.outer_radius_ = std_geometries[n].outer_radius_;
                update_offset_rad_cache(g);
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
    tmp = g.inner_radius_;
    if(ImGui::InputDouble("Inner radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.inner_radius_ = tmp;
    }
    tmp = g.outer_radius_;
    if(ImGui::InputDouble("Outer radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.outer_radius_ = tmp;
    }
    if (ImGui::Button("Reset radii"))
    {
        g.inner_radius_ = inner_min_std;
        g.outer_radius_ = outer_max_std;
    }
    ImGui::SameLine();
    if (ImGui::Button("Optimize"))
    {
        g = optimize_geometry(g, 5.0, 5.0);
        update_offset_rad_cache(g);
        modded = true;
    }
    if (modded)
    {
        recompute(g, data);
    }
    ImGui::Separator();
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
    ImGui::Separator();
    if (ImGui::BeginTabBar("Select Graph", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Tracking Error"))
        {
            uistate.graph_type = graph_type_tracking_error;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Tracking Distortion")) 
        {
            uistate.graph_type = graph_type_tracking_distortion;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Skating Force"))
        {
            uistate.graph_type = graph_type_skating_force;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::SameLine();
	if (ImGui::Button("Export Data"))
	{
		export_data(uistate, gp, datap);
	}
    switch (uistate.graph_type)
    {
    case graph_type_tracking_error:
        if(ImPlot::BeginPlot("Tracking Error", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes("radius (mm)","error (deg)");
            if(gp[0].display_)
                ImPlot::PlotLine("Tracking Error", x_axis.data(), datap[0].tracking_error_.data(), static_cast<int>(datap[0].tracking_error_.size()));
            if(gp[1].display_)
                ImPlot::PlotLine("Tracking Error", x_axis.data(), datap[1].tracking_error_.data(), static_cast<int>(datap[1].tracking_error_.size()));
            ImPlot::EndPlot();
        }
        break;
    case graph_type_tracking_distortion:
        if(ImPlot::BeginPlot("Tracking Distortion", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes("radius (mm)","distortion (%)");
            if(gp[0].display_)
		        ImPlot::PlotLine("Tracking Distortion", x_axis.data(), datap[0].tracking_distortion_.data(), static_cast<int>(datap[0].tracking_distortion_.size()));
            if(gp[1].display_)
		    ImPlot::PlotLine("Tracking Distortion", x_axis.data(), datap[1].tracking_distortion_.data(), static_cast<int>(datap[1].tracking_distortion_.size()));
            ImPlot::EndPlot();
        }
        break;
    case graph_type_skating_force:
        if(ImPlot::BeginPlot("Skating Force", ImVec2(-1, -1)))
        {
            ImPlot::SetupAxes("radius (mm)", "skating force Nmm");
            if (gp[0].display_)
                ImPlot::PlotLine("Skating force", x_axis.data(), datap[0].skating_force_.data(), static_cast<int>(datap[0].skating_force_.size()));
            if (gp[1].display_)
                ImPlot::PlotLine("Skating force", x_axis.data(), datap[1].skating_force_.data(), static_cast<int>(datap[1].skating_force_.size()));
            ImPlot::EndPlot();
        }
        break;
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
	uistate_t uistate;
	uistate.current_custom_geometry = 0;
    uistate.graph_type = graph_type_tracking_error;
	uistate.fn_ = "tracking_error_export.dat";

    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "Tracking Error Evaluator";
        runnerParams.callbacks.ShowGui = [&]() {
        draw(uistate, g, data);
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

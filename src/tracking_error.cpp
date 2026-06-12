#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#if 1
#include "implot.h"

struct geometry_t
{
    double pivot_spindle_;
    double pivot_stylus_;
    double min_radius_;
    double max_radius_;
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
    double tmp = g.pivot_spindle_;
    if(ImGui::InputDouble("Pivot - Spindle", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.pivot_spindle_ = tmp;
    }
    tmp = g.pivot_stylus_;
    if(ImGui::InputDouble("Pivot - Stylus", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.pivot_stylus_ = tmp;
    }
    tmp = g.min_radius_;
    if(ImGui::InputDouble("Min radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.min_radius_ = tmp;
    }
    tmp = g.max_radius_;
    if(ImGui::InputDouble("Max radius", &tmp, 0.1, 0.5, "%.2f"))
    {
        g.max_radius_ = tmp;
    }
    ImGui::Separator();  
	ImGui::TextUnformatted("Graph");
    ImGui::End();
}

int main()
{
    geometry_t g{211.0, 223.0, 80.0, 120.0};
    auto showGui = [&]() {draw(g);};
    HelloImGui::Run(showGui, "Implot Demo", true);

    return 0;
}

#else
int main(int, char *[]) {
    int nb_cpp = 10, nb_cmake = 2;
    auto showGui = [&]() {
        ImGui::TextWrapped("How many lines for this app that works on computers and mobile devices?");
        ImGui::SliderInt(ICON_FA_FILE_CODE " C++ lines", &nb_cpp, 0, 100);
        ImGui::InputInt( ICON_FA_FILE_CODE " Cmake lines", &nb_cmake);
    };
    HelloImGui::Run(showGui, "Hello, Dear ImGui!", true);
    return 0;
}
#endif
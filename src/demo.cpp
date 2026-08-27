#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/icons_font_awesome_4.h"

#include "implot.h"

int main()
{
    HelloImGui::RunnerParams runnerParams;
    runnerParams.appWindowParams.windowTitle = "ImGui Demo";
	bool open = true;
    runnerParams.callbacks.ShowGui = [&]() {
        ImPlot::ShowDemoWindow();
        };
    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
        HelloImGui::DefaultImGuiWindowType::ProvideFullScreenWindow;
    runnerParams.callbacks.PostInit = [] { ImPlot::CreateContext(); };
    runnerParams.callbacks.BeforeExit = [] { ImPlot::DestroyContext(); };

    HelloImGui::Run(runnerParams);
    return 0;
}

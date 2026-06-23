cmake_minimum_required (VERSION 3.22)

HELLO_IMGUI_ADD_APP(tracking_error tracking_error.cpp ${implot_SOURCE_DIR}/implot.cpp ${implot_SOURCE_DIR}/implot_items.cpp ${implot_SOURCE_DIR}/implot_demo.cpp)
target_link_libraries(tracking_error PRIVATE fmt::fmt)

add_executable(calc calc.cpp)
target_link_libraries(calc PRIVATE fmt::fmt)
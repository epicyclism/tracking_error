# AGENTS.md

## Project overview

This repository evaluates turntable tracking error for a pivoted arm. The core math lives in `src/tracking_common.h`, the GUI app is in `src/tracking_error.cpp`, and the standalone helper/console experiments are in `src/calc.cpp`.

The project is a CMake-based C++23 app that uses `hello_imgui` and `implot` via `FetchContent` on macOS and Windows, and depends on `fmt`.

## Build and run

Use the CMake presets defined in `CMakePresets.json` instead of hand-written compiler commands.

- macOS debug:
  - `cmake --preset macos-debug`
  - `cmake --build out/build/macos-debug`
  - Run the app binary from the build tree, e.g. `out/build/macos-debug/src/tracking_error`
- macOS release:
  - `cmake --preset macos-release`
  - `cmake --build out/build/macos-release`

Equivalent presets exist for Windows and Linux. If you add or change targets, keep them aligned with the `src/CMakeLists.txt` entries.

## Code conventions

- Prefer C++23 idioms and keep code compatible with the existing `constexpr` and `inline` math helpers.
- Treat `src/tracking_common.h` as the source of truth for geometry and calculations. Changes to formulas or geometry data should be made there unless the task is specifically UI-only.
- `geometry_t` holds the arm geometry, while `geometry_data_t` holds the computed arrays for plotting and analysis. `recompute()` is the central pipeline that refreshes those arrays.
- Convert between degrees and radians via `from_degrees()` and `to_degrees()` when exposing values to the UI or printing output.
- Keep GUI state changes small and explicit; the app updates graphs from the cached geometry data after a mutation.
- `calc.cpp` is a diagnostic/test harness, not the main app. It is useful for quick numerical experiments and optimization checks.

## Important files

- `src/tracking_common.h` — physics model, geometry constants, and recomputation logic
- `src/tracking_error.cpp` — main GUI app and plot rendering
- `src/calc.cpp` — numerical experiments and tuning utilities
- `src/demo.cpp` — bare `implot` demo window for UI experimentation
- `CMakeLists.txt` and `src/CMakeLists.txt` — build configuration

## Working style for agents

- Confirm the relevant target before editing build logic or source files.
- Keep changes localized. This project is compact and relies on shared math helpers rather than a deeper framework.
- When investigating behavior, prefer tracing the data from `geometry_t` into `recompute()` and into the UI plots.
- Do not introduce new dependency frameworks unless the task specifically requires it.
- If a change affects user-visible geometry or physics, validate it with the appropriate build and a quick runtime check.

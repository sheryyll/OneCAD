# Repository Guidelines

## Project Structure & Module Organization
- `src/`: C++ source code, organized by domain (`app/`, `core/`, `kernel/`, `render/`, `ui/`, `io/`). Entry point is `src/main.cpp`.
  - `src/app/`: application singleton and startup wiring (`Application`).
  - `src/render/`: camera + grid rendering utilities (`Camera3D`, `Grid3D`).
  - `src/ui/`: Qt Widgets UI (`MainWindow`, `Viewport`, `ViewCube`, `ThemeManager`, `ModelNavigator`, `PropertyInspector`, `ContextToolbar`).
  - `src/kernel/`: OCCT-related kernel utilities; `elementmap/ElementMap.h` holds topological naming.
  - `src/core/`, `src/io/`: interface libraries with placeholders for sketch/modeling and I/O.
- `resources/`: runtime assets like `icons/`, `shaders/`, and `themes/`.
- `tests/`: prototype executables under `tests/prototypes/`.
- `third_party/`: vendored dependencies (if any).
- `build/`: out-of-tree build output (local only).

## Build, Test, and Development Commands
From the repo root:
- `mkdir -p build && cd build`: create an out-of-tree build directory.
- `cmake ..`: configure with CMake (C++20, Qt6, OCCT, Eigen).
- `cmake --build .` (or `make`): build the app and any test targets.
- `./OneCAD`: run the desktop app from the build directory.
- `cmake --build . --target proto_tnaming`: build a prototype test executable.
- `./tests/proto_tnaming`: run that prototype (repeat for `proto_custom_map`).
- `cmake --build . --target proto_custom_map`: build custom map prototype.
- `cmake --build . --target proto_elementmap_rigorous`: build ElementMap rigorous prototype.
- `./tests/proto_custom_map` / `./tests/proto_elementmap_rigorous`: run prototypes from the build directory.

## Coding Style & Naming Conventions
- C++20 is required (`CMakeLists.txt` sets the standard).
- Follow existing formatting: 4-space indentation, braces on the same line.
- Classes use PascalCase (e.g., `MainWindow`), functions/variables use lower camelCase.
- Keep file pairs as `.h`/`.cpp` and match class names to filenames.

## Testing Guidelines
- There is no automated test suite wired to `ctest` yet; tests are prototype executables.
- Add new prototypes under `tests/prototypes/` and register them in `tests/CMakeLists.txt`.
- If you add real tests, also add `add_test(...)` so `ctest` works.

## Architecture & Runtime Notes
- `src/main.cpp` configures a Core Profile OpenGL 4.1 context via `QSurfaceFormat` before creating `QApplication`.
- Qt6 Widgets + `QOpenGLWidget` are the current UI/rendering path; rendering utilities live under `src/render/`.
- OpenCASCADE, Eigen, and Qt6 are linked at the top level; kernel/core/io modules are currently lightweight interface libraries.

## Key Documents
- `SPECIFICATION.md`: primary product requirements and architectural targets.
- `PHASES.md`: implementation roadmap and phase checklist.
- `SHAPR_UX.md` / `GEMINI.md`: UX and interaction research notes.
- `ELEMENT_MAP.md` / `TOPO.md`: topological naming analysis and background.
- `STEP.md`: STEP import/export research notes and OCCT/XDE guidance.

## Commit & Pull Request Guidelines
- Recent commits use Conventional Commit-style prefixes (e.g., `feat:`). Prefer `feat:`, `fix:`, or `chore:` with a short, imperative subject.
- PRs should include: a brief summary, build instructions or output, and screenshots for UI changes.
- Link related issues or specs when applicable (e.g., `SPECIFICATION.md`).

## Configuration Tips
- Qt6 is expected at `/opt/homebrew/opt/qt` or via `CMAKE_PREFIX_PATH`.
- OpenCASCADE and Eigen are expected from Homebrew on macOS.

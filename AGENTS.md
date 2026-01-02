# Repository Guidelines

## Project Structure & Module Organization
- `src/`: C++ source code, organized by domain (`app/`, `core/`, `kernel/`, `render/`, `ui/`, `io/`). Entry point is `src/main.cpp`.
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

## Coding Style & Naming Conventions
- C++20 is required (`CMakeLists.txt` sets the standard).
- Follow existing formatting: 4-space indentation, braces on the same line.
- Classes use PascalCase (e.g., `MainWindow`), functions/variables use lower camelCase.
- Keep file pairs as `.h`/`.cpp` and match class names to filenames.

## Testing Guidelines
- There is no automated test suite wired to `ctest` yet; tests are prototype executables.
- Add new prototypes under `tests/prototypes/` and register them in `tests/CMakeLists.txt`.
- If you add real tests, also add `add_test(...)` so `ctest` works.

## Commit & Pull Request Guidelines
- Recent commits use Conventional Commit-style prefixes (e.g., `feat:`). Prefer `feat:`, `fix:`, or `chore:` with a short, imperative subject.
- PRs should include: a brief summary, build instructions or output, and screenshots for UI changes.
- Link related issues or specs when applicable (e.g., `SPECIFICATION.md`).

## Configuration Tips
- Qt6 is expected at `/opt/homebrew/opt/qt` or via `CMAKE_PREFIX_PATH`.
- OpenCASCADE and Eigen are expected from Homebrew on macOS.

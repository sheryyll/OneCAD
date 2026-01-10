# OneCAD Context

## Project Overview
OneCAD is a free, open-source 3D CAD application designed for makers and hobbyists. It aims to provide an intuitive "sketch-first" workflow similar to Shapr3D, built on a robust, industry-standard technology stack.

**Core Technologies:**
- **Language:** C++20
- **Build System:** CMake (3.25+)
- **UI Framework:** Qt 6 (Widgets + OpenGL)
- **Geometry Kernel:** OpenCASCADE Technology (OCCT)
- **Math:** Eigen3
- **Constraint Solver:** PlaneGCS (integrated static lib)
- **File Format:** Custom ZIP-based `.onecad` with JSON + BREP

## Directory Structure
- `src/app/`: Application layer.
    - `commands/`: Command pattern implementation (Undo/Redo).
    - `document/`: Document model, OperationRecord.
    - `history/`: Parametric history, DependencyGraph, RegenerationEngine.
- `src/core/`: Core CAD logic.
    - `sketch/`: Sketch entities, SolverAdapter, PlaneGCS wrapper.
    - `loop/`: LoopDetector, FaceBuilder (Sketch regions to Faces).
    - `modeling/`: Solid operations (Extrude, Revolve, Boolean).
- `src/kernel/`: Geometry kernel wrappers and the **ElementMap** topological naming system.
- `src/render/`: Qt RHI rendering pipeline (Metal on macOS), Camera3D, Grid3D.
- `src/ui/`: Qt Widgets user interface.
    - `mainwindow/`: Main window layout.
    - `viewport/`: OpenGL viewport, RenderDebugPanel.
    - `tools/`: Sketch and Modeling tool implementations.
    - `history/`: History panel, feature parameters.
    - `navigator/`: Scene graph, visibility/selection control.
    - `start/`: Project browser overlay.
- `src/io/`: Import/Export layer.
    - Native `.onecad` (ZipPackage) and directory-based `.onecadpkg`.
    - JSON serialization (Sketches, Manifest, History).
    - STEP Import/Export.
- `tests/`:
    - `prototypes/`: Isolated test executables for core systems.
- `docs/`: Project documentation and research.

## Build & Run
The project uses a standard CMake out-of-source build workflow.

### Prerequisites (macOS)
- Xcode Command Line Tools
- Homebrew
- Dependencies: `cmake`, `qt`, `opencascade`, `eigen`
- **PlaneGCS**: Included as a static library in `third_party/planegcs`.

### Commands
```bash
# 1. Create build directory
mkdir build && cd build

# 2. Configure (ensure Qt6 path is found)
cmake ..

# 3. Build
make -j$(sysctl -n hw.ncpu)

# 4. Run Application
./OneCAD

# 5. Run Prototypes (Examples)
cmake --build . --target proto_tnaming
./tests/prototypes/proto_tnaming

cmake --build . --target proto_sketch_solver
./tests/prototypes/proto_sketch_solver
```

## Development Conventions

### Coding Style
- **Standard:** C++20
- **Formatting:** 4-space indentation, braces on same line (K&R/1TBS variant).
- **Naming:**
    - Classes: `PascalCase` (e.g., `MainWindow`)
    - Methods/Variables: `camelCase` (e.g., `registerElement`)
    - Constants: `kPascalCase` or `UPPER_CASE`
    - Namespaces: `snake_case` (e.g., `onecad::kernel::elementmap`)
- **File Structure:** Class names match filenames (`MyClass.h` / `MyClass.cpp`).

### Architecture Notes
1.  **ElementMap (Topological Naming):**
    - Located in `src/kernel/elementmap/ElementMap.h`.
    - **CRITICAL:** Tracks topology (Faces, Edges, Vertices) through boolean operations using persistent UUIDs and geometric descriptors.
    - Foundation for parametric modeling. **Do not modify without understanding descriptor matching.**

2.  **Sketch Engine & Solver:**
    - Uses **PlaneGCS** for geometric constraint solving.
    - `Sketch` entities (Lines, Arcs) are mapped to GCS objects via `SolverAdapter`.
    - `LoopDetector` (graph-based DFS) converts sketch curves into closed regions (`TopoDS_Face`) for extrusion.

3.  **Parametric History:**
    - Operations are recorded in `OperationRecord`.
    - `DependencyGraph` manages rebuild order.
    - `RegenerationEngine` replays operations to rebuild the model from the base cache.

4.  **Application Singleton:**
    - `onecad::app::Application` manages lifecycle, global state, and the active `Document`.

## Current Status (as of Jan 2026)
- **Phase 1 (Foundation):** COMPLETE. ElementMap, App Shell, Rendering.
- **Phase 2 (Sketching):** COMPLETE. Full constraint solver, all sketch tools, loop detection.
- **Phase 3 (Solid Modeling):** IN PROGRESS (~90%).
    - **Done:** I/O (Save/Load/STEP), Modeling Ops (Extrude, Revolve, Boolean, Fillet, Chamfer, Shell).
    - **In Progress:** Parametric history replay (UI polish needed).
    - **Missing:** Pattern operations.

## Documentation
- `docs/SPECIFICATION.md`: Single Source of Truth for features and UX.
- `docs/PHASES.md`: Detailed implementation roadmap and status.
- `AGENTS.md`: Guidelines for AI agents.
- `docs/ELEMENT_MAP.md`: Details on the topological naming system.
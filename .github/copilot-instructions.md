# OneCAD AI Agent Instructions

## Project Identity
C++ CAD application (C++20) for makers and hobbyists. Architecture: Qt6 + OpenCASCADE (OCCT) + Eigen3 + PlaneGCS solver. Currently Phase 2 complete (full sketching engine). Target: macOS 14+ (Apple Silicon).

## Build & Test Commands
```bash
# From repo root
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt  # macOS Homebrew path
cmake --build .
./OneCAD  # Run app

# Prototypes (unit-test-like executables)
cmake --build . --target proto_tnaming  # Build specific prototype
./tests/proto_tnaming                    # Run from build dir
```

**Dependencies**: Qt6 (`/opt/homebrew/opt/qt`), OCCT, Eigen3 from Homebrew. No `ctest` suite yet; tests are prototype executables in `tests/prototypes/`.

## Critical Architecture Knowledge

### Module Organization
- `src/app/`: Application singleton (`Application.h`), lifecycle management
- `src/core/sketch/`: **2600+ LOC** sketch engine (entities, tools, solver, constraints)
- `src/kernel/elementmap/`: **Topological naming system** (ElementMap.h, 956 LOC) — persistent geometry IDs via descriptor hashing
- `src/render/`: Camera3D, Grid3D, SketchRenderer (OpenGL 4.1 Core)
- `src/ui/`: Qt Widgets (MainWindow, Viewport, ContextToolbar, ViewCube)
- `third_party/planegcs/`: Constraint solver (42MB static lib)

### Non-Standard Coordinate System ⚠️
**Sketch XY plane mapping** (see `SketchPlane::XY()` in `src/core/sketch/Sketch.h:64`):
- Sketch X → World Y+ (0,1,0)
- Sketch Y → World X- (-1,0,0)
- Normal → World Z+ (0,0,1)

This is **intentional** for viewport alignment (Front = World X+, Right = World Y+). Do not "fix" this.

### ElementMap: Topological Naming (REGRESSION-SENSITIVE)
`src/kernel/elementmap/ElementMap.h` provides persistent IDs for geometry across operations:
- Uses **14-field descriptor hashing** (geometry type, center, normals, adjacency, quantized coords)
- **Hash ordering changes** can remap all IDs → validate with prototype tests before merging
- Tracks OCCT operation history (Modified/Generated/Deleted)
- See `tests/prototypes/proto_elementmap_rigorous.cpp` for validation patterns

**Pattern**: Always null-check `Handle<>` objects before dereferencing OCCT shapes.

### PlaneGCS Solver Integration
- **1450 LOC** production implementation in `src/core/sketch/solver/ConstraintSolver.cpp`
- **Direct parameter binding**: Solver uses raw pointers to entity coordinates (zero-copy)
- **15 constraint types** (Horizontal, Vertical, Distance, Angle, Tangent, etc.) in `constraints/Constraints.h`
- **DogLeg algorithm** (default) with LM/BFGS fallback
- Backup/restore for failed solves, redundancy detection

### Sketch Tools Pattern
- Each tool inherits from `SketchTool` (see `src/core/sketch/tools/`)
- Lifecycle: `handleMousePress/Move/Release/DoubleClick`
- Managed by `SketchToolManager` in Viewport
- Examples: LineTool (322 LOC), ArcTool (369 LOC), MirrorTool (444 LOC)

### SnapManager (2mm Radius)
- **Zoom-independent** 2mm snap radius in sketch coordinates
- Priority: Vertex > Endpoint > Midpoint > Center > Quadrant > Intersection > OnCurve > Grid
- Always check `snapped` flag before using `SnapResult.position`

### AutoConstrainer (±5° Tolerance)
- Infers constraints during drawing (Horizontal/Vertical ±5°, Coincidence 2mm)
- Returns ghost constraints (50% opacity) before commit
- 7 inference rules in `src/core/sketch/AutoConstrainer.cpp` (1091 LOC)

## Coding Conventions
- **C++20 required** (`CMakeLists.txt` enforces)
- **PascalCase** classes (`MainWindow`), **camelCase** functions/vars
- **4-space indent**, braces on same line
- **EntityID = std::string (UUID)** for all sketch entities
- **Non-copyable, movable** entities (deleted copy constructors)
- File pairs: `.h`/`.cpp` matching class names

## Qt-Specific Patterns
- **OpenGL context setup**: `src/main.cpp` sets Core Profile 4.1 **before** `QApplication` creation (critical)
- **Signals**: Queued for cross-thread, Direct for same-thread
- **Parent ownership**: Ensure proper parent for memory management

## Key Documents
- `docs/SPECIFICATION.md`: Product requirements (3700+ lines), architecture targets
- `docs/PHASES.md`: Roadmap (Phase 2 complete, Phase 3 pending)
- `docs/ELEMENT_MAP.md`: Topological naming deep dive
- `AGENTS.md`: Original agent guidelines (this file supersedes for GitHub Copilot)

## Testing Strategy
- **Prototype executables** under `tests/prototypes/` (no `ctest` yet)
- Add new prototypes to `tests/CMakeLists.txt`
- ElementMap changes require rigorous validation (see `proto_elementmap_rigorous`)

## Known Issues / Deviations
- Grid3D: **Fixed 10mm spacing** despite header claiming "adaptive" (SPEC deviation)
- Camera3D: Missing inertia physics, sticky pivot
- DOF visualization: Calculation works, full UI color-coding pending

## Commit Style
Conventional Commits: `feat:`, `fix:`, `chore:` with imperative subject. PRs should include build output and screenshots for UI changes.

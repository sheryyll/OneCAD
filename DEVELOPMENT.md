# OneCAD Development Guide

> **For Contributors & Developers**: Comprehensive guide to understanding, building, and extending the OneCAD codebase.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Project Overview](#project-overview)
3. [Architecture](#architecture)
4. [Directory Structure](#directory-structure)
5. [Environment Setup](#environment-setup)
6. [Building & Running](#building--running)
7. [Testing Strategy](#testing-strategy)
8. [Code Best Practices](#code-best-practices)
9. [Understanding Key Systems](#understanding-key-systems)
10. [Common Development Tasks](#common-development-tasks)
11. [Debugging Guide](#debugging-guide)
12. [Performance Considerations](#performance-considerations)
13. [Important Gotchas & Patterns](#important-gotchas--patterns)

---

## Quick Start

### For the Impatient (5 minutes)

```bash
# 1. Clone and navigate
cd /path/to/OneCAD

# 2. Install dependencies (macOS)
brew install cmake eigen opencascade qt6

# 3. Build
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt
cmake --build .

# 4. Run
./OneCAD
```

### For the Curious (10 minutes)

1. Read the [Architecture](#architecture) section below
2. Check [Understanding Key Systems](#understanding-key-systems)
3. Explore the codebase: `src/main.cpp` → `src/app/Application.h` → `src/ui/mainwindow/MainWindow.h`
4. Run a prototype: `cmake --build . --target proto_sketch_geometry && ./tests/proto_sketch_geometry`

---

## Project Overview

### What is OneCAD?

**OneCAD** is a free, open-source 3D CAD application for makers and hobbyists. It combines:
- **Sketching engine** (2D parametric drawing with constraints)
- **3D modeling** (extrude, revolve, boolean operations)
- **Direct modeling** (push/pull faces interactively)
- **Pattern tools** (linear & circular arrays)

### Technology Stack

| Component | Purpose | Version |
|-----------|---------|---------|
| **C++** | Core language | C++20 |
| **Qt6** | GUI framework | 6.0+ |
| **OpenCASCADE (OCCT)** | Geometric modeling kernel | 7.5+ |
| **Eigen3** | Linear algebra | 3.3+ |
| **PlaneGCS** | Constraint solver | 42MB static lib |
| **CMake** | Build system | 3.25+ |

### Current Development Status

- ✅ **Phase 2 Complete**: Full sketching engine with constraints, tools, and solver integration
- 🚀 **Phase 3**: 3D modeling operations (extrude, revolve, etc.)
- 🎯 **Target Platform**: macOS 14+ (Apple Silicon)

See [docs/PHASES.md](docs/PHASES.md) and [docs/SPECIFICATION.md](docs/SPECIFICATION.md) for detailed roadmap.

---

## Architecture

### High-Level System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      MainWindow (Qt6)                        │
│  ┌────────────────┬──────────────────┬─────────────────┐   │
│  │  ModelNavigator│  Viewport (3D)   │ PropertyPanel   │   │
│  │  (Left Dock)   │  (OpenGL)        │ (Right Dock)    │   │
│  └────────────────┴──────────────────┴─────────────────┘   │
└─────────────────────────────────────────────────────────────┘
              ↓         ↓         ↓         ↓
┌─────────────────────────────────────────────────────────────┐
│              Application (Singleton)                         │
│  - Document lifecycle                                       │
│  - Global state management                                  │
│  - Subsystem coordination                                   │
└─────────────────────────────────────────────────────────────┘
              ↓         ↓         ↓         ↓
┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐
│  Sketch Engine   │  │ Rendering System │  │ ElementMap   │
│  (2D geometry,   │  │ (Camera3D,       │  │ (Topological │
│  Constraints,    │  │  SketchRenderer, │  │  Naming)     │
│  Solver)         │  │  Grid3D)         │  │              │
└──────────────────┘  └──────────────────┘  └──────────────┘
              ↓
┌──────────────────────────────────────────────────────────────┐
│ OpenCASCADE (OCCT) + PlaneGCS Constraint Solver              │
└──────────────────────────────────────────────────────────────┘
```

### Module Responsibilities

#### 1. **src/app/** - Application Singleton

**Files**: `Application.h/cpp`

**Responsibility**: 
- Manages application lifecycle (initialization, shutdown)
- Holds global state and singleton instances
- Coordinates between subsystems
- Manages document lifecycle

**Key Classes**:
- `Application`: Main app controller

**When to use**: Access from any subsystem that needs global context.

```cpp
auto& app = Application::instance();
```

#### 2. **src/ui/** - User Interface Layer

**Directories**: `mainwindow/`, `viewport/`, `components/`, `toolbar/`, `panels/`, `theme/`, `viewcube/`

**Responsibility**:
- Qt Widgets-based UI rendering
- Event handling (mouse, keyboard)
- View management (main window, docks, panels)
- Theme management

**Key Classes**:
- `MainWindow`: Top-level window, layout management
- `Viewport`: 3D OpenGL viewport, sketch interactions
- `SketchToolManager`: Manages sketch tools lifecycle
- `ContextToolbar`: Contextual tool suggestions
- `ModelNavigator`: Document tree view
- `PropertyInspector`: Property panel for selected objects
- `ThemeManager`: Light/dark theme handling

**When to use**: Any user-facing feature, input handling, visual feedback.

#### 3. **src/core/sketch/** - Sketch Engine (2600+ LOC)

**Directories**: `constraints/`, `solver/`, `tools/`

**Responsibility**:
- 2D parametric sketch geometry (points, lines, arcs, circles, ellipses)
- Constraint system (15 constraint types)
- Constraint solver integration (PlaneGCS)
- Tool implementations (draw, delete, mirror, trim, etc.)
- Snap/intersection detection
- Auto-constraining (intelligent constraint inference)

**Key Classes**:
- `Sketch`: Main sketch container
- `SketchEntity`: Base class for geometry
- `SketchPoint`, `SketchLine`, `SketchArc`, `SketchCircle`, `SketchEllipse`: Geometry types
- `SketchConstraint`: Base constraint class
- `ConstraintSolver`: Solver adapter (1450 LOC, directly binds to PlaneGCS)
- `SketchTool`: Base class for tools (`LineTool`, `ArcTool`, `RectangleTool`, `MirrorTool`, `TrimTool`, etc.)
- `SnapManager`: Snap detection (2mm radius, 9 priority levels)
- `AutoConstrainer`: Intelligent constraint inference (7 rules, ±5° tolerance)
- `SketchRenderer`: OpenGL rendering of sketch (450+ LOC)

**When to use**: Any sketch-related operation (geometry, constraints, drawing).

#### 4. **src/kernel/** - Low-Level Kernel

**Directories**: `elementmap/`, `shape/`, `solver/`

**Responsibility**:
- OpenCASCADE integration utilities
- **Topological naming** (persistent geometry IDs across operations)
- Shape utilities
- Element mapping

**Key Classes**:
- `ElementMap`: Topological naming system (956 LOC)
  - 14-field descriptor hashing
  - Tracks OCCT operation history (Modified/Generated/Deleted)
  - Regression-sensitive: hash ordering changes remap all IDs

**When to use**: Geometric operations requiring persistent ID tracking.

#### 5. **src/render/** - Rendering Utilities

**Files**: `Camera3D.h/cpp`, `Grid3D.h/cpp`

**Responsibility**:
- 3D camera management (perspective, view transformations)
- Viewport grid rendering (fixed 10mm spacing)
- Rendering math utilities

**Key Classes**:
- `Camera3D`: Perspective camera with zoom/pan/rotate
- `Grid3D`: XY plane grid visualization

**When to use**: Viewport transformations, camera operations.

#### 6. **src/io/** - File I/O

**Directories**: `native/`, `step/`

**Responsibility**:
- File I/O operations (STEP, native format)
- Document serialization/deserialization

**When to use**: File operations and import/export.

---

## Directory Structure

```
OneCAD/
├── src/                          # Main source code
│   ├── main.cpp                  # Entry point (OpenGL setup critical)
│   ├── app/
│   │   ├── Application.h/cpp     # App singleton
│   │   ├── commands/             # Command/undo-redo system
│   │   ├── document/             # Document management
│   │   └── tools/                # High-level tool definitions
│   ├── core/
│   │   ├── sketch/               # 2600+ LOC sketch engine ⭐
│   │   │   ├── Sketch.h/cpp
│   │   │   ├── SketchEntity.h/cpp
│   │   │   ├── SketchPoint.h/cpp, SketchLine.h/cpp, SketchArc.h/cpp, etc.
│   │   │   ├── SketchConstraint.h/cpp
│   │   │   ├── SketchRenderer.h/cpp   # OpenGL rendering
│   │   │   ├── constraints/      # Constraint type definitions
│   │   │   ├── solver/           # Constraint solver integration
│   │   │   ├── tools/            # Sketch tools (LineTool, ArcTool, etc.)
│   │   │   ├── SnapManager.h/cpp
│   │   │   ├── IntersectionManager.h/cpp
│   │   │   └── AutoConstrainer.h/cpp
│   │   ├── loop/                 # Loop detection (face boundaries)
│   │   ├── modeling/             # 3D operations (extrude, revolve, etc.)
│   │   └── pattern/              # Pattern tools
│   ├── kernel/
│   │   ├── elementmap/
│   │   │   └── ElementMap.h      # Topological naming (956 LOC) ⚠️ REGRESSION-SENSITIVE
│   │   ├── shape/                # OCCT shape utilities
│   │   └── solver/               # Solver integration
│   ├── render/
│   │   ├── Camera3D.h/cpp
│   │   ├── Grid3D.h/cpp
│   │   ├── viewport/             # Viewport rendering
│   │   ├── visual/               # Visual effects
│   │   └── tessellation/         # Mesh generation
│   ├── ui/
│   │   ├── mainwindow/
│   │   │   └── MainWindow.h/cpp  # Top-level window
│   │   ├── viewport/
│   │   │   └── Viewport.h/cpp    # 3D OpenGL widget
│   │   ├── components/           # Reusable UI components
│   │   ├── toolbar/              # Toolbars
│   │   ├── panels/               # Side panels
│   │   ├── theme/                # Theme system
│   │   └── viewcube/             # ViewCube widget
│   └── CMakeLists.txt
├── tests/                        # Testing (prototype executables)
│   ├── CMakeLists.txt
│   ├── prototypes/
│   │   ├── proto_sketch_geometry.cpp
│   │   ├── proto_sketch_constraints.cpp
│   │   ├── proto_sketch_solver.cpp
│   │   ├── proto_elementmap_rigorous.cpp
│   │   ├── proto_planegcs_integration.cpp
│   │   └── ... (more prototypes)
│   └── test_compile.cpp
├── third_party/
│   └── planegcs/                 # PlaneGCS constraint solver (42MB)
├── resources/
│   ├── resources.qrc
│   ├── icons/                    # SVG icons
│   ├── shaders/                  # GLSL shaders (OpenGL 4.1 Core)
│   └── themes/                   # Theme files
├── docs/
│   ├── SPECIFICATION.md          # Full product specification (3700+ lines)
│   ├── PHASES.md                 # Implementation roadmap
│   ├── ELEMENT_MAP.md            # Topological naming deep dive
│   └── researches/               # Research documents
├── CMakeLists.txt                # Top-level CMake config
├── README.md                     # Project overview & quick links
├── DEVELOPMENT.md                # This file
└── LICENSE                       # MIT or Apache 2.0
```

---

## Environment Setup

### Prerequisites

#### macOS (Homebrew)

```bash
# 1. Install Xcode Command Line Tools
xcode-select --install

# 2. Verify brew
brew --version

# 3. Install dependencies
brew install cmake eigen opencascade qt6
```

#### Verify Installation

```bash
# Check versions
cmake --version          # Should be 3.25+
clang++ --version        # Should be recent
pkg-config --modversion eigen3
```

### Configuration

#### Qt6 Path (Critical for macOS)

CMake expects Qt6 at `/opt/homebrew/opt/qt` (default Homebrew location). If Qt6 is installed elsewhere, set:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
```

#### OpenGL 4.1 Core Profile

**Critical**: `src/main.cpp` sets OpenGL 4.1 Core Profile **before** `QApplication` creation. This is required for all shaders and rendering. Do not change this without updating all shaders.

```cpp
QSurfaceFormat format;
format.setVersion(4, 1);
format.setProfile(QSurfaceFormat::CoreProfile);
QSurfaceFormat::setDefaultFormat(format);  // Must be BEFORE QApplication
```

---

## Building & Running

### Standard Build

```bash
# Create out-of-tree build directory
mkdir -p build
cd build

# Configure (macOS)
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt

# Build
cmake --build .

# Run
./OneCAD
```

### Build Options

```bash
# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug build (with symbols)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Verbose build (see compiler commands)
cmake --build . --verbose
```

### Build Individual Targets

```bash
# Build only the app
cmake --build . --target OneCAD

# Build specific prototype
cmake --build . --target proto_sketch_geometry
```

### Run Prototypes (Unit Tests)

Prototypes are executable-style tests for specific subsystems. They do not use `ctest`; they're direct executables.

```bash
# Sketch geometry prototype
./tests/proto_sketch_geometry

# Constraint system prototype
./tests/proto_sketch_constraints

# Solver integration prototype
./tests/proto_sketch_solver

# ElementMap rigorous validation (CRITICAL for topological naming changes)
./tests/proto_elementmap_rigorous

# PlaneGCS integration prototype
./tests/proto_planegcs_integration
```

### Clean Build

```bash
rm -rf build
mkdir -p build && cd build
cmake ..
cmake --build .
```

---

## Testing Strategy

### Current Approach: Prototype Executables

OneCAD uses **prototype executables** instead of a traditional `ctest` suite. These are located in `tests/prototypes/` and registered in `tests/CMakeLists.txt`.

**Why?**
- Prototypes allow deep testing of specific subsystems (geometry, constraints, solver)
- Each prototype can be run independently and quickly
- Easier to debug with direct execution

### Running Tests

```bash
cd build

# Build all prototypes
cmake --build .

# Run individual test
./tests/proto_sketch_geometry
./tests/proto_sketch_constraints
./tests/proto_sketch_solver
./tests/proto_elementmap_rigorous
./tests/proto_planegcs_integration
./tests/proto_tnaming
./tests/proto_custom_map
./tests/proto_face_builder
./tests/proto_loop_detector
```

### Creating New Tests

1. Create a new `.cpp` file in `tests/prototypes/`
2. Register it in `tests/CMakeLists.txt`:

```cmake
add_executable(proto_my_feature prototypes/proto_my_feature.cpp)
target_link_libraries(proto_my_feature PRIVATE onecad_core)
```

3. Build and run:

```bash
cmake --build . --target proto_my_feature
./tests/proto_my_feature
```

### ElementMap Regression Testing ⚠️

**Critical**: Changes to ElementMap require running `proto_elementmap_rigorous` to validate that descriptor hash ordering hasn't changed. Hash ordering changes will remap all geometry IDs.

```bash
./tests/proto_elementmap_rigorous
```

Check output for failures before committing.

---

## Code Best Practices

### C++20 Standards

- **Standard**: C++20 is required (enforced by CMakeLists.txt)
- **Prefer**: Modern C++ idioms (smart pointers, structured bindings, ranges)
- **Avoid**: Raw `new`/`delete`, `malloc`, C-style arrays

```cpp
// ✅ Good: Smart pointers
auto entity = std::make_unique<SketchPoint>(x, y);

// ❌ Bad: Raw pointers
SketchPoint* entity = new SketchPoint(x, y);
```

### Naming Conventions

| Item | Style | Example |
|------|-------|---------|
| Classes | PascalCase | `SketchEntity`, `MainWindow`, `LineTool` |
| Functions | camelCase | `addEntity()`, `handleMousePress()` |
| Variables | camelCase | `entityId`, `snapRadius` |
| Constants | UPPER_SNAKE_CASE | `MAX_ITERATIONS`, `DEFAULT_SNAP_RADIUS` |
| Member vars (private) | m_camelCase | `m_entities`, `m_solver` |
| Enums | PascalCase (values UPPER_SNAKE_CASE) | `enum class ToolType { DRAW_LINE, DRAW_ARC }` |

```cpp
// ✅ Good
class SketchLine {
private:
    std::vector<SketchEntity> m_entities;
    
public:
    void addEntity(const SketchEntity& entity) {
        constexpr double DEFAULT_TOLERANCE = 1e-6;
        // ...
    }
};

// ❌ Bad
class Sketch_Line {
private:
    vector<SketchEntity> entities;
    
public:
    void AddEntity(const SketchEntity& entity) {
        // ...
    }
};
```

### File Organization

- **One class per file** (with rare exceptions for tightly coupled classes)
- **File name matches class name** (e.g., `SketchLine.h` for `class SketchLine`)
- **Header + implementation pair**: `SketchLine.h` / `SketchLine.cpp`
- **Public interface in header**, implementation in `.cpp`

```cpp
// SketchLine.h
#ifndef ONECAD_CORE_SKETCH_SKETCHLINE_H
#define ONECAD_CORE_SKETCH_SKETCHLINE_H

#include "SketchEntity.h"

namespace onecad::core::sketch {

class SketchLine : public SketchEntity {
public:
    SketchLine(double x1, double y1, double x2, double y2);
    ~SketchLine() override;
    
    void update() override;
    
private:
    double m_x1, m_y1, m_x2, m_y2;
};

} // namespace

#endif
```

### Memory Management

**Rule**: Use smart pointers exclusively. No raw `new`/`delete`.

```cpp
// ✅ Good
class Sketch {
private:
    std::vector<std::unique_ptr<SketchEntity>> m_entities;
    std::shared_ptr<ConstraintSolver> m_solver;
};

// ❌ Bad
class Sketch {
private:
    std::vector<SketchEntity*> m_entities;  // Raw pointers
    ConstraintSolver* m_solver;
};
```

### Entity Lifecycle

All sketch entities are **non-copyable, movable**:

```cpp
class SketchEntity {
public:
    SketchEntity(const SketchEntity&) = delete;
    SketchEntity& operator=(const SketchEntity&) = delete;
    
    SketchEntity(SketchEntity&&) = default;
    SketchEntity& operator=(SketchEntity&&) = default;
};
```

### Qt Patterns

#### Signals & Slots

- Use **Queued connections** for cross-thread signals
- Use **Direct connections** for same-thread signals (performance)

```cpp
// ✅ Good: Cross-thread signal
connect(solver, &ConstraintSolver::solved, this, &SketchRenderer::update, Qt::QueuedConnection);

// ✅ Good: Same-thread signal
connect(sketch, &Sketch::entityAdded, this, &MainWindow::onEntityAdded, Qt::DirectConnection);
```

#### Parent Ownership

Always ensure proper Qt parent hierarchy for memory management:

```cpp
// ✅ Good: MainWindow owns Viewport
auto viewport = new Viewport(this);  // 'this' is parent, auto-deleted

// ❌ Bad: Missing parent
auto viewport = new Viewport();      // Manual deletion required
```

#### OpenGL Context

OpenGL operations must happen in the correct context:

```cpp
// ✅ Good: Called from QOpenGLWidget::paintGL() (context is active)
void SketchRenderer::render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // ... OpenGL calls
}

// ❌ Bad: Called outside rendering context
void SomeClass::someMethod() {
    glClear(GL_COLOR_BUFFER_BIT);  // Context may not be active
}
```

### Error Handling

Use exceptions for **expected errors**, assertions for **logic errors**:

```cpp
// ✅ Good: Expected error (malformed input)
if (radius < 0) {
    throw std::invalid_argument("Radius must be positive");
}

// ✅ Good: Logic error (should never happen)
Q_ASSERT(entity != nullptr);

// ❌ Bad: Silent failure
if (radius < 0) {
    // Just continue silently
}
```

### Documentation

- **Document public APIs**: Use Doxygen comments on headers
- **Document assumptions**: Non-standard coordinate systems, regression-sensitive code
- **Document edge cases**: Null checks, special values

```cpp
/**
 * @brief Create a circle constraint.
 * 
 * @param center The center point (must be in sketch plane)
 * @param radius The radius value (must be positive)
 * @return Constraint ID on success, empty string on failure
 * 
 * @note This function is NOT thread-safe. Call from main thread only.
 * @warning REGRESSION-SENSITIVE: Constraint ordering affects solver convergence
 */
std::string addCircleConstraint(const SketchPoint& center, double radius);
```

### Code Formatting

- **Indentation**: 4 spaces (NOT tabs)
- **Braces**: Same line as declaration
- **Line length**: Aim for <100 chars, break long lines logically
- **Spacing**: Spaces around operators, no space before function calls

```cpp
// ✅ Good
if (x > 0 && y > 0) {
    entity->update(x, y);
}

// ❌ Bad
if(x>0&&y>0){
    entity -> update(x,y);
}
```

---

## Understanding Key Systems

### 1. Sketch Engine (2600+ LOC)

**The Sketch** is the heart of OneCAD. It manages 2D parametric geometry with constraints.

#### Core Concept: Entities + Constraints

```
Sketch
├── Entities (geometry)
│   ├── SketchPoint (1 entity = 2 DOF: x, y)
│   ├── SketchLine (2 entities = 4 DOF: p1.x, p1.y, p2.x, p2.y)
│   ├── SketchArc (center, start, end = 5 DOF)
│   ├── SketchCircle (center, radius = 3 DOF)
│   └── SketchEllipse (center, major, minor axes = 5 DOF)
│
└── Constraints (reduce DOF)
    ├── Geometric (Horizontal, Vertical, Tangent, Coincident, Perpendicular, Parallel, etc.)
    └── Dimensional (Distance, Angle, Radius, etc.)
```

Each entity has a unique **EntityID** (UUID string):

```cpp
std::string entityId = sketch->addPoint(10.0, 20.0);  // Returns UUID
auto point = sketch->getEntity(entityId);
```

#### Constraint Solving Pipeline

1. **User draws geometry** (LineTool, ArcTool, etc.)
2. **AutoConstrainer infers constraints** (Horizontal ±5°, Vertical ±5°, Coincident 2mm)
3. **Ghost constraints displayed** (50% opacity) until committed
4. **User adds explicit constraints** (Distance, Angle, etc.)
5. **ConstraintSolver solves** (PlaneGCS DogLeg algorithm)
6. **Entity coordinates updated** (zero-copy: solver writes directly to entity pointers)
7. **Renderer displays result**

#### Key Pattern: Direct Binding to Solver

The ConstraintSolver uses **direct parameter binding** — it holds raw pointers to entity coordinates and updates them directly:

```cpp
// In ConstraintSolver::setup()
double* xPtr = &entity.m_x;  // Solver will write directly here
double* yPtr = &entity.m_y;
solver->addVariable(xPtr);
solver->addVariable(yPtr);

// After solve()
// entity.m_x, entity.m_y are automatically updated (zero-copy)
```

**Always consider**: Changing entity data structures may break solver binding.

#### Snap Manager (2mm Radius)

Snap detection is **zoom-independent** (2mm in sketch coordinates):

```cpp
// Priority order (most specific first):
enum SnapType {
    Vertex,           // Sketch point
    Endpoint,         // Line/arc endpoint
    Midpoint,         // Line/arc midpoint
    Center,           // Circle/arc center
    Quadrant,         // Circle quadrant (0°, 90°, 180°, 270°)
    Intersection,     // Curve intersections
    OnCurve,          // Closest point on curve
    Grid              // Grid point
};
```

**Always check `snapped` flag** before using SnapResult:

```cpp
SnapResult result = snapManager.snap(mousePos);
if (result.snapped) {
    // Use result.position
} else {
    // Use mousePos directly
}
```

#### Auto-Constrainer (±5° Tolerance)

7 inference rules automatically suggest constraints during drawing:

```cpp
// If line is drawn at ±5° from horizontal:
→ Suggest Horizontal constraint

// If line is drawn at ±5° from vertical:
→ Suggest Vertical constraint

// If point is within 2mm of another point:
→ Suggest Coincident constraint

// If lines are drawn perpendicular (±5°):
→ Suggest Perpendicular constraint

// ... and 3 more rules
```

Ghost constraints (50% opacity) preview the suggestion. User commits with Enter/Space or manually adds constraints.

### 2. Topological Naming (ElementMap)

**Problem**: OCCT operations (Boolean, Extrude) create new geometry. How do we track "which face is the extruded top face"?

**Solution**: ElementMap uses **14-field descriptor hashing** to create persistent IDs:

```cpp
struct ElementDescriptor {
    TopAbs_ShapeEnum shapeType;          // Face, Edge, Vertex
    gp_Pnt center;                        // Bounding box center
    double size;                          // Bounding box diagonal
    gp_Pnt normalDirectionVector[3];      // Surface normal (if face)
    gp_Pnt adjacency[4];                  // Nearby geometry
    uint8_t quantizedCoords[12];          // Quantized position
};
```

Each field is hashed in order. **Hash ordering is critical**: changing field order remap all IDs → regression.

**Always validate with `proto_elementmap_rigorous`** before committing ElementMap changes.

```bash
./tests/proto_elementmap_rigorous
# Output should show no hash mismatches
```

### 3. Constraint Solver (1450 LOC)

**Location**: `src/core/sketch/solver/ConstraintSolver.cpp`

**Integration**: Wraps PlaneGCS (external solver library)

**15 Constraint Types Supported**:
- Geometric: Horizontal, Vertical, Tangent, Coincident, Perpendicular, Parallel, Equal, SymmetricX, SymmetricY, SymmetricZ, PointOnObject
- Dimensional: Distance, Angle, Radius, Diameter

**Solving Algorithm** (DogLeg with fallback):
1. **Primary**: DogLeg algorithm (Newton variant for quadratic convergence)
2. **Fallback**: Levenberg-Marquardt (LM)
3. **Fallback**: BFGS (Broyden–Fletcher–Goldfarb–Shanno)

**Error Recovery**:
- Backup sketch state before solving
- Restore if solve fails
- Report redundancy detection

**Direct Parameter Binding** (Zero-Copy):
```cpp
// Entity coordinates are passed as raw pointers
// Solver updates them directly in-place
solver->addVariable(&entity.m_x);
solver->addVariable(&entity.m_y);

// After solve():
// entity.m_x, entity.m_y contain solution
```

### 4. Sketch Tools Pattern

All sketch tools follow the same lifecycle pattern:

```cpp
class SketchTool {
public:
    virtual void handleMousePress(const QMouseEvent* event) {}
    virtual void handleMouseMove(const QMouseEvent* event) {}
    virtual void handleMouseRelease(const QMouseEvent* event) {}
    virtual void handleDoubleClick(const QMouseEvent* event) {}
    virtual void handleKeyPress(const QKeyEvent* event) {}
    virtual void cancel() {}  // Esc key
};
```

**Example: LineTool (322 LOC)**

```
handleMousePress(p1) → Start line from p1
handleMouseMove(p2)  → Draw preview line p1→p2
handleMousePress(p2) → Commit first segment, start p2→p3 (continuous drawing)
...
Esc or RightClick    → Cancel, exit tool
DoubleClick / Enter  → Finish, exit tool
```

**SketchToolManager** manages tool lifecycle:

```cpp
auto& toolManager = viewport->toolManager();
toolManager.activateTool(ToolType::LINE);
// → LineTool receives all mouse/keyboard events
// → Draws on sketch
// → When finished, exits tool and returns control
```

### 5. Rendering System

#### OpenGL 4.1 Core Profile

All rendering uses **OpenGL 4.1 Core Profile** (macOS requirement). Shaders are GLSL 4.1:

```glsl
#version 410 core
// ...
```

**No fixed-function pipeline allowed** (no glBegin/glEnd, no legacy matrix stacks).

#### SketchRenderer (450+ LOC)

Renders 2D sketch in 3D space:

```cpp
class SketchRenderer {
public:
    void renderEntities();        // Draw points, lines, arcs, circles
    void renderConstraints();     // Draw constraint symbols
    void renderGrid();            // Draw grid background
    void renderSelectionHighlight(); // Highlight selected entities
    void renderGhostConstraints(); // Preview constraints (50% opacity)
};
```

**Coordinate System**: Sketch XY plane is transformed to world 3D space via SketchPlane.

#### Camera3D

Manages perspective camera:

```cpp
class Camera3D {
public:
    void setPerspective(float fov, float aspect, float near, float far);
    void setViewMatrix(const glm::mat4& view);
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewMatrix() const;
    
    void zoom(float factor);
    void pan(float dx, float dy);
    void rotate(float angles);
};
```

#### Grid3D

Renders XY plane grid. **Note**: Currently **fixed 10mm spacing** (spec claims adaptive but not implemented).

---

## Common Development Tasks

### Adding a New Sketch Tool

**Example: How to add a new "Rectangle" tool**

1. **Create RectangleTool.h/cpp**

```cpp
// src/core/sketch/tools/RectangleTool.h
#ifndef ONECAD_CORE_SKETCH_TOOLS_RECTANGLETOOL_H
#define ONECAD_CORE_SKETCH_TOOLS_RECTANGLETOOL_H

#include "SketchTool.h"

namespace onecad::core::sketch {

class RectangleTool : public SketchTool {
public:
    explicit RectangleTool(Sketch& sketch, SnapManager& snapMgr);
    
    void handleMousePress(const QMouseEvent* event) override;
    void handleMouseMove(const QMouseEvent* event) override;
    void handleMouseRelease(const QMouseEvent* event) override;
    void cancel() override;
    
private:
    Sketch& m_sketch;
    SnapManager& m_snapMgr;
    
    bool m_drawing = false;
    double m_x1, m_y1, m_x2, m_y2;
};

} // namespace

#endif
```

2. **Implement RectangleTool.cpp**

```cpp
#include "RectangleTool.h"
#include "../Sketch.h"
#include "../SnapManager.h"

namespace onecad::core::sketch {

RectangleTool::RectangleTool(Sketch& sketch, SnapManager& snapMgr)
    : m_sketch(sketch), m_snapMgr(snapMgr) {}

void RectangleTool::handleMousePress(const QMouseEvent* event) {
    auto result = m_snapMgr.snap(event->pos());
    m_x1 = result.snapped ? result.position.x() : event->x();
    m_y1 = result.snapped ? result.position.y() : event->y();
    m_drawing = true;
}

void RectangleTool::handleMouseMove(const QMouseEvent* event) {
    if (!m_drawing) return;
    
    m_x2 = event->x();
    m_y2 = event->y();
    // Preview rendering handled by SketchRenderer
}

void RectangleTool::handleMouseRelease(const QMouseEvent* event) {
    if (!m_drawing) return;
    
    auto result = m_snapMgr.snap(event->pos());
    m_x2 = result.snapped ? result.position.x() : event->x();
    m_y2 = result.snapped ? result.position.y() : event->y();
    
    // Create 4 lines (bottom, right, top, left)
    m_sketch.addLine(m_x1, m_y1, m_x2, m_y1);
    m_sketch.addLine(m_x2, m_y1, m_x2, m_y2);
    m_sketch.addLine(m_x2, m_y2, m_x1, m_y2);
    m_sketch.addLine(m_x1, m_y2, m_x1, m_y1);
    
    m_drawing = false;
    // Tool exits (user continues or presses Esc)
}

void RectangleTool::cancel() {
    m_drawing = false;
}

} // namespace
```

3. **Register tool in SketchToolManager**

```cpp
// src/ui/viewport/Viewport.h
enum class ToolType {
    // ... existing tools
    DRAW_RECTANGLE
};

// src/ui/viewport/Viewport.cpp
case ToolType::DRAW_RECTANGLE:
    m_activeTool = std::make_unique<RectangleTool>(sketch, snapMgr);
    break;
```

4. **Add to CMakeLists.txt**

```cmake
# src/core/sketch/CMakeLists.txt
target_sources(onecad_core PRIVATE
    tools/RectangleTool.cpp
    tools/RectangleTool.h
)
```

5. **Create prototype test**

```cpp
// tests/prototypes/proto_rectangle_tool.cpp
#include <src/core/sketch/Sketch.h>
#include <src/core/sketch/tools/RectangleTool.h>

int main() {
    auto sketch = onecad::core::sketch::Sketch::create();
    // Test tool behavior
    return 0;
}
```

### Adding a New Constraint Type

**Example: Add "Fix" constraint (lock geometry in place)**

1. **Define constraint in constraints/Constraints.h**

```cpp
enum class ConstraintType {
    // ... existing types
    FIX_POINT  // New: Fix a point at its current position
};

class FixConstraint : public SketchConstraint {
public:
    FixConstraint(const std::string& entityId);
    ConstraintType type() const override { return ConstraintType::FIX_POINT; }
};
```

2. **Implement in constraints/Constraints.cpp**

```cpp
FixConstraint::FixConstraint(const std::string& entityId)
    : SketchConstraint(entityId) {}
```

3. **Add solver binding in ConstraintSolver::setup()**

```cpp
case ConstraintType::FIX_POINT: {
    auto point = sketch.getEntity(constraint.entityId());
    // Fix point by creating constraints: x = point.x, y = point.y
    solver->constrainPointToX(point, point->x());
    solver->constrainPointToY(point, point->y());
    break;
}
```

4. **Render constraint symbol in SketchRenderer**

```cpp
// Render a small lock icon at the point
void SketchRenderer::renderConstraintSymbol(const FixConstraint& constraint) {
    // Draw lock icon at entity position
}
```

### Debugging Solver Issues

**Problem**: Solver fails to converge or produces wrong results.

**Diagnostics**:

```cpp
// In ConstraintSolver::solve()
if (!converged) {
    std::cerr << "Solver diverged at iteration " << iterations << std::endl;
    std::cerr << "Residual: " << residual << " (target: 1e-6)" << std::endl;
    std::cerr << "DOF: " << degreesOfFreedom << std::endl;
    
    // Check for redundant constraints
    if (redundancyDetected) {
        std::cerr << "Redundant constraints detected" << std::endl;
    }
}
```

**Common Issues**:
1. **Overdetermined sketch**: Too many constraints for DOF
   - Solution: Remove redundant constraints
2. **Conflicting constraints**: Two constraints contradict
   - Solution: Review constraint logic
3. **Numerical instability**: Very large/small coordinates
   - Solution: Scale geometry to reasonable range (~10-1000 units)

### Working with OCCT Shapes

**Always null-check Handle<> objects**:

```cpp
// ✅ Good
Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, a, b);
if (!curve.IsNull()) {
    // Use curve
}

// ❌ Bad
Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, a, b);
curve->evaluatePoint(0.5);  // May crash if null
```

**OCCT Memory Management**:
- OCCT uses **reference counting** (Handle<>)
- No manual `delete` needed
- Be careful with **shape copies**: TopoDS copies are lightweight (shell copies), not deep copies

```cpp
// Lightweight copy (shares data)
TopoDS_Face face1 = shape.Face();
TopoDS_Face face2 = face1;  // Same underlying data

// Deep copy (if needed)
TopoDS_Shape shapeCopy = BRepBuilderAPI_Copy(shape).Shape();
```

---

## Debugging Guide

### Visual Debugging

#### Debug Grid/Axes

Enable in SketchRenderer:

```cpp
// src/core/sketch/SketchRenderer.cpp
void SketchRenderer::render() {
    // Draw debug axes
    renderDebugAxes();
    
    // Draw entity bounding boxes
    for (const auto& entity : m_sketch.entities()) {
        renderBoundingBox(entity);
    }
}
```

#### Constraint Visualization

Ghost constraints (50% opacity) preview inferred constraints. Check AutoConstrainer output:

```cpp
// src/core/sketch/AutoConstrainer.cpp
auto ghostConstraints = autoConstrainer.inferConstraints();
sketchRenderer.renderGhostConstraints(ghostConstraints);  // 50% opacity preview
```

### LLDB Debugging (macOS)

```bash
lldb ./build/OneCAD
(lldb) b Sketch::addLine       # Set breakpoint
(lldb) run                     # Start app
# ... interact in app ...
(lldb) c                       # Continue
(lldb) p entity               # Print entity
(lldb) p entity.m_x           # Print coordinate
```

### Print Debugging

```cpp
// In src/core/sketch/Sketch.cpp
std::cerr << "Added entity: " << entityId 
          << " at (" << x << ", " << y << ")" << std::endl;
```

### CMake Debug Flags

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-g -O0"
```

---

## Performance Considerations

### Sketch Performance Targets

| Operation | Target | Current |
|-----------|--------|---------|
| Add entity | <1ms | ✅ |
| Solve constraints (100 entities) | <100ms | ✅ |
| Render frame (60 FPS) | <16ms | ✅ |
| Snap detection | <5ms | ✅ |

### Optimization Strategies

#### 1. Solver Convergence

The constraint solver is the main bottleneck:

```cpp
// Profile with:
auto start = std::chrono::high_resolution_clock::now();
solver.solve();
auto end = std::chrono::high_resolution_clock::now();
std::cerr << "Solve time: " << 
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
    << "ms" << std::endl;
```

**Optimization**: Redundancy detection → remove redundant constraints → faster solve.

#### 2. Snap Detection

Snap detection checks all entities. For large sketches (1000+ entities), this becomes slow.

**Optimization**: Spatial indexing (quad-tree) for snap candidates.

#### 3. Rendering

Rendering uses immediate-mode OpenGL (glBegin/glEnd style in immediate mode). For large sketches:

**Optimization**: Use VAO/VBO (vertex arrays) instead of immediate rendering.

#### 4. Entity Lookup

Entity lookup by ID uses `std::unordered_map`:

```cpp
// O(1) average, O(n) worst case
auto entity = sketch.getEntity(entityId);
```

**Optimization**: This is already optimal for ID-based lookup.

---

## Important Gotchas & Patterns

### ⚠️ Non-Standard Coordinate System

**Critical**: Sketch XY plane uses a **non-standard mapping** for viewport alignment:

```
User Sketch X → World Y+ (0,1,0)
User Sketch Y → World X- (-1,0,0)
Normal        → World Z+ (0,0,1)
```

**Why**: Aligns viewport conventions (Front = World X+, Right = World Y+)

**Never "fix" this** without updating all code that depends on the mapping.

See `SketchPlane::XY()` in [src/core/sketch/Sketch.h](src/core/sketch/Sketch.h#L64).

### ⚠️ ElementMap is Regression-Sensitive

Changes to ElementMap descriptor hash ordering will **remap all geometry IDs**. This breaks:
- File save/load (IDs no longer match)
- Feature history (operations reference old IDs)
- Any downstream code depending on ID persistence

**Always run `proto_elementmap_rigorous`** before committing ElementMap changes:

```bash
./tests/proto_elementmap_rigorous
# Check output for "Hash order validation: PASS"
```

### ⚠️ Solver Direct Binding

The constraint solver holds **raw pointers** to entity coordinates. If you:

1. **Move entities in memory** (e.g., reallocate vector):
   - Solver pointers become invalid → **crash or wrong results**
   - Use `std::vector<std::unique_ptr<>>` to prevent reallocation

2. **Delete entity while solver holds pointer**:
   - Solver accesses dangling pointer → **crash**
   - Always clear solver before deleting entities

### ⚠️ OpenGL Context Setup (main.cpp)

**Critical**: OpenGL format must be set **BEFORE QApplication creation**:

```cpp
// ✅ Correct (from src/main.cpp)
QSurfaceFormat format;
format.setVersion(4, 1);
format.setProfile(QSurfaceFormat::CoreProfile);
QSurfaceFormat::setDefaultFormat(format);
QApplication app(argc, argv);  // Now context is correct

// ❌ Wrong (context won't be Core Profile 4.1)
QApplication app(argc, argv);
QSurfaceFormat format;
format.setVersion(4, 1);
QSurfaceFormat::setDefaultFormat(format);
```

### ⚠️ Qt Parent Ownership

**Always ensure proper parent hierarchy**:

```cpp
// ✅ Good
auto viewport = new Viewport(mainWindow);  // mainWindow is parent
// Deleted when mainWindow is deleted

// ❌ Bad
auto viewport = new Viewport();  // No parent
// Manual delete required, easy to leak
```

### ⚠️ OCCT Handle<> Null Checks

**Always null-check** Handle<> objects from OCCT:

```cpp
// ✅ Good
Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, a, b);
if (curve.IsNull()) {
    // Handle error
    return;
}
// Use curve safely

// ❌ Bad
Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, a, b);
curve->evaluatePoint(0.5);  // May crash if null
```

### Pattern: Entity ID Management

**Never hardcode entity IDs**. Always use the returned ID from add operations:

```cpp
// ✅ Good
std::string pointId = sketch.addPoint(10.0, 20.0);
sketch.addConstraint(ConstraintType::DISTANCE, pointId, otherPointId, 5.0);

// ❌ Bad
sketch.addPoint(10.0, 20.0);
// Assume ID is "point_1"? It's generated (UUID)
sketch.addConstraint(ConstraintType::DISTANCE, "point_1", ...);
```

### Pattern: Undo/Redo

Currently **no formal undo/redo system**. Planned for Phase 3.

For now: Save sketch state before risky operations, restore on failure.

```cpp
auto stateBackup = sketch.serialize();
try {
    // Risky operation
    solver.solve();
} catch (...) {
    sketch.deserialize(stateBackup);
    throw;
}
```

---

## Key Documents Reference

- [docs/SPECIFICATION.md](docs/SPECIFICATION.md) — Full product requirements (3700+ lines)
- [docs/PHASES.md](docs/PHASES.md) — Implementation roadmap
- [docs/ELEMENT_MAP.md](docs/ELEMENT_MAP.md) — Topological naming deep dive
- [.github/copilot-instructions.md](.github/copilot-instructions.md) — AI agent guidelines
- [AGENTS.md](AGENTS.md) — Repository guidelines for agents

---

## Getting Help

### Common Questions

**Q: Where do I start if I'm new to the codebase?**
A: Read [Quick Start](#quick-start) → [Architecture](#architecture) → Explore `src/main.cpp` → Run prototypes.

**Q: How do I add a new constraint type?**
A: See [Adding a New Constraint Type](#adding-a-new-constraint-type).

**Q: How do I debug a solver convergence issue?**
A: See [Debugging Solver Issues](#debugging-solver-issues).

**Q: Why does my code crash when accessing a Handle<> object?**
A: See [⚠️ OCCT Handle<> Null Checks](#-occt-handle-null-checks).

**Q: How do I run tests?**
A: See [Running Tests](#running-tests).

---

## Contributing

### Before Submitting a PR

1. ✅ Code compiles with `cmake --build .`
2. ✅ All prototypes pass: `./tests/proto_*`
3. ✅ For ElementMap changes: `./tests/proto_elementmap_rigorous` passes
4. ✅ Follows [Code Best Practices](#code-best-practices)
5. ✅ Commit message: Conventional Commits (`feat:`, `fix:`, `chore:`)

### Commit Message Format

```
feat: Add rectangle sketch tool

- Implements RectangleTool with continuous drawing
- Integrates with SnapManager for vertex snapping
- Adds auto-constraints for horizontal/vertical edges

Closes #42
```

---

**Last Updated**: January 2026
**Maintainer**: OneCAD Development Team

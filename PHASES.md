# OneCAD Detailed Implementation Roadmap

This document outlines the phased implementation strategy for OneCAD, ensuring a robust foundation before adding complexity.

## Phase 1: Core Foundation & Topological Infrastructure
**Focus:** establishing the critical architectural bedrock—specifically the geometry kernel integration and the topological naming system to prevent future refactoring.

### 1.1 Project & Rendering Setup
- [x] **Build System**: CMake with Qt6, OCCT, Eigen3.
- [x] **App Shell**: Main window layout.
- [ ] **RHI Viewport**: Metal-based rendering pipeline (Qt RHI).
- [ ] **Camera Controls**: Orbit, Pan, Zoom, standard views.
- [ ] **Grid System**: Adaptive 3D grid with visual cues.

### 1.2 OCCT Kernel Integration
- [ ] **Shape Wrappers**: `onecad::kernel::Shape` decoupled from `TopoDS_Shape`.
- [ ] **Geometry Factory**: Utilities for creating primitives (Box, Cylinder, Plane).
- [ ] **BREP Utilities**: Explorer for traversing Faces, Edges, Vertices.

### 1.3 Topological Naming System (ElementMap)
*Critical Path Item: Must be rigorous.*
- [ ] **ElementMap Architecture**:
    - Persistent UUID system for topological entities.
    - History tracking architecture (Generated/Modified/Deleted).
- [ ] **Evolution Tracking**:
    - Integration with OCCT history (`BRepTools_History` / `BRepAlgoAPI`).
- [ ] **Serialization**: Logic to save/load UUID mappings.
- [ ] **Rigorous Testing Suite**:
    - Automated tests for stable IDs across boolean ops.
    - Regression tests for parametric updates.

### 1.4 Command & Document Infrastructure
- [ ] **Document Model**: Entity registry, ownership hierarchy.
- [ ] **Selection Manager**: Ray-casting picking, highlighting logic.
- [ ] **Command Processor**: Transaction-based Undo/Redo stack.

---

## Phase 2: Sketching Engine
**Focus:** Creating a robust 2D constraint-based sketcher.

### 2.1 Sketch Infrastructure
- [ ] **Sketch Entity**: Data model for 2D geometry containers.
- [ ] **Workplane System**: Coordinate transforms (3D World <-> 2D Local).
- [ ] **Snap System**: Object snapping (Endpoint, Midpoint, Center).

### 2.2 Sketch Tools
- [ ] **Primitives**: Line, Rectangle, Circle, Arc, Ellipse.
- [ ] **Construction Geometry**: Toggling construction/normal mode.
- [ ] **Visual Feedback**: Real-time preview while drawing.

### 2.3 Constraint Solver (PlaneGCS)
- [ ] **Solver Integration**: Linking FreeCAD's PlaneGCS.
- [ ] **Constraint Types**: Coincident, Parallel, Perpendicular, Tangent, Distance, Angle.
- [ ] **Interactive Solver**: Real-time solving during drag.
- [ ] **DOF Visualization**: Color-coding (Green/Blue/Orange) for constraint status.

### 2.4 Loop Detection & Face Creation
- [ ] **Graph Analysis**: Planar graph traversal to find cycles.
- [ ] **Region Detection**: Identifying closed loops.
- [ ] **Auto-Highlighting**: Visual cues for closed regions (Shapr3D style).
- [ ] **MakeFace Command**: Converting regions to `TopoDS_Face`.

---

## Phase 3: Solid Modeling Operations
**Focus:** Enabling 3D geometry creation and manipulation.

### 3.1 Feature Management
- [ ] **Feature History Tree**: Parametric history management.
- [ ] **Dependency Graph**: Tracking relationships between sketches and features.
- [ ] **Regeneration Engine**: Replaying history on modification.

### 3.2 Core Operations
- [ ] **Extrude**:
    - Vector-based extrusion.
    - Draft angle support.
    - Boolean options (New/Add/Cut/Intersect).
- [ ] **Revolve**: Axis selection, angle parameters.
- [ ] **Boolean Operations**: Union, Subtract, Intersect (Multi-body).

### 3.3 Direct Modeling (Hybrid)
- [ ] **Push/Pull**: 
    - Face detection under cursor.
    - Context-aware extrude/offset.
- [ ] **Direct Face Manipulation**: Moving faces with valid topology.

---

## Phase 4: Advanced Modeling & Refinement
**Focus:** Adding depth to modeling capabilities and UI polish.

### 4.1 Modification Tools
- [ ] **Fillet/Chamfer**: Variable radius, edge chain selection.
- [ ] **Shell**: Hollow solids with thickness.
- [ ] **Patterns**: Linear and Circular duplication features.

### 4.2 UI/UX Polish
- [ ] **Contextual Toolbar**: floating tool palette near selection.
- [ ] **Property Inspector**: Fully functional attribute editor.
- [ ] **Visual Styles**: Wireframe, Shaded, Shaded with Edges.
- [ ] **Dark/Light Themes**: Full UI consistency.

### 4.3 I/O & Persistence
- [ ] **Native File Format**: Full serialization of Document, ElementMap, and History.
- [ ] **STEP Support**: Import/Export with interoperability.

---

## Phase 5: Optimization & Release
**Focus:** Performance tuning and stability.

### 5.1 Performance
- [ ] **Tessellation Control**: Adjustable LOD (Coarse/Fine).
- [ ] **Background Processing**: Threading complex boolean ops.
- [ ] **Rendering Optimization**: Instance rendering for patterns.

### 5.2 Stability & Release
- [ ] **Crash Recovery**: Autosave and backup systems.
- [ ] **Memory Management**: Cleanup of large OCCT structures.
- [ ] **Packaging**: macOS Bundle creation and signing.

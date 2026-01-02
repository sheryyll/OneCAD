
# ChatGPT Research

Persistent Topological Naming in OpenCASCADE CAD Applications
Introduction
The Topological Naming Problem (TNP) refers to the difficulty of giving stable, persistent identifiers to geometric entities (faces, edges, vertices) in a CAD model so that they remain consistent across model edits or recomputations. In history-based parametric modeling (common in CAD software), features often reference earlier geometry by name or index. If those references change unpredictably after a model update, it can break the parametric history – this is exactly what happens in OpenCASCADE without special handling
novedge.com
. OpenCASCADE’s geometry kernel represents solids as collections of topology (vertices, edges, faces) and underlying geometry (points, curves, surfaces). By default, it does not assign persistent IDs or names to sub-shapes; instead, references are typically ephemeral memory pointers or sequential indices that can change whenever the shape is recomputed
quaoar.su
. This makes robust parametric modeling challenging unless a topological naming scheme is implemented.
Why the Topological Naming Problem Arises in OpenCASCADE
OpenCASCADE (OCCT) separates topology from geometry – a TopoDS_Face is a topological entity that holds a reference to a geometric surface, for example
quaoar.su
. The kernel provides tools to traverse and modify these topological structures, but it does not inherently provide a stable way to identify a given face or edge across modifications. Most modeling algorithms use transient references (like pointers or indices in a list of sub-shapes), which are not persistent
quaoar.su
. When a feature is recomputed (e.g. after changing a sketch or a parameter), the solid is rebuilt and its sub-shapes often get new memory addresses and possibly a different ordering. As a result, a face that was previously “Face6” might now be “Face7”, or an entirely new object, invalidating any references to the old face. This instability is a fundamental issue first articulated in research by J. Kripac (1997), who described a mechanism for persistently naming topological entities in history-based CAD models
quaoar.su
. A naive approach is to number every face/edge when first created and hope those numbers stay the same. However, sequential IDs are not robust – any topological change (like adding or removing a feature) will renumber subsequent elements, making old indices meaningless
quaoar.su
. The problem becomes especially evident in parametric modeling: if a sketch or feature is attached to “Face3” of a part, and then an earlier operation changes the part’s topology, “Face3” might no longer be the same face – or might not exist at all – causing the model update to fail or misbehave
novedge.com
. In assemblies, mates that refer to specific faces or edges can likewise break when those references shift. OpenCASCADE itself does not store custom metadata on TopoDS shapes (there’s no built-in persistent name field)
analysis-situs.medium.com
. All identification must therefore be done externally or at a higher level. This is why the TNP is universal to CAD kernels that rely on B-Rep topology: without a strategy for persistent naming, any significant change in the topology graph (adding/removing faces, splitting edges, etc.) can scramble the “names” of entities. In practice, this means developers and advanced users must implement mechanisms to track geometry through the modeling operations.
Impact on Parametric Modeling and History-Based Operations
In a parametric, feature-based CAD system, the user expects to be able to modify early features (like sketches, extrusions, booleans) and have later features update correctly. The TNP undermines this by causing later features to lose track of their targets. For example, consider a parametric model of a box with a hole on one face. If the hole feature “remembers” the face by a reference (say face index or ID), adding another feature before it (like a fillet or a chamfer on a different edge) might change the internal ordering of faces. The hole might then end up on a different face or fail to find its face at all. This leads to errors where features either break (error out) or, worse, get attached to the wrong geometry – a nightmare for model correctness. Such issues severely affect the robustness of history-based modeling
novedge.com
. Users experience this as models that “blow up” or require reattachment of sketches and constraints whenever they make design changes. In assembly contexts, constraints that refer to a particular edge or face of a component can suddenly refer to nothing or to an incorrect element after the component is edited. Clearly, solving the TNP is critical for any CAD application that wants reliable parametric updates. Developers have sometimes introduced workarounds to mitigate the problem even without a full naming solution. A common best-practice is to avoid linking features to generated geometry (e.g. attach sketches to primary reference planes or origin, rather than on faces created by previous features), thereby reducing the chance that a referenced face disappears
hackaday.com
. While this can make models more stable, it limits modeling flexibility and is not a true fix. A robust solution requires changes at the software architecture level: giving each topological entity a persistent identity that survives model alterations, or a procedure to consistently re-identify entities after each operation.
Strategies for Solving the Topological Naming Problem
Over the years, several strategies have been developed to tackle the TNP in OpenCASCADE-based applications. These include algorithmic naming schemes, the use of persistent identifiers and mapping data structures, as well as leveraging OpenCASCADE’s own OCAF framework for naming. Below we discuss the major approaches:
Persistent Naming Algorithms and Identifiers
A general strategy (inspired by research like Kripac’s paper) is to assign each topological entity a name or ID based on the history of how it was created, rather than just its position in a list
quaoar.su
quaoar.su
. The idea is to capture enough information about a face/edge’s genesis so that when the model changes, the system can recognize which new face corresponds to the old one. This often involves:
Mapping input to output shapes for each operation: When a modeling operation (extrude, cut, fillet, etc.) is performed, record which original faces/edges led to which new faces/edges. OpenCASCADE’s modeling algorithms actually provide this via the BRepBuilderAPI_MakeShape interface: methods like Generated(oldShape) and Modified(oldShape) return the new shapes that resulted from a given original shape, and IsDeleted(oldShape) checks if an original shape was removed
github.com
github.com
.
Assigning stable IDs/names: Using the above mapping, one can propagate an identifier from the original shape to the new ones. For example, if face F1 on a solid gets split into two faces by a cut, the new faces can be named in relation to F1 (say F1.1 and F1.2 or some scheme) rather than as completely new unrelated entities.
Encoding topology context: More advanced naming schemes incorporate the surrounding context or combination of elements as part of a name. For instance, if a new face is defined by the intersection of two original faces, the persistent name might encode both source faces’ IDs. This way, even if one of them changes, the system can attempt to find a face in the new shape that is at the intersection of the updated corresponding faces.
The core challenge is designing the naming algorithm to handle all modeling operations. Some operations are topology-preserving (e.g. a pure geometric tweak that doesn’t add or remove faces), while others radically alter topology (booleans, fillets, etc.). A robust algorithm must handle both gracefully, preserving existing names when possible and generating new identifiers for new geometry. It’s generally accepted that no approach can be 100% perfect for all cases (the problem is theoretically hard), but practical solutions can cover the vast majority of modeling scenarios.
OpenCASCADE’s OCAF TNaming Service (Persistent Labels)
OpenCASCADE itself provides a solution for persistent naming as part of its Application Framework (OCAF). The OCAF data model uses a document/attribute system to store CAD data along with auxiliary information (like names, colors, etc.). Within OCAF, the TNaming package is dedicated to the naming problem. It allows you to store a TNaming_NamedShape attribute that records a shape’s evolution through operations
dev.opencascade.org
. In essence, TNaming works by keeping a history of modifications: it stores pairs of shapes – an old shape and the corresponding new shape – along with an evolution type (such as PRIMITIVE, MODIFY, GENERATE, or DELETE)
dev.opencascade.org
dev.opencascade.org
. Each document has a special section (a TNaming_UsedShapes map) that keeps all unique shapes by reference, and NamedShape attributes placed on various labels refer into this map so that identities are shared globally
dev.opencascade.org
. When using OCAF for modeling, the typical pattern is:
Record initial shapes: When a feature creates a new shape (say you create a box), you add it to the OCAF document. Using TNaming_Builder, you might call TNaming_Builder::Generated(label, newShape) to record a new shape under a label. Commonly, you also record all its significant sub-shapes (e.g., for a solid, you might store each face under a child label) so that they each get a persistent name
dev.opencascade.org
.
Record operations: When an operation modifies a shape, you record the mapping. For example, if you fillet Edge3 of the box, you use TNaming_Builder::Modify(label, oldShape, newShape) to record that the original box shape was modified into the new filleted shape
dev.opencascade.org
. Additionally, any brand new sub-shapes introduced by the fillet (like the new fillet faces) can be recorded with TNaming_Builder::Generated on appropriate sub-labels. All these labels together form a tree that represents the model’s topological history
dev.opencascade.org
dev.opencascade.org
.
Selections by name: If a higher-level operation or user selection refers to a sub-shape (for instance, a sketch is mapped to “Face1” of the box), OCAF will store that selection as a reference to the NamedShape attribute of that face. Later, if the model is recomputed or modified, you can use TNaming_Selector to resolve the selection: it will traverse the history recorded in TNaming to find the up-to-date shape corresponding to the originally selected one. In our example, after filleting and resizing the box, TNaming_Selector::Solve can still find “Edge3” on the latest solid because it uses the recorded history graph to map the old edge to its counterpart in the new shape
dev.opencascade.org
. Essentially, the naming mechanism “replays” the sequence of modifications to track the entity through changes.
One advantage of OCAF’s approach is that it is systematic and built into the framework – if you use OCAF for your application data model, you get a ready-made way to handle naming. The topological naming algorithm in OCAF relies on the modeling algorithms providing “correct” history information (the lists of generated/modified shapes)
dev.opencascade.org
. As long as each operation logs its results properly, the naming service can maintain consistent references. It’s worth noting that this was originally developed as part of OpenCASCADE’s sample application framework, which is why it’s somewhat decoupled from the low-level geometry library. In fact, early FreeCAD developers noted that OpenCASCADE already had a naming service (TNaming), but it was tightly integrated with OCAF and felt like an afterthought rather than a core part of the modeling kernel
devtalk.freecad.org
. Many projects (FreeCAD included, historically) did not use OCAF’s document structure and thus had to implement custom solutions. Nonetheless, for those who do use OCAF, classes like TNaming_Builder, TNaming_NamedShape, and TNaming_Selector provide a robust C++ API to tackle the TNP. Below is a simplified snippet illustrating how one might use these in an OCAF context:
// Pseudocode: Using TNaming in OCAF to record a feature operation
TDF_Label featureLab = ...;           // label in the OCAF document for this feature
TopoDS_Shape oldShape = ...;          // shape before the operation (if any)
TopoDS_Shape newShape = resultShape;  // result of a modeling operation (e.g., fillet)
// If this is a modification of an existing shape:
TNaming_Builder builder(featureLab);
if (!oldShape.IsNull()) {
    builder.Modify(oldShape, newShape);   // record that oldShape was modified into newShape
} else {
    builder.Generated(newShape);          // record a new generated shape (primitive)
}
// Record sub-shapes if needed (e.g., all faces of newShape for detailed references)
for (TopoDS_Iterator it(newShape); it.More(); it.Next()) {
    TopoDS_Shape sub = it.Value();
    TNaming_Builder subBuilder(featureLab.NewChild()); 
    subBuilder.Generated(sub);
}
In practice, OCAF will store the “old -> new” shape pairs in the TNaming_NamedShape attribute on featureLab. If later an external reference (say a sketch attached to a face) points to one of these sub-labels, OCAF can retrieve the current TopoDS_Shape via TNaming_Selector::NamedShape()->Get() or TNaming_Tool::GetShape() on that label
dev.opencascade.org
. This returns the current shape corresponding to the named reference after all operations have been applied.
FreeCAD’s Approach: RealThunder’s Topological Naming Algorithm
FreeCAD, an open-source CAD program built on OpenCASCADE, historically suffered from the TNP – a well-known pain point for its users. Instead of using OCAF, FreeCAD developed its own application framework and thus needed a custom solution. In recent versions (as of FreeCAD 0.21+ and the upcoming 1.0), FreeCAD has integrated a robust topological naming algorithm devised by developer realthunder. This was a major breakthrough for the project
hackaday.com
hackaday.com
. Realthunder’s algorithm works by analyzing the shape history for each modeling operation and assigning hierarchical, persistent names to every sub-shape. At a high level, the process can be summarized in four steps
github.com
github.com
:
Match Unchanged Geometry: Compare the input shapes and the resulting shape of an operation to find any sub-elements that are identical and already named. Those keep their names in the new shape
github.com
. (This handles cases where a face or edge passes through the operation unaltered.)
Assign Names via History Mapping: For each input element (face/edge/vertex) that was modified or that generated new elements, use the OpenCASCADE mapping information to name the outcome. The algorithm queries the OCCT Mapper (which wraps BRepBuilderAPI_MakeShape history) to get all Modified and Generated sub-shapes from each original element
github.com
github.com
. New elements inherit a base name from the source. For example, if face F2 produced a new face, that new face might get a provisional name derived from F2. If multiple source elements combine to form one, it might temporarily get multiple names.
Name Unreferenced Elements by Hierarchy: After step 2, some sub-shapes may still be unnamed (e.g. a brand new edge that wasn’t directly generated from a single input edge). The algorithm then assigns names based on an upper-level context. For instance, if a face is new but sits on a solid that has a name, it could be named as a child of that solid. In practice, the algorithm uses an indexed scheme: if a face is named F1, its edges that remain unnamed will get names like F1;:U1, F1;:U2, etc., denoting “unnamed edge 1 of face F1”
github.com
. Interestingly, if an edge lies on multiple faces, it may receive multiple such names (one per face context) to improve stability – ensuring that if one face disappears, another context still identifies that edge
github.com
.
Resolve Remaining Names by Lower Elements: For any element still unnamed after the above, use the combination of its lower-level components to define its identity. For example, if a new face’s bounding edges are all named, the face can be named as a function of those edges (essentially, a combination like (EdgeA,EdgeB,...);:L where ;:L might denote a face derived from those lower edges)
github.com
github.com
. This way, even if the face is new, as long as the set of its boundary edges can be recognized, the face’s name can persist through changes.
After these steps, every sub-shape of the result shape has at least one name (often a stable textual identifier encoding its lineage). The names generated by FreeCAD’s algorithm look like structured tokens. For example, you might see something like:
Edge2;:G1(2_Edge3:2,3_Edge4:2);XTR;:T1:4:F
This string encodes that the edge was generated (;:G) as the first generation (1) from an operation “XTR” (maybe Extrude) on sources Edge3 of shape 2 and Edge4 of shape 3, etc., along with tags indicating original shape IDs and element types
github.com
github.com
. The details are complex, but the key point is that these names contain enough information about the element’s history that FreeCAD can later trace them. If the model changes, FreeCAD uses these names to match entities in the new shape by comparing the history signatures. The approach is heavily based on leveraging OpenCASCADE’s BRepBuilderAPI_MakeShape history APIs (Modified/Generated), and where those fail (some algorithms don’t fully report history), it falls back to geometric comparisons or heuristics
github.com
github.com
. The result is a dramatically improved stability in FreeCAD’s parametric models. In scenarios that used to reliably break a model (like inserting a feature in the middle of the history, or editing an earlier sketch), the new naming system preserves references for the majority of cases. This has been demonstrated by the FreeCAD community and is slated to be a cornerstone of FreeCAD 1.0
hackaday.com
hackaday.com
. The implementation is in C++ (within FreeCAD’s TopoShape class and related structures) but is abstracted so that FreeCAD users don’t see the cryptic names – they simply notice that features remain attached to the correct geometry. The FreeCAD development wiki and realthunder’s Assembly3 project provide detailed documentation of this algorithm
github.com
github.com
, and the code itself can serve as a reference pattern for implementing persistent naming in other projects.
Geometry-Based Matching and Other Techniques
Besides the structured approaches above, there are other techniques and considerations in mitigating the TNP:
Geometry signature matching: If no explicit history map is available, one can try to identify entities by their geometric properties. For example, if a particular face can be recognized by being the largest planar face in a certain orientation, an algorithm can attempt to find that face again after a model change by searching for a similar face. Some CAD systems use heuristic or geometric hashing methods to guess correspondence of faces between model revisions. This is not foolproof, but can handle simple cases (like a hole staying on the same planar face that just grew or moved slightly). FreeCAD’s older approaches and user macros sometimes did this – e.g., using face centroids, normals, and areas to detect which face is “the same” one after a boolean operation.
Graph-based topology analysis: The Analysis Situs project (open-source) provides advanced tools for interrogating OpenCASCADE shapes. It introduces structures like the Attributed Adjacency Graph (AAG), which represents the connectivity of faces and edges in a graph form
quaoar.su
. Such a graph can help in feature recognition and persistent naming by providing context. For instance, a cylindrical face in the middle of a through-hole feature might be identified by the cycle of faces around it. After a change, one can find a similar subgraph in the new model’s AAG to relocate that face. Analysis Situs also has a module asiAlgo_Naming which is essentially a naming service similar to OCAF’s, designed to trace modifications. It can take an initial shape with named sub-shapes (for example, names imported from a STEP file or assigned by the user) and then propagate those names through modeling operations so that new faces keep the same names as their progenitors
analysis-situs.medium.com
. This confirms again that maintaining an external data structure “near” the shape is essential, since OCCT’s native shapes cannot hold names internally
analysis-situs.medium.com
.
User-space solutions and scripting: In scenarios where a full-blown naming algorithm is not available, CAD modelers sometimes resort to scripting or higher-level logic to maintain references. For example, a script might explicitly find a face by searching for a feature (like "the face at coordinate X" or "the face with a hole through it") every time the model updates. This is obviously not ideal for performance or generality, but can solve very specific cases.
Preventive modeling practices: As mentioned, a practical (though limiting) strategy is to avoid unstable references altogether. Many FreeCAD users were advised to attach sketches to base planes or use datum geometry (reference planes/axes) instead of faces from the solid, because those base features are less likely to disappear. Similarly, in assemblies, using coordinate systems or datum points for mating can avoid relying on a specific face that might change. These practices don't solve TNP in the kernel, but they mitigate the symptoms at the expense of some modeling convenience.
C++ Implementation Techniques and Examples
Implementing a solution for the topological naming problem in C++ (using OpenCASCADE) typically involves working with the shape history and maintaining a mapping from one stage of the model to the next. Below are some techniques and examples:
Using OpenCASCADE’s History API: Many OCCT modeling algorithms derive from BRepBuilderAPI_MakeShape, which provides methods to query the history of shapes. In C++, after performing an operation, you can call e.g. aTool.Modified(oldShape) to get the shapes in the result that come from oldShape, or aTool.Generated(oldShape) to get brand new shapes derived from it. There is also aTool.IsDeleted(oldShape) to check if an original shape was removed entirely. By iterating over the sub-shapes of the input and collecting these mappings, you can build a correspondence table. For example:
// Example: boolean cut operation and capturing face mapping
TopoDS_Shape tool = ...; 
TopoDS_Shape base = ...;
BRepAlgoAPI_Cut cut(base, tool);
cut.Build();
TopoDS_Shape result = cut.Shape();

// Iterate over faces of the original base shape
for (TopExp_Explorer exp(base, TopAbs_FACE); exp.More(); exp.Next()) {
    TopoDS_Face origFace = TopoDS::Face(exp.Current());
    if (cut.IsDeleted(origFace)) {
        std::cout << "Face " << /* some ID */ " was deleted by the cut.\n";
    }
    // Faces modified from this original face:
    for (TopoDS_Shape newFace : cut.Modified(origFace)) {
        // Assign persistent ID of origFace to newFace (for example)
    }
    // Faces generated from this original face (e.g., new faces on the cutting edges):
    for (TopoDS_Shape sideFace : cut.Generated(origFace)) {
        // Assign a new ID, perhaps derived from origFace's ID
    }
}
In this snippet, for each face of the original object, we check if it survived (Modified) or if new faces were created from it (Generated). A real implementation would need to decide how to form identifiers. One might store a structure like PersistentID origID -> list<PersistentID> newIDs for modifications, and similarly for generated elements. The persistent IDs could be simple strings or structured tokens. Notably, FreeCAD’s algorithm uses a tag for each shape (essentially an ID per feature) and text encoding as shown earlier. Another implementation could use integers or GUIDs.
Embedding in the Data Model: You need a place to store these mappings across the life of the CAD model. OCAF does this within its document (each feature’s label contains a TNaming_NamedShape that holds the mapping for that feature). If you’re not using OCAF, you might implement your own structure. For example, FreeCAD maintains a custom TopoShape class that holds the naming information parallel to the actual TopoDS_Shape. Another approach is to store a map from TopoDS_Shape (or more reliably, from the TopoDS_TShape which is the underlying shared geometry pointer) to a persistent name. The Analysis Situs library’s asiAlgo_Naming likely builds an internal graph (nodes = shapes, edges = topological adjacency) and attaches names to the nodes
analysis-situs.medium.com
. The key is that whenever the shape changes, you update this map rather than throwing it away.
Using Unique Properties for Matching: In addition to direct mapping, robust implementations include fallbacks for when the history is ambiguous or incomplete. For instance, if two faces swap roles or a feature reorders faces, an algorithm might compare geometric properties. C++ allows using OCCT’s geometry toolkit to compute things like surface type, bounding box, center of mass of a face, etc. These can help identify if a face in the new shape probably corresponds to a face in the old shape (e.g., same surface area and same neighbors). Implementing such a heuristic requires careful tolerance management (to decide if two faces are “the same” within tolerance) and is usually a last resort, but it can handle corner cases.
Leverage Existing Libraries: If implementing from scratch is daunting, consider leveraging existing work:
OpenCASCADE’s OCAF can be used either fully (managing your model in OCAF document) or partially. You could study TNaming_Builder and TNaming_Selector usage from OCCT’s examples (e.g., the OCAF sample in OCCT) to guide your implementation
dev.opencascade.org
dev.opencascade.org
. The OCAF approach essentially does the heavy lifting of storing name records for you.
FreeCAD’s source code is open and provides an example of a non-OCAF solution. The naming code is primarily in the src/Mod/Part/App/TopoShape.cpp and related files of FreeCAD. It defines how names are generated and looked up. For example, FreeCAD uses an internal class TopoShape::Mapper to abstract the mapping from various OCCT algorithms (since not all use the same history interface)
github.com
. Studying this could give insight into handling those inconsistencies (realthunder had to add special cases for things like sweeps and sewings which don’t report history in the standard way
github.com
).
The Analysis Situs project (on GitLab) may have components you can reuse. Its naming tool (asiAlgo_Naming) and related documentation show how to integrate naming with reading/writing STEP files and performing operations while preserving names
analysis-situs.medium.com
analysis-situs.medium.com
. This could be especially useful if your application needs to maintain names through import/export or use OCCT’s XDE (Extended Data Exchange) which can carry names for faces.
Implementing a complete persistent naming system is certainly a complex task, but these strategies and examples show it’s tractable with a combination of OpenCASCADE’s APIs and careful software design. The payoff is significant: parametric models become far more reliable when each topological element can be consistently tracked through the model’s evolution. The success seen in FreeCAD’s recent development and the capabilities in OCAF demonstrate that the Topological Naming Problem, while notoriously difficult, can be practically overcome with a robust algorithm and supporting data structures
hackaday.com
hackaday.com
.
Conclusion
The Topological Naming Problem in OpenCASCADE-based CAD applications arises from the fundamental way B-Rep models handle (or don’t handle) identity for faces, edges, and vertices. It affects any history-based modeling operation where features depend on others. Solving it requires giving the CAD system a “memory” of how geometry changes – through persistent naming, mapping of modifications, or geometry tracking. OpenCASCADE’s OCAF framework provides one solution via the TNaming mechanism, and projects like FreeCAD have developed their own naming algorithms to address the issue outside of OCAF. Key techniques include using OCCT’s shape history methods to propagate IDs, storing those IDs in a structured way (labels, graphs, maps), and employing naming conventions that survive typical modeling changes. For developers working in C++, there are now multiple references and tools to draw on: the official OCCT documentation for TNaming
dev.opencascade.org
dev.opencascade.org
, open-source implementations in FreeCAD
github.com
github.com
, and research projects like Analysis Situs
quaoar.su
analysis-situs.medium.com
. By applying these strategies, one can significantly mitigate the TNP. The end result is more stable parametric models where features and constraints remain reliably attached to the intended geometry, even as the design evolves – a crucial requirement for advanced CAD systems. Sources:
Kripac, J. (1997). A mechanism for persistently naming topological entities in history-based parametric solid models. Computer-Aided Design 29(2), 113–122 (as cited in 
quaoar.su
).
OpenCASCADE Technology, OCAF User’s Guide – Naming Data (excerpt on TNaming_NamedShape and naming algorithm)
dev.opencascade.org
dev.opencascade.org
.
FreeCAD Forum – Discussion on robust references (developer commentary on OCC’s TNaming service)
devtalk.freecad.org
.
FreeCAD Assembly3 Wiki – Topological Naming Algorithm (algorithm overview and pseudo-code by realthunder)
github.com
github.com
.
Analysis Situs Blog – Indexing the topology of OpenCascade (explanation of persistent IDs and naming issues)
quaoar.su
quaoar.su
.
Analysis Situs Medium Article – CAD feature recognition with persistent IDs (notes on storing names and using a naming service)
analysis-situs.medium.com
analysis-situs.medium.com
.
Novedge Blog – Design Software History: From Geometry-First to Intent-Driven CAD (mention of topological naming problem in CAD)
novedge.com
.
OpenCASCADE Forum – Proper use of TNaming (illustrative example of using TNaming_Builder and TNaming_Selector in an OCAF document)
dev.opencascade.org
dev.opencascade.org
.
Citácie

Design Software History: From Geometry-First to Intent-Driven CAD ...

https://novedge.com/blogs/design-news/design-software-history-from-geometry-first-to-intent-driven-cad-pro-engineer-and-the-rise-of-parametrics?srsltid=AfmBOoomddpJi-mZEKHm5lSnjCM_FOhP45x_NHeHG9lccSzTH9SBr_4n

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

CAD feature recognition with persistent IDs | by Analysis Situs | Medium

https://analysis-situs.medium.com/cad-feature-recognition-with-persistent-ids-20866b17d202

FreeCAD Foray: From Brick To Shell | Hackaday

https://hackaday.com/2025/09/09/freecad-foray-from-brick-to-shell/

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

"Robust references" - Page 4 - General Development - FreeCAD Developer Forum

https://devtalk.freecad.org/t/robust-references/1701?page=4

Open CASCADE Technology: OCAF

https://dev.opencascade.org/doc/overview/html/occt_user_guides__ocaf.html

FreeCAD Foray: From Brick To Shell | Hackaday

https://hackaday.com/2025/09/09/freecad-foray-from-brick-to-shell/

FreeCAD Foray: From Brick To Shell | Hackaday

https://hackaday.com/2025/09/09/freecad-foray-from-brick-to-shell/

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Indexing the topology of OpenCascade

https://quaoar.su/blog/page/indexing-the-topology-of-opencascade

CAD feature recognition with persistent IDs | by Analysis Situs | Medium

https://analysis-situs.medium.com/cad-feature-recognition-with-persistent-ids-20866b17d202

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Proper Use of TNaming - Forum Open Cascade Technology

https://dev.opencascade.org/content/proper-use-tnaming

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

Topological Naming Algorithm · realthunder/FreeCAD_assembly3 Wiki · GitHub

https://github.com/realthunder/FreeCAD_assembly3/wiki/Topological-Naming-Algorithm

CAD feature recognition with persistent IDs | by Analysis Situs | Medium

https://analysis-situs.medium.com/cad-feature-recognition-with-persistent-ids-20866b17d202




---

# Gemini Research

Architectural Paradigms for Persistent Topological Naming in C++ CAD Kernels1. The Phenomenology of Topological Instability in Parametric ModelingThe development of parametric Computer-Aided Design (CAD) applications represents a distinct and formidable challenge within the broader field of software engineering. Unlike direct modeling, where geometry is static and explicit, parametric modeling relies on a procedural history—a recipe of operations—to define shape. This history-based approach empowers engineers to modify early design parameters and have those changes propagate through the entire dependency chain to update the final part. However, this flexibility introduces a fundamental fragility at the core of the system: the Topological Naming Problem (TNP).The TNP is not merely a software bug; it is a profound problem of identity within a dynamic system.1 It arises when the internal identifiers of topological entities—faces, edges, and vertices—change unpredictably following a model re-evaluation. When a user modifies a parameter earlier in the history tree, the boundary representation (B-Rep) of the model is regenerated. This regeneration can alter the number of entities, their order, and their geometric characteristics. If downstream features, such as sketches or fillets, reference these entities by unstable identifiers (such as an index in a list), the links break, or worse, reattach to incorrect geometry.3For the C++ architect tasked with building a robust CAD application, relying on the raw geometric kernel is insufficient. Standard kernels, including the widely used OpenCASCADE Technology (OCCT), are deterministic in their generation of geometry but generally stateless regarding the preservation of "identity" across diverse evaluations.5 The kernel generates a valid shape for the current parameters, but it does not inherently know that "Face 5" in the previous version corresponds to "Face 7" in the current version. The application layer must therefore bridge this gap by implementing a rigorous system of persistent naming.1.1 The Mechanics of Failure in B-Rep SystemsTo understand the necessity of a complex naming architecture, one must first dissect the mechanism of failure. In a Boundary Representation model, a solid object is a graph of connected topological elements. A cube, for instance, is not stored as a primitive "Cube" object in the final evaluated state; it is a collection of six Faces, twelve Edges, and eight Vertices, linked by connectivity data (the topology) and defining the underlying surfaces and curves (the geometry).7In a naive implementation, a CAD system might identify these elements by their index. When the kernel generates a cube, it produces a list of faces: [F0, F1, F2, F3, F4, F5]. If a user selects the top face to attach a sketch, the system records: "Attach Sketch to Face at Index 2".Consider the scenario where the user introduces a chamfer operation to the base block before the sketch operation. The chamfer replaces one edge with a new face and modifies two adjacent faces. When the kernel recomputes the B-Rep, the list of faces is regenerated. The face that was physically at the top might now be at Index 3, or Index 2 might now refer to the small chamfer face.9The consequences of this misalignment are severe:Dangling References: The referenced index no longer exists (e.g., if the face was consumed by a cut), causing the feature to fail with an error.3Semantic Corruption: The reference exists but points to the wrong entity. The sketch attaches to the side of the object instead of the top, causing the subsequent extrusion to shoot off into empty space or intersect self-geometry.4Catastrophic Model Failure: The misaligned geometry causes boolean operations to fail (e.g., trying to cut a solid with a tool that is now completely outside the target volume), leading to a "red tree" of errors that requires manual repair by the user.41.2 The "Ship of Theseus" Identity ParadoxThe theoretical crux of the TNP is the "Ship of Theseus" paradox applied to digital topology. If a face is split in two by a cut operation, does the original face cease to exist? Do the two new faces inherit the identity of the parent? If a face is stretched by a non-uniform scale, is it the same face?In a robust parametric system, identity must be defined by provenance (history) rather than current state. A face is "Face A" not because it is square or located at Z=10, but because it was generated by the "Top" output of "Extrude 1".10 This necessitates a shift from referencing by pointer or referencing by index to referencing by recipe. The C++ implementation of this logic requires a persistent naming engine that can serialize this recipe into a stable identifier (a "Topological ID") and a resolution engine that can interpret this ID to locate the corresponding geometry in any future version of the model.11The following table contrasts the stability of different referencing strategies typically found in CAD development:StrategyMechanismStability ProfileFailure ModeIndex-BasedFaceListExtremely LowFails on any topology change (add/remove features).GeometricFace at (0,0,10)Low to MediumFails on parametric modification (size changes).TopologicalFace bound by E1, E2..MediumFails on split/merge events; computationally expensive graph matching.ProvenanceFace form Extrude1.TopHighFails only if the source feature is deleted or fundamentally altered.The consensus in modern CAD research and implementation is that a hybrid approach—primarily Provenance-based, backed by Topological and Geometric heuristics—is the only viable solution for professional-grade applications.122. Theoretical Foundations and Naming TaxonomiesBefore analyzing specific C++ implementations, it is vital to establish the theoretical framework that governs persistent naming. Research distinguishes between three primary taxonomies of naming: Topology-based, Geometry-based, and History-based.22.1 Topology-Based Naming and Graph IsomorphismTopology-based naming identifies an entity by its adjacency relationships within the B-Rep graph. A face is uniquely identified by the loop of edges that bounds it; an edge is identified by its start and end vertices and the faces it divides.11In this paradigm, re-identifying a face after a model recompute becomes a problem of Subgraph Isomorphism. The system must find a mapping between the nodes of the old topological graph and the new one that preserves adjacency. While robust against rigid body transformations (moving the part does not change its topology), this method is brittle in the face of topological modification. If an operation splits an edge, the graph structure changes fundamentally, breaking the isomorphism.15Furthermore, subgraph isomorphism is known to be an NP-Hard problem in the general case, although B-Rep graphs (being planar or near-planar) allow for more efficient matching algorithms. Libraries such as VFLib or Boost Graph Library (BGL) can be employed in C++ to perform these checks, but the performance overhead for complex models with thousands of faces can be prohibitive for real-time interaction.172.2 Geometry-Based NamingGeometry-based naming relies on the metric properties of the entity. A face might be identified by the equation of its underlying surface (e.g., a plane equation $Ax + By + Cz + D = 0$) or by a point contained within it (e.g., the center of mass).While this method is immune to topological splitting (a split face still resides on the same plane), it is critically flawed for parametric design. The defining characteristic of parametric modeling is the ability to change dimensions. If a user changes a parameter Length = 100 to Length = 200, the geometric signature of every face moves. A purely geometric naming system would lose track of the faces immediately.11However, geometry plays a crucial role as a secondary heuristic. When history-based naming results in ambiguity (e.g., one face splits into two, both sharing the same history), geometric overlap is often the only way to determine which of the new faces corresponds to the user's original selection.122.3 History-Based (Provenance) NamingHistory-based naming, or "Persistent Naming," identifies an entity by its genealogical lineage. This is the industry standard for robust CAD systems. An identifier in this scheme effectively records the path through the dependency graph that created the entity.10For example, a face on a filleted cube might be named:Box_Feature.TopFace -> Fillet_Feature.GeneratedFace(Context:Edge_12)This string describes the intent: "Take the top face of the box, and track it through the fillet operation." Even if the box dimensions change, the "Top Face" concept remains valid. Even if the fillet radius changes, the relationship holds. This method requires the CAD kernel to provide Evolution Support—the ability to report for every operation which input entities generated which output entities.5The implementation of History-Based naming in C++ requires a "wrapper" architecture around the kernel, as standard kernels often discard this fine-grained provenance data once the operation is complete.3. State-of-the-Art in Open Source KernelsTo design a custom solution, we must examine the reference implementations available in the open-source ecosystem, primarily OpenCASCADE Technology (OCCT) and the advanced modifications found in FreeCAD's "LinkBranch" (developed by Realthunder).3.1 OpenCASCADE Technology (OCCT) and OCAFOCCT is the underlying kernel for many open-source CAD applications. It provides a data framework called OCAF (Open CASCADE Application Framework) which includes a built-in, albeit complex, persistent naming mechanism.53.1.1 The TNaming PackageThe core of OCAF's solution is the TNaming package, specifically the TNaming_NamedShape attribute. This attribute does not store the shape itself; it stores the evolution of the shape. It maintains a list of "Old Shape" and "New Shape" pairs, categorized by an enumeration TNaming_Evolution 5:PRIMITIVE: Used for shapes created from scratch (e.g., the faces of a generic box).GENERATED: Used when a shape is created from a lower-dimensional shape (e.g., an extrusion creating a face from an edge).MODIFY: Used when a shape is geometrically modified but topologically similar (e.g., a face deformed by a draft angle).DELETE: Used when a shape is removed from the model.SELECTED: Used to mark a shape for selection.3.1.2 TNaming_Builder and TNaming_SelectorTo use this system, the C++ developer must manually instrument every modeling algorithm using TNaming_Builder. When a boolean operation is performed, the developer must query the algorithm for the generated/modified lists and register them into the OCAF data structure.20To retrieve a shape, the TNaming_Selector class is used. A "Name" in OCAF is essentially a pointer to a label in the data tree. The Selector::Solve() method traverses the history of evolutions recorded in the tree to find the current version of the named shape.22Limitations: While powerful, OCAF's mechanism is often criticized for its complexity and fragility. It requires strict discipline in registering history; if a single algorithm in the chain fails to report its evolution correctly, the chain is broken, and references are lost. Furthermore, standard OCAF selectors can struggle with complex split/merge scenarios where the evolution is not 1-to-1.233.2 FreeCAD and the Realthunder AlgorithmsFreeCAD, which is based on OCCT, famously suffers from the TNP. However, the community developer "Realthunder" created a highly experimental branch (Assembly3/LinkBranch) that effectively mitigates the issue through a novel algorithm.43.2.1 The TopoShape::MapperRealthunder's approach involves wrapping the OCCT shape in a TopoShape class that includes a Mapper. This mapper generates stable element names based on a combination of history and hierarchy.The algorithm uses a specific syntax for naming elements, such as Face1;:U1. This implies "The first unnamed child of Face 1". If a face cannot be uniquely identified by its direct history, the algorithm attempts to name it by the combination of its neighbors or sub-elements (e.g., "The face bounded by Edge1, Edge2, and Edge3").123.2.2 The Shadow GraphA critical innovation in this branch is the concept of the Shadow Graph. When a model is recomputed, the system keeps a "shadow" of the topology from the previous valid state. If the strict history lookup fails (e.g., due to a topology split that the kernel handled non-deterministically), the system compares the new topology against the shadow. It uses geometric hashing and adjacency checks to perform a "best fit" match, recovering references that would otherwise be lost.4 This heuristic layer acts as a safety net beneath the strict history tracking.4. Architectural Design of a Persistent Naming EngineBased on the analysis of OCAF and Realthunder's work, we can define the architecture for a robust "TNP-Safe" C++ CAD application. This architecture rests on three pillars: The Topological ID Registry, The History Keeper, and The Resolution Engine.4.1 Data Structure Design: The TopologicalIDWe must abandon the use of transient pointers (TopoDS_Shape*) or simple integers for referencing. The fundamental unit of reference must be a custom class, TopologicalID, which encapsulates the provenance of the entity.C++// Architectural definition of a stable Topological ID
struct TopologicalID {
    // The history stack is the "DNA" of the entity.
    // It contains the UUIDs of every operation in the lineage.
    // Example: { "Extrude_UUID", "Fillet_UUID", "Cut_UUID" }
    std::vector<std::string> history_stack;

    // The local index distinguishes siblings generated by the same operation.
    // This index must be deterministic (sorted geometrically).
    int local_index;

    // The type of topology being referenced.
    TopAbs_ShapeEnum type;

    // A geometric signature (hash) used for ambiguity resolution.
    // This might be a hash of the center of mass or bounding box
    // at the moment of creation.
    uint64_t geometric_signature;

    // Equality operator for hash map lookups
    bool operator==(const TopologicalID& other) const {
        return history_stack == other.history_stack && 
               local_index == other.local_index &&
               type == other.type;
    }
};
This structure allows the application to distinguish between "Face A from Extrude 1" and "Face A from Extrude 1, modified by Cut 1". The geometric_signature provides the necessary data for the heuristic fallback mechanisms discussed in Section 6.124.2 The History Keeper (Journaling System)The History Keeper is a subsystem that sits between the application logic and the geometric kernel. Its responsibility is to intercept every modeling command and record the mapping between input IDs and output IDs.For a boolean operation like Cut, the History Keeper must:Accept the TopologicalID of the tool and the target bodies.Perform the boolean operation using the kernel (e.g., BRepAlgoAPI_Cut).Iterate through the Generated and Modified lists provided by the kernel.Update the ID registry:If Face A is modified into Face A', create a new ID for A' that appends the Cut Operation's UUID to Face A's history stack.If Face A generates new Face B, create a new ID for B derived from A.This creates a continuous, unbroken chain of custody for every topological element in the model.104.3 The Resolution EngineThe Resolution Engine is the consumer of these IDs. When a Sketch needs to attach to a face, it queries the Resolution Engine with a TopologicalID. The engine's job is to look at the current state of the model and find the face that best matches that ID.This involves a multi-stage lookup:Direct History Match: Does a face exist in the current model with this exact ID history?Ambiguity Resolution: If the history points to multiple faces (Split), use the geometric_signature to pick the best candidate.Semantic Fallback: If history fails, use semantic rules (e.g., "Top Face") if they were recorded.55. Detailed C++ Implementation StrategiesImplementing this architecture requires rigorous C++ coding standards and careful memory management. The sheer volume of topological entities (a complex model may have tens of thousands of faces) demands efficient data structures.5.1 Efficient Hashing and MapsString comparisons for the history_stack are computationally expensive. In a production C++ implementation, these strings should be hashed into 64-bit or 128-bit integers using high-performance algorithms like MurmurHash3 or CityHash.The registry should use std::unordered_map (Hash Map) rather than std::map (Red-Black Tree) to ensure O(1) lookup times.C++// Custom Hasher for TopologicalID
struct TopologicalIDHasher {
    std::size_t operator()(const TopologicalID& id) const {
        std::size_t seed = 0;
        // Combine hashes of the history stack
        for (const auto& uuid : id.history_stack) {
            hash_combine(seed, std::hash<std::string>{}(uuid));
        }
        hash_combine(seed, std::hash<int>{}(id.local_index));
        hash_combine(seed, std::hash<int>{}(static_cast<int>(id.type)));
        return seed;
    }
};

// The Registry Type
using ElementMap = std::unordered_map<TopologicalID, TopoDS_Shape, TopologicalIDHasher>;
Memory Consideration: TopoDS_Shape objects in OpenCASCADE are handle-based (smart pointers) that share underlying data (TopoDS_TShape). Storing them in maps is lightweight. However, the TopologicalID itself can grow large if the history is deep. Implementation should consider "interning" the history strings or using a Flyweight pattern to reduce memory footprint.255.2 Wrapping BRepAlgoAPIDirectly calling BRepAlgoAPI_Cut is forbidden in this architecture. Instead, we define a wrapper function or class.C++void PerformCut(ModelObject& target, const ModelObject& tool, const std::string& op_uuid) {
    // 1. Setup the Kernel Algorithm
    BRepAlgoAPI_Cut algo(target.Shape(), tool.Shape());
    algo.Build();
    if (!algo.IsDone()) throw std::runtime_error("Boolean Cut Failed");

    TopoDS_Shape result_shape = algo.Shape();
    ElementMap new_map;

    // 2. Process History for Target Faces
    for (auto& entry : target.GetElementMap()) {
        const TopologicalID& old_id = entry.first;
        const TopoDS_Shape& old_shape = entry.second;

        // Check Modifications
        const TopTools_ListOfShape& mods = algo.Modified(old_shape);
        if (!mods.IsEmpty()) {
            int sub_index = 0;
            for (TopExp_Explorer expl(mods.First(), TopAbs_FACE); expl.More(); expl.Next()) {
                TopologicalID new_id = old_id;
                new_id.history_stack.push_back(op_uuid);
                new_id.local_index = sub_index++; // Note: Needs geometric sorting for stability!
                new_map[new_id] = expl.Current();
            }
        }
        
        // Check Generations (e.g., side faces of the cut)
        //... (Similar logic for Generated)
    }

    // 3. Update the ModelObject
    target.SetShape(result_shape);
    target.SetElementMap(new_map);
}
Critical Detail: In the code above, sub_index assignment is simplistic. In a robust implementation, the list of modified shapes returned by algo.Modified() must be sorted deterministically (e.g., by the X coordinate of their center of mass). This ensures that if the operation is re-run, the same face gets the same index, even if the kernel's internal iteration order changes.125.3 Serialization and PersistenceThe TopologicalID must be saved to disk. When the user saves the CAD file, the references in sketches (e.g., "Constraint to FaceID_X") must be serialized.Format: JSON or binary formats are suitable.Stability: Do not serialize the TopoDS_Shape pointer. Serialize the TopologicalID.Restoration: On file load, the parametric history is replayed. The TopologicalIDs are regenerated by the history keeper. The constraints then use the serialized IDs to re-bind to the newly generated geometry.56. Advanced Heuristics and Ambiguity ResolutionDespite the best history tracking, ambiguity is inevitable. A single face may be split into two disjoint faces by a cut. Which one is "Face A"? Both share the exact same history stack. This is where the Resolution Engine employs advanced heuristics.6.1 Geometric Signatures and Spatial HashingWhen a split occurs (1-to-Many evolution), the resolution engine must decide which child inherits the reference.Heuristic: "The child that overlaps most with the parent."Implementation:Store the bounding box or a discrete voxel grid of the original face in the TopologicalID.During resolution, if multiple candidates are found, compute the intersection volume of each candidate with the stored bounding box.Select the candidate with the highest overlap ratio.For vertex matching, a simple Euclidean distance check against the original position (stored in the ID) is usually sufficient. For edges, checking the overlap of the curve geometry is required.136.2 Semantic Selection ("Top Face" Logic)Often, the user's intent was not to select "Face #42" but "The Top Face". A robust system allows the UI to capture this intent.Semantic Constraints: When a selection is made, analyze the face. Is it planar? Is its normal parallel to Z? Is it the highest face in the bounding box?Storage: Store these attributes as a SemanticRule object alongside the TopologicalID.Fallback: If the specific ID cannot be resolved (e.g., the face was deleted), the resolution engine runs a query: Find(Face WHERE Normal=(0,0,1) AND Z=Max). This allows the model to self-heal even when the original geometry is completely replaced.56.3 Graph Isomorphism for ValidationIn extreme cases, the system can use Local Graph Isomorphism to validate a match.Concept: "The face I am looking for was connected to a Cylinder and a Plane."Check: Inspect the neighbors of the candidate face. Do they match the expected types?Algorithms: The VF2 algorithm or Ullmann's algorithm can be adapted for B-Rep graphs. Given that B-Rep graphs are generally sparse and planar, these checks are performant enough for local neighborhoods (though not for the global model).157. Performance Optimization and Data StructuresTracking history for every face in a model with thousands of operations is resource-intensive. Optimization is critical for a responsive application.7.1 Lazy EvaluationWe do not need to construct the full TopologicalID map for every intermediate step of the model if those steps are never referenced.Strategy: The History Keeper should store the delta (the journal of changes) for each operation but defer the construction of the full ID maps.Execution: Only when a feature (like a Sketch) requests Resolve(ID), does the system traverse the journal from the source operation to the current tip, constructing the necessary chains. This reduces the overhead from $O(N \times M)$ (Operations $\times$ Faces) to $O(K \times D)$ (Queries $\times$ History Depth).67.2 Spatial IndexingTo speed up the geometric heuristic checks (e.g., finding overlapping faces), the faces in the current model should be indexed in a spatial data structure like an R-Tree or Octree.Benefit: This allows the resolution engine to quickly discard candidates that are geometrically far from the original entity, avoiding expensive exact intersection tests.308. Future Paradigms: AI and Implicit ModelingThe TNP is an artifact of the B-Rep data structure. Emerging technologies may render it obsolete or solve it through radically different means.8.1 Machine Learning and BRepNetRecent research into "BRepNet" and similar neural architectures suggests that AI models can learn to segment and identify topological features.Application: An AI model could be trained to recognize "The Top Face" or "The Mounting Hole" regardless of the underlying topology numbering.Potential: A "Neural Resolution Engine" could replace the hard-coded heuristics, using pattern recognition to infer which face corresponds to the user's intent after a complex edit.328.2 Implicit Modeling and FieldsImplicit modeling (using Signed Distance Fields or SDFs) avoids the TNP entirely by not using B-Rep boundaries during the modeling process.Mechanism: Geometry is defined by math functions. A boolean union is a max() function, not a graph restitching operation.Relevance: While implicit modeling is powerful (used in tools like nTopology), it currently lacks the precision required for traditional mechanical CAD. However, hybrid kernels that generate B-Reps from Implicit fields deterministically could offer a path forward, as the generation process would be mathematically stable.34ConclusionThe Topological Naming Problem remains one of the most sophisticated challenges in computational geometry. For the C++ CAD architect, "solving" it is not about finding a magic algorithm that works 100% of the time—such an algorithm is mathematically impossible due to the ambiguity of user intent in the face of topological change.Instead, the solution lies in a multi-layered Defense-in-Depth architecture:Layer 1 (The Bedrock): A rigorous History-Based Naming system that tracks the provenance of every entity through every kernel operation using a wrapper layer.Layer 2 (The Solver): A Resolution Engine that handles the complex logic of splits, merges, and modifications using geometric signatures and spatial hashing.Layer 3 (The Safety Net): Semantic Selection rules that capture high-level design intent to guide the solver when low-level tracking fails.Layer 4 (The User Interface): A transparent error-handling system that, rather than crashing or guessing silently, visualizes the broken reference and allows the user to re-bind it with minimal friction.By meticulously implementing these layers using modern C++ data structures (std::unordered_map, std::vector) and leveraging the history-tracking capabilities of kernels like OpenCASCADE (via TNaming), a developer can build a parametric CAD application that offers the stability and robustness expected by professional engineers. The code samples and architectural patterns provided in this report serve as the blueprint for such an undertaking.
## Topological Naming Research Report

Summary of current understanding and target approach for ElementMap and the topological naming problem.

### Findings
- OCCT history API (`IsDeleted`, `Modified`, `Generated`) provides old→new sub-shape lineage; algorithms also expose `TopTools_History`.
- OCAF TNaming uses `TNaming_UsedShapes` + per-label `TNaming_NamedShape` with `TNaming_Builder` to record evolution; `TNaming_Selector` resolves persistent selections.
- Raw `TopoDS_Shape` handles are unstable across rebuilds; geometric fingerprints (bbox center/diagonal, area/length/volume, orientation) improve re-identification when history is ambiguous.
- Shape hashing via `TopTools_ShapeMapHasher` keys on TShape+Location+Orientation; different locations/orientations do not alias, but clones without location do.

### Recommended Strategy
- Maintain `ElementMap` keyed by `ElementId` (hierarchical UUID paths) storing `TopoDS_Shape`, kind, operation ID, sources, and descriptors.
- On each modeling op: consult OCCT history; update modified shapes with best match; create IDs for generated shapes; drop deleted ones; attach descriptors for fallback matching.
- Persist mappings in document/scene to serve parametric regeneration, selection resolution, and constraint anchoring.
- For user-stored selections (sketch-on-face, fillet targets), prefer `TNaming_Selector` or equivalent selector leveraging ElementMap + descriptors.

### Open Questions / Next Steps
- Decide on OCAF adoption boundary: full TNaming in document vs bespoke ElementMap with lightweight persistence.
- Define descriptor enrichment (adjacency signature, curvature stats) for ambiguous matches (splits/merges).
- Add prototype coverage for splits (plane cut), merges (boolean fuse), and generated edges/vertices (fillet/chamfer).

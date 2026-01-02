#pragma once

#include <atomic>
#include <cmath>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <NCollection_DataMap.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <gp_Pnt.hxx>

namespace onecad::kernel::elementmap {

enum class ElementKind {
    Body,
    Face,
    Edge,
    Vertex,
    Unknown
};

struct ElementId {
    std::string value;

    std::string toString() const { return value; }
    static ElementId From(const std::string& v) { return ElementId{v}; }
};

struct ElementDescriptor {
    TopAbs_ShapeEnum shapeType{TopAbs_SHAPE};
    gp_Pnt center{0.0, 0.0, 0.0};
    double size{0.0};      // Diagonal length of the bounding box
    double magnitude{0.0}; // Area/length/volume approximation
};

struct Entry {
    ElementId id;
    ElementKind kind{ElementKind::Unknown};
    TopoDS_Shape shape;
    ElementDescriptor descriptor;
    std::string opId;
    std::vector<ElementId> sources;
};

class ElementMap {
public:
    void registerElement(const ElementId& id, ElementKind kind, const TopoDS_Shape& shape,
                         const std::string& opId = {}, std::vector<ElementId> sources = {});

    const Entry* find(const ElementId& id) const;
    Entry* find(const ElementId& id);
    bool contains(const ElementId& id) const;
    std::vector<ElementId> ids() const;

    // Updates tracked shapes using the history from a boolean operation. Returns IDs that were deleted.
    std::vector<ElementId> update(BRepAlgoAPI_BooleanOperation& algo, const std::string& opId);

private:
    ElementDescriptor computeDescriptor(const TopoDS_Shape& shape) const;
    double score(const ElementDescriptor& target, const ElementDescriptor& candidate) const;
    TopoDS_Shape pickBestShape(const TopTools_ListOfShape& list, const ElementDescriptor& target) const;
    ElementKind inferKind(const TopoDS_Shape& shape, ElementKind fallback) const;
    ElementId makeAutoChildId(const ElementId& parent, ElementKind kind);
    void bindShape(const TopoDS_Shape& shape, const ElementId& id);
    void unbindShape(const TopoDS_Shape& shape);

    std::unordered_map<std::string, Entry> entries_;
    NCollection_DataMap<TopoDS_Shape, std::string, TopTools_ShapeMapHasher> shapeToId_;
    std::atomic_uint64_t counter_{0};
};

// --- Inline implementation -------------------------------------------------

inline void ElementMap::registerElement(const ElementId& id, ElementKind kind, const TopoDS_Shape& shape,
                                        const std::string& opId, std::vector<ElementId> sources) {
    Entry entry{ id, kind, shape, computeDescriptor(shape), opId, std::move(sources) };
    entries_[id.value] = entry;
    bindShape(shape, id);
}

inline const Entry* ElementMap::find(const ElementId& id) const {
    auto it = entries_.find(id.value);
    if (it == entries_.end()) return nullptr;
    return &it->second;
}

inline Entry* ElementMap::find(const ElementId& id) {
    auto it = entries_.find(id.value);
    if (it == entries_.end()) return nullptr;
    return &it->second;
}

inline bool ElementMap::contains(const ElementId& id) const {
    return entries_.find(id.value) != entries_.end();
}

inline std::vector<ElementId> ElementMap::ids() const {
    std::vector<ElementId> out;
    out.reserve(entries_.size());
    for (auto const& [key, entry] : entries_) {
        out.push_back(entry.id);
    }
    return out;
}

inline ElementDescriptor ElementMap::computeDescriptor(const TopoDS_Shape& shape) const {
    ElementDescriptor desc;
    if (shape.IsNull()) return desc;

    desc.shapeType = shape.ShapeType();

    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    if (!box.IsVoid()) {
        Standard_Real xmin, ymin, zmin, xmax, ymax, zmax;
        box.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        desc.center = gp_Pnt((xmin + xmax) * 0.5, (ymin + ymax) * 0.5, (zmin + zmax) * 0.5);
        const double dx = xmax - xmin;
        const double dy = ymax - ymin;
        const double dz = zmax - zmin;
        desc.size = std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    GProp_GProps props;
    switch (desc.shapeType) {
    case TopAbs_FACE:
        BRepGProp::SurfaceProperties(shape, props);
        desc.magnitude = props.Mass();
        break;
    case TopAbs_EDGE:
        BRepGProp::LinearProperties(shape, props);
        desc.magnitude = props.Mass();
        break;
    case TopAbs_SOLID:
    case TopAbs_COMPOUND:
    case TopAbs_COMPSOLID:
        BRepGProp::VolumeProperties(shape, props);
        desc.magnitude = props.Mass();
        break;
    default:
        desc.magnitude = 0.0;
        break;
    }

    return desc;
}

inline double ElementMap::score(const ElementDescriptor& target, const ElementDescriptor& candidate) const {
    const double centerDistance = target.center.Distance(candidate.center);
    const double sizeDiff = std::abs(target.size - candidate.size);
    const double magDiff = std::abs(target.magnitude - candidate.magnitude);
    return centerDistance + 0.1 * sizeDiff + 0.01 * magDiff;
}

inline TopoDS_Shape ElementMap::pickBestShape(const TopTools_ListOfShape& list, const ElementDescriptor& target) const {
    if (list.IsEmpty()) return TopoDS_Shape();

    TopoDS_Shape best;
    double bestScore = std::numeric_limits<double>::max();

    for (TopTools_ListIteratorOfListOfShape it(list); it.More(); it.Next()) {
        const TopoDS_Shape& candidate = it.Value();
        const ElementDescriptor candDesc = computeDescriptor(candidate);
        const double s = score(target, candDesc);
        if (s < bestScore) {
            bestScore = s;
            best = candidate;
        }
    }

    return best;
}

inline ElementKind ElementMap::inferKind(const TopoDS_Shape& shape, ElementKind fallback) const {
    switch (shape.ShapeType()) {
    case TopAbs_FACE: return ElementKind::Face;
    case TopAbs_EDGE: return ElementKind::Edge;
    case TopAbs_VERTEX: return ElementKind::Vertex;
    case TopAbs_SOLID: return ElementKind::Body;
    default: return fallback;
    }
}

inline ElementId ElementMap::makeAutoChildId(const ElementId& parent, ElementKind kind) {
    const auto next = counter_.fetch_add(1) + 1;
    std::string suffix;
    switch (kind) {
    case ElementKind::Face: suffix = "face"; break;
    case ElementKind::Edge: suffix = "edge"; break;
    case ElementKind::Vertex: suffix = "vertex"; break;
    case ElementKind::Body: suffix = "body"; break;
    default: suffix = "elem"; break;
    }
    if (parent.value.empty()) {
        return ElementId{suffix + "-" + std::to_string(next)};
    }
    return ElementId{parent.value + "/" + suffix + "-" + std::to_string(next)};
}

inline void ElementMap::bindShape(const TopoDS_Shape& shape, const ElementId& id) {
    if (shape.IsNull()) return;
    if (shapeToId_.IsBound(shape)) {
        shapeToId_.UnBind(shape);
    }
    shapeToId_.Bind(shape, id.value);
}

inline void ElementMap::unbindShape(const TopoDS_Shape& shape) {
    if (shape.IsNull()) return;
    if (shapeToId_.IsBound(shape)) {
        shapeToId_.UnBind(shape);
    }
}

inline std::vector<ElementId> ElementMap::update(BRepAlgoAPI_BooleanOperation& algo, const std::string& opId) {
    std::vector<ElementId> deleted;

    // Track shapes to erase after iteration to avoid invalidating iterators.
    std::vector<std::pair<std::string, TopoDS_Shape>> toErase;

    for (auto& [key, entry] : entries_) {
        const TopoDS_Shape oldShape = entry.shape;

        if (algo.IsDeleted(oldShape)) {
            deleted.push_back(entry.id);
            toErase.emplace_back(key, oldShape);
            continue;
        }

        const TopTools_ListOfShape& modified = algo.Modified(oldShape);
        if (!modified.IsEmpty()) {
            TopoDS_Shape best = pickBestShape(modified, entry.descriptor);
            if (!best.IsNull()) {
                unbindShape(oldShape);
                entry.shape = best;
                entry.descriptor = computeDescriptor(best);
                entry.opId = opId;
                bindShape(entry.shape, entry.id);
            }
        }

        const TopTools_ListOfShape& generated = algo.Generated(oldShape);
        if (!generated.IsEmpty()) {
            for (TopTools_ListIteratorOfListOfShape git(generated); git.More(); git.Next()) {
                const TopoDS_Shape& child = git.Value();
                ElementKind childKind = inferKind(child, entry.kind);
                ElementId childId = makeAutoChildId(entry.id, childKind);
                registerElement(childId, childKind, child, opId, {entry.id});
            }
        }
    }

    for (auto const& [idKey, shape] : toErase) {
        unbindShape(shape);
        entries_.erase(idKey);
    }

    return deleted;
}

} // namespace onecad::kernel::elementmap

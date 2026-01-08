/**
 * @file PushPullTool.cpp
 */
#include "PushPullTool.h"

#include "../viewport/Viewport.h"
#include "../../app/commands/ModifyBodyCommand.h"
#include "../../app/commands/CommandProcessor.h"
#include "../../app/document/Document.h"
#include "../../core/modeling/BooleanOperation.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <TopoDS.hxx>
#include <TopAbs_Orientation.hxx>
#include <gp_Vec.hxx>
#include <gp_Pln.hxx>

#include <QVector3D>

#include <cmath>

namespace onecad::ui::tools {

namespace {
constexpr double kMinOffset = 1e-3;
} // namespace

PushPullTool::PushPullTool(Viewport* viewport, app::Document* document)
    : viewport_(viewport), document_(document) {
}

void PushPullTool::setDocument(app::Document* document) {
    document_ = document;
}

void PushPullTool::setCommandProcessor(app::commands::CommandProcessor* processor) {
    commandProcessor_ = processor;
}

void PushPullTool::begin(const app::selection::SelectionItem& selection) {
    dragging_ = false;
    currentOffset_ = 0.0;
    booleanMode_ = app::BooleanMode::Add;
    active_ = prepareInput(selection);
    if (!active_) {
        clearPreview();
    }
}

void PushPullTool::cancel() {
    clearPreview();
    active_ = false;
    dragging_ = false;
}

bool PushPullTool::handleMousePress(const QPoint& screenPos, Qt::MouseButton button) {
    if (!active_ || button != Qt::LeftButton) {
        return false;
    }
    dragStart_ = screenPos;
    currentOffset_ = 0.0;
    dragging_ = true;
    return true;
}

bool PushPullTool::handleMouseMove(const QPoint& screenPos) {
    if (!active_ || !dragging_) {
        return false;
    }

    const double pixelScale = viewport_ ? viewport_->pixelScale() : 1.0;
    const double deltaY = static_cast<double>(screenPos.y() - dragStart_.y());
    const double offset = -deltaY * pixelScale;

    if (std::abs(offset - currentOffset_) < 1e-6) {
        return true;
    }

    currentOffset_ = offset;
    detectBooleanMode(offset);

    if (std::abs(currentOffset_) < kMinOffset) {
        clearPreview();
        return true;
    }

    updatePreview(currentOffset_);
    return true;
}

bool PushPullTool::handleMouseRelease(const QPoint& screenPos, Qt::MouseButton button) {
    if (!active_ || !dragging_ || button != Qt::LeftButton) {
        return false;
    }

    const double pixelScale = viewport_ ? viewport_->pixelScale() : 1.0;
    const double deltaY = static_cast<double>(screenPos.y() - dragStart_.y());
    const double offset = -deltaY * pixelScale;
    dragging_ = false;

    if (std::abs(offset) < kMinOffset) {
        clearPreview();
        return true;
    }

    detectBooleanMode(offset);

    TopoDS_Shape toolShape = buildOffsetShape(offset);
    if (toolShape.IsNull()) {
        clearPreview();
        return true;
    }

    // Apply boolean operation
    if (document_ && commandProcessor_ && !targetBodyId_.empty() && !targetShape_.IsNull()) {
        TopoDS_Shape resultShape = core::modeling::BooleanOperation::perform(
            toolShape, targetShape_, booleanMode_);

        if (!resultShape.IsNull()) {
            auto command = std::make_unique<app::commands::ModifyBodyCommand>(
                document_, targetBodyId_, resultShape);
            commandProcessor_->execute(std::move(command));
        }
    }

    clearPreview();
    currentOffset_ = 0.0;
    active_ = false;
    return true;
}

bool PushPullTool::prepareInput(const app::selection::SelectionItem& selection) {
    if (!document_) {
        return false;
    }

    targetBodyId_.clear();
    targetShape_.Nullify();
    selectedFace_.Nullify();

    if (selection.kind != app::selection::SelectionKind::Face) {
        return false;
    }

    targetBodyId_ = selection.id.ownerId;
    const TopoDS_Shape* bodyShape = document_->getBodyShape(targetBodyId_);
    if (!bodyShape || bodyShape->IsNull()) {
        return false;
    }
    targetShape_ = *bodyShape;

    // Find the face from elementMap
    const auto* entry = document_->elementMap().find(
        kernel::elementmap::ElementId{selection.id.elementId});
    if (!entry || entry->kind != kernel::elementmap::ElementKind::Face || entry->shape.IsNull()) {
        return false;
    }

    selectedFace_ = TopoDS::Face(entry->shape);

    // Check if face is planar
    if (!isPlanarFace(selectedFace_)) {
        return false;
    }

    // Extract normal and center
    BRepAdaptor_Surface surface(selectedFace_, true);
    gp_Pln plane = surface.Plane();
    faceNormal_ = plane.Axis().Direction();
    if (selectedFace_.Orientation() == TopAbs_REVERSED) {
        faceNormal_.Reverse();
    }

    // Calculate face center
    GProp_GProps props;
    BRepGProp::SurfaceProperties(selectedFace_, props);
    if (props.Mass() > 0.0) {
        faceCenter_ = props.CentreOfMass();
    } else {
        faceCenter_ = gp_Pnt(0, 0, 0);
    }

    return true;
}

bool PushPullTool::isPlanarFace(const TopoDS_Face& face) const {
    if (face.IsNull()) {
        return false;
    }

    BRepAdaptor_Surface surface(face, true);
    return surface.GetType() == GeomAbs_Plane;
}

void PushPullTool::detectBooleanMode(double offset) {
    // Pull out (positive along normal) = Add
    // Push in (negative along normal) = Cut
    booleanMode_ = (offset >= 0) ? app::BooleanMode::Add : app::BooleanMode::Cut;
}

void PushPullTool::updatePreview(double offset) {
    if (!viewport_) {
        return;
    }

    TopoDS_Shape toolShape = buildOffsetShape(offset);
    if (toolShape.IsNull()) {
        clearPreview();
        return;
    }

    previewElementMap_.clear();
    render::SceneMeshStore::Mesh mesh = previewTessellator_.buildMesh(
        "pushpull_preview", toolShape, previewElementMap_);
    viewport_->setModelPreviewMeshes({std::move(mesh)});
}

void PushPullTool::clearPreview() {
    if (viewport_) {
        viewport_->clearModelPreviewMeshes();
    }
}

TopoDS_Shape PushPullTool::buildOffsetShape(double offset) const {
    if (selectedFace_.IsNull() || std::abs(offset) < kMinOffset) {
        return TopoDS_Shape();
    }

    try {
        gp_Vec prismVec(faceNormal_.X() * offset,
                        faceNormal_.Y() * offset,
                        faceNormal_.Z() * offset);

        BRepPrimAPI_MakePrism prism(selectedFace_, prismVec, true);
        if (prism.IsDone()) {
            return prism.Shape();
        }
    } catch (...) {
        // Prism creation failed
    }

    return TopoDS_Shape();
}

std::optional<ModelingTool::Indicator> PushPullTool::indicator() const {
    if (!active_ || selectedFace_.IsNull()) {
        return std::nullopt;
    }

    const double offset = dragging_ ? currentOffset_ : 0.0;
    const gp_Pnt originPoint = faceCenter_.Translated(gp_Vec(
        faceNormal_.X() * offset,
        faceNormal_.Y() * offset,
        faceNormal_.Z() * offset));

    QVector3D dir(faceNormal_.X(), faceNormal_.Y(), faceNormal_.Z());
    if (dragging_ && currentOffset_ < 0.0) {
        dir = -dir;
    }

    Indicator ind;
    ind.origin = QVector3D(originPoint.X(), originPoint.Y(), originPoint.Z());
    ind.direction = dir;
    ind.distance = std::abs(currentOffset_);
    ind.showDistance = dragging_ && std::abs(currentOffset_) >= kMinOffset;
    ind.booleanMode = booleanMode_;
    return ind;
}

} // namespace onecad::ui::tools

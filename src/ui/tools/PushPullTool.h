/**
 * @file PushPullTool.h
 * @brief Push/pull tool for face extrusion with automatic boolean operations.
 */
#ifndef ONECAD_UI_TOOLS_PUSHPULLTOOL_H
#define ONECAD_UI_TOOLS_PUSHPULLTOOL_H

#include "ModelingTool.h"

#include "../../render/tessellation/TessellationCache.h"
#include "../../kernel/elementmap/ElementMap.h"

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <string>

namespace onecad::app {
class Document;
}
namespace onecad::app::commands {
class CommandProcessor;
}

namespace onecad::ui {
class Viewport;
}

namespace onecad::ui::tools {

class PushPullTool : public ModelingTool {
public:
    explicit PushPullTool(Viewport* viewport, app::Document* document);

    void setDocument(app::Document* document);
    void setCommandProcessor(app::commands::CommandProcessor* processor);

    void begin(const app::selection::SelectionItem& selection) override;
    void cancel() override;
    bool isActive() const override { return active_; }
    bool isDragging() const override { return dragging_; }

    bool handleMousePress(const QPoint& screenPos, Qt::MouseButton button) override;
    bool handleMouseMove(const QPoint& screenPos) override;
    bool handleMouseRelease(const QPoint& screenPos, Qt::MouseButton button) override;
    std::optional<Indicator> indicator() const override;

private:
    bool prepareInput(const app::selection::SelectionItem& selection);
    bool isPlanarFace(const TopoDS_Face& face) const;
    void updatePreview(double offset);
    void clearPreview();
    TopoDS_Shape buildOffsetShape(double offset) const;
    void detectBooleanMode(double offset);

    Viewport* viewport_ = nullptr;
    app::Document* document_ = nullptr;
    app::commands::CommandProcessor* commandProcessor_ = nullptr;

    std::string targetBodyId_;
    TopoDS_Shape targetShape_;
    TopoDS_Face selectedFace_;
    gp_Pnt faceCenter_;
    gp_Dir faceNormal_;

    bool active_ = false;
    bool dragging_ = false;
    QPoint dragStart_;
    double currentOffset_ = 0.0;
    app::BooleanMode booleanMode_ = app::BooleanMode::Add;

    render::TessellationCache previewTessellator_;
    kernel::elementmap::ElementMap previewElementMap_;
};

} // namespace onecad::ui::tools

#endif // ONECAD_UI_TOOLS_PUSHPULLTOOL_H

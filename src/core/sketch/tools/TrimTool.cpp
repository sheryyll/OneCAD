#include "TrimTool.h"
#include "../Sketch.h"
#include "../SketchRenderer.h"
#include "../SketchEntity.h"
#include "../SketchLine.h"
#include "../SketchArc.h"
#include "../SketchCircle.h"
#include "../SketchEllipse.h"
#include "../SketchPoint.h"

#include <gp_Pnt2d.hxx>

#include <algorithm>
#include <cmath>

namespace onecad::core::sketch::tools {

TrimTool::TrimTool() {
    // Trim tool is always in Idle state, ready to click
    state_ = State::Idle;
}

void TrimTool::onMousePress(const Vec2d& pos, Qt::MouseButton button) {
    if (button == Qt::RightButton) {
        cancel();
        return;
    }

    if (button != Qt::LeftButton || !sketch_) {
        return;
    }

    entityDeleted_ = false;

    // Find and delete entity at click position
    EntityID entityId = findEntityAtPosition(pos);

    if (!entityId.empty()) {
        // Don't delete points directly - they may be referenced
        auto* entity = sketch_->getEntity(entityId);
        if (entity && entity->type() != EntityType::Point) {
            if (sketch_->removeEntity(entityId)) {
                entityDeleted_ = true;
                hoverEntityId_.clear();
            }
        }
    }
}

void TrimTool::onMouseMove(const Vec2d& pos) {
    // Update hover entity for visual feedback
    hoverEntityId_ = findEntityAtPosition(pos);
}

void TrimTool::onMouseRelease(const Vec2d& /*pos*/, Qt::MouseButton /*button*/) {
    // Nothing on release
}

void TrimTool::onKeyPress(Qt::Key key) {
    if (key == Qt::Key_Escape) {
        cancel();
    }
}

void TrimTool::cancel() {
    hoverEntityId_.clear();
    entityDeleted_ = false;
}

void TrimTool::render(SketchRenderer& renderer) {
    // Highlight the entity that will be deleted on click
    if (!hoverEntityId_.empty()) {
        renderer.setHoverEntity(hoverEntityId_);
    } else {
        renderer.setHoverEntity({});
    }
    renderer.clearPreview();
    renderer.clearPreviewDimensions();
}

EntityID TrimTool::findEntityAtPosition(const Vec2d& pos) const {
    if (!sketch_) {
        return {};
    }

    constexpr double PICK_TOLERANCE = 3.0;  // mm
    gp_Pnt2d testPoint(pos.x, pos.y);

    // Search through entities (skip points, we only trim lines/arcs/circles)
    for (const auto& entity : sketch_->getAllEntities()) {
        if (entity->type() == EntityType::Point) {
            continue;
        }

        // For lines, check directly
        if (entity->type() == EntityType::Line) {
            auto* line = static_cast<const SketchLine*>(entity.get());
            auto* startPt = sketch_->getEntityAs<SketchPoint>(line->startPointId());
            auto* endPt = sketch_->getEntityAs<SketchPoint>(line->endPointId());

            if (startPt && endPt) {
                gp_Pnt2d p1 = startPt->position();
                gp_Pnt2d p2 = endPt->position();

                // Point-to-line-segment distance
                double dx = p2.X() - p1.X();
                double dy = p2.Y() - p1.Y();
                double lenSq = dx * dx + dy * dy;

                if (lenSq > 1e-10) {
                    double t = ((testPoint.X() - p1.X()) * dx +
                                (testPoint.Y() - p1.Y()) * dy) / lenSq;
                    t = std::max(0.0, std::min(1.0, t));

                    double closestX = p1.X() + t * dx;
                    double closestY = p1.Y() + t * dy;
                    double dist = std::sqrt((testPoint.X() - closestX) * (testPoint.X() - closestX) +
                                            (testPoint.Y() - closestY) * (testPoint.Y() - closestY));

                    if (dist < PICK_TOLERANCE) {
                        return entity->id();
                    }
                }
            }
        }
        // For arcs and circles, use isNear if center point is available
        else if (entity->type() == EntityType::Arc) {
            auto* arc = static_cast<const SketchArc*>(entity.get());
            auto* centerPt = sketch_->getEntityAs<SketchPoint>(arc->centerPointId());
            if (centerPt) {
                if (arc->isNearWithCenter(testPoint, centerPt->position(), PICK_TOLERANCE)) {
                    return entity->id();
                }
            }
        }
        else if (entity->type() == EntityType::Circle) {
            auto* circle = static_cast<const SketchCircle*>(entity.get());
            auto* centerPt = sketch_->getEntityAs<SketchPoint>(circle->centerPointId());
            if (centerPt) {
                if (circle->isNearWithCenter(testPoint, centerPt->position(), PICK_TOLERANCE)) {
                    return entity->id();
                }
            }
        }
        else if (entity->type() == EntityType::Ellipse) {
            auto* ellipse = static_cast<const SketchEllipse*>(entity.get());
            auto* centerPt = sketch_->getEntityAs<SketchPoint>(ellipse->centerPointId());
            if (centerPt) {
                if (ellipse->isNearWithCenter(testPoint, centerPt->position(), PICK_TOLERANCE)) {
                    return entity->id();
                }
            }
        }
    }

    return {};
}

} // namespace onecad::core::sketch::tools

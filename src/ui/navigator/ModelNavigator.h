#ifndef ONECAD_UI_NAVIGATOR_MODELNAVIGATOR_H
#define ONECAD_UI_NAVIGATOR_MODELNAVIGATOR_H

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QStackedLayout;
class QToolButton;
class QFrame;

namespace onecad {
namespace ui {

/**
 * @brief Model navigator showing document structure.
 * 
 * Displays hierarchical tree of:
 * - Bodies
 * - Sketches  
 * - Feature History (when parametric mode)
 */
class ModelNavigator : public QWidget {
    Q_OBJECT

public:
    explicit ModelNavigator(QWidget* parent = nullptr);
    ~ModelNavigator() override = default;
    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return m_collapsed; }

signals:
    void itemSelected(const QString& itemId);
    void itemDoubleClicked(const QString& itemId);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    void setupUi();
    void createPlaceholderItems();
    void applyCollapseState();
    void updateOverlayButtonIcon();

    QStackedLayout* m_stack = nullptr;
    QFrame* m_panel = nullptr;
    QToolButton* m_collapseButton = nullptr;
    QToolButton* m_expandButton = nullptr;
    QTreeWidget* m_treeWidget = nullptr;
    QTreeWidgetItem* m_bodiesRoot = nullptr;
    QTreeWidgetItem* m_sketchesRoot = nullptr;
    bool m_collapsed = false;
};

} // namespace ui
} // namespace onecad

#endif // ONECAD_UI_NAVIGATOR_MODELNAVIGATOR_H

#include "ThemeConfig.h"

namespace onecad::ui {

QString toQssColor(const QColor& color) {
    if (!color.isValid()) {
        return {};
    }
    if (color.alpha() < 255) {
        return color.name(QColor::HexArgb);
    }
    return color.name();
}

namespace {

ThemeDefinition makeLightTheme() {
    ThemeDefinition theme;
    theme.id = QStringLiteral("Light");
    theme.displayName = QStringLiteral("Light");
    theme.isDark = false;

    theme.ui.windowBackground = QColor(243, 243, 243);
    theme.ui.widgetBackground = QColor(243, 243, 243);
    theme.ui.widgetText = QColor(51, 51, 51);
    theme.ui.navigatorBackground = QColor(246, 246, 246);
    theme.ui.panelBackground = QColor(246, 246, 246);
    theme.ui.inspectorBackground = QColor(255, 255, 255);
    theme.ui.panelBorder = QColor(204, 204, 204);
    theme.ui.menuBarBackground = QColor(238, 238, 238);
    theme.ui.menuBarText = QColor(51, 51, 51);
    theme.ui.menuBarBorder = QColor(204, 204, 204);
    theme.ui.menuItemSelectedBackground = QColor(204, 232, 255);
    theme.ui.menuItemSelectedText = QColor(0, 0, 0);
    theme.ui.menuBackground = QColor(255, 255, 255);
    theme.ui.menuText = QColor(51, 51, 51);
    theme.ui.menuBorder = QColor(204, 204, 204);
    theme.ui.menuSeparator = QColor(221, 221, 221);
    theme.ui.statusBarBackground = QColor(243, 243, 243);
    theme.ui.statusBarText = QColor(17, 17, 17);
    theme.ui.statusBarBorder = QColor(204, 204, 204);
    theme.ui.dockTitleBackground = QColor(238, 238, 238);
    theme.ui.dockTitleBorder = QColor(204, 204, 204);
    theme.ui.treeBackground = QColor(255, 255, 255);
    theme.ui.treeText = QColor(51, 51, 51);
    theme.ui.treeHoverBackground = QColor(240, 240, 240);
    theme.ui.treeSelectedBackground = QColor(204, 232, 255);
    theme.ui.treeSelectedText = QColor(0, 0, 0);
    theme.ui.sidebarButtonBackground = QColor(246, 246, 246);
    theme.ui.sidebarButtonText = QColor(34, 34, 34);
    theme.ui.sidebarButtonBorder = QColor(204, 204, 204);
    theme.ui.sidebarButtonHoverBackground = QColor(255, 255, 255);
    theme.ui.sidebarButtonHoverBorder = QColor(187, 187, 187);
    theme.ui.sidebarButtonPressedBackground = QColor(204, 232, 255);
    theme.ui.sidebarButtonPressedBorder = QColor(204, 232, 255);
    theme.ui.toolButtonBackground = QColor(246, 246, 246);
    theme.ui.toolButtonText = QColor(51, 51, 51);
    theme.ui.toolButtonBorder = QColor(204, 204, 204);
    theme.ui.toolButtonHoverBackground = QColor(255, 255, 255);
    theme.ui.toolButtonHoverBorder = QColor(187, 187, 187);
    theme.ui.toolButtonPressedBackground = QColor(204, 232, 255);
    theme.ui.toolButtonPressedBorder = QColor(204, 232, 255);
    theme.ui.inspectorHintText = QColor(102, 102, 102);
    theme.ui.inspectorTipText = QColor(58, 143, 58);
    theme.ui.inspectorEntityIdText = QColor(102, 102, 102);
    theme.ui.inspectorSeparator = QColor(204, 204, 204);
    theme.ui.inspectorPlaceholderText = QColor(136, 136, 136);
    theme.ui.scrollbarTrack = QColor(243, 243, 243);
    theme.ui.scrollbarHandle = QColor(193, 193, 193);
    theme.ui.scrollbarHandleHover = QColor(168, 168, 168);

    theme.dimensionEditor.background = QColor(255, 255, 255);
    theme.dimensionEditor.border = QColor(0, 120, 212);
    theme.dimensionEditor.borderFocus = QColor(0, 90, 158);

    theme.constraints.unsatisfiedText = QColor(255, 100, 100);

    theme.status.dofError = QColor(255, 0, 0);
    theme.status.dofOk = QColor(0, 128, 0);
    theme.status.dofWarning = QColor(255, 165, 0);

    theme.navigator.placeholderText = QColor(128, 128, 128);

    theme.viewCube.face = QColor(200, 200, 200, 191);
    theme.viewCube.faceHover = QColor(0, 122, 204, 191);
    theme.viewCube.text = QColor(0, 0, 0);
    theme.viewCube.textHover = QColor(255, 255, 255);
    theme.viewCube.edgeHover = QColor(0, 122, 204, 191);
    theme.viewCube.cornerHover = QColor(0, 122, 204, 191);
    theme.viewCube.faceBorder = QColor(150, 150, 150, 191);
    theme.viewCube.axisX = QColor(220, 80, 80, 191);
    theme.viewCube.axisY = QColor(80, 200, 120, 191);
    theme.viewCube.axisZ = QColor(80, 120, 220, 191);

    theme.viewport.background = QColor(243, 243, 243);
    theme.viewport.grid.major = QColor(200, 200, 200);
    theme.viewport.grid.minor = QColor(225, 225, 225);
    theme.viewport.grid.axisX = QColor(255, 100, 100);
    theme.viewport.grid.axisY = QColor(100, 255, 100);
    theme.viewport.grid.axisZ = QColor(100, 100, 255);
    theme.viewport.planes.xy = QColor(80, 160, 255, 90);
    theme.viewport.planes.xz = QColor(120, 200, 140, 90);
    theme.viewport.planes.yz = QColor(255, 170, 90, 90);
    theme.viewport.planes.labelText = QColor(30, 30, 30);
    theme.viewport.overlay.previewDimensionText = QColor(0, 0, 0);
    theme.viewport.overlay.previewDimensionBackground = QColor(255, 255, 255, 180);
    theme.viewport.overlay.toolIndicator = QColor(32, 96, 255, 230);
    theme.viewport.overlay.toolLabelText = QColor(20, 20, 20);
    theme.viewport.overlay.toolLabelBackground = QColor(255, 255, 255, 200);
    theme.viewport.selection.faceFillHover = QColor(128, 128, 255, 64);
    theme.viewport.selection.faceOutlineHover = QColor(128, 128, 255, 160);
    theme.viewport.selection.faceFillSelected = QColor(32, 96, 255, 90);
    theme.viewport.selection.faceOutlineSelected = QColor(32, 96, 255, 200);
    theme.viewport.selection.edgeHover = theme.viewport.selection.faceOutlineHover;
    theme.viewport.selection.edgeSelected = theme.viewport.selection.faceOutlineSelected;
    theme.viewport.selection.vertexHover = theme.viewport.selection.faceOutlineHover;
    theme.viewport.selection.vertexSelected = theme.viewport.selection.faceOutlineSelected;
    theme.viewport.body.base = QColor(140, 140, 140);
    theme.viewport.body.edge = QColor(80, 80, 80);

    theme.sketch.normalGeometry = QColor(255, 255, 255);
    theme.sketch.constructionGeometry = QColor(128, 128, 230);
    theme.sketch.selectedGeometry = QColor(51, 153, 255);
    theme.sketch.previewGeometry = QColor(179, 179, 179);
    theme.sketch.errorGeometry = QColor(255, 77, 77);
    theme.sketch.constraintIcon = QColor(230, 179, 51);
    theme.sketch.dimensionText = QColor(102, 102, 255);
    theme.sketch.conflictHighlight = QColor(255, 0, 0);
    theme.sketch.fullyConstrained = QColor(26, 102, 230);
    theme.sketch.underConstrained = QColor(255, 128, 0);
    theme.sketch.overConstrained = QColor(255, 0, 0);
    theme.sketch.gridMajor = QColor(77, 77, 77);
    theme.sketch.gridMinor = QColor(38, 38, 38);
    theme.sketch.regionFill = QColor(102, 179, 255);

    return theme;
}

ThemeDefinition makeDarkTheme() {
    ThemeDefinition theme;
    theme.id = QStringLiteral("Dark");
    theme.displayName = QStringLiteral("Dark");
    theme.isDark = true;

    theme.ui.windowBackground = QColor(30, 30, 30);
    theme.ui.widgetBackground = QColor(30, 30, 30);
    theme.ui.widgetText = QColor(204, 204, 204);
    theme.ui.navigatorBackground = QColor(45, 45, 48);
    theme.ui.panelBackground = QColor(45, 45, 48);
    theme.ui.inspectorBackground = QColor(45, 45, 48);
    theme.ui.panelBorder = QColor(62, 62, 66);
    theme.ui.menuBarBackground = QColor(45, 45, 48);
    theme.ui.menuBarText = QColor(204, 204, 204);
    theme.ui.menuBarBorder = QColor(62, 62, 66);
    theme.ui.menuItemSelectedBackground = QColor(9, 71, 113);
    theme.ui.menuItemSelectedText = QColor(204, 204, 204);
    theme.ui.menuBackground = QColor(45, 45, 48);
    theme.ui.menuText = QColor(204, 204, 204);
    theme.ui.menuBorder = QColor(62, 62, 66);
    theme.ui.menuSeparator = QColor(62, 62, 66);
    theme.ui.statusBarBackground = QColor(31, 31, 31);
    theme.ui.statusBarText = QColor(255, 255, 255);
    theme.ui.statusBarBorder = QColor(62, 62, 66);
    theme.ui.dockTitleBackground = QColor(45, 45, 48);
    theme.ui.dockTitleBorder = QColor(62, 62, 66);
    theme.ui.treeBackground = QColor(45, 45, 48);
    theme.ui.treeText = QColor(204, 204, 204);
    theme.ui.treeHoverBackground = QColor(62, 62, 66);
    theme.ui.treeSelectedBackground = QColor(9, 71, 113);
    theme.ui.treeSelectedText = QColor(204, 204, 204);
    theme.ui.sidebarButtonBackground = QColor(45, 45, 48);
    theme.ui.sidebarButtonText = QColor(245, 245, 245);
    theme.ui.sidebarButtonBorder = QColor(62, 62, 66);
    theme.ui.sidebarButtonHoverBackground = QColor(56, 56, 61);
    theme.ui.sidebarButtonHoverBorder = QColor(90, 90, 96);
    theme.ui.sidebarButtonPressedBackground = QColor(0, 122, 204);
    theme.ui.sidebarButtonPressedBorder = QColor(0, 122, 204);
    theme.ui.toolButtonBackground = QColor(45, 45, 48);
    theme.ui.toolButtonText = QColor(204, 204, 204);
    theme.ui.toolButtonBorder = QColor(62, 62, 66);
    theme.ui.toolButtonHoverBackground = QColor(56, 56, 61);
    theme.ui.toolButtonHoverBorder = QColor(90, 90, 96);
    theme.ui.toolButtonPressedBackground = QColor(0, 122, 204);
    theme.ui.toolButtonPressedBorder = QColor(0, 122, 204);
    theme.ui.inspectorHintText = QColor(136, 136, 136);
    theme.ui.inspectorTipText = QColor(107, 159, 107);
    theme.ui.inspectorEntityIdText = QColor(136, 136, 136);
    theme.ui.inspectorSeparator = QColor(62, 62, 66);
    theme.ui.inspectorPlaceholderText = QColor(102, 102, 102);
    theme.ui.scrollbarTrack = QColor(30, 30, 30);
    theme.ui.scrollbarHandle = QColor(66, 66, 66);
    theme.ui.scrollbarHandleHover = QColor(104, 104, 104);

    theme.dimensionEditor.background = QColor(255, 255, 255);
    theme.dimensionEditor.border = QColor(0, 120, 212);
    theme.dimensionEditor.borderFocus = QColor(0, 90, 158);

    theme.constraints.unsatisfiedText = QColor(255, 100, 100);

    theme.status.dofError = QColor(255, 0, 0);
    theme.status.dofOk = QColor(0, 128, 0);
    theme.status.dofWarning = QColor(255, 165, 0);

    theme.navigator.placeholderText = QColor(128, 128, 128);

    theme.viewCube.face = QColor(220, 220, 220, 191);
    theme.viewCube.faceHover = QColor(64, 128, 255, 191);
    theme.viewCube.text = QColor(0, 0, 0);
    theme.viewCube.textHover = QColor(255, 255, 255);
    theme.viewCube.edgeHover = QColor(64, 128, 255, 191);
    theme.viewCube.cornerHover = QColor(64, 128, 255, 191);
    theme.viewCube.faceBorder = QColor(150, 150, 150, 191);
    theme.viewCube.axisX = QColor(220, 80, 80, 191);
    theme.viewCube.axisY = QColor(80, 200, 120, 191);
    theme.viewCube.axisZ = QColor(80, 120, 220, 191);

    theme.viewport.background = QColor(45, 45, 48);
    theme.viewport.grid.major = QColor(80, 80, 80);
    theme.viewport.grid.minor = QColor(50, 50, 50);
    theme.viewport.grid.axisX = QColor(255, 100, 100);
    theme.viewport.grid.axisY = QColor(100, 255, 100);
    theme.viewport.grid.axisZ = QColor(100, 100, 255);
    theme.viewport.planes.xy = QColor(80, 160, 255, 90);
    theme.viewport.planes.xz = QColor(120, 200, 140, 90);
    theme.viewport.planes.yz = QColor(255, 170, 90, 90);
    theme.viewport.planes.labelText = QColor(230, 230, 230);
    theme.viewport.overlay.previewDimensionText = QColor(255, 255, 255);
    theme.viewport.overlay.previewDimensionBackground = QColor(0, 0, 0, 180);
    theme.viewport.overlay.toolIndicator = QColor(96, 160, 255, 230);
    theme.viewport.overlay.toolLabelText = QColor(240, 240, 240);
    theme.viewport.overlay.toolLabelBackground = QColor(0, 0, 0, 180);
    theme.viewport.selection.faceFillHover = QColor(96, 96, 255, 64);
    theme.viewport.selection.faceOutlineHover = QColor(96, 96, 255, 160);
    theme.viewport.selection.faceFillSelected = QColor(64, 128, 255, 90);
    theme.viewport.selection.faceOutlineSelected = QColor(64, 128, 255, 200);
    theme.viewport.selection.edgeHover = theme.viewport.selection.faceOutlineHover;
    theme.viewport.selection.edgeSelected = theme.viewport.selection.faceOutlineSelected;
    theme.viewport.selection.vertexHover = theme.viewport.selection.faceOutlineHover;
    theme.viewport.selection.vertexSelected = theme.viewport.selection.faceOutlineSelected;
    theme.viewport.body.base = QColor(160, 160, 160);
    theme.viewport.body.edge = QColor(210, 210, 210);

    theme.sketch.normalGeometry = QColor(255, 255, 255);
    theme.sketch.constructionGeometry = QColor(128, 128, 230);
    theme.sketch.selectedGeometry = QColor(51, 153, 255);
    theme.sketch.previewGeometry = QColor(179, 179, 179);
    theme.sketch.errorGeometry = QColor(255, 77, 77);
    theme.sketch.constraintIcon = QColor(230, 179, 51);
    theme.sketch.dimensionText = QColor(102, 102, 255);
    theme.sketch.conflictHighlight = QColor(255, 0, 0);
    theme.sketch.fullyConstrained = QColor(26, 102, 230);
    theme.sketch.underConstrained = QColor(255, 128, 0);
    theme.sketch.overConstrained = QColor(255, 0, 0);
    theme.sketch.gridMajor = QColor(77, 77, 77);
    theme.sketch.gridMinor = QColor(38, 38, 38);
    theme.sketch.regionFill = QColor(102, 179, 255);

    return theme;
}

ThemeCatalog buildCatalog() {
    ThemeCatalog catalog;
    catalog.systemLightId = QStringLiteral("Light");
    catalog.systemDarkId = QStringLiteral("Dark");
    catalog.themes = {makeLightTheme(), makeDarkTheme()};
    return catalog;
}

} // namespace

const ThemeCatalog& themeCatalog() {
    static const ThemeCatalog catalog = buildCatalog();
    return catalog;
}

const ThemeDefinition* findTheme(const QString& id) {
    if (id.isEmpty()) {
        return nullptr;
    }
    const auto& catalog = themeCatalog();
    for (const auto& theme : catalog.themes) {
        if (theme.id == id) {
            return &theme;
        }
    }
    return nullptr;
}

const ThemeDefinition& themeById(const QString& id) {
    if (const ThemeDefinition* theme = findTheme(id)) {
        return *theme;
    }
    const auto& catalog = themeCatalog();
    if (!catalog.themes.empty()) {
        return catalog.themes.front();
    }
    static ThemeDefinition fallback;
    return fallback;
}

const ThemeDefinition& systemTheme(bool dark) {
    const auto& catalog = themeCatalog();
    const QString id = dark ? catalog.systemDarkId : catalog.systemLightId;
    return themeById(id);
}

QStringList themeIds() {
    QStringList ids;
    const auto& catalog = themeCatalog();
    ids.reserve(static_cast<int>(catalog.themes.size()));
    for (const auto& theme : catalog.themes) {
        ids.append(theme.id);
    }
    return ids;
}

QStringList themeDisplayNames() {
    QStringList names;
    const auto& catalog = themeCatalog();
    names.reserve(static_cast<int>(catalog.themes.size()));
    for (const auto& theme : catalog.themes) {
        names.append(theme.displayName);
    }
    return names;
}

} // namespace onecad::ui

#include "ConfigManager.hpp"
#include "../helpers/Log.hpp"
#include <hyprutils/path/Path.hpp>
#include <Qt>
#include <qpa/qplatformtheme.h>
#include <qpa/qplatformdialoghelper.h>

std::string CConfigManager::getMainConfigPath() {
    static const auto paths = Hyprutils::Path::findConfig("hyprqt6engine");
    if (paths.first.has_value())
        return paths.first.value();

    Debug::log(LOG, "hyprqt6engine: could not find config in HOME, XDG_CONFIG_HOME, XDG_CONFIG_DIRS or /etc/hypr. Will proceed using defaults.");
    return "";
}

CConfigManager::CConfigManager() : m_config(getMainConfigPath().c_str(), Hyprlang::SConfigOptions{.throwAllErrors = true, .allowMissingConfig = true}) {
    ;
}

void CConfigManager::init() {
    m_config.addConfigValue("theme:color_scheme", Hyprlang::STRING{""});
    m_config.addConfigValue("theme:icon_theme", Hyprlang::STRING{""});
    m_config.addConfigValue("theme:style", Hyprlang::STRING{""});
    m_config.addConfigValue("theme:style_sheet", Hyprlang::STRING{""});
    m_config.addConfigValue("theme:font_fixed", Hyprlang::STRING{"monospace"});
    m_config.addConfigValue("theme:font_fixed_style", Hyprlang::STRING{"Regular"});
    m_config.addConfigValue("theme:font_fixed_size", Hyprlang::INT{11});
    m_config.addConfigValue("theme:font", Hyprlang::STRING{"Sans Serif"});
    m_config.addConfigValue("theme:font_style", Hyprlang::STRING{"Regular"});
    m_config.addConfigValue("theme:font_size", Hyprlang::INT{11});
    m_config.addConfigValue("theme:dialogs", Hyprlang::STRING{"default"});

    m_config.addConfigValue("misc:dialog_have_icons", Hyprlang::INT{1});
    m_config.addConfigValue("misc:menus_have_icons", Hyprlang::INT{1});
    m_config.addConfigValue("misc:single_click_activate", Hyprlang::INT{1});
    m_config.addConfigValue("misc:shortcuts_for_context_menus", Hyprlang::INT{1});
    // https://doc.qt.io/qt-6/qt.html#ToolButtonStyle-enum
    m_config.addConfigValue("misc:tool_button_style", Hyprlang::INT{Qt::ToolButtonFollowStyle});
    // https://github.com/qt/qtbase/blob/255a036f0449e1dfb4e41054376ed89edc00f1e4/src/gui/kernel/qplatformtheme.h#L263
    m_config.addConfigValue("misc:keyboard_scheme", Hyprlang::INT{QPlatformTheme::KdeKeyboardScheme});
    // https://doc.qt.io/qt-6/qdialogbuttonbox.html#ButtonLayout-enum
    m_config.addConfigValue("misc:button_box_layout", Hyprlang::INT{QPlatformDialogHelper::WinLayout});

    m_config.commence();
}

void CConfigManager::refresh() {
  auto result = m_config.parse();
  if (result.error)
    Debug::log(ERR, "Config has errors:\n{}\nProceeding ignoring faulty entries", result.getError());
}
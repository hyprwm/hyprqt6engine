#include "ConfigManager.hpp"
#include "../helpers/Log.hpp"
#include <hyprutils/path/Path.hpp>

static std::string getMainConfigPath() {
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
    m_config.addConfigValue("theme:font_fixed", Hyprlang::STRING{"monospace"});
    m_config.addConfigValue("theme:font_fixed_size", Hyprlang::INT{11});
    m_config.addConfigValue("theme:font", Hyprlang::STRING{"Sans Serif"});
    m_config.addConfigValue("theme:font_size", Hyprlang::INT{11});

    m_config.addConfigValue("misc:single_click_activate", Hyprlang::INT{1});
    m_config.addConfigValue("misc:menus_have_icons", Hyprlang::INT{1});
    m_config.addConfigValue("misc:shortcuts_for_context_menus", Hyprlang::INT{1});

    m_config.commence();

    auto result = m_config.parse();

    if (result.error)
        Debug::log(ERR, "Config has errors:\n{}\nProceeding ignoring faulty entries", result.getError());
}

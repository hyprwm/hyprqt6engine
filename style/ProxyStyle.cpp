#include <QSettings>
#include <QStyleFactory>
#include <string_view>
#include "ProxyStyle.hpp"
#include "config/ConfigManager.hpp"

CHyprProxyStyle::CHyprProxyStyle() {
    CHyprStyle::registerStyleInstance(this);
    g_pConfigManager = makeUnique<CConfigManager>();
    g_pConfigManager->init();

    static auto PSTYLE = g_pConfigManager->getValue<Hyprlang::STRING>("theme:style");
    if (!std::string_view{*PSTYLE}.empty())
        setBaseStyle(QStyleFactory::create(*PSTYLE));
    else
        setBaseStyle(QStyleFactory::create("Fusion"));
}

CHyprProxyStyle::~CHyprProxyStyle() {
    CHyprStyle::unregisterStyleInstance(this);
}

int CHyprProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const {
    static auto PICONS     = g_pConfigManager->getValue<Hyprlang::INT>("misc:menus_have_icons");
    static auto PSCA       = g_pConfigManager->getValue<Hyprlang::INT>("misc:single_click_activate");
    static auto PSHORTCUTS = g_pConfigManager->getValue<Hyprlang::INT>("misc:shortcuts_for_context_menus");

    switch (hint) {
        case QStyle::SH_DialogButtonBox_ButtonsHaveIcons: return *PICONS ? 1 : 0;
        case QStyle::SH_ItemView_ActivateItemOnSingleClick: return *PSCA ? 1 : 0;
        case QStyle::SH_UnderlineShortcut: return *PSHORTCUTS ? 1 : 0;
        default: break;
    }

    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

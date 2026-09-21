// #include <QSettings>
#include <QStyleFactory>
// #include <string_view>
#include "ProxyStyle.hpp"
#include "config/ConfigManager.hpp"

CHyprProxyStyle::CHyprProxyStyle() {
    g_pConfigManager = makeUnique<CConfigManager>();
    g_pConfigManager->init();
    CHyprStyle::registerStyleInstance(this);
    CHyprProxyStyle::reloadSettings();
}

CHyprProxyStyle::~CHyprProxyStyle() {
    CHyprStyle::unregisterStyleInstance(this);
}

void CHyprProxyStyle::reloadSettings() {
    g_pConfigManager->refresh();
    m_dialogButtonsHaveIcons    = !!*g_pConfigManager->getValue<Hyprlang::INT>("misc:dialog_have_icons");
    m_activateItemOnSingleClick = !!*g_pConfigManager->getValue<Hyprlang::INT>("misc:single_click_activate");
    m_underlineShortcut         = !!*g_pConfigManager->getValue<Hyprlang::INT>("misc:shortcuts_for_context_menus");
    QString style               = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:style");

    if (style == "hypr-style"_L1 || !QStyleFactory::keys().contains(style, Qt::CaseInsensitive))
        style = u"Fusion"_s;

    if (style != m_style) {
        setBaseStyle(QStyleFactory::create(style) ?: new QCommonStyle);
        m_style = style;
    }
}

int CHyprProxyStyle::styleHint(QStyle::StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const {
    switch (hint) {
        case QStyle::SH_DialogButtonBox_ButtonsHaveIcons: return m_dialogButtonsHaveIcons;
        case QStyle::SH_ItemView_ActivateItemOnSingleClick: return m_activateItemOnSingleClick;
        case QStyle::SH_UnderlineShortcut: return m_underlineShortcut;
        default: return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
}

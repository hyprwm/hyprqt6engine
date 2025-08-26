#include <QDir>
#include <QLocale>
#include <QLatin1String>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QFile>
#include <QSettings>
#include <QtDebug>
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
#include <KSharedConfig>
#include <KColorScheme>
#endif
#include "common.hpp"

#ifndef HYPR_DATADIR
#define HYPR_DATADIR "/usr/share"
#endif

QSet<CHyprStyle::CStyleInstance*> CHyprStyle::styleInstances;

//
std::optional<QPalette> CHyprStyle::loadColorScheme(const QString& filePath) {
    if (filePath.isEmpty())
        return std::nullopt;

#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
    if (filePath.toLower().endsWith(".colors"))
        return KColorScheme::createApplicationPalette(KSharedConfig::openConfig(filePath));
#endif

    QSettings settings(filePath, QSettings::IniFormat);
    settings.beginGroup("ColorScheme"_L1);
    QStringList activeColors   = settings.value("active_colors"_L1).toStringList();
    QStringList inactiveColors = settings.value("inactive_colors"_L1).toStringList();
    QStringList disabledColors = settings.value("disabled_colors"_L1).toStringList();
    settings.endGroup();

#if (QT_VERSION >= QT_VERSION_CHECK(6, 6, 0))
    if (activeColors.count() == QPalette::Accent)
        activeColors << activeColors.at(QPalette::Highlight);
    if (inactiveColors.count() == QPalette::Accent)
        inactiveColors << inactiveColors.at(QPalette::Highlight);
    if (disabledColors.count() == QPalette::Accent)
        disabledColors << disabledColors.at(QPalette::Highlight);
#endif

    if (activeColors.count() < QPalette::NColorRoles || inactiveColors.count() < QPalette::NColorRoles || disabledColors.count() < QPalette::NColorRoles)
        return std::nullopt;

    QPalette customPalette;
    for (int i = 0; i < QPalette::NColorRoles; i++) {
        QPalette::ColorRole role = QPalette::ColorRole(i);
        customPalette.setColor(QPalette::Active, role, QColor(activeColors.at(i)));
        customPalette.setColor(QPalette::Inactive, role, QColor(inactiveColors.at(i)));
        customPalette.setColor(QPalette::Disabled, role, QColor(disabledColors.at(i)));
    }
    return customPalette;
}

void CHyprStyle::registerStyleInstance(CHyprStyle::CStyleInstance* instance) {
    styleInstances.insert(instance);
}

void CHyprStyle::unregisterStyleInstance(CHyprStyle::CStyleInstance* instance) {
    styleInstances.remove(instance);
}

void CHyprStyle::reloadStyleInstanceSettings() {
    for (auto instance : std::as_const(styleInstances))
        instance->reloadSettings();
}

/*
 * Copyright (c) 2020-2025, Ilya Kotov <forkotov02@ya.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QVariant>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>
#include <QFont>
#include <QDir>
#include <QStandardPaths>
#include <QPalette>
#include <QTimer>
#include <QIcon>
#include <QRegularExpression>
#include <QMimeDatabase>
#ifdef QT_WIDGETS_LIB
#include <QStyle>
#include <QStyleFactory>
#include <QApplication>
#include <QWidget>
#if QT_CONFIG(graphicsview)
#include <QGraphicsScene>
#endif
#include <private/qapplication_p.h>
#endif
#include <QFile>
#include <QFileSystemWatcher>
#ifdef QT_QUICKCONTROLS2_LIB
#include <QQuickStyle>
#endif

#include <QStringList>
#include <qpa/qplatformthemefactory_p.h>
#include <qpa/qwindowsysteminterface.h>

#ifdef KF_ICONTHEMES_LIB
#include <KIconEngine>
#include <KIconLoader>
#endif

#include <string_view>

#include "PlatformTheme.hpp"
#include "config/ConfigManager.hpp"
#include "helpers/MiscFunctions.hpp"
#include "helpers/Log.hpp"
#include "common.hpp"

//QT_QPA_PLATFORMTHEME=hyprqt6engine

CHyprPlatformTheme::CHyprPlatformTheme() : m_fixedFont(*QGenericUnixTheme::font(QPlatformTheme::FixedFont)), m_font(*QGenericUnixTheme::font(QPlatformTheme::SystemFont)) {
    g_pConfigManager = makeUnique<CConfigManager>();
    g_pConfigManager->init();

    static auto PCOLORSCHEME = g_pConfigManager->getValue<Hyprlang::STRING>("theme:color_scheme");

    if (QGuiApplication::desktopSettingsAware()) {
        applySettings();
        QMetaObject::invokeMethod(this, &CHyprPlatformTheme::applySettings, Qt::QueuedConnection);
        //must be applied before Q_COREAPP_STARTUP_FUNCTION execution
        if (isKColorScheme(*PCOLORSCHEME))
            qApp->setProperty("KDE_COLOR_SCHEME_PATH", *PCOLORSCHEME);
#if defined QT_WIDGETS_LIB && defined QT_QUICKCONTROLS2_LIB
        if (hasWidgets())
            //don't override the value explicitly set by the user
            if (QQuickStyle::name().isEmpty() || QQuickStyle::name() == QLatin1String("Fusion"))
                QQuickStyle::setStyle(QLatin1String("org.kde.desktop"));
#endif
    }
    QCoreApplication::instance()->installEventFilter(this);
}

bool CHyprPlatformTheme::usePlatformNativeDialog(DialogType type) const {
    return m_theme ? m_theme->usePlatformNativeDialog(type) : QGenericUnixTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper* CHyprPlatformTheme::createPlatformDialogHelper(DialogType type) const {
    return m_theme ? m_theme->createPlatformDialogHelper(type) : QGenericUnixTheme::createPlatformDialogHelper(type);
}

const QPalette* CHyprPlatformTheme::palette(QPlatformTheme::Palette type) const {
    if (type == QPlatformTheme::SystemPalette && m_palette)
        return &*m_palette;
    return QGenericUnixTheme::palette(type);
}

const QFont* CHyprPlatformTheme::font(QPlatformTheme::Font type) const {
    if (type == QPlatformTheme::FixedFont)
        return &m_fixedFont;
    return &m_font;
}

static QStringList iconPaths() {
    QStringList paths = {QDir::homePath() + QStringLiteral("/.icons")};

    for (const QString& p : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)) {
        paths << (p + QStringLiteral("/icons"));
    }
    paths.removeDuplicates();

    //remove invalid
    QStringList::iterator it = paths.begin();
    while (it != paths.end()) {
        if (QDir(*it).exists())
            ++it;
        else
            it = paths.erase(it);
    }

    return paths;
}

QVariant CHyprPlatformTheme::themeHint(QPlatformTheme::ThemeHint hint) const {
    if (m_isIgnored)
        return QGenericUnixTheme::themeHint(hint);

    static auto PICONTHEME                = g_pConfigManager->getValue<Hyprlang::STRING>("theme:icon_theme");
    static auto PSHORTCUTSFORCONTEXTMENUS = g_pConfigManager->getValue<Hyprlang::INT>("misc:shortcuts_for_context_menus");

    switch (hint) {
        case QPlatformTheme::CursorFlashTime: return 1000;
        case MouseDoubleClickInterval: return 400;
        case QPlatformTheme::ToolButtonStyle: return 4;
        case QPlatformTheme::SystemIconThemeName: return *PICONTHEME;
        case QPlatformTheme::StyleNames: return {u"hypr-style"_s};
        case QPlatformTheme::IconThemeSearchPaths: return iconPaths();
        case QPlatformTheme::DialogButtonBoxLayout: return 0;
        case QPlatformTheme::KeyboardScheme: return 2;
        case QPlatformTheme::UiEffects: return {};
        case QPlatformTheme::WheelScrollLines: return 3;
        case QPlatformTheme::ShowShortcutsInContextMenus: return !!*PSHORTCUTSFORCONTEXTMENUS;
        default: return QGenericUnixTheme::themeHint(hint);
    }
}

QIcon CHyprPlatformTheme::fileIcon(const QFileInfo& fileInfo, QPlatformTheme::IconOptions iconOptions) const {
    if ((iconOptions & DontUseCustomDirectoryIcons) && fileInfo.isDir())
        return QIcon::fromTheme(QLatin1String("inode-directory"));

    QMimeDatabase db;
    QMimeType     type = db.mimeTypeForFile(fileInfo);
    return QIcon::fromTheme(type.iconName());
}

#ifdef KF_ICONTHEMES_LIB
QIconEngine* CHyprPlatformTheme::createIconEngine(const QString& iconName) const {
    return new KIconEngine(iconName, KIconLoader::global());
}
#endif

void CHyprPlatformTheme::applySettings() {
    if (!QGuiApplication::desktopSettingsAware() || m_isIgnored) {
        m_update = true;
        return;
    }

    static auto PCOLORSCHEME   = g_pConfigManager->getValue<Hyprlang::STRING>("theme:color_scheme");
    static auto PFONTFIXED     = g_pConfigManager->getValue<Hyprlang::STRING>("theme:font_fixed");
    static auto PFONTFIXEDSIZE = g_pConfigManager->getValue<Hyprlang::INT>("theme:font_fixed_size");
    static auto PFONT          = g_pConfigManager->getValue<Hyprlang::STRING>("theme:font");
    static auto PFONTSIZE      = g_pConfigManager->getValue<Hyprlang::INT>("theme:font_size");

    m_font      = QFont(QString{*PFONT}, *PFONTSIZE);
    m_fixedFont = QFont(QString{*PFONTFIXED}, *PFONTFIXEDSIZE);
    m_palette   = CHyprStyle::loadColorScheme(std::string_view(*PCOLORSCHEME).empty() ? QString() : *PCOLORSCHEME);

    if (isKColorScheme(*PCOLORSCHEME))
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", *PCOLORSCHEME);
    else if (m_update)
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", QVariant());

    if (m_update) {
        QWindowSystemInterface::handleThemeChange();
        QCoreApplication::postEvent(qGuiApp, new QEvent(QEvent::ApplicationFontChange));
    }

#ifdef QT_WIDGETS_LIB
    if (hasWidgets() && m_update) {
#if QT_CONFIG(graphicsview)
        for (auto scene : std::as_const(QApplicationPrivate::instance()->scene_list))
            QCoreApplication::postEvent(scene, new QEvent(QEvent::ApplicationFontChange));
#endif

        for (QWidget* w : QApplication::allWidgets())
            QCoreApplication::postEvent(w, new QEvent(QEvent::ThemeChange));
    }
#endif

    m_update = true;
}

#ifdef QT_WIDGETS_LIB
bool CHyprPlatformTheme::hasWidgets() {
    return qobject_cast<QApplication*>(qApp) != nullptr;
}
#endif

QString CHyprPlatformTheme::loadStyleSheets(const QStringList& paths) {
    QString content;
    for (const QString& path : std::as_const(paths)) {
        if (!QFile::exists(path))
            continue;

        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            content.append(QString::fromUtf8(file.readAll()));
            if (!content.endsWith(QChar::LineFeed))
                content.append(QChar::LineFeed);
        }
    }
    static const QRegularExpression regExp(u"//.*\n"_s);
    content.replace(regExp, u"\n"_s);
    return content;
}

//There's such a thing as KColorSchemeManager that lets the user to change the color scheme
//application-wide and we should re-apply the color scheme if KCSM resets it to the default
//which leads KColorScheme to get the color scheme from kdeglobals which won't help us.
bool CHyprPlatformTheme::eventFilter(QObject* obj, QEvent* e) {
    static auto PCOLORSCHEME = g_pConfigManager->getValue<Hyprlang::STRING>("theme:color_scheme");

    if (obj == qApp && e->type() == QEvent::DynamicPropertyChange && static_cast<QDynamicPropertyChangeEvent*>(e)->propertyName() == "KDE_COLOR_SCHEME_PATH" &&
        qApp->property("KDE_COLOR_SCHEME_PATH").toString().isEmpty() && isKColorScheme(*PCOLORSCHEME))
        applySettings();
    return QObject::eventFilter(obj, e);
}

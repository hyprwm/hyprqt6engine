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
// #include <QFont>
#include <QFontDatabase>
#include <QDir>
#include <QStandardPaths>
// #include <QPalette>
#include <QTimer>
// #include <QIcon>
#include <QRegularExpression>
#include <QMimeDatabase>
#ifdef QT_WIDGETS_LIB
// #include <QStyle>
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

// #include <QStringList>
#include <qpa/qplatformthemefactory_p.h>
#include <qpa/qwindowsysteminterface.h>

#ifdef KF_ICONTHEMES_LIB
#include <KIconEngine>
#include <KIconLoader>
#endif

// #include <string_view>

#include "PlatformTheme.hpp"
#include "config/ConfigManager.hpp"
// #include "helpers/MiscFunctions.hpp"
// #include "helpers/Log.hpp"


//QT_QPA_PLATFORMTHEME=hyprqt6engine

CHyprPlatformTheme::CHyprPlatformTheme() : m_font(*QGenericUnixTheme::font(QPlatformTheme::SystemFont)), m_fixedFont(*QGenericUnixTheme::font(QPlatformTheme::FixedFont)) {
    g_pConfigManager = makeUnique<CConfigManager>();
    g_pConfigManager->init();
    if (QGuiApplication::desktopSettingsAware()) {
        readSettings();
        QMetaObject::invokeMethod(this, &CHyprPlatformTheme::applySettings, Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, &CHyprPlatformTheme::createFSWatcher, Qt::QueuedConnection);
        //must be applied before Q_COREAPP_STARTUP_FUNCTION execution
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
        if (CHyprStyle::isKColorScheme(m_schemePath))
            qApp->setProperty("KDE_COLOR_SCHEME_PATH", m_schemePath);
#endif
#ifdef QT_WIDGETS_LIB
        if (hasWidgets()) {
          //style could use various interfaces aren't initialized yet
          QTimer::singleShot(0, this, [this] {
            m_style.reset(QStyleFactory::create(u"hypr-style"_s));
            applySettings();
          });
#ifdef QT_QUICKCONTROLS2_LIB
          //don't override the value explicitly set by the user
          if (QQuickStyle::name().isEmpty() || QQuickStyle::name() == "Fusion"_L1)
            QQuickStyle::setStyle("org.kde.desktop"_L1);
#endif
        }
#endif
    }
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
    QCoreApplication::instance()->installEventFilter(this);
#endif
}

bool CHyprPlatformTheme::usePlatformNativeDialog(DialogType type) const {
    return m_theme ? m_theme->usePlatformNativeDialog(type) : QGenericUnixTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper* CHyprPlatformTheme::createPlatformDialogHelper(DialogType type) const {
    return m_theme ? m_theme->createPlatformDialogHelper(type) : QGenericUnixTheme::createPlatformDialogHelper(type);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
Qt::ColorScheme CHyprPlatformTheme::colorScheme() const {
    //m_inColorScheme prevents QPlatformTheme::palette -> QPlatformThemePrivate::initializeSystemPalette
    //-> qt_fusionPalette -> Qt6CTPlatformTheme::colorScheme infinite recursion
    if (m_isIgnored || m_inColorScheme)
        return QGenericUnixTheme::colorScheme();

    m_inColorScheme = true;
#ifdef QT_WIDGETS_LIB
    //follow QApplicationPrivate::basePalette() logic
    QPalette palette = m_style ? m_style->standardPalette() : Qt::gray;
    if (const QPalette *themePalette = this->palette())
        palette = themePalette->resolve(palette);
    palette.setResolveMask(0);
    if (m_style)
        m_style->polish(palette);
#else
    //follow QGuiApplicationPrivate::basePalette() logic
    const QPalette palette = this->palette() ? *this->palette() : Qt::gray;
#endif
    m_inColorScheme = false;

    return palette.windowText().color().lightness() > palette.window().color().lightness()
        ? Qt::ColorScheme::Dark
        : palette.windowText().color().lightness() < palette.window().color().lightness()
        ? Qt::ColorScheme::Light
        : QGenericUnixTheme::colorScheme();
}
#endif

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

    switch (hint) {
        case QPlatformTheme::CursorFlashTime: return 1000;
        case QPlatformTheme::MouseDoubleClickInterval: return 400;
        case QPlatformTheme::ShowIconsInMenus: return m_showIconsInMenus;
        case QPlatformTheme::ToolButtonStyle: return m_toolButtonStyle;
        case QPlatformTheme::SystemIconThemeName: return m_iconTheme;
        case QPlatformTheme::StyleNames: return {u"hypr-style"_s};
        case QPlatformTheme::IconThemeSearchPaths: return iconPaths();
        case QPlatformTheme::DialogButtonBoxLayout: return m_buttonBoxLayout;
        case QPlatformTheme::KeyboardScheme: return m_keyboardScheme;
        case QPlatformTheme::UiEffects: return {};
        case QPlatformTheme::WheelScrollLines: return 3;
        case QPlatformTheme::ShowShortcutsInContextMenus: return m_showShortcutsInContextMenus;
        default: return QGenericUnixTheme::themeHint(hint);
    }
}

QIcon CHyprPlatformTheme::fileIcon(const QFileInfo& fileInfo, QPlatformTheme::IconOptions iconOptions) const {
    if ((iconOptions & DontUseCustomDirectoryIcons) && fileInfo.isDir())
        return QIcon::fromTheme("inode-directory"_L1);

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

    //load dialogs
    if (!m_update) {
        //do not mix gtk2 style and gtk3 dialogs
        QStringList keys = QPlatformThemeFactory::keys();
        QString dialogs = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:dialogs");
        QString style = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:style");

        if (style.endsWith(u"gtk2"_s) && dialogs == "gtk3"_L1)
            dialogs = u"gtk2"_s;
        if (keys.contains(dialogs))
            m_theme.reset(QPlatformThemeFactory::create(dialogs));
    }
    
#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
    if (CHyprStyle::isKColorScheme(m_schemePath))
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", m_schemePath);
    else if (m_update)
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", QVariant());
#endif

#ifdef QT_WIDGETS_LIB
    if(hasWidgets()) {
        if(m_update) {
            if(FontHash *hash = qt_app_fonts_hash(); hash && hash->size())
                hash->clear();
            CHyprStyle::reloadStyleInstanceSettings();
        }

        if(m_userStyleSheet != m_prevStyleSheet) {
            // prepend our stylesheet to that of the application
            // (first removing any previous stylesheet we have set)
            QString appStyleSheet = qApp->styleSheet();
            int prevIndex = appStyleSheet.indexOf(m_prevStyleSheet);
            if (prevIndex >= 0) {
                appStyleSheet.remove(prevIndex, m_prevStyleSheet.size());
                qApp->setStyleSheet(m_userStyleSheet + appStyleSheet);
            }
            m_prevStyleSheet = m_userStyleSheet;
        }
    }
#endif
    if (m_update) {
        QWindowSystemInterface::handleThemeChange();
        QCoreApplication::postEvent(qGuiApp, new QEvent(QEvent::ApplicationFontChange));
        QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, !m_showIconsInMenus);
        QCoreApplication::setAttribute(Qt::AA_DontShowShortcutsInContextMenus, !m_showShortcutsInContextMenus);
        QIcon::setThemeName(m_iconTheme);
    }
#ifdef QT_WIDGETS_LIB
    if (hasWidgets() && m_update) {
#if QT_CONFIG(graphicsview)
        for (auto scene : std::as_const(QApplicationPrivate::instance()->scene_list))
            QCoreApplication::postEvent(scene, new QEvent(QEvent::ApplicationFontChange));
#endif
        for (QWidget* w : QApplication::allWidgets()) {
          QCoreApplication::postEvent(w, new QEvent(QEvent::ThemeChange));
          w->update();
        }
    }
#endif

    m_update = true;
}

void CHyprPlatformTheme::createFSWatcher() {
    static const QString configPath = QString::fromStdString(CConfigManager::getMainConfigPath());
    QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
    watcher->addPath(configPath);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(3000);
    connect(watcher, &QFileSystemWatcher::fileChanged, timer, qOverload<>(&QTimer::start));
    connect(timer, &QTimer::timeout, this, &CHyprPlatformTheme::updateSettings);
}

void CHyprPlatformTheme::updateSettings() {
  readSettings();
  applySettings();
}

void CHyprPlatformTheme::readSettings() {
    QStringList families = QFontDatabase::families();
    QStringList styles;

    g_pConfigManager->refresh();

    QString font_fixed = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:font_fixed");
    int font_fixed_size = *g_pConfigManager->getValue<Hyprlang::INT>("theme:font_fixed_size");
    QString font_fixed_style = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:font_fixed_style");

    QString font = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:font");
    int font_size = *g_pConfigManager->getValue<Hyprlang::INT>("theme:font_size");
    QString font_style = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:font_style");

    m_schemePath = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:color_scheme");
    m_palette = CHyprStyle::loadColorScheme(m_schemePath);
    m_iconTheme = *g_pConfigManager->getValue<Hyprlang::STRING>("theme:icon_theme");
    
    m_font = *QGenericUnixTheme::font(QPlatformTheme::SystemFont);
    m_fixedFont = *QGenericUnixTheme::font(QPlatformTheme::FixedFont);
    m_keyboardScheme = QGenericUnixTheme::themeHint(QPlatformTheme::KeyboardScheme).toInt();
    m_buttonBoxLayout = QGenericUnixTheme::themeHint(QPlatformTheme::DialogButtonBoxLayout).toInt();
    
    if (families.contains(font)) {
      m_font = QFont(font, font_size);
      styles = QFontDatabase::styles(m_font.family());
      if (styles.contains(font_style)) {
        m_font.setStyleName(font_style);
      }
    }

    if (families.contains(font_fixed)) {
      m_fixedFont = QFont(font_fixed, font_fixed_size);
      styles = QFontDatabase::styles(m_fixedFont.family());
      if (styles.contains(font_fixed_style)) {
        m_fixedFont.setStyleName(font_fixed_style);
      }
    }

    m_showShortcutsInContextMenus = !!*g_pConfigManager->getValue<Hyprlang::INT>("misc:shortcuts_for_context_menus");
    m_showIconsInMenus = !!*g_pConfigManager->getValue<Hyprlang::INT>("misc:menus_have_icons");
    m_toolButtonStyle = *g_pConfigManager->getValue<Hyprlang::INT>("misc:tool_button_style");
    m_keyboardScheme = *g_pConfigManager->getValue<Hyprlang::INT>("misc:keyboard_scheme");
    m_buttonBoxLayout = *g_pConfigManager->getValue<Hyprlang::INT>("misc:button_box_layout");
    
    // m_doubleClickInterval = QGenericUnixTheme::themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt();
       
    // m_cursorFlashTime = QGenericUnixTheme::themeHint(QPlatformTheme::CursorFlashTime).toInt();
    
    // m_wheelScrollLines = QGenericUnixTheme::themeHint(QPlatformTheme::WheelScrollLines).toInt();

    //load style sheets
#ifdef QT_WIDGETS_LIB
    QStringList qssPaths = QStringList(*g_pConfigManager->getValue<Hyprlang::STRING>("theme:style_sheet"));
    m_userStyleSheet = loadStyleSheets(qssPaths);
#endif
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

#if defined KF_CONFIGCORE_LIB && defined KF_COLORSCHEME_LIB
//There's such a thing as KColorSchemeManager that lets the user to change the color scheme
//application-wide and we should re-apply the color scheme if KCSM resets it to the default
//which leads KColorScheme to get the color scheme from kdeglobals which won't help us.
bool CHyprPlatformTheme::eventFilter(QObject* obj, QEvent* e) {
    if (obj == qApp && e->type() == QEvent::DynamicPropertyChange && static_cast<QDynamicPropertyChangeEvent*>(e)->propertyName() == "KDE_COLOR_SCHEME_PATH" &&
        qApp->property("KDE_COLOR_SCHEME_PATH").toString().isEmpty() && CHyprStyle::isKColorScheme(m_schemePath))
        applySettings();
    return QObject::eventFilter(obj, e);
}
#endif
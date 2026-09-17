#pragma once

#include <qpa/qplatformtheme.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))
#include <private/qgenericunixtheme_p.h>
#else
#include <private/qgenericunixthemes_p.h>
#endif
#include <QObject>
#include <QFont>
// #include <QPalette>
// #include <QLoggingCategory>
#include <QIcon>
#include <QFileInfo>
#ifdef QT_WIDGETS_LIB
#include <QStyle>
#endif

#include "common.hpp"

class CHyprPlatformTheme : public QObject, public QGenericUnixTheme {
    Q_OBJECT
  public:
    CHyprPlatformTheme();

    virtual ~CHyprPlatformTheme() = default;

    //virtual QPlatformMenuItem* createPlatformMenuItem() const;
    //virtual QPlatformMenu* createPlatformMenu() const;
    //virtual void showPlatformMenuBar() {}
    virtual bool                   usePlatformNativeDialog(DialogType type) const override;
    virtual QPlatformDialogHelper* createPlatformDialogHelper(DialogType type) const override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    virtual Qt::ColorScheme colorScheme() const override;
#endif
    virtual const QPalette*        palette(Palette type = SystemPalette) const override;
    virtual const QFont*           font(Font type = SystemFont) const override;
    virtual QVariant               themeHint(ThemeHint hint) const override;
    virtual QIcon                  fileIcon(const QFileInfo& fileInfo, QPlatformTheme::IconOptions iconOptions = {}) const override;
    //virtual QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const;
    //virtual QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
    //                               QPlatformTheme::IconOptions iconOptions = 0) const;

#ifdef KF_ICONTHEMES_LIB
    virtual QIconEngine* createIconEngine(const QString& iconName) const override;
#endif
    //virtual QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const;
    //virtual QString standardButtonText(int button) const;

  protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private slots:
    void applySettings();
    void createFSWatcher();
    void updateSettings();

private:
    void readSettings();
#ifdef QT_WIDGETS_LIB
    bool hasWidgets();
#endif
    QString loadStyleSheets(const QStringList &paths);
    QString m_schemePath, m_iconTheme, m_userStyleSheet, m_prevStyleSheet;
    std::optional<QPalette> m_palette;
    QFont m_font, m_fixedFont;
    int m_doubleClickInterval;
    int m_cursorFlashTime;
    int m_uiEffects;
    int m_buttonBoxLayout;
    int m_keyboardScheme;
    int m_toolButtonStyle;
    int m_wheelScrollLines;
    bool m_showShortcutsInContextMenus;
    bool m_showIconsInMenus;
    bool m_isIgnored = false;
    bool m_update = false;
    mutable bool m_inColorScheme = false;
    std::unique_ptr<QPlatformTheme> m_theme;
#ifdef QT_WIDGETS_LIB
    std::unique_ptr<QStyle> m_style;
#endif
};

// Q_DECLARE_LOGGING_CATEGORY(hypr)

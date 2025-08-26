#pragma once

#include <qpa/qplatformtheme.h>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 10, 0))
#include <private/qgenericunixtheme_p.h>
#else
#include <private/qgenericunixthemes_p.h>
#endif
#include <QObject>
#include <QFont>
#include <QPalette>
#include <QLoggingCategory>
#include <QIcon>
#include <QFileInfo>

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

  private:
#ifdef QT_WIDGETS_LIB
    bool hasWidgets();
#endif
    QString                         loadStyleSheets(const QStringList& paths);
    QString                         m_prevStyleSheet;
    QFont                           m_fixedFont, m_font;
    std::optional<QPalette>         m_palette;
    bool                            m_update    = false;
    bool                            m_isIgnored = false;
    std::unique_ptr<QPlatformTheme> m_theme;
};


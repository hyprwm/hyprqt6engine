#include <QLibraryInfo>
#include <qpa/qplatformthemeplugin.h>

#include "PlatformTheme.hpp"
#include "helpers/Log.hpp"

class CHyprQT6EngineThemePlugin : public QPlatformThemePlugin {
    Q_OBJECT;
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "engine.json");
  public:
    QPlatformTheme* create(const QString& key, const QStringList& params) override {
        Q_UNUSED(params);
        QVersionNumber v = QLibraryInfo::version();
        if (v.majorVersion() != QT_VERSION_MAJOR) {
            Debug::log(ERR, "hyprqt6engine was compiled against an incompatible qt version. Compiled against {} but has {}", QT_VERSION_MAJOR, v.majorVersion());
            return nullptr;
        }

        if (v.minorVersion() != QT_VERSION_MINOR) {
            Debug::log(WARN, "hyprqt6engine was compiled against an incompatible qt version. Compiled against {}.{} but has {}.{}. Things might break!", QT_VERSION_MAJOR, QT_VERSION_MINOR,
                       v.majorVersion(), v.minorVersion());
        }

        if (key.toLower() == QLatin1String("hyprqt6engine"))
            return new CHyprPlatformTheme();
        return nullptr;
    }
};

#include "main.moc"

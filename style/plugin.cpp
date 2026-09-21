#include <QStylePlugin>
#include "ProxyStyle.hpp"

class CHyprStylePlugin : public QStylePlugin {
    Q_OBJECT;
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "engine.json")

  public:
    QStyle* create(const QString& key) override {
        if (key == "hypr-style"_L1)
            return new CHyprProxyStyle();
        return nullptr;
    }
};

#include "plugin.moc"

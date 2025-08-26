#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QPalette>

#ifdef HYPR_LIBRARY
#define HYPR_EXPORT Q_DECL_EXPORT
#else
#define HYPR_EXPORT Q_DECL_IMPORT
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)

#include <QLatin1String>

namespace Qt {
    inline namespace Literals {
        inline namespace StringLiterals {

            inline QString operator""_s(const char16_t* str, size_t size) noexcept {
                return QString(QStringPrivate(nullptr, const_cast<char16_t*>(str), qsizetype(size)));
            }

            constexpr inline QLatin1String operator""_L1(const char* str, size_t size) noexcept {
                return QLatin1String(str, int(size));
            }

            inline QByteArray operator""_ba(const char* str, size_t size) noexcept {
                return QByteArray(QByteArrayData(nullptr, const_cast<char*>(str), qsizetype(size)));
            }

        } // StringLiterals
    } // Literals
} // Qt

using QLatin1StringView = QLatin1String;

#endif

using namespace Qt::Literals::StringLiterals;

class HYPR_EXPORT CHyprStyle {
  public:
    class CStyleInstance {
      public:
        virtual void reloadSettings() = 0;
    };

    static std::optional<QPalette> loadColorScheme(const QString& filePath);

    static void                    registerStyleInstance(CStyleInstance* instance);
    static void                    unregisterStyleInstance(CStyleInstance* instance);
    static void                    reloadStyleInstanceSettings();

  private:
    static QSet<CStyleInstance*> styleInstances;
};

#include "MiscFunctions.hpp"

bool isKColorScheme(const QString& filePath) {
    return filePath.toLower().endsWith(".colors");
}

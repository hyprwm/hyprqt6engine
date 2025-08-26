#pragma once

#include "../helpers/Memory.hpp"

#include <hyprlang.hpp>

class CConfigManager {
  public:
    CConfigManager();
    void init();

    template <typename T>
    Hyprlang::CSimpleConfigValue<T> getValue(const std::string& name) {
        return Hyprlang::CSimpleConfigValue<T>(&m_config, name.c_str());
    }

  private:
    Hyprlang::CConfig m_config;
};

inline CUniquePointer<CConfigManager> g_pConfigManager;

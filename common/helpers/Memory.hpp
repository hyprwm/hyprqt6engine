#pragma once

#include <hyprutils/memory/UniquePtr.hpp>
#include <hyprutils/memory/WeakPtr.hpp>

using namespace Hyprutils::Memory;

#define UP CUniquePointer
#define SP CSharedPointer
#define WP CWeakPointer
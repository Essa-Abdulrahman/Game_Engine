#pragma once

// Precompiled header for the Odyssey engine library. Include it first in every engine .cpp file.
// Only stable, widely used headers belong here. Never include platform headers (windows.h, X11, Cocoa).

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Odyssey/Core/Base.h"
#include "Odyssey/Core/Log.h"
#include "Odyssey/Core/Assert.h"

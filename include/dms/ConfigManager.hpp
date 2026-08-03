#pragma once

#include "dms/Config.hpp"

#include <string>

namespace dms {

// Loads/saves Config from a JSON file using OpenCV's FileStorage (no extra
// dependency). Missing keys keep their compiled-in defaults, so a partial or
// absent config file is always safe.
class ConfigManager {
public:
    // Overlay values from `path` onto `cfg`. Returns false if the file is absent
    // or unreadable (cfg is left at defaults).
    static bool load(const std::string& path, Config& cfg);

    // Write the current config (all documented keys) to `path` as JSON.
    static bool writeDefault(const std::string& path, const Config& cfg);
};

} // namespace dms

#pragma once

#include <string>

struct Package {
    std::string name;
    std::string version;
    std::string url;
    std::string sha256;
    std::string build_system;
};

Package read_recipe(const std::string& path);

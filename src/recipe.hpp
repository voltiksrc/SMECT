#pragma once

#include <string>

struct Package {
    std::string name;
    std::string version;
    std::string url;
};

Package read_recipe(const std::string& path);

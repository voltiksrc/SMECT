#pragma once

#include <filesystem>
#include "recipe.hpp"

bool install_source(
    const Package &info,
    const std::filesystem::path &source_dir,
    const std::filesystem::path &stage_dir
);

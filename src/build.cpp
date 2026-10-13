#include "build.hpp"
#include "output.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>

// I dont think i need to explain this file, it does what it says
bool build_source(const Package &info, const std::filesystem::path &source_dir) {
    namespace fs = std::filesystem;

    // Save the original path so we can swap back when a build fails.
    fs::path original_dir = fs::current_path();

    fs::current_path(source_dir);

    output::info("Build system: " + info.build_system);
    
    if (info.build_system == "make") {
        if (fs::exists("configure")) {
            output::info("Running configure...");

            if (std::system("./configure") != 0) {
                output::error("Make configure failed.");
                fs::current_path(original_dir);
                return false;
            }
        }

        output::info("Running make...");

        if (std::system("make -j4") != 0) {
            output::error("Make compile failed.");
            fs::current_path(original_dir);
            return false;
        }

    } else if (info.build_system == "meson") {
        output::info("Configuring with Meson...");

        if (std::system("meson setup build") != 0) {
            output::error("Meson setup failed.");
            fs::current_path(original_dir);
            return false;
        }

        output::info("Compiling with Meson...");

        if (std::system("meson compile -C build") != 0) {
            output::error("Meson compile failed.");
            fs::current_path(original_dir);
            return false;
        }

    } else if (info.build_system == "cmake") {
        output::info("Configuring with CMake...");

        if (std::system("cmake -S . -B build") != 0) {
            output::error("CMake configure failed.");
            fs::current_path(original_dir);
            return false;
        }

        output::info("Compiling with CMake...");

        if (std::system("cmake --build build --parallel") != 0) {
            output::error("CMake build failed.");
            fs::current_path(original_dir);
            return false;
        }

    } else {
        output::error("Unsupported build system: " + info.build_system);
        fs::current_path(original_dir);
        return false;
    }

    // return to the original path we made earlier.
    fs::current_path(original_dir);

    output::info("Build completed successfully.");
    return true;
}

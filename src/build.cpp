#include "build.hpp"
#include <iostream>
#include <cstdlib>
#include <filesystem>

// I dont think i need to explain this file, it does what it says
bool build_source(const Package &info, const std::filesystem::path &source_dir) {
    namespace fs = std::filesystem;

    // Save the original path so we can swap back when a build fails.
    fs::path original_dir = fs::current_path();
    
    fs::current_path(source_dir);
    if (info.build_system == "make") {
        std::cout<<"Building "<<info.name<<" using make...\n";
      if (fs::exists("configure")) {
         if (std::system("./configure") != 0) {
             std::cerr<<"Make configure failed.\n";
             fs::current_path(original_dir);
             return false;
         }
      }
        if (std::system("make") != 0) {
            std::cerr<<"Make failed.\n";
            fs::current_path(original_dir);
            return false;
        }
    } else if (info.build_system == "meson") {
        std::cout<<"Building "<<info.name<<" using meson...\n";

        if (std::system("meson setup build") != 0) {
            std::cerr<<"Meson setup failed.\n";
            fs::current_path(original_dir);
            return false;
        }
        if (std::system("meson compile -C build") != 0) {
            std::cerr<<"Meson compile failed.\n";
            fs::current_path(original_dir);
            return false;
        }
    } else if (info.build_system == "cmake") {
        if (std::system("cmake -S . -B build") != 0) {
            std::cerr<<"CMake build setup failed.\n";
            fs::current_path(original_dir);
            return false;
        }
        if (std::system("cmake --build build") != 0) {
            std::cerr<<"CMake build failed.\n";
            fs::current_path(original_dir);
            return false;
        }
    } else {
        std::cerr<<"Unsupported build system: "<<info.build_system<<'\n';
        fs::current_path(original_dir);
        return false;
    }
    // return to the original path we made earlier.
    fs::current_path(original_dir);
    return true;
}

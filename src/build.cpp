#include "build.hpp"
#include <iostream>
#include <cstdlib>

bool build_source(const Package &info, const std::filesystem::path &source_dir) {
    namespace fs = std::filesystem;

    fs::current_path(source_dir);
    if (info.build_system == "make") {
        std::cout<<"Building "<<info.name<<" using make...\n";

        if (std::system("./configure") != 0) {
            std::cerr<<"Make configure failed.\n";
            return false;
        }

        if (std::system("make") != 0) {
            std::cerr<<"Make failed.\n";
            return false;
        }
    } else if (info.build_system == "meson") {
        std::cout<<"Building "<<info.name<<" using meson...\n";

        if (std::system("meson setup build") != 0) {
            std::cerr<<"Meson setup failed.\n";
            return false;
        }
        if (std::system("meson compile -C build") != 0) {
            std::cerr<<"Meson compile failed.\n";
            return false;
        }
    } else if (info.build_system == "cmake") {
        if (std::system("cmake -S . -B build") != 0) {
            std::cerr<<"CMake build setup failed.\n";
            return false;
        }
        if (std::system("cmake --build build") != 0) {
            std::cerr<<"CMake build failed.\n";
            return false;
        }
    } else {
        std::cerr<<"Unsupported build system: "<<info.build_system<<'\n';
        return false;
    }
    return true;
}

#include <filesystem>
#include "recipe.hpp"
#include <cstdlib>
#include <iostream>

bool install_source(const Package &info, const std::filesystem::path &source_dir, const std::filesystem::path &stage_dir) {
  namespace fs = std::filesystem;

  fs::path original_dir = fs::current_path();
  fs::path absolute_path = fs::absolute(stage_dir);

  fs::current_path(source_dir);
  fs::create_directories(absolute_path);
  if (info.build_system == "make") {
    std::string command = "make DESTDIR=" + absolute_path.string() + " install";
    if (std::system(command.c_str()) != 0) {
      std::cerr<<"Failed to stage Make files.\n";
      fs::current_path(original_dir);
      return false;
    }
  }
  else if (info.build_system == "meson") {
    std::string command = "DESTDIR=" + absolute_path.string() + " meson install -C build";
    if (std::system(command.c_str()) != 0) {
      std::cerr<<"Failed to stage Meson files.\n";
      fs::current_path(original_dir);
      return false;
    }
  }
  else if (info.build_system == "cmake") {
    std::string command = "DESTDIR=" + absolute_path.string() + " cmake --install build";
    if (std::system(command.c_str()) != 0) {
      std::cerr<<"Failed to stage CMake files.\n";
       fs::current_path(original_dir);
       return false;
    }
  }
  else {
    std::cerr<<"Build type unsupported.\n";
    fs::current_path(original_dir);
    return false;
  }
  fs::current_path(original_dir);
  return true;
}

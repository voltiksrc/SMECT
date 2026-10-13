#include <filesystem>
#include "recipe.hpp"
#include "output.hpp"
#include <cstdlib>
#include <iostream>

bool install_source(const Package &info, const std::filesystem::path &source_dir, const std::filesystem::path &stage_dir) {
  namespace fs = std::filesystem;

  // Save the original directory so we can return to it later.
  fs::path original_dir = fs::current_path();

  // Get the absolute stage path before changing directories, otherwise itll point to the wrong place.
  fs::path absolute_path = fs::absolute(stage_dir);

  // Enter the source directory and create the directory where the files will be staged.
  fs::current_path(source_dir);
  fs::create_directories(absolute_path);

  // Run the install command depending on which build system the package uses.
  if (info.build_system == "make") {
    std::string command = "make DESTDIR=" + absolute_path.string() + " install";

    if (std::system(command.c_str()) != 0) {
      output::error("Failed to stage Make files.");
      fs::current_path(original_dir);
      return false;
    }
  }
  else if (info.build_system == "meson") {
    // Meson uses DESTDIR as an environment variable instead of a make argument.
    std::string command = "DESTDIR=" + absolute_path.string() + " meson install -C build";

    if (std::system(command.c_str()) != 0) {
      output::error("Failed to stage Meson files.");
      fs::current_path(original_dir);
      return false;
    }
  }
  else if (info.build_system == "cmake") {
    // CMake also uses DESTDIR as an environment variable.
    std::string command = "DESTDIR=" + absolute_path.string() + " cmake --install build";

    if (std::system(command.c_str()) != 0) {
      output::error("Failed to stage CMake files.");
       fs::current_path(original_dir);
       return false;
    }
  }
  else {
    // If we dont support the build system then theres no point trying to install it.
    output::error("Build type unsupported.");
    fs::current_path(original_dir);
    return false;
  }

  // Go back to the original directory after staging everything.
  fs::current_path(original_dir);
  return true;
}

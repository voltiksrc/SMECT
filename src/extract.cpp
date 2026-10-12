
#include "extract.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <iostream>

using std::string;

static int copy_data(struct archive *ar, struct archive *aw) {
  int r;
  const void *buff;
  size_t size;
  la_int64_t offset;

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      return (ARCHIVE_OK);
    if (r < ARCHIVE_OK)
      return (r); // fuck this im not commenting it
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(aw));
      return (r);
    }
  }
}

bool extract_source(const string &archive_path, const string &name,
                    const string &version,
                    const std::filesystem::path &destination) {
  namespace fs = std::filesystem; // This makes an alias for filesystem called fs, its for convenience.

  struct archive *reader = archive_read_new(); // Create the reader which opens and looks inside archives,
  struct archive *writer = archive_write_disk_new(); // and create the writer which writes the data and extracts.
  int flags{};

  // Make sure libarchive actually initialized before we do anything.
  if (!reader || !writer) {
    std::cerr << "Failed to initialize libarchive.\n";
    if (reader)
      archive_read_free(reader);
    if (writer)
      archive_write_free(writer);
    return false;
  }

  flags |= ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
  flags |= ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS;
  flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;

  archive_read_support_filter_all(reader);
  archive_read_support_format_all(reader); // These are flags to enable compression, and different archive formats.
  archive_write_disk_set_options(writer, flags);

  // Make the build directories named by the package so no package gets overridden.
  fs::path builddir = destination / (name + "-" + version);

  // Dont overwrite an old build directory or follow a symlink into somewhere else.
  std::error_code ec;
  if (fs::exists(fs::symlink_status(builddir, ec))) {
    std::cerr << "Build directory already exists.\n";
    archive_read_free(reader);
    archive_write_free(writer);
    return false;
  }

  // Check that the parent directories arent symlinks either.
  fs::path parent = builddir.parent_path();
  for (fs::path path = parent; !path.empty(); path = path.parent_path()) {
    if (fs::is_symlink(fs::symlink_status(path))) {
      std::cerr << "Unsafe build directory.\n";
      archive_read_free(reader);
      archive_write_free(writer);
      return false;
    }

    if (path == path.root_path())
      break;
  }

  int result = archive_read_open_filename(reader, archive_path.c_str(), 10240); // Try open the archive and if it fails then error and exit.
  if (result != ARCHIVE_OK) {
    std::cerr << archive_error_string(reader) << '\n';
    archive_read_free(reader);
    archive_write_free(writer);
    return false;
  }

  // Create the directory before assuming it exists.
  fs::create_directories(builddir, ec);
  if (ec) {
    std::cerr << "Failed to create build directory: " << ec.message() << '\n';
    archive_read_free(reader);
    archive_write_free(writer);
    return false;
  }

  struct archive_entry *entry;
  bool success = true;

  while (true) {
    result = archive_read_next_header(reader, &entry);

    if (result == ARCHIVE_EOF)
      break;

    // If reading fails, dont pretend we extracted everything.
    if (result != ARCHIVE_OK) {
      std::cerr << archive_error_string(reader) << '\n';
      success = false;
      break;
    }

    fs::path relative{archive_entry_pathname(entry)};

    // Reject absolute paths and empty paths so they cant escape the build directory.
    if (relative.empty() || relative.is_absolute()) {
      std::cerr << "Invalid archive path.\n";
      success = false;
      break;
    }

    // Check each part of the path for .. because that could escape our directory.
    bool unsafe = false;
    for (const auto &part : relative) {
      if (part == "..") {
        unsafe = true;
        break;
      }
    }

    if (unsafe) {
      std::cerr << "Unsafe archive path: " << relative << '\n';
      success = false;
      break;
    }

    // Symlinks and hardlinks are a pain in the ass, so reject them for now.
    if (archive_entry_symlink(entry) || archive_entry_hardlink(entry)) {
      std::cerr << "Archive links arent supported yet.\n";
      success = false;
      break;
    }

    // Only extract regular files and directories for now.
    auto type = archive_entry_filetype(entry);
    if (type != AE_IFREG && type != AE_IFDIR) {
      std::cerr << "Unsupported archive entry: " << relative << '\n';
      success = false;
      break;
    }

    fs::path output = builddir / relative;
    archive_entry_set_pathname(entry, output.c_str());

    result = archive_write_header(writer, entry);
    if (result != ARCHIVE_OK) {
      std::cerr << archive_error_string(writer) << '\n';
      success = false;
      break;
    }

    // Do copy data and if it fails then make SURE to print the error and
    // cleanup the resources such as reader/writer.
    result = copy_data(reader, writer);
    if (result != ARCHIVE_OK) {
      std::cerr << "Failed to copy archive data.\n";
      success = false;
      break;
    }

    result = archive_write_finish_entry(writer); // This finalizes the extraction and metadata.
    if (result != ARCHIVE_OK) {
      std::cerr << archive_error_string(writer) << '\n';
      success = false;
      break;
    }
  }

  // Remember to free all the resources at the end so we dont leak!!
  archive_read_free(reader);
  archive_write_free(writer);

  // Dont leave an incomplete build directory if something failed.
  if (!success) {
    fs::remove_all(builddir, ec);
    if (ec)
      std::cerr << "Failed to clean build directory: " << ec.message() << '\n';
    return false;
  }

  std::cout << "Extraction completed successfully.\n";
  return true;
}

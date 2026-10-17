#include <iostream>
#include <toml++/toml.hpp>
#include <curl/curl.h>
#include <fstream>
#include <string>
#include <filesystem>

#include "output.hpp"

using std::string;

// This is gonna write the downloaded index file to our cache directory,
// and if ur curious why we made it static is because of linker symbol collisions
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;

    auto *file = static_cast<std::ofstream *>(userdata);

    file->write(ptr, total);

    if (!*file) {
        return 0;
    }

    return total;
}

bool sync_repositories() {
    // INDEX url which we will download for package recipes.
    string url = "https://raw.githubusercontent.com/voltiksrc/omi-inventory/main/index.toml";

    // Make variables for paths so I dont typo.
    namespace fs = std::filesystem;

    const fs::path index_path = ".cache/repos/main/index.toml";
    const fs::path temp_path = ".cache/repos/main/index.toml.tmp";

    CURL *curl = curl_easy_init();

    if (!curl) {
      std::cerr<<"Failed to initalize libcurl.\n";
      return false;
    }

    // Create the indexdir before assuming it exists.
    fs::create_directories(".cache/repos/main");
    // Open index.toml in cache in binary mode.    
    std::ofstream file(temp_path, std::ios::binary);
    // Check if it failed to open our output file we just activated.
    // 
    if (!file) {
      std::cerr<<"Failed to open output file.\n";
      curl_easy_cleanup(curl);
      return false;
    }
    
    // Main curl options where we set the url, register the callback function we made
    // earlier, tell libcurl the place to write data, and support redirects and error codes.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Require HTTPS and verify TLS certificates.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");

    // Perform the download here:
    output::stage("Syncing repositories..");
    CURLcode result = curl_easy_perform(curl);
    file.close();
  
    if (result != CURLE_OK) {
       std::cerr<<"Download failed: "<<curl_easy_strerror(result)<<'\n';
       std::cout<<"Removing failed index file..\n";
       // Cleanup and remove the useless index file.
       fs::remove(temp_path);
       curl_easy_cleanup(curl);
       return false;
    }
    // auto index = toml::parse_file(
    //    ".cache/repos/main/index.toml.tmp"
    // );

    std::error_code ec;
    fs::rename(temp_path, index_path, ec);
    if (ec) {
        output::error("Failed to update index: " + ec.message());
        return false;
    }
    
    output::info("Repositories synced!");
    curl_easy_cleanup(curl);
    return true;    
}

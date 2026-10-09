#include "download.hpp"
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using std::string;

// EBI means extract, build, and install.

// Main libcurl function that writes our data to a file so we can EBI
// peacefully.
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;

    auto *file = static_cast<std::ofstream *>(userdata);

    file->write(ptr, total);

    return total;
}

string download_source(const string &source_url, const string &name,
                       const string &version) {
    // Initialize libcurl so we can download and write data
    // using its functions.
    CURL *curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initalize libcurl.\n";
        return "";
    }

    string output_path = ".cache/" + name + "-" + version + ".tar.bz2";

    // create .cache so it never fails to EBI.
    std::filesystem::create_directories(".cache");
    std::ofstream file(output_path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open output file\n";
        curl_easy_cleanup(curl);
        return "";
    }

    // Configure the source URL for the download.
    curl_easy_setopt(curl, CURLOPT_URL, source_url.c_str());

    // Register our callback to write incoming data.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Pass the output file to our callback.
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    // Follow HTTP redirects.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Treat HTTP error responses (such as 404) as failures.
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // Require HTTPS and verify TLS certificates.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");

    CURLcode result =
    curl_easy_perform(curl); // The actual download process here.
    std::cout << '\n';
    long http_code{0};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    // print a newline and make sure to exit if the download fails.
    file.close();

    if (result != CURLE_OK) {
        std::cerr << "Download failed: " << curl_easy_strerror(result) << '\n';
        std::cout << "HTTP status: " << http_code << '\n';
        std::filesystem::remove(output_path);
        curl_easy_cleanup(curl);
        return "";
    }
    // close file and free resources so we dont memory leak
    file.close();
    curl_easy_cleanup(curl);
    return output_path;
}

#include "sha256.hpp"
#include "output.hpp"

#include <openssl/evp.h>

#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

bool verify_sha256(const std::string& file_path,
                   const std::string& expected_hash) {
    // Open the downloaded archive in binary mode.
    std::ifstream file(file_path, std::ios::binary);

    if (!file) {
        output::error("Could not open file for SHA-256 verification.");
        return false;
    }

    // Set up the hashing context.
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        output::error("Failed to create SHA-256 context.");
        return false;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        output::error("Failed to initialize SHA-256.");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    // Read the archive in chunks instead of loading the whole thing.
    std::array<char, 8192> buffer{};

    while (file) {
        file.read(buffer.data(), buffer.size());
        const std::streamsize bytes_read = file.gcount();

        if (bytes_read > 0) {
            if (EVP_DigestUpdate(ctx, buffer.data(),
                                 static_cast<std::size_t>(bytes_read)) != 1) {
                output::error("Failed to update SHA-256.");
                EVP_MD_CTX_free(ctx);
                return false;
            }
        }
    }

    if (file.bad()) {
        output::error("Failed to read file during verification.");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    // Finish hashing and get the raw digest.
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0;

    if (EVP_DigestFinal_ex(ctx, digest, &digest_length) != 1) {
        output::error("Failed to finalize SHA-256.");
        EVP_MD_CTX_free(ctx);
        return false;
    }

    EVP_MD_CTX_free(ctx);

    // A SHA-256 digest should always be 32 bytes.
    if (digest_length != 32) {
        output::error("Unexpected SHA-256 digest length.");
        return false;
    }

    // Turn the raw bytes into a normal hexadecimal checksum.
    std::ostringstream hex_hash;

    for (unsigned int i = 0; i < digest_length; ++i) {
        hex_hash << std::hex << std::setw(2) << std::setfill('0')
                 << static_cast<unsigned int>(digest[i]);
    }

    const std::string actual_hash = hex_hash.str();

    // Don't accept recipes with missing or invalid checksums.
    if (expected_hash.size() != 64 ||
        expected_hash.find_first_not_of("0123456789abcdefABCDEF") !=
            std::string::npos) {
        output::error("Invalid SHA-256 checksum in recipe.");
        return false;
    }

    if (actual_hash != expected_hash) {
        output::error("SHA-256 verification failed!");
        output::info("Expected: " + expected_hash);
        output::info("Actual:   " + actual_hash);
        return false;
    }
    output::info("SHA-256 verified successfully.");
    output::info("HASH DOES = "+actual_hash);
    return true;
}

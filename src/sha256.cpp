#include "sha256.hpp"

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
        std::cerr << "Could not open file for SHA-256 verification.\n";
        return false;
    }

    // Set up the hashing context.
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        std::cerr << "Failed to create SHA-256 context.\n";
        return false;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        std::cerr << "Failed to initialize SHA-256.\n";
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
                std::cerr << "Failed to update SHA-256.\n";
                EVP_MD_CTX_free(ctx);
                return false;
            }
        }
    }

    if (file.bad()) {
        std::cerr << "Failed to read file during verification.\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    // Finish hashing and get the raw digest.
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length = 0;

    if (EVP_DigestFinal_ex(ctx, digest, &digest_length) != 1) {
        std::cerr << "Failed to finalize SHA-256.\n";
        EVP_MD_CTX_free(ctx);
        return false;
    }

    EVP_MD_CTX_free(ctx);

    // A SHA-256 digest should always be 32 bytes.
    if (digest_length != 32) {
        std::cerr << "Unexpected SHA-256 digest length.\n";
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
        std::cerr << "Invalid SHA-256 checksum in recipe.\n";
        return false;
    }

    if (actual_hash != expected_hash) {
        std::cerr << "SHA-256 verification failed!\n";
        std::cerr << "Expected: " << expected_hash << '\n';
        std::cerr << "Actual:   " << actual_hash << '\n';
        return false;
    }

    std::cout << "SHA-256 verified successfully.\n";
    return true;
}

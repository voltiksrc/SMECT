#include <iostream>
#include <string>
#include <string_view>
#include "recipe.hpp"
#include "download.hpp"

using std::string;

void smversion() {
    constexpr std::string_view version{"0.01"};
    std::cout<<"smect v"<<version<<'\n';
}

void show_help() {
    std::cout<<"SMECT Package manager help\n";
    std::cout<<'\n';
    std::cout<<"Example: smect install fastfetch\n";
    std::cout<<"Options:\n";
    std::cout<<"smect install <package>\n";
    std::cout<<"smect remove <package>\n";
    std::cout<<"smect search <package>\n";
    std::cout<<"smect --help\n";
}

int main(int argc, char* argv[]) {
    // We need to check before declaring the command as argv[1].
    // If we dont then running smect without arguments could cause an error.
    if (argc < 2) {
        std::cout<<"Please enter a command and optionally a package.\n";
        return 1;
    }
    string command = argv[1];
    // command line interactions so commands actually do something
    if (command == "--help") {
        show_help();
        return 0;
    } else if (command == "--version") {
        smversion();
        return 0;
    }

    // Make sure they enter a package so we can safely assign
    // argv[2] to the package variable placeholder.
    if (argc != 3) {
        std::cout<<"Usage: smect <command> [package]\n";
        return 1;
    }
    string package = argv[2];
    if (command == "install") {
        Package info = read_recipe("recipes/" + package + "/package.toml");

        if (info.name.empty() || info.version.empty() || info.url.empty()) {
            std::cerr<<"Invalid or incomplete package recipe.\n";
            return 1;
        }
        string archive = download_source(info.url, info.name, info.version);

        if (archive.empty()) {
            return 1;
        }
    } else if (command == "remove") {
        std::cout<<"Removing "<<package<<"..\n";
        return 0;
    } else if (command == "search") {
        std::cout<<"Searching for "<<package<<"...\n";
        std::cout<<"Found package: "<<package<<".\n";
    } else { // check if its a proper command if not then quit with an error.
        std::cout<<"Unknown command: "<<argv[1]<<".\n";
        return 1;
    }
}

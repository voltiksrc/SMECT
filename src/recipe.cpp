#include <toml++/toml.hpp>
#include "recipe.hpp"

Package read_recipe(const std::string& path) {
   // Read and parse the recipe.
   auto recipe = toml::parse_file(path);   

    Package package;

    package.name = recipe["package"]["name"].value_or("");
    package.version = recipe["package"]["version"].value_or("");
    package.url = recipe["source"]["url"].value_or("");
    package.sha256 = recipe["source"]["sha256"].value_or("");

    return package;
}

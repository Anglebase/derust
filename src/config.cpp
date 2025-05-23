#include "config.h"
#include <filesystem>
#include <stdexcept>

Config::Config(const std::filesystem::path &path)
{
    std::ifstream ifs(path);
    if (!ifs)
        throw std::runtime_error("Failed to open config file" + path.string());
    this->config = toml::parse(ifs);
}
#include "config.h"
#include <filesystem>
#include <stdexcept>

Config::Config(const std::filesystem::path &path_)
{
    auto path = path_;
    path.make_preferred();
    this->path = path.string();
    std::ifstream ifs(path);
    if (!ifs)
        throw std::runtime_error("Failed to open config file '" + path.string() + "'");
    this->config = toml::parse(ifs);
}
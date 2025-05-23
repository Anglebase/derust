#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <thread>
namespace fs = std::filesystem;

namespace cmake
{
    enum Config
    {
        Debug,
        Release,
    };
    class Execute
    {
        std::vector<std::string> commands;

    public:
        Execute();

        Execute &source(const fs::path &source_dir);
        Execute &build_dir(const fs::path &build_dir);
        Execute &generator(const std::string &generator);

        Execute &target(const std::string &target);
        Execute &build(const fs::path &build_dir);
        Execute &config(Config config);

        Execute &jobs(int num_jobs = std::thread::hardware_concurrency());

        std::string as_command() const;
    };
} // namespace cmake
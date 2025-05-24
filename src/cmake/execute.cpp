#include <cmake/execute.h>
#include <utils.h>
using namespace cmake;

cmake::Execute::Execute()
{
    this->commands.push_back("cmake");
}

Execute &cmake::Execute::source(const fs::path &source_dir)
{
    this->commands.push_back("-S");
    this->commands.push_back(source_dir.string());
    return *this;
}

Execute &cmake::Execute::build_dir(const fs::path &build_dir)
{
    this->commands.push_back("-B");
    this->commands.push_back(build_dir.string());
    return *this;
}

Execute &cmake::Execute::generator(const std::string &generator)
{
    this->commands.push_back("-G");
    this->commands.push_back('"' + generator + '"');
    return *this;
}

Execute &cmake::Execute::target(const std::string &target)
{
    this->commands.push_back("--target");
    this->commands.push_back(target);
    return *this;
}

Execute &cmake::Execute::build(const fs::path &build_dir)
{
    this->commands.push_back("--build");
    this->commands.push_back(build_dir.string());
    return *this;
}

Execute &cmake::Execute::config(Config config)
{
    this->commands.push_back("--config");
    this->commands.push_back(config == Config::Debug ? "Debug" : "Release");
    return *this;
}

Execute &cmake::Execute::jobs(int num_jobs)
{
    this->commands.push_back("-j");
    this->commands.push_back(std::to_string(num_jobs));
    return *this;
}

std::string cmake::Execute::as_command() const
{
    return join(this->commands, " ");
}

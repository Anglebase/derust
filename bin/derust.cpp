#include <iostream>
#include <sstream>
#include <toml++/toml.hpp>
#include <config.h>
#include <log.h>
#include <cmd.h>
#include <core.h>
#include <unordered_map>
#include <filesystem>
namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    CmdLine cmd(argc, argv);
    LOG_DEBUG(cmd);

    if (cmd.getArgs().empty())
    {
        return show_help();
    }

    if (cmd.hasSetting("dir"))
    {
        if (cmd.getSettings("dir").size() == 1)
        {
            fs::path path = cmd.getSettings("dir").begin()->c_str();
            if (fs::exists(path) && fs::is_directory(path))
                fs::current_path(path);
            else
            {
                LOG_ERROR("Cannot open directory: ", path);
                return 1;
            }
        }
        else
        {
            LOG_ERROR("Invalid directory setting: '[-]-dir=<directory>'");
            return 1;
        }
    }
    try
    {
        auto command = cmd.getArgs()[0];
        std::unordered_map<std::string, int (*)(const CmdLine &)> commands = {
            {"new", exec_new},
            {"build", exec_build},
            {"run", exec_run},
            {"clean", exec_clean},
            {"help", [](const CmdLine &)
             { return show_help(); }},
        };
        auto it = commands.find(command);
        if (it == commands.end())
        {
            LOG_ERROR("Unknown command: ", command);
            return 1;
        }
        return it->second(cmd);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        return 1;
    }

    return 0;
}

#include <core/run.h>
#include <core/build.h>
#include <filesystem>
namespace fs = std::filesystem;

int exec_run(const CmdLine &cmd_line)
{
    Config config(fs::current_path() / CONFIG_FILE_NAME);
    auto name = config.get<std::string>("project.name");
    CmdLine cmd = cmd_line;
    if (cmd.Args().size() < 2)
        cmd.Args().push_back(name);
    exec_build(cmd_line);
    name = cmd.Args()[1];
    LOG_INFO("");
    std::vector<std::string> command = {
        (fs::current_path() / TARGET / (name + ".exe")).string(),
        cmd.getCatchArgs(),
    };
    std::string command_str = join(command, " ");
    LOG_SUCCESS("# Running [", command_str, "]:");
    LOG_DEBUG("Executing command: ", command_str);
    auto res = system(command_str.c_str());
    if (res != 0)
        LOG_WARN("# Exit code: ", res);
    else
        LOG_SUCCESS("# Success");
    return 0;
}
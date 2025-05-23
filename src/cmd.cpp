#include "cmd.h"
#include <filesystem>
using namespace std;

CmdLine::CmdLine(int argc, char *argv[])
    : CmdLine(vector<string>(argv + 1, argv + argc))
{
}

CmdLine::CmdLine(const vector<string> &ls)
{
    // auto ls = vector<string>(argv + 1, argv + argc);
    size_t index = 0;
    bool catch_args = false;
    for (; index < ls.size(); index++)
    {
        auto str = ls[index];
        if (str == "-args" || str == "--args")
        {
            catch_args = true;
            break;
        }
        else if (str.starts_with("--"))
        {
            auto set = str.substr(2);
            auto pos = set.find("=");
            if (pos == string::npos)
            {
                this->options.insert(set);
            }
            else
            {
                auto name = set.substr(0, pos);
                auto value = set.substr(pos + 1);
                this->settings[name].insert(value);
            }
        }
        else if (str.starts_with("-"))
        {
            auto set = str.substr(1);
            auto pos = set.find("=");
            if (pos == string::npos)
            {
                this->options.insert(set);
            }
            else
            {
                auto name = set.substr(0, pos);
                auto value = set.substr(pos + 1);
                this->settings[name].insert(value);
            }
        }
        else
        {
            this->args.push_back(str);
        }
    }
    if (catch_args)
    {
        std::string catch_str;
        index++;
        for (; index < ls.size(); index++)
        {
            catch_str += " " + ls[index];
        }
        this->catch_args = catch_str;
    }
}

bool CmdLine::hasOption(const std::string &option) const
{
    return this->options.count(option) > 0;
}

bool CmdLine::hasSetting(const std::string &setting) const
{
    return this->settings.count(setting) > 0;
}

const std::set<std::string> &CmdLine::getSettings(const std::string &setting) const
{
    return this->settings.at(setting);
}

const std::vector<std::string> &CmdLine::getArgs() const
{
    return this->args;
}

const std::string &CmdLine::getCatchArgs() const
{
    return this->catch_args;
}

std::ostream &operator<<(std::ostream &os, const CmdLine &cmdLine)
{
    os << "Args: ";
    for (auto arg : cmdLine.args)
        os << arg << " ; ";
    os << endl;
    os << "Options: ";
    for (auto option : cmdLine.options)
        os << option << " ; ";
    os << endl;
    os << "Settings: " << endl;
    for (auto setting : cmdLine.settings)
    {
        os << setting.first << " = ";
        for (auto value : setting.second)
            os << value << " ; ";
        os << endl;
    }
    os << "Catch Args: " << cmdLine.catch_args << endl;
    return os;
}

namespace fs = std::filesystem;
void cd_dir(const std::string &dir)
{
    if (dir.empty())
        return;
    auto current_dir = fs::current_path();
    auto target_dir = fs::path(dir);
    if (target_dir.is_absolute())
        current_dir = target_dir;
    else
        current_dir /= target_dir;
    fs::current_path(current_dir);
}

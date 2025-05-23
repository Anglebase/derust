#pragma once
#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>

/**
 * @brief 命令行参数解析器
 * @details 命令行参数格式：
 *   -<option>
 *   --<option>
 *   -<setting>=<value>[;<value>;...]
 *   --<setting>=<value>[;<value>;...]
 *   其余非'-'开头参数全部按照顺序作为参数值
 *   特殊参数 [-]-args 用于将其后所有参数作为一个字符串捕获
 */
class CmdLine
{
    std::set<std::string> options;
    std::map<std::string, std::set<std::string>> settings;
    std::vector<std::string> args;
    std::string catch_args;

public:
    CmdLine(int argc, char *argv[]);
    CmdLine(const std::vector<std::string>& ls);
    bool hasOption(const std::string &option) const;
    bool hasSetting(const std::string &setting) const;
    const std::set<std::string> &getSettings(const std::string &setting) const;
    const std::vector<std::string> &getArgs() const;
    const std::string &getCatchArgs() const;

    std::vector<std::string>& Args() { return args; }

public:
    friend std::ostream &operator<<(std::ostream &os, const CmdLine &cmdLine);
};

void cd_dir(const std::string &dir);
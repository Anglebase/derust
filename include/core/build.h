#pragma once

#include<cmd.h>
#include<config.h>
#include<unordered_set>
#include<string>

int exec_build(const CmdLine &cmd);
int generate_cmake_files(const CmdLine &cmd, const Config &config);

extern const char *CONFIG_FILE_NAME;
extern const char *TARGET;
extern const char *CACHE;
extern const char *PACKAGE;
extern const std::unordered_set<std::string> C_CXX_EXT;
extern const std::unordered_set<std::string> HEADER_EXT;
#include <core/new.h>
#include <log.h>
#include <filesystem>
#include <fstream>
#include <utils.h>
namespace fs = std::filesystem;

const char *derust_toml = R"([project]
name = "${name}"
version = "0.1.0"

[build]
cpp = 20
target = "${target}"
)";

const char *cpp_exe = R"(#include <iostream>

int main()
{
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
)";

const char *h_lib = R"(#pragma once

int my_${name}();
)";

const char *cpp_lib = R"(#include <iostream>

int my_${name}()
{
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
)";

const char *main_lib = R"(#include <${name}.h>

int main()
{
    return my_${name}();
}
)";

const char *h_dll = R"(#pragma once
#ifndef DLL_EXPORT
#   ifdef _MSC_VER
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT
#   endif
#endif

DLL_EXPORT int my_${name}();
)";

const char *cpp_dll = R"(#include <${name}.h>

DLL_EXPORT int my_${name}()
{
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
)";

const char *main_dll = R"(#include <${name}.h>

int main()
{
    return my_${name}();
}
)";

int exec_new(const CmdLine &cmd)
{
    if (cmd.getArgs().size() < 2)
    {
        LOG_ERROR("Missing argument: name");
        LOG_INFO("Usage: derust new <name> [target]");
        return 1;
    }
    const std::string name = cmd.getArgs()[1];
    std::string target = "exe";
    if (cmd.hasOption("lib"))
    {
        target = "lib";
    }
    if (cmd.hasOption("dll"))
    {
        target = "dll";
    }
    auto root = fs::current_path() / name;
    if (fs::exists(root))
    {
        LOG_ERROR("Directory '", name, "' already exists.");
        return 1;
    }
    fs::create_directory(root);
    LOG_INFO("Created directory '", name, "'.");
    fs::path exe_dir;
    std::vector<std::string> dirs{"include", "src"};
    if (target == "exe")
    {
        dirs.push_back("bin");
        exe_dir = root / "bin";
    }
    else
    {
        dirs.push_back("examples");
        exe_dir = root / "examples";
    }
    for (const auto &dir : dirs)
    {
        fs::create_directories(root / dir);
    }
    fs::path include_dir = root / "include";
    fs::path src_dir = root / "src";

    LOG_INFO("Generated template files in '", root, "'.");

    fs::path toml_file = root / "derust.toml";
    std::string toml_code = derust_toml;
    replace(toml_code, "${name}", name);
    replace(toml_code, "${target}", target);

    std::ofstream toml_file_stream(toml_file);
    toml_file_stream << toml_code;
    toml_file_stream.close();

    if (target == "exe")
    {
        fs::path main_file = exe_dir / (name + ".cpp");
        std::string main_code = cpp_exe;
        replace(main_code, "${name}", name);

        std::ofstream main_file_stream(main_file);
        main_file_stream << main_code;
        main_file_stream.close();
    }
    else if (target == "lib")
    {
        fs::path h_file = include_dir / (name + ".h");
        std::string h_code = h_lib;
        replace(h_code, "${name}", name);

        std::ofstream h_file_stream(h_file);
        h_file_stream << h_code;
        h_file_stream.close();

        fs::path cpp_file = src_dir / (name + ".cpp");
        std::string cpp_code = cpp_lib;
        replace(cpp_code, "${name}", name);

        std::ofstream cpp_file_stream(cpp_file);
        cpp_file_stream << cpp_code;
        cpp_file_stream.close();

        fs::path main_file = exe_dir / (name + ".cpp");
        std::string main_code = main_lib;
        replace(main_code, "${name}", name);

        std::ofstream main_file_stream(main_file);
        main_file_stream << main_code;
        main_file_stream.close();
    }
    else if (target == "dll")
    {
        fs::path h_file = include_dir / (name + ".h");
        std::string h_code = h_dll;
        replace(h_code, "${name}", name);

        std::ofstream h_file_stream(h_file);
        h_file_stream << h_code;
        h_file_stream.close();

        fs::path cpp_file = src_dir / (name + ".cpp");
        std::string cpp_code = cpp_dll;
        replace(cpp_code, "${name}", name);

        std::ofstream cpp_file_stream(cpp_file);
        cpp_file_stream << cpp_code;
        cpp_file_stream.close();

        fs::path main_file = exe_dir / (name + ".cpp");
        std::string main_code = main_dll;
        replace(main_code, "${name}", name);

        std::ofstream main_file_stream(main_file);
        main_file_stream << main_code;
        main_file_stream.close();
    }

    LOG_INFO("Done.");
    return 0;
}
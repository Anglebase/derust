#include <core/build.h>
#include <optional>
#include <filesystem>
#include <unordered_set>
#include <cmake/generator.h>
#include <cmake/execute.h>
namespace fs = std::filesystem;

const char *CONFIG_FILE_NAME = "derust.toml";
const char *TARGET = "target";
const char *CACHE = ".cache";
const char *PACKAGE = ".package";
const std::unordered_set<std::string> C_CXX_EXT{".c", ".cpp", ".cxx", ".cc", ".c++"};
const std::unordered_set<std::string> HEADER_EXT{".h", ".hpp", ".hxx", ".hh", ".h++", ".inl"};
const std::unordered_set<std::string> TARGET_TYPE{"exe", "lib", "dll"};

std::string exe(std::string name)
{
    return name + "_exe";
}

std::string lib(std::string name)
{
    return name + "_lib";
}

std::string dll(std::string name)
{
    return name + "_dll";
}

int exec_build(const CmdLine &cmd)
{
    std::optional<std::string> build_target;
    if (cmd.getArgs().size() >= 2)
        build_target = cmd.getArgs()[1];
    cmake::Config cmake_config = cmake::Config::Debug;
    if (cmd.hasOption("release") || cmd.hasOption("r"))
        cmake_config = cmake::Config::Release;

    Config config(fs::current_path() / CONFIG_FILE_NAME);
    auto name = config.get<std::string>("project.name");
    auto generator = config.need<std::string>("build.generator", "MinGW Makefiles");
    generator = '"' + generator + '"';
    LOG_INFO("Generating cmake files for project: ", name);
    generate_cmake_files(cmd, config);

    cmake::Execute gen;
    gen.source(fs::current_path());
    gen.build_dir(fs::current_path() / CACHE);
    gen.generator(generator);

    LOG_DEBUG("Executing cmake command: ", gen.as_command());
    auto res = system(gen.as_command().c_str());
    if (res != 0)
    {
        LOG_ERROR("Failed to generate cmake files for project: ", name);
        return 1;
    }

    cmake::Execute build;
    build.build(fs::current_path() / CACHE);
    if (build_target.has_value())
        build.target(exe(build_target.value()));
    build.config(cmake_config);
    build.jobs();

    LOG_DEBUG("Executing cmake build command: ", build.as_command());
    res = system(build.as_command().c_str());
    if (res != 0)
    {
        LOG_ERROR("Failed to build project: ", name);
        return 1;
    }

    return 0;
}

void find_all_source_files(const fs::path &dir, std::vector<fs::path> &files)
{
    for (auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_directory())
            find_all_source_files(entry.path(), files);
        else if (entry.is_regular_file())
        {
            auto ext = entry.path().extension().string();
            if (C_CXX_EXT.count(ext) > 0)
                files.push_back(entry.path());
        }
    }
}

std::vector<fs::path> find_all_exe_files(const fs::path &dir)
{
    std::vector<fs::path> files;
    for (auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file())
        {
            auto ext = entry.path().extension().string();
            if (C_CXX_EXT.count(ext) > 0)
                files.push_back(entry.path());
        }
    }
    return files;
}

int generate_cmake_files(const CmdLine &cmd, const Config &config)
{
    cmake::Generator gen_cmake;
    auto name = config.get<std::string>("project.name");
    auto target = config.get<std::string>("build.target");
    if (TARGET_TYPE.count(target) == 0)
    {
        LOG_ERROR("Invalid target type: " + target);
        return 1;
    }
    auto stdc = config.need<int>("build.c", 17);
    auto stdcxx = config.need<int>("build.cxx", 20);

    cmake::Config cmake_config = cmake::Config::Debug;
    if (cmd.hasOption("release") || cmd.hasOption("r"))
        cmake_config = cmake::Config::Release;

    gen_cmake.project(name);
    gen_cmake.set_c_standard(stdc);
    gen_cmake.set_cxx_standard(stdcxx);
    gen_cmake.include_directories({fs::current_path() / "include"});

    gen_cmake.if_("MSVC");
    gen_cmake.add_complie_options({"/W4", "/Zi"});
    gen_cmake.else_();
    gen_cmake.add_complie_options({"-Wall", "-Wextra", "-Werror", "-g", "-O2"});
    gen_cmake.endif_();

    if (cmake_config == cmake::Config::Debug)
        gen_cmake.add_defines({"DEBUG", "_DEBUG"});
    else
        gen_cmake.add_defines({"NDEBUG", "_NDEBUG"});

    gen_cmake.set_execute_output_path(fs::current_path() / TARGET);
    if (target != "exe")
        gen_cmake.set_library_output_path(fs::current_path() / TARGET);

    std::vector<fs::path> source_files;
    find_all_source_files(fs::current_path() / "src", source_files);

    fs::path exe_dir =
        target == "exe"
            ? fs::current_path() / "bin"
            : fs::current_path() / "examples";

    auto exe_files = find_all_exe_files(exe_dir);

    if (target == "exe")
    {
        for (const auto &file : exe_files)
        {
            auto all_src = source_files;
            all_src.push_back(file);
            auto name = fs::path(file).replace_extension().filename().string();
            gen_cmake.add_executable(exe(name), all_src);
            gen_cmake.set_target_output_name(exe(name), name);
            gen_cmake.target_include_directories(exe(name), cmake::Visual::Public, {fs::current_path() / "include"});
        }
    }
    else if (target == "lib")
    {
        gen_cmake.add_library(lib(name), cmake::LibaryType::Static, source_files);
        gen_cmake.set_target_output_name(lib(name), name);
        for (const auto &file : exe_files)
        {
            auto name = fs::path(file).replace_extension().filename().string();
            gen_cmake.add_executable(exe(name), {file});
            gen_cmake.set_target_output_name(exe(name), name);
            gen_cmake.target_link_directories(exe(name), cmake::Visual::Public, {fs::current_path() / TARGET});
            gen_cmake.target_link_libraries(exe(name), cmake::Visual::Public, {lib(name)});
            gen_cmake.target_include_directories(exe(name), cmake::Visual::Public, {fs::current_path() / "include"});
        }
    }
    else if (target == "dll")
    {
        gen_cmake.add_library(dll(name), cmake::LibaryType::Shared, source_files);
        gen_cmake.set_target_output_name(dll(name), name);
        for (const auto &file : exe_files)
        {
            auto name = fs::path(file).replace_extension().filename().string();
            gen_cmake.add_executable(exe(name), {file});
            gen_cmake.set_target_output_name(exe(name), name);
            gen_cmake.target_link_directories(exe(name), cmake::Visual::Public, {fs::current_path() / TARGET});
            gen_cmake.target_link_libraries(exe(name), cmake::Visual::Public, {dll(name)});
            gen_cmake.target_include_directories(exe(name), cmake::Visual::Public, {fs::current_path() / "include"});
        }
    }

    std::ofstream cmakelists(fs::current_path() / "CMakeLists.txt");
    gen_cmake.write_to(cmakelists);
    cmakelists.close();

    return 0;
}
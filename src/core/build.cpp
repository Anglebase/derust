#include <core/build.h>
#include <optional>
#include <filesystem>
#include <unordered_set>
#include <cmake/generator.h>
#include <cmake/execute.h>
#include <md5.h>
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

void find_all_dependecies_(std::vector<fs::path> &dependecies, const Config &config)
{
    if (!config.config.contains("dependencies") || !config.config["dependencies"].is_table())
        return;
    auto dependencies = *config.config["dependencies"].as_table();
    for (const auto &[key, _info] : dependencies)
    {
        auto name = key.str();
        auto path = fs::path(config.get<std::string>("dependencies." + std::string(name) + ".path"));
        dependecies.push_back(path);
        find_all_dependecies_(dependecies, Config(path / CONFIG_FILE_NAME));
    }
}

std::vector<fs::path> find_all_dependecies(const Config &config)
{
    std::vector<fs::path> dependecies;
    find_all_dependecies_(dependecies, config);
    return dependecies;
}

struct Package
{
    std::vector<fs::path> include_dirs;
    std::vector<fs::path> lib_dirs;
    std::vector<std::string> libs;
};

Package build_all_dependecies(const fs::path &cache, const std::vector<fs::path> &dependecies)
{
    Package package;
    for (const auto &dep : dependecies)
    {
        LOG_INFO("#[ Begin: Building dependecy: ", dep.string(), "]");
        auto md5 = MD5(dep);
        auto cache_dir = cache / (dep.filename().string() + "-" + md5.toStr());
        auto build_dir = cache_dir / "build";

        auto before = fs::current_path();
        fs::current_path(dep);

        Config config(dep / CONFIG_FILE_NAME);
        generate_cmake_files(CmdLine({"build", "-r"}), config);

        cmake::Execute gen;
        gen.source(dep);
        gen.build_dir(build_dir);
        gen.generator(config.need<std::string>("build.generator", "MinGW Makefiles"));
        if (system(gen.as_command().c_str()) != 0)
            throw std::runtime_error("Failed to generate cmake files for dependecy: " + dep.string());

        cmake::Execute build;
        build.build(build_dir);
        auto name = config.get<std::string>("project.name");
        build.target(name + "_" + config.get<std::string>("build.target"));
        build.config(cmake::Config::Release);
        build.jobs(config.need<int>("build.jobs", std::thread::hardware_concurrency(), false));
        if (system(build.as_command().c_str()) != 0)
            throw std::runtime_error("Failed to build dependecy: " + dep.string());

        fs::current_path(before);
        package.include_dirs.push_back(dep / "include");
        package.lib_dirs.push_back(dep / "target");
        package.libs.push_back(name);

        auto target = config.get<std::string>("build.target");
        if (target == "dll")
        {
            auto dll_path = dep / "target" / (name + ".dll");
            if (!fs::exists(dll_path))
                dll_path = dep / "target" / ("lib" + name + ".dll");

            fs::copy_file(dll_path, before / "target" / dll_path.filename(), fs::copy_options::overwrite_existing);
        }
        else if (target == "exe")
        {
            LOG_WARN("Warning: The dependecy '", name, "' at '", dep.string(), "' is 'exe'");
        }
        LOG_INFO("#[ End: Building dependecy: ", dep.string(), "]");
    }
    return package;
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
    LOG_INFO("Generating cmake files for project: ", name);
    generate_cmake_files(cmd, config);

    cmake::Execute gen;
    gen.source(fs::current_path());
    gen.build_dir(fs::current_path() / CACHE / name);
    gen.generator(generator);

    LOG_DEBUG("Executing cmake command: ", gen.as_command());
    if (system(gen.as_command().c_str()) != 0)
        throw std::runtime_error("Failed to generate cmake files for project: " + name);

    cmake::Execute build;
    build.build(fs::current_path() / CACHE / name);
    if (build_target.has_value())
        build.target(exe(build_target.value()));
    build.config(cmake_config);
    build.jobs();

    LOG_DEBUG("Executing cmake build command: ", build.as_command());
    if (system(build.as_command().c_str()) != 0)
        throw std::runtime_error("Failed to build project: " + name);

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
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return std::vector<fs::path>();
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
        throw std::runtime_error("Invalid target type: " + target);

    auto stdc = config.need<int>("build.c", 17, false);
    auto stdcxx = config.need<int>("build.cxx", 20, false);

    cmake::Config cmake_config = cmake::Config::Debug;
    if (cmd.hasOption("release") || cmd.hasOption("r"))
        cmake_config = cmake::Config::Release;

    gen_cmake.project(name);
    gen_cmake.set_c_standard(stdc);
    gen_cmake.set_cxx_standard(stdcxx);
    if (fs::exists(fs::current_path() / "include"))
        gen_cmake.include_directories({fs::current_path() / "include"});

    gen_cmake.if_("MSVC");
    gen_cmake.add_complie_options({"/W4", "/Zi"});
    gen_cmake.else_();
    gen_cmake.add_complie_options({"-Wall", "-Wextra", "-Werror", "-g", "-O2"});
    gen_cmake.endif_();

    auto build = *config.config["build"].as_table();
    if (build.contains("options") && build["options"].is_array())
    {
        auto options = *build["options"].as_array();
        for (const auto &option : options)
        {
            if(!option.is_string())
                throw std::runtime_error("Invalid build option.");
            auto str = option.value<std::string>().value();
            gen_cmake.add_complie_options({str});
        }
    }

    if (cmake_config == cmake::Config::Debug)
        gen_cmake.add_defines({"DEBUG", "_DEBUG"});
    else
        gen_cmake.add_defines({"NDEBUG", "_NDEBUG"});

    gen_cmake.set_execute_output_path(fs::current_path() / TARGET);
    if (target != "exe")
        gen_cmake.set_library_output_path(fs::current_path() / TARGET);

    std::vector<fs::path> source_files;
    if (fs::exists(fs::current_path() / "src"))
        find_all_source_files(fs::current_path() / "src", source_files);

    fs::path exe_dir =
        target == "exe"
            ? fs::current_path() / "bin"
            : fs::current_path() / "examples";

    auto exe_files = find_all_exe_files(exe_dir);

    auto dep_info = build_all_dependecies(fs::current_path() / CACHE, find_all_dependecies(config));
    gen_cmake.include_directories(dep_info.include_dirs);
    gen_cmake.link_directories(dep_info.lib_dirs);
    gen_cmake.link_libraries(dep_info.libs);

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
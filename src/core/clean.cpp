#include <core/clean.h>
#include <filesystem>
#include <core/build.h>
namespace fs = std::filesystem;

int exec_clean(const CmdLine &)
{
    if (fs::exists(fs::current_path() / CACHE))
        fs::remove_all(CACHE);
    return 0;
}
#include "core/help.h"
#include <iostream>

int show_help()
{
    std::cout << R"(Usage:
    derust [command] <options/settings> [[-]-dir=<directory>]
Commands:
    help                                            Show this help message.
    new <name> [--exe|--lib|--dll]  Create a new project with the given name.
    build [<name>] [-r|--release]   Build the current project or 'dir' if specified.
    run [<name>] [-r|--release]     Run the current project or 'dir' if specified.
    clean                           Clean the build directory of the current project or 'dir' if specified.
    )" << std::endl;
    return 0;
}
#include "core/help.h"
#include <iostream>

int show_help()
{
    std::cout << R"(Usage:
    derust [command] <options/settings> [[-]-dir=<directory>]
Commands:
    help                    Show this help message.
    new <name> [options]    Create a new project with the given name.
    build [options]         Build the current project or 'dir' if specified.
    run [options]           Run the current project or 'dir' if specified.
    clean [options]         Clean the build directory of the current project or 'dir' if specified.
    )" << std::endl;
    return 0;
}
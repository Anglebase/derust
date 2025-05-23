#include "log.h"

void log(ConsoleColor color, const std::string &message)
{
    std::cout << color << message << ConsoleColor::None << std::endl;
}
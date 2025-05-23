#include <cmake/generator.h>
#include <log.h>

int main()
{
    cmake::Generator generator;
    generator.project("MyProject");
    generator.add_executable("main", {"src/main.cpp"});
    LOG_INFO(generator.generator());
    return 0;
}
#include <log.h>
#include <config.h>

int main()
{
    try
    {
        Config config("derust.toml");
        LOG_INFO("Get 'project.version': ", config.need<std::string>("project.version", "0.1.0"));
        LOG_INFO("Get 'project.name': ", config.get<std::string>("project.name"));
        LOG_INFO("Get 'project.author.age': ", config.need<int>("project.author.age", 18));
        LOG_INFO("Get 'project.author.name': ", config.get<std::string>("project.author.name"));
    }
    catch (std::exception &e)
    {
        LOG_ERROR(e.what());
    }
    return 0;
}
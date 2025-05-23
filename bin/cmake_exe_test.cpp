#include<cmake/execute.h>
#include<log.h>

int main(){
    cmake::Execute make;
    make.source(".");
    make.build_dir(".cache");
    make.generator("MinGW Makefiles");
    LOG_INFO("Executing cmake command: ", make.as_command());

    cmake::Execute build;
    build.build(".cache");
    build.target("demo");
    build.config(cmake::Config::Release);
    build.jobs();
    LOG_INFO("Executing cmake build command: ", build.as_command());
}
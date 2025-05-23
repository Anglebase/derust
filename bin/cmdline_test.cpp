#include <cmd.h>
#include <log.h>

int main(int argc, char *argv[]) {
    CmdLine cmd(argc, argv);
    LOG_INFO(cmd);
}
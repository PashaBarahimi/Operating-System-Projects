#include <stdlib.h>

#include "include/log.h"
#include "include/user.h"

void interruptHandler(int sig)
{
    log_warn("Interrupted");
    exit(0);
}

int main(int argc, const char* argv[])
{
    set_log_level(LOG_INFO);
    setupUser(argc, argv);

    return 0;
}

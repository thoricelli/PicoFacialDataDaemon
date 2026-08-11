#include <binder/ProcessState.h>

#include "facial-tracking-socket.hpp"

int detach()
{
    int pid = fork();
    if (pid > 0)
    {
        return pid;
    }

    setsid();

    return EXIT_SUCCESS;
}

int main()
{
    int pid = detach();

    if (pid > 0)
    {
        printf("Daemon started on PID: %d\n", pid);
        return EXIT_SUCCESS;
    }

    android::ProcessState::self()->startThreadPool();

    FacialTrackingSocket *socket = new FacialTrackingSocket();
    socket->Listen();

    return 0;
}
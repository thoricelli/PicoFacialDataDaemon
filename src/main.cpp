#include <binder/ProcessState.h>

#include <cstdio>
#include <cstring>

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

void printUsage(const char *name)
{
    printf("Usage: %s [--client <IPv4 address>]\n", name);
    printf("  --client  Only accept discovery requests from, and send tracking data to, this address.\n");
    printf("            Without it, any device on the network can request the tracking data.\n");
}

int main(int argc, char *argv[])
{
    in_addr_t allowedClient = INADDR_ANY;
    const char *allowedClientArgument = nullptr;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--client") == 0 && i + 1 < argc)
        {
            in_addr address{};
            allowedClientArgument = argv[++i];

            if (inet_pton(AF_INET, allowedClientArgument, &address) != 1 || address.s_addr == INADDR_ANY)
            {
                fprintf(stderr, "Invalid --client address: %s\n", allowedClientArgument);
                return EXIT_FAILURE;
            }

            allowedClient = address.s_addr;
        }
        else
        {
            printUsage(argv[0]);
            return strcmp(argv[i], "--help") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

    int pid = detach();

    if (pid > 0)
    {
        printf("Daemon started on PID: %d\n", pid);

        if (allowedClientArgument != nullptr)
            printf("Only accepting client: %s\n", allowedClientArgument);

        return EXIT_SUCCESS;
    }

    android::ProcessState::self()->startThreadPool();

    FacialTrackingSocket *socket = new FacialTrackingSocket(allowedClient);
    socket->Listen();

    return 0;
}
#pragma once

#include <sys/socket.h>
#include <linux/in.h>
#include <sys/endian.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <ifaddrs.h>
#include <linux/if.h>
#include <vector>

#include "facial-tracking.hpp"

#define PORT 9030
#define MULTICAST_ADDRESS "239.255.255.250"

#define DISCOVER_PING "DISCOVER_DAEMON"

// hehe
#define PING "MARCO"
#define REPLY "POLO"
#define DAEMON_STOP "STOP"

class FacialTrackingSocket
{
public:
    /**
     * @param allowedClient Only accept discovery requests from this IPv4 address, in network byte order.
     * INADDR_ANY accepts a discovery request from any address.
     */
    explicit FacialTrackingSocket(in_addr_t allowedClient = INADDR_ANY);
    void Listen();

private:
    /**
     * Manually polls the shared memory data buffer, which is pretty much how it is done through the Unity API.
     * Adding a service listener doesn't work, so this is a stand-in replacement.
     */
    void Poll(std::chrono::nanoseconds pollInterval);
    bool Send();
    void Ping();
    void WaitForStop();

    void RegisterSigKillHandler();
    static void SigKillHandler(int signalNumber);

    /**
     * Waits for a discovery request from an allowed client and stores its address in client.
     * Returns false if no client was found because the daemon is stopping or the socket failed.
     */
    bool Discover(sockaddr_in *client);
    void SetupClientSocket();

    in_addr_t allowedClient;
    int facialDataSocket;
    std::atomic<bool> connected{false};
    std::atomic<bool> active{false};
    std::atomic<bool> kill{false};

    std::atomic<bool> stopThreadRunning{false};
    std::condition_variable cv;
    std::mutex cvMutex;

    FacialTracking *facialTracking;
};
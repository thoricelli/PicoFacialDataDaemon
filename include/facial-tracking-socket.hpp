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
    FacialTrackingSocket();
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

    sockaddr_in Discover();
    void SetupClientSocket();

    int facialDataSocket;
    std::atomic<bool> connected;
    std::atomic<bool> active;
    std::atomic<bool> kill;

    std::atomic<bool> stopThreadRunning;
    std::condition_variable cv;
    std::mutex cvMutex;

    FacialTracking *facialTracking;
};
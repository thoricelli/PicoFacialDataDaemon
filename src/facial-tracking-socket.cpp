#include <thread>

#include "facial-tracking-socket.hpp"

FacialTrackingSocket::FacialTrackingSocket()
{
    this->facialTracking = new FacialTracking();
}

void FacialTrackingSocket::Listen()
{
    while (true)
    {
        // Discover the client from the multicast broadcast.
        sockaddr_in client = this->Discover();

        this->facialDataSocket = socket(AF_INET, SOCK_DGRAM, 0);

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(this->facialDataSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

        connect(this->facialDataSocket, (struct sockaddr *)&client, sizeof(client));
        this->connected.store(true);

        std::thread pingThread(&FacialTrackingSocket::Ping, this);

        // The headset might be sleeping, therefore we cannot start the eye tracking algorithm. We'll have to poll and wait...
        do
        {
            this->active.store(this->facialTracking->Start());

            if (!this->active.load())
                std::this_thread::sleep_for(std::chrono::seconds(1));

        } while (this->connected && !this->active.load());

        this->RegisterSigKillHandler();

        // The show is on! Poll is blocking and quits once the ping thread detects no reply anymore.
        if (this->connected.load())
            this->Poll(std::chrono::milliseconds(10));

        this->facialTracking->Stop();

        if (pingThread.joinable())
            pingThread.join();

        close(this->facialDataSocket);

        if (this->kill)
            return;
    }
}

void FacialTrackingSocket::Poll(std::chrono::nanoseconds pollInterval)
{
    while (this->Send() && this->connected.load())
    {
        std::this_thread::sleep_for(pollInterval);
    }
}

void FacialTrackingSocket::Ping()
{
    while (this->connected.load())
    {
        bool pingReceived = false;
        for (int i = 0; i < 5; i++)
        {
            send(this->facialDataSocket, PING, sizeof(PING), 0);

            char buffer[128];
            ssize_t bytesRead = recv(this->facialDataSocket, buffer, sizeof(buffer), 0);

            if (bytesRead > 0)
            {
                if (std::string_view(buffer, bytesRead) == REPLY)
                {
                    pingReceived = true;
                }
                else if (std::string_view(buffer, bytesRead) == DAEMON_STOP)
                {
                    this->connected.store(false);

                    return;
                }

                break;
            }
        }

        if (!pingReceived)
        {
            this->connected.store(false);
            return;
        }

        this->stopThreadRunning.store(true);

        // While we wait, we spin up a STOP thread if the module sends STOP we can then immediately stop.
        std::thread stopThread(&FacialTrackingSocket::WaitForStop, this);

        // Ping every second when we are waiting for the headset to wake up, to keep the module active.
        std::this_thread::sleep_for(
            this->active.load() ? std::chrono::seconds(25) : std::chrono::seconds(1));

        // Once we are done, stop the thread that waits for a STOP signal.
        this->stopThreadRunning.store(false);
        this->cv.notify_all();

        if (stopThread.joinable())
            stopThread.join();
    }
}

void FacialTrackingSocket::WaitForStop()
{
    while (this->stopThreadRunning.load())
    {
        char buffer[4];
        ssize_t bytesRead = recv(this->facialDataSocket, buffer, sizeof(buffer), MSG_DONTWAIT);

        if (bytesRead > 0 && std::string_view(buffer, bytesRead) == DAEMON_STOP)
        {
            this->connected = false;

            return;
        }

        std::unique_lock<std::mutex> lock(this->cvMutex);

        bool stopped = this->cv.wait_for(lock, std::chrono::milliseconds(500), [this]
                                         { return !this->stopThreadRunning.load(); });

        if (stopped)
            break;
    }
}

bool FacialTrackingSocket::Send()
{
    PxrFTInfo *faceTrackingData;
    pxr_eyepose_data_v2_0 *eyeTrackingData;

    this->facialTracking->GetFacialData(&faceTrackingData, &eyeTrackingData);

    if (faceTrackingData == nullptr || eyeTrackingData == nullptr)
    {
        this->active.store(false);
        return true;
    }

    struct iovec iov[2];
    iov[0].iov_base = faceTrackingData;
    iov[0].iov_len = sizeof(PxrFTInfo);
    iov[1].iov_base = eyeTrackingData;
    iov[1].iov_len = sizeof(pxr_eyepose_data_v2_0);

    struct msghdr msg = {};
    msg.msg_name = 0;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;

    ssize_t bytesSent = sendmsg(this->facialDataSocket, &msg, 0);

    if (bytesSent < 0)
        return false;

    return true;
}

sockaddr_in FacialTrackingSocket::Discover()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(PORT);

    bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));

    // To find what PC we can send the tracking data to, we join this multicast group, wait for a ping, and then start sending data to that IP.
    ip_mreq group{};
    group.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
    group.imr_interface.s_addr = INADDR_ANY;

    setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));

    char buffer[sizeof(DISCOVER_PING)];
    sockaddr_in sender_addr{};
    socklen_t sender_len = sizeof(sender_addr);

    long bytesReceived = recvfrom(
        sock,
        buffer,
        sizeof(buffer) - 1,
        0,
        (struct sockaddr *)&sender_addr,
        &sender_len);

    char sender_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender_addr.sin_addr, sender_ip, sizeof(sender_ip));

    setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&group, sizeof(group));
    close(sock);

    return sender_addr;
}

FacialTrackingSocket *instance = nullptr;

void FacialTrackingSocket::RegisterSigKillHandler()
{
    instance = this;

    struct sigaction sa{};
    sa.sa_handler = &FacialTrackingSocket::SigKillHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
}

void FacialTrackingSocket::SigKillHandler(int signalNumber)
{
    if ((signalNumber == SIGTERM || signalNumber == SIGINT) && instance != nullptr)
    {
        instance->connected.store(false);
        instance->kill.store(true);
    }
}
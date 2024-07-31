#pragma once

#include <unordered_set>

#include "../IServer.hpp"
#include "../utils/socket.hpp"

namespace servers
{

class EpollServer: public IServer
{
public:
    EpollServer();

    bool start() override;

private:
    void handleClientConnections() override;
    int acceptNewClient() override;
    void processClientMessage(int clientFd) override;
    void removeClient(int clientFd) override;

    Socket socket_;
    std::unordered_set<int> clients_;
};

}
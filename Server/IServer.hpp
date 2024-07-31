#pragma once

namespace servers {

class IServer
{
public:
    virtual bool start() = 0;

    virtual ~IServer() {}

private:
    virtual void handleClientConnections() = 0;
    virtual int acceptNewClient() = 0;
    virtual void processClientMessage(int clientFd) = 0;
    virtual void removeClient(int clientFd) = 0;
};

}
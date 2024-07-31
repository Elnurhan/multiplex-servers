#include "poll.hpp"

#include <iostream>
#include <sys/socket.h>
#include <poll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <cstring>

#include "../utils/utils.hpp"


namespace servers 
{

namespace
{

constexpr int POLL_SIZE = 3000;

}

PollServer::PollServer()
{
    if (!socket_.initSocket()) {
        std::cerr << "can't create socket\n";
    }
}

bool PollServer::start() 
{
    if (socket_.getServerSocket() <= 0) {
        std::cerr << "socket is not existing\n";
        return false;
    }

    setNonblock(socket_.getServerSocket());

    if (!socket_.setSocketOptions()) return false;
    if (!socket_.bindSocket()) return false;
    if (!socket_.startListening()) return false;

    handleClientConnections();

    return true;
}

void PollServer::handleClientConnections()
{
    struct pollfd pollSet[POLL_SIZE];
    pollSet[0].fd = socket_.getServerSocket();
    pollSet[0].events = POLLIN;

    while (true) 
    {
        int index = 1;
        for (int sd: clients_)
        {
            pollSet[index].fd = sd;
            pollSet[index].events = POLLIN;
            ++index;
        }

        unsigned int setSize = 1 + clients_.size();
        if (poll(pollSet, setSize, -1) < 0)
        {
            std::cerr << "poll error";
            return;
        }

        for (size_t i = 0; i < setSize; ++i)
        {
            if (pollSet[i].revents & POLLIN) 
            {
                if (i == 0) 
                {
                    if (acceptNewClient() < 0)
                    {
                        std::cerr << "can't accept new client";
                        return;
                    }
                }
                else 
                {
                    processClientMessage(pollSet[i].fd);
                }
            }
        }


    }
}

int PollServer::acceptNewClient()
{
    int clientFd = accept(socket_.getServerSocket(), static_cast<sockaddr*>(NULL), NULL);
    if (clientFd < 0) {
        std::cerr << "accept error\n";
        return clientFd;
    }

    setNonblock(clientFd);

    clients_.insert(clientFd);
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    getpeername(clientFd, (sockaddr*)&clientAddr, &addrLen);
    std::cout << "new client connection\n";
    std::cout << "new connection, socket fd is " << clientFd << ", ip is: "
              << inet_ntoa(clientAddr.sin_addr) << ", port: " << ntohs(clientAddr.sin_port) << "\n";

    return clientFd;
}

void PollServer::processClientMessage(int clientFd)
{
    char message[1024];
    size_t valread = read(clientFd, message, 1024);
    if (valread == 0) 
    {
        removeClient(clientFd);
    }
    else 
    {
        message[valread] = '\0';
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        getpeername(clientFd, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);

        std::string senderIp = inet_ntoa(clientAddr.sin_addr);
        int senderPort = ntohs(clientAddr.sin_port);

        std::string fullMessage = "Message from " + senderIp + ":" + std::to_string(senderPort) + ":> " + message;

        int msgLen = strlen(message);
        for (const int client: clients_) 
        {
            if (client == clientFd) 
            {
                continue;
            }

            int sendResult = send(client, fullMessage.c_str(), fullMessage.length(), 0);
            if (sendResult < 0)
            {
                std::cerr << "send message to client error\n";
                return;
            }
        }
    }
}

void PollServer::removeClient(int clientFd)
{
    std::cout << "client disconnected\n";
    sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);
    getpeername(clientFd, reinterpret_cast<sockaddr*>(&serverAddr), reinterpret_cast<socklen_t*>(&serverAddr));

    std::cout << "host disconnected, ip: " << inet_ntoa(serverAddr.sin_addr) << ", port: " << ntohs(serverAddr.sin_port) << "\n";
    close(clientFd);
    clients_.erase(clientFd);
}

}
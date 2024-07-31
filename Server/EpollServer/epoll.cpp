#include "epoll.hpp"

#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
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

constexpr int MAX_EVENTS = 3000;

}

EpollServer::EpollServer() 
{
    if (!socket_.initSocket()) {
        std::cerr << "can't create socket\n";
    }
}

bool EpollServer::start() 
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

void EpollServer::handleClientConnections()
{
    int ePoll = epoll_create1(0);
    epoll_event event;

    event.data.fd = socket_.getServerSocket();
    event.events = EPOLLIN;
    epoll_ctl(ePoll, EPOLL_CTL_ADD, socket_.getServerSocket(), &event);

    while (true)
    {
        epoll_event events[MAX_EVENTS];
        int quantityEvents = epoll_wait(ePoll, events, MAX_EVENTS, -1);

        for (size_t i = 0; i < quantityEvents; ++i) 
        {
            if (events[i].data.fd == socket_.getServerSocket())
            {
                int clientFd = acceptNewClient();
                if (clientFd < 0) {
                    return;
                }

                event.data.fd = clientFd;
                event.events = EPOLLIN;
                epoll_ctl(ePoll, EPOLL_CTL_ADD, clientFd, &event);
            }
            else 
            {
                processClientMessage(events[i].data.fd);
            }
        }
    }   
}

int EpollServer::acceptNewClient()
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

void EpollServer::processClientMessage(int clientFd)
{
    char message[1024];
    size_t valread = recv(clientFd, message, 1024, MSG_NOSIGNAL);

    if ((valread == 0) && (errno != EAGAIN)) 
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

void EpollServer::removeClient(int clientFd)
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
#include "select.hpp"

#include <iostream>
#include <sys/socket.h>
#include <sys/select.h>
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

SelectServer::SelectServer() 
{
    if (!socket_.initSocket()) {
        std::cerr << "can't create socket\n";
    }
}

bool SelectServer::start() 
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

void SelectServer::handleClientConnections()
{
    fd_set readFds;
    int maxFd;

    while (true) 
    {
        FD_ZERO(&readFds);
        FD_SET(socket_.getServerSocket(), &readFds);
        maxFd  = socket_.getServerSocket();

        for (int sd : clients_)
        {
            FD_SET(sd, &readFds);
            if (sd > maxFd)
            {
                maxFd = sd;
            }
        }

        int activity = select(maxFd + 1, &readFds, NULL, NULL, NULL);
        if (activity < 0) 
        {
            std::cerr << "select error\n";
            continue;
        }

        if (FD_ISSET(socket_.getServerSocket(), &readFds)) {
            if (acceptNewClient() < 0)
            {
                return;
            }
        }

        for (const int sd : clients_) 
        {
            if (FD_ISSET(sd, &readFds)) 
            {
                processClientMessage(sd);
            }
        }
    }
}

int SelectServer::acceptNewClient()
{
    int clientFd = accept(socket_.getServerSocket(), static_cast<sockaddr*>(NULL), NULL);
    if (clientFd < 0) {
        std::cerr << "accept error\n";
        return clientFd;
    }

    clients_.insert(clientFd);
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    getpeername(clientFd, (sockaddr*)&clientAddr, &addrLen);
    std::cout << "new client connection\n";
    std::cout << "new connection, socket fd is " << clientFd << ", ip is: "
              << inet_ntoa(clientAddr.sin_addr) << ", port: " << ntohs(clientAddr.sin_port) << "\n";

    return clientFd;
}

void SelectServer::processClientMessage(int clientFd)
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

void SelectServer::removeClient(int clientFd)
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
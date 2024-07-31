#include "socket.hpp"

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

namespace servers
{

namespace 
{

constexpr int DEFAULT_PORT = 12345;

}

Socket::Socket() 
{
    if (!initSocket()) {
        std::cerr << "can't create socket\n";
    }
}

bool Socket::initSocket()
{
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    return serverSocket_ > 0;
}

bool Socket::setSocketOptions() 
{
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "setSocketopt error\n";
        return false;
    }
    return true;
}

bool Socket::bindSocket()
{
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(DEFAULT_PORT);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if ((bind(serverSocket_, (sockaddr*) &sockAddr, sizeof(sockAddr))) < 0)
    {
        std::cerr << "can't bind socket\n";
        return false;
    }

    return true;
}

bool Socket::startListening() 
{
    if (listen(serverSocket_, SOMAXCONN) < 0) {
        std::cerr << "listen error\n";
        return false;
    }
    return true;
}

int Socket::getServerSocket() const
{
    return serverSocket_;
}

}
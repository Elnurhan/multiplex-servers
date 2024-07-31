#pragma once

namespace servers 
{

class Socket 
{
public:
    Socket();

    int getServerSocket() const;

    bool initSocket();
    bool setSocketOptions();
    bool bindSocket();
    bool startListening();


private:
    int serverSocket_;
};

}
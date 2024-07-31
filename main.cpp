#include <iostream>
#include <memory>

#include "Server/SelectServer/select.hpp"
#include "Server/PollServer/poll.hpp"
#include "Server/EpollServer/epoll.hpp"

int main()
{    
    std::cout << "Choose multiplex server:\n";
    std::cout << "[1] - SelectServer\n";
    std::cout << "[2] - PollServer\n";
    std::cout << "[3] - EpollServer\n";
    std::cout << "Enter the choice: ";
    int choice = 0;
    std::cin >> choice;

    std::unique_ptr<servers::IServer> server;

    switch (choice) 
    {
        case 1:
            server = std::make_unique<servers::SelectServer>();
            server->start();
            break;
        case 2:
            server = std::make_unique<servers::PollServer>();
            server->start();
            break;
        case 3:
            server = std::make_unique<servers::EpollServer>();
            server->start();
            break;
        default:
            std::cerr << "invalid choose\n";
            break;
    }
}
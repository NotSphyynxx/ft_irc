#include "Server.hpp"


Server::Server(char *pt, char *pass)
{
    int check  = -1;
    sockfd = -1;
    struct addrinfo hints;
    struct addrinfo *p;
    int port = myport(pt);
    mypass(pass);
        
    memset(&hints, 0 , sizeof(hints));
    memset(server_info, 0 , sizeof(server_info));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((check = getaddrinfo(NULL, pt, &hints, &server_info)) != 0)
    {
       throw std::runtime_error("getaddrinfo failed !");
    }
    for (p = server_info; p != NULL; p = p->ai_next)
    { 
        if (sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol) == =1)
        {
            std::cerr << "socket failed !\n";
            continue;
        }
        break;
    }
    if (bind(sockfd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        freeaddrinfo(server_info);
        close(sockfd);
        throw std::runtime_error("bind failed !");
    }
    int truee = 1;
   if  (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, truee, sizeof(truee)) == -1)
    {
        freeaddrinfo(server_info);
        close(sockfd);
        throw std::runtime_error("setsockopt failed !");
    }
    if (listen(sockfd, REQUEST) == -1)
    {
        freeaddrinfo(server_info);
        close(sockfd);
        throw std::runtime_error("listen failed !");
    }
    freeaddrinfo(server_info);
    server_info  = NULL;
}

Server::~Server()
{
    freeaddrinfo(server_info);
    close(sockfd);
}
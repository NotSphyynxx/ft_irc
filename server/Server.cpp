#include "Server.hpp"


Server::Server(char *pt, char *pass)
{
    struct addrinfo hints;
    struct addrinfo *p;
    int check  = -1;
    int port = myport(pt);
    
    sockfd = -1;
    if (mypass(pass))
        this->password = pass;   
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
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
    {
        freeaddrinfo(server_info);
        close(sockfd);
        throw std::runtime_error("fcntl failed !");
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
    //you may need to save server ip
    if (listen(sockfd, REQUEST) == -1)
    {
        freeaddrinfo(server_info);
        close(sockfd);
        throw std::runtime_error("listen failed !");
    }
    freeaddrinfo(server_info);
    server_info  = NULL;
}


int Server::getsocket()
{
    return (this->sockfd);
}

int Server::run()
{
    std::vector <struct pollfd> sockarray;
    int sock = getsocket();
    sockarray.push_back({sock, POLLIN, 0});
    int p = -1;
    while (1)
    {
        p = poll(sockarray.data(), sockarray.size(), -1);
        if (p == -1)
        {
            throw std::runtime_error("poll failed");
        }
        if (p == 0)
        {
            return 0;// nothing happened (timeout)
        }
        else
        {
            for (size_t i = 0; i < sockarray.size(); i++)
            {
                if (sockarray[i].revents == POLLIN)
                {
                    if (sockarray[i].fd == sock) // from listener (a new connection)
                    {
                        handleListener(sockarray, i, sock);
                    }
                    else // a client
                    {
                        handleClient(sockarray, i, sock);
                    }
                }
                
            }
        }



    }

}


int Server::handleListener(std::vector <struct pollfd> &fds, unsigned int i, int sock)
{
    struct sockaddr_storage st;
    struct sockaddr_in *hp;
    socklen_t sz = sizeof(st);
    char    ipv4char[IPV4LEN];

    int new_fd = -1;
    if ((new_fd == accept(sock,(sockaddr *) &st, &sz)) == -1)
    {
        return -1;
    }
    fds.push_back({new_fd, POLLIN, 0});
    hp =  (struct sockaddr_in *) &st;
    std::cout << "new connection  from : "
    << inet_ntop(hp.sa_family, &hp.sin_addr, ipv4char, sizeof(ipv4char))
    << std::cout;

    return 1;
}

int Server::handleClient(std::vector <struct pollfd> &fds, unsigned int i, int sock)
{
    char buff[BUFFER];
    struct 
    
}

const std::string Server::getpass()
{
    return this->password;
}
Server::~Server()
{
    freeaddrinfo(server_info);
    close(sockfd);
}
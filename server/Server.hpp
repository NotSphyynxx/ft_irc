#ifndef SERVER_HPP 
#define SERVER_HPP

#include <sys/socket.h> // Core socket functions
#include <netinet/in.h> // sockaddr_in structure
#include <arpa/inet.h>  // inet_addr tools
#include <unistd.h>     // close()
#include <fcntl.h>      // fcntl() for non-blocking
#include <poll.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

#define PORT 2020
#define REQUEST 10
#define IPV4LEN 16
#define BUFFER 1024
#define SERVER_NAME "ft_irc.2004.ma"



class Server
{
    private :
        int         sockfd;
        std::string password;
        Server &operator=(const Server  &other);
        Server(const Server &other);
        Server();
    protected :
        struct addrinfo *server_info;
        struct sockaddr_in addsock_in;//you need to set it to zero (the kernel expects the unused bytes to be zero)
        struct sockaddr_storage both_ipv // in case you dont know wich ipv (4 or 6)
        
        
    public :
        Server(char *port, char *password);
        ~Server();
        int run();
        int getsocket();
        int handleListener(std::vector <struct pollfd> &fds, unsigned int i, int sock);
        int handleClient(std::vector <struct pollfd> &fds, unsigned int i, int sock);
        const std::string &getpass();
    

};

int     myport(char *port);
bool    mypass(char *pass);
int     parsechannel();

#endif



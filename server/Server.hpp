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

#define PORT 2020
#define REQUEST 10



class Server
{
    private :
        int sockfd;
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

    

};

int     myport(char *port);
bool    mypass(char *pass);

#endif



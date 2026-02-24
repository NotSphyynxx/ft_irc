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
// #include <winsock2.h>
// #include <ws2tcpip.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <string>
#include <ctime>
// #include "windows.h"
#include <cerrno>
#include <sstream>
#include <cstdlib>

//#include <Client.hpp>


#define PORT 2020
#define REQUEST 10
#define IPV4LEN 16
#define BUFFER 1024
#define SERVER_NAME "ft_irc.2004.ma"
#define MAX_CHANNEL 10
#define MAX_CLIENT 10000

class Client;
// #include "Client.hpp" // i guess you gonna remove it 
typedef std::map <int , Client> cmaps;
typedef std::vector <struct pollfd> pollvec;

class Server
{
   
    private :
        int         sockfd;
        std::string password;
        Server &operator=(const Server  &other);
        Server(const Server &other);
        Server();
        cmaps _client;
        pollvec sockarrayy;
        struct addrinfo *serverI;

        
        
    public :
        Server(char *port, char *password);
        ~Server();
        int run();
        int getsocket();
        int NewConnection(std::vector <struct pollfd> &fds, int sock);
        int RecieveMessage(std::vector <struct pollfd> &fds, int sock);
        int sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock);
        const std::string &getpass() const;
        void removeClient(int fd);
        bool clientExists(int fd) const;
        Client& getClient(int fd);
        const cmaps & getcmaps();
        pollvec &getpollstruct();
        bool sameName(std::string &nickname);
        void closeSocket(pollvec &fds, int sock);
        int checkTimeout(pollvec &fds);
        int checkPollout(pollvec &fds);
        struct addrinfo *getServerI();
        void addClient(int fd);
        void processCommand(pollvec &fds, std::string line, int sock);
        void broadcast(pollvec &fds, std::string message);

    

};

int     myport(char *port);
bool    mypass(char *pass);
int     parsechannel();

#endif



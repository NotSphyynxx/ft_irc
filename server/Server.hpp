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
// #include <windows.h>
#include <cerrno>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include "Channel.hpp"
//#include <Client.hpp>


#define PORT 2020
#define REQUEST 10
#define IPV4LEN 16 // the ipv4 to char has 16 char including \0
#define BUFFER 1024
#define SERVER_NAME "ft_irc.2004.ma"
#define MAX_CHANNEL 10
#define MAX_CLIENT 10000

#define ERR_NOTREGISTERED(server) (":" + std::string(server) + " 451 * :You have not registered\r\n")
#define ERR_CLOSINGLINK(host, reason) ("ERROR :Closing Link: " + std::string(host) + " (" + std::string(reason) + ")\r\n")
#define ERR_PASSWDMISMATCH(s) (":" + std::string(s) + " 464 * :Password incorrect\r\n")
#define ERR_NONICKNAME(s) (":" + std::string(s) + " 431 * :No nickname given\r\n")
#define ERR_ERRONEUSNICK(s, n) (":" + std::string(s) + " 432 * " + std::string(n) + " :Erroneous nickname\r\n")
#define ERR_NICKINUSE(s, n) (":" + std::string(s) + " 433 * " + std::string(n) + " :Nickname is already in use\r\n")
#define ERR_ALREADYREG(s) (":" + std::string(s) + " 462 * :Unauthorized command (already registered)\r\n")
#define ERR_NEEDMOREPARAMS(s, c) (":" + std::string(s) + " 461 * " + std::string(c) + " :Not enough parameters\r\n")


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
        std::map<std::string, Channel> _channels;
        pollvec sockarrayy;
        struct addrinfo *serverI;
        std::string serverIp;

        
        
    public :
        Server(char *port, char *password);
        ~Server();
        int run();
        int getsocket();
        std::string getServerIp();
        void setServerIp(std::string ip);
        int NewConnection(std::vector <struct pollfd> &fds, int sock);
        int RecieveMessage(std::vector <struct pollfd> &fds, int sock);
        int sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock);
        std::string getpass();
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
        Channel* getChannel(std::string name);
        void createChannel(std::string name, Client &cl);
    

	private :
		int         sockfd;
		std::string password;
		Server &operator=(const Server  &other);
		Server(const Server &other);
		Server();
		cmaps _client;
		pollvec sockarrayy;
		struct addrinfo *serverI;
		std::string serverIp;



	public :
		Server(char *port, char *password);
		~Server();
		int run();
		int getsocket();
		std::string getServerIp();
		void setServerIp(std::string ip);
		int NewConnection(std::vector <struct pollfd> &fds, int sock);
		int RecieveMessage(std::vector <struct pollfd> &fds, int sock);
		int sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock);
		std::string getpass();
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
		void	addClient(int fd);
		void	processCommand(pollvec &fds, std::string line, int sock);
		void	broadcast(pollvec &fds, std::string message);
		long	getclientbyNick(const std::string &nick);



};

int     myport(char *port);
bool    mypass(char *pass);
int     parsechannel();

#endif



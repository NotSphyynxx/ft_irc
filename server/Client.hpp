#ifndef CLIENT_HPP
#define CLIENT_HPP

// #include <sys/socket.h> // Core socket functions
// #include <netinet/in.h> // sockaddr_in structure
// #include <arpa/inet.h>  // inet_addr tools
// #include <unistd.h>     // close()
// #include <fcntl.h>      // fcntl() for non-blocking
// #include <poll.h>
// #include <sys/types.h>
// #include <netdb.h>
// #include <cstring>
// // #include <winsock2.h>
// // #include <ws2tcpip.h>
// #include <iostream>
// #include <stdexcept>
// #include <vector>
// #include <fcntl.h>
// #include <unistd.h>
// #include <map>
// #include <string>
// #include <ctime>
// // #include "windows.h"
// #include <cerrno>

#include "Server.hpp"
class Server;
enum Level
{
    hasPASS,
    hasNICK,
    hasUSER,
    REGISTRED,
    EMPTY,
};



class Client
{
    private :
        int         mysocket;
        std::string buffer;
        std::string outbuffer;
        Level       rank[4];
        std::string username;
        std::string nickname;
        std::string realname;
        std::string myIp;
        time_t      connectTime;
        time_t      lastActivity;


        Client();
    public:
        Client(int sock);
        Level         getlevel(unsigned int i);
        std::string   &getusername();
        std::string   &getnickname();
        std::string   &getrealname();
        std::string   &getBuffer();
        std::string   &getIp();
        time_t        &getconnecttime();
        std::string   &getoutbuffer();
        time_t        &getLastActivity();
        void          setoutbuffer(std::string outbuffer);
        void          setIp(std::string Ip);
        void          setlevel(unsigned int index, Level value);
        void          setusername(std::string _username);
        void          setnickname(std::string _nickname);
        void          setrealname(std::string _realname);
        void          setBuffer(std::string buffer);
        void          setconnecttinme(time_t tm);
        void          setLastActivity(time_t tm);
        std::string   appand(std::string buf);
        void        process_buffer();
        int         Authentication(Server &sv);
        bool         user(std::string &extracted);
        bool        pass(std::string &pass,  Server &sv);
        bool        nick(std::string &nickname, Server &sv);
        int         getsock();
        void        sendWelcome();
        bool        Emptynames();
       

};
#endif
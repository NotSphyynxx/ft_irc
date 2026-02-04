#ifndef CLIENT_HPP
#define CLIENT_HPP


#include "Server.hpp"

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
        std::string buffer[BUFFER];
        Level       rank[4];
        std::string username;
        std::string nickname;
        std::string realname;
        std::string myIp;


        Client();
    public:
        Client(int sock);
        Level         getlevel(unsigned int i);
        std::string   &getusername();
        std::string   &getnickname();
        std::string   &getrealname();
        std::string   &getIp();
        void          setIP(std::string Ip);
        void          setlevel(unsigned int index, Level value);
        void          setusername(std::string _username);
        void          setnickname(std::string _nickname);
        void          setrealname(std::string _nickname);
        std::string &appand(std::string buf);
        void        process_buffer();
        int         Authentication(const Server &sv);
        int         user(std::string &extracted);
        bool        pass(std::string &extracted);
        int         nick(std::string &extracted);
        const int   getsock();
        void        sendWelcome(const Server &sv);
        bool        Emptynames();

}
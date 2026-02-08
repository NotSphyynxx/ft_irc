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
        std::string buffer;
        std::string outbuffer;
        Level       rank[4];
        std::string username;
        std::string nickname;
        std::string realname;
        std::string myIp;
        time_t      connectTime;


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
        void          setoutbuffer(std::string outbuffer);
        void          setIp(std::string Ip);
        void          setlevel(unsigned int index, Level value);
        void          setusername(std::string _username);
        void          setnickname(std::string _nickname);
        void          setrealname(std::string _realname);
        void          setBuffer(std::string buffer);
        void          setconnecttinme();
        std::string &appand(std::string buf);
        void        process_buffer();
        int         Authentication(const Server &sv);
        bool         user(std::string &extracted, const Server &sv);
        bool        pass(std::string &pass, const Server &sv);
        bool         nick(std::string &nickname, const Server &sv);
        const int   getsock();
        void        sendWelcome(const Server &sv);
        bool        Emptynames();
       

}
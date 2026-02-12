#include "Client.hpp"

std::string  Client::appand(std::string buf)
{
    if (buf.empty())
        return (this->buffer);
    this->buffer += buf;
    return this->buffer;
}

//getters and setters
int Client::getsock()
{
    return this->mysocket;
}
std::string &Client::getnickname()
{
    return this->nickname;
}
std::string &Client::getusername()
{
    return this->username;
}
std::string &Client::getrealname()
{
    return this->realname;
}
std::string &Client::getBuffer()
{
    return this->buffer;
}
void Client::setnickname(std::string s)
{
    std::string clean;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] != '\r' && s[i] != '\n')
            clean += s[i];
    }
    this->nickname = clean;
    std::string endmsg = "you're nickname is " + getnickname() + "\r\n";
    send(getsock(), endmsg.c_str(), endmsg.size(), 0);
}
void Client::setusername(std::string s)
{
    std::string clean;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] != '\r' && s[i] != '\n')
            clean += s[i];
    }
    this->username = clean;
    std::string endmsg = "you're username is " + getusername() + "\r\n";
    send(getsock(), endmsg.c_str(), endmsg.size(), 0);
}
void Client::setrealname(std::string s)
{
    std::string clean;

    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] != '\r' && s[i] != '\n')
            clean += s[i];
    }
    this->realname = clean;
    std::string endmsg = "you're realname is " + getrealname() + "\r\n";
    send(getsock(), endmsg.c_str(), endmsg.size(), 0);
}
void Client::setBuffer(std::string buf)
{
    this->buffer += buf;
}
Client::Client(int sock) : mysocket(sock)
{
    for (int i = 0; i < 4 ; i++)  
        setlevel(i , EMPTY);
}

Level Client::getlevel(unsigned int  index)
{
    return this->rank[index];
}
void Client::setlevel(unsigned int index, Level value)
{
    this->rank[index] = value;
}

std::string &Client::getIp()
{
    return this->myIp;
}
void Client::setIp(std::string ip)
{
    this->myIp = ip;
}

time_t &Client::getconnecttime()
{
    return this->connectTime;
}

void Client::setconnecttinme(time_t tm)
{
    this->connectTime = tm;
}

std::string &Client::getoutbuffer()
{
    return this->outbuffer;
}
void Client::setoutbuffer(std::string outbuff) // 0 for erase
{
     this->outbuffer = outbuff;
}


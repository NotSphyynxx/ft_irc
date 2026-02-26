#include "Client.hpp"
#include "Server.hpp"


/******************************************************/
/*                     SERVER                         */
/******************************************************/

// ─────────────── GETTERS ───────────────
int Server::getsocket()
{
    return (this->sockfd);
}

std::string Server::getpass()
{
    return this->password;
}

Client& Server::getClient(int fd)
{
    return _client.at(fd); 
}

const cmaps &Server::getcmaps()
{
    return this->_client;
}

pollvec &Server::getpollstruct()
{
    return this->sockarrayy;
}

struct addrinfo *Server::getServerI()
{
    return this->serverI;
}

std::string Server::getServerIp()
{
    return this->serverIp;
}

// ─────────────── SETTERS ───────────────
void Server::setServerIp(std::string ip)
{
    this->serverIp = ip;
}








/******************************************************/
/*                     CLIENT                         */
/******************************************************/



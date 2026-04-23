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


Channel* Server::getChannel(std::string name) {
    std::map<std::string, Channel>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return &(it->second);
    return NULL;
}

void Server::createChannel(std::string name, Client &cl) {
    // Create the channel and insert it into the map
    _channels.insert(std::make_pair(name, Channel(name)));
    // Add the creator to the channel
    _channels[name].addMember(&cl);
}
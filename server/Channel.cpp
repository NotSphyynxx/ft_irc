#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel() : _name("") {}

Channel::Channel(std::string name) : _name(name) {}

Channel::~Channel() {}

std::string Channel::getName() const {
    return _name;
}

std::vector<Client*> Channel::getMembers() const {
    return _clients;
}

void Channel::addMember(Client* client) {
    _clients.push_back(client);
}

void Channel::broadcastMessage(std::string message, Client* excludeClient) {
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (_clients[i] != excludeClient) {
            _clients[i]->getoutbuffer() += message;
        }
    }
}

std::string Channel::getMemberListAsString() {
    std::string list = "";
    for (size_t i = 0; i < _clients.size(); ++i) {
        if (isOperator(_clients[i]))
            list += "@";
        list += _clients[i]->getnickname();
        if (i < _clients.size() - 1)
            list += " ";
    }
    return list;
}

void Channel::addOperator(Client* client){
    _operators.push_back(client); 
}

bool Channel::isOperator(Client* client){
    for (size_t i = 0; i < _operators.size(); ++i){
        if (_operators[i] == client)
            return true;
    }
    return false;
}


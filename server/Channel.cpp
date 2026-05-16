#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel() : _name(""), _histopic(true) {}

Channel::Channel(std::string name) : _name(name), _histopic(true) {}

Channel::~Channel() {}
void Channel::settrueHistopic() {
    _histopic = true;
}
void Channel::setfalsehistopic() {
    _histopic = false;
}
bool Channel::gethistopic() const {
    return _histopic;
}

std::string Channel::getTopicSetter() const {
    return _topicSetter;
}
time_t Channel::getTopicSetTime() const {
    return _topicSetTime;
};

void Channel::setTopicSetter(const std::string& setter) {
    _topicSetter = setter;
};

void Channel::setTopicSetTime(time_t time) {
    _topicSetTime = time;
};

void Channel:: setTopic(const std::string& topic) {
    _topic = topic;
};
std::string Channel::gettopic() const {
    return _topic;
};
// std::string Channel::getModesString() const
// {
//     std::string modes = "+";
//     std::string params;

//     if (this->_inviteOnly)
//         modes += "i";

//     if (this->_topicRestricted)
//         modes += "t";

//     if (this->_hasKey)
//     {
//         modes += "k";
//         params += " " + this->_key;
//     }

//     // if (this->_hasLimit)
//     // {
//     //     modes += "l";
//     //     std::stringstream ss;
//     //     ss << this->_limit;
//     //     params += " " + ss.str();
//     // }

//     return modes + params;
// }

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

void Channel::inviteUser(std::string nickname){
    _invited.push_back(nickname);
}

bool Channel::isInvited(std::string nickname){
    for(size_t i = 0; i < _invited.size(); ++i){
        if (_invited[i] == nickname)
            return true;
    }
    return false;
}

bool Channel::isMember(Client* client){
    for (size_t i = 0; i < _clients.size(); ++i){
        if (_clients[i] == client)
            return true;
    }
    return false;
}

void Channel::removeMember(Client* client){
    for (std::vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it){
        if (*it == client){
            _clients.erase(it);
            break;
        }
    }
    for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it){
        if (*it == client) {
            _operators.erase(it);
            break;
        }
    }
}


// tal3i funcrtion

Client* Channel::getClientByNickname(const std::string& nickname)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i]->getnickname() == nickname)
            return _clients[i];
    }
    return NULL;
}

void Channel::removeOperator(Client* client)
{
    for (size_t i = 0; i < _operators.size(); i++)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            return;
        }
    }
}

void Channel::setKey(const std::string& key)
{
    _key = key;
}

std::string Channel::getKey() const
{
    return _key;
}

void Channel::removeKey()
{
    _key = "";
}

bool Channel::hasKey() const
{
    return !_key.empty();
}

void Channel::setLimit(int limit)
{
    _limit = limit;
}

void Channel::removeLimit()
{
    _limit = 0;
}

int Channel::getLimit() const
{
    return _limit;
}

bool Channel::hasLimit() const
{
    return _limit > 0;
}
void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
}
bool Channel::getInviteOnly() const
{
    return _inviteOnly;
}
std::string Channel::getModesString() const {
    std::string modes = "+";
    if (this->_inviteOnly)
        modes += "i";
    if (this->_histopic)
        modes += "t";
    if (!this->_key.empty())
        modes += "k";
    if (this->_limit > 0)
        modes += "l";

    if (!this->_key.empty())
        modes += " " + this->_key;
    if (this->_limit > 0)
    {
        std::stringstream ss;
        ss << this->_limit;
        modes += " " + ss.str();
    }
    return modes;
}
//tal3i fnuction
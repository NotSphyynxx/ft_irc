#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
// #include "Server.hpp"

class Client;
class Channel {
    private:
        std::string _name;
        std::vector<Client*> _clients;
        std::vector<Client*> _operators;
        std::vector<std::string> _invited;

    public:
        Channel();
        Channel(std::string name);
        ~Channel();

        std::string getName() const;
        std::vector<Client*> getMembers() const;
        std::string getMemberListAsString(); 

        void addOperator(Client* client);
        bool isOperator(Client* client);

        void addMember(Client* client);
        bool isMember(Client* client);
        void removeMember(Client* client); 
        void inviteUser(std::string nickname);
        bool isInvited(std::string nickname);

        void broadcastMessage(std::string message, Client* excludeClient = NULL);

};

#endif
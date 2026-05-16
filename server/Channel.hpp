#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <ctime>
// #include "Server.hpp"

class Client;
class Channel {
    private:
        std::string _name;
        std::string _key;
        std::vector<Client*> _clients;
        std::vector<Client*> _operators;
        std::vector<std::string> _invited;
        std::string _topic;
        int _limit;
        bool _histopic;
        bool _inviteOnly;
        std::string _topicSetter;
        time_t _topicSetTime;

    public:
    Channel();
    Channel(std::string name);
    ~Channel();

    std::string getName() const;
    std::vector<Client*> getMembers() const;
    std::string getMemberListAsString(); 
    void settrueHistopic() ;
    std::string getTopicSetter() const;
    time_t getTopicSetTime() const;
    void setTopicSetter(const std::string& setter);
    void setTopicSetTime(time_t time);

    bool gethistopic() const ;

    void setTopic(const std::string& topic);
    std::string gettopic() const;


    void setfalsehistopic() ;
    void addOperator(Client* client);
    bool isOperator(Client* client);
    std::string getModesString() const;

    void addMember(Client* client);
    bool isMember(Client* client);
    void removeMember(Client* client); 
    void inviteUser(std::string nickname);
    bool isInvited(std::string nickname);

    void broadcastMessage(std::string message, Client* excludeClient = NULL);

    Client* getClientByNickname(const std::string& nickname);
    void removeOperator(Client* client);

    void setKey(const std::string& key);
    std::string getKey() const;
    void removeKey();
    bool hasKey() const;

    void setLimit(int limit);
    void removeLimit();
    int getLimit() const;
    bool hasLimit() const;
    void setInviteOnly(bool inviteOnly);
    bool getInviteOnly() const;
};

#endif
#include "Client.hpp"
#include "Server.hpp"
#include <sstream>

bool removeCRLF(std::string &str)// true there is crlf
{
    std::string cleaned;
    bool flag = false;
    size_t i = 0;
    if (str.find("\r\n") == std::string::npos)
        return flag;

    for (; i < str.size(); ++i)
    {
        if (str[i] != '\r' && str[i] != '\n')
            cleaned += str[i];
        if (str[i] == '\r' || str[i] == '\n')
            flag = true;
    }

    str = cleaned;
    return flag;
}

void Server::processCommand(pollvec &fds, std::string line, int sock)
{
    try
    {
        std::string cmd = line;
        if (removeCRLF(cmd) == false)
            return ;

        Client &cl = getClient(sock); // Get the client once at the top

        if (cmd.find("PONG") == 0)
        {
            cl.setLastActivity(time(NULL));
            cl.setping(false);
        }
        // --- JOIN COMMAND ---
        else if (cmd.find("JOIN") == 0) 
        {
            std::stringstream ss(cmd);
            std::string instruction;
            std::string channelName;
            
            ss >> instruction >> channelName;

            if (channelName.empty()) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 461 JOIN :Not enough parameters\r\n";
                return;
            }

            // Slice 1: Server Memory
            Channel* chan = getChannel(channelName);
            if (chan == NULL) {
                createChannel(channelName, cl);
                chan = getChannel(channelName);
                chan->addOperator(&cl); // Grant the Crown
            } else {
                chan->addMember(&cl);
            }

            // Slice 2: The Protocol Handshake
            std::string nick = cl.getnickname();
            std::string user = cl.getusername();
            std::string host = cl.getIp();

            std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN " + channelName + "\r\n";
            chan->broadcastMessage(joinMsg);

            std::string topicMsg = ":ft_irc.2004.ma 332 " + nick + " " + channelName + " :No topic set\r\n";
            cl.getoutbuffer() += topicMsg;

            std::string nameList = chan->getMemberListAsString();
            std::string namesMsg = ":ft_irc.2004.ma 353 " + nick + " = " + channelName + " :" + nameList + "\r\n";
            cl.getoutbuffer() += namesMsg;

            std::string endNamesMsg = ":ft_irc.2004.ma 366 " + nick + " " + channelName + " :End of /NAMES list.\r\n";
            cl.getoutbuffer() += endNamesMsg;
        }
        // --- INVITE COMMAND ---
        else if (cmd.find("INVITE") == 0)
        {
            std::stringstream ss(cmd);
            std::string instruction, targetNick, channelName;
            
            ss >> instruction >> targetNick >> channelName;

            if (targetNick.empty() || channelName.empty()) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " INVITE :Not enough parameters\r\n";
                return;
            }

            Channel* chan = getChannel(channelName);
            if (chan == NULL) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 403 " + cl.getnickname() + " " + channelName + " :No such channel\r\n";
                return;
            }

            if (!chan->isOperator(&cl)) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 482 " + cl.getnickname() + " " + channelName + " :You're not channel operator\r\n";
                return;
            }

            // We need this function added to GettersSetters.cpp / Server.hpp!
            Client* targetClient = getClientByNickname(targetNick);
            if (targetClient == NULL) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 401 " + cl.getnickname() + " " + targetNick + " :No such nick/channel\r\n";
                return;
            }

            // Add to invite list
            chan->inviteUser(targetNick);

            // Success reply to sender
            cl.getoutbuffer() += ":ft_irc.2004.ma 341 " + cl.getnickname() + " " + targetNick + " " + channelName + "\r\n";
            
            // Actual invite sent to the target user
            std::string inviteMsg = ":" + cl.getnickname() + "!" + cl.getusername() + "@" + cl.getIp() + " INVITE " + targetNick + " :" + channelName + "\r\n";
            targetClient->getoutbuffer() += inviteMsg;
        }
        // --- KICK COMMAND ---
        else if (cmd.find("KICK") == 0)
        {
            std::stringstream ss(cmd);
            std::string instruction, channelName, targetNick;
            
            ss >> instruction >> channelName >> targetNick;

            // 1. Check basic parameters
            if (channelName.empty() || targetNick.empty()) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " KICK :Not enough parameters\r\n";
                return;
            }

            Channel* chan = getChannel(channelName);

            // 2. Check if channel exists
            if (chan == NULL) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 403 " + cl.getnickname() + " " + channelName + " :No such channel\r\n";
                return;
            }

            // 3. Check if the SENDER is actually in the channel
            if (!chan->isMember(&cl)) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 442 " + cl.getnickname() + " " + channelName + " :You're not on that channel\r\n";
                return;
            }

            // 4. Check if SENDER is an Operator
            if (!chan->isOperator(&cl)) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 482 " + cl.getnickname() + " " + channelName + " :You're not channel operator\r\n";
                return;
            }

            // 5. Check if TARGET exists and is in the channel
            Client* targetClient = getClientByNickname(targetNick);
            if (targetClient == NULL || !chan->isMember(targetClient)) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 441 " + cl.getnickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
                return;
            }

            // 6. Extract the reason (Everything after the ':')
            std::string reason = "No reason given";
            size_t colonPos = cmd.find(':', cmd.find(targetNick));
            if (colonPos != std::string::npos) {
                reason = cmd.substr(colonPos + 1);
            }

            // 7. BROADCAST the kick BEFORE removing them!
            // We must send it while the target is still technically in the _clients vector
            // so that targetClient->getoutbuffer() receives this broadcast too.
            std::string kickMsg = ":" + cl.getnickname() + "!" + cl.getusername() + "@" + cl.getIp() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
            chan->broadcastMessage(kickMsg);

            // 8. Physically delete them from the channel's memory
            chan->removeMember(targetClient);
        }
    } 
    catch (const std::out_of_range& e)
    {
       (void)e;
        std::cerr << "getClient() failed (at()) !" << std::endl;
    }
}
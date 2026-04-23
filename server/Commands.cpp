#include "Client.hpp"

bool removeCRLF(std::string &str)// true there is crlf
{
    std::string cleaned;
    bool flag = false;
    size_t i = 0;

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
        removeCRLF(cmd);
        Client &cl = getClient(sock);

        if (cmd.find("PONG") != std::string::npos)
        {
            cl.setLastActivity(time(NULL));
            cl.setping(false);
        }
        // sphynnxxx
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

            // --- SLICE 1: Server Memory ---
            Channel* chan = getChannel(channelName);
            if (chan == NULL) {
                createChannel(channelName, cl);
                chan = getChannel(channelName);
                chan->addOperator(&cl);
            } else {
                // FIXED: Changed addClient to addMember to match your Channel.hpp
                chan->addMember(&cl);
            }

            // --- SLICE 2: The Protocol Handshake ---
            std::string nick = cl.getnickname();
            std::string user = cl.getusername();
            std::string host = cl.getIp();

            // Message 1: The JOIN Echo
            std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN " + channelName + "\r\n";
            chan->broadcastMessage(joinMsg);

            // Message 2: RPL_TOPIC (332)
            std::string topicMsg = ":ft_irc.2004.ma 332 " + nick + " " + channelName + " :No topic set\r\n";
            cl.getoutbuffer() += topicMsg;

            // Message 3: RPL_NAMREPLY (353)
            std::string nameList = chan->getMemberListAsString();
            std::string namesMsg = ":ft_irc.2004.ma 353 " + nick + " = " + channelName + " :" + nameList + "\r\n";
            cl.getoutbuffer() += namesMsg;

            // Message 4: RPL_ENDOFNAMES (366)
            std::string endNamesMsg = ":ft_irc.2004.ma 366 " + nick + " " + channelName + " :End of /NAMES list.\r\n";
            cl.getoutbuffer() += endNamesMsg;
        }
    } // <--- THE BRACE MOVED HERE! Everything is safely inside the try block now.
    catch (const std::out_of_range& e)
    {
       (void)e;
        std::cerr << "getClient() failed (at()) !" << std::endl;
    }
}

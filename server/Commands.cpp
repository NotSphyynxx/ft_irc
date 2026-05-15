#include "Client.hpp"
#include "Server.hpp"
#include <sstream>

std::string timeToString(time_t t)
{
    std::stringstream ss;
    ss << t;
    return ss.str();
}

void Server::processBuffer(Client &cl)
{
	std::string &buffer = cl.getBuffer();
	size_t pos;

	if (cl.getlevel(3) != REGISTRED)
		return ;
	// We search for '\n' instead of "\r\n" to fully support Netcat!
	while ((pos = buffer.find('\n')) != std::string::npos)
	{
		// A. The Cleaver: Cut out the command up to the newline
		std::string singleCommand = buffer.substr(0, pos);

		// B. THE ERASER: Remove the command AND the '\n' byte from the tank
		buffer.erase(0, pos + 1);

		// C. The Netcat Fix: If the command came from HexChat, it will have
		// a '\r' stuck at the very end. We trim it off cleanly!
		if (!singleCommand.empty() && singleCommand[singleCommand.length() - 1] == '\r')
				singleCommand.erase(singleCommand.length() - 1);

		// D. Safety Check: Ignore empty lines (e.g., if they just spammed Enter)
		if (!singleCommand.empty())
		{
			processCommand(singleCommand, cl.getsock());
		}

	}
}


bool	Server::Privmsg(Client &cl , std::string allCmd)
{
	std::string cmd , target , message;
	std::stringstream ss(allCmd);
	ss >> cmd >> target;

	if (target.empty())
	{
		cl.getoutbuffer() += ERR_NORECIPIENT(SERVER_NAME, cl.getnickname(), "PRIVMSG");
		return false;
	}
	std::getline(ss, message);
	size_t pos = message.find(":");
	if (pos != std::string::npos)
		message = message.substr(pos + 1);
	else
		{
			size_t leading_spaces = message.find_first_not_of(" \t");
			if (leading_spaces != std::string::npos)
			{
				size_t remaining_spaces = message.find_last_not_of(" \t");
				message = message.substr(leading_spaces, remaining_spaces - leading_spaces + 1);
			}
			else
				message = "";
		}
	if (message.empty())
	{
		cl.getoutbuffer() += ERR_NOTEXTTOSEND(SERVER_NAME, cl.getnickname());
		return false;
	}
// For Channel
	if (target[0] == '&' || target[0] == '#')
	{
		Channel *chn = getChannel(target);
		if (!chn)
		{
			cl.getoutbuffer() += ERR_NOSUCHNICK(SERVER_NAME, cl.getnickname() , target);
			return false;
		}
		if (chn->isMember(&cl) == false)
		{
			cl.getoutbuffer() += ERR_CANNOTSENDTOCHAN(SERVER_NAME, cl.getnickname() , target);
			return false;
		}
		chn->broadcastMessage(CMD_PRIVMSG(cl.getPrefix(), target, message), &cl);
		return true;
	}
// For Client
	Client *c_target = getClientByNickname(target);

	if (c_target == NULL)
	{
		cl.getoutbuffer() += ERR_NOSUCHNICK(SERVER_NAME, cl.getnickname() , target);
		return false;
	}
	c_target->getoutbuffer() += CMD_PRIVMSG(cl.getPrefix(), target, message);
	return true;
}

bool Server::changeNICK(Client &cl, std::string &nickname)
{
	if (nickname.empty())
	{
		cl.getoutbuffer() += ERR_NONICKNAME(SERVER_NAME);
		return false;
	}
	if (nickname.size() > 9)
	{
		//Nickname too long
		cl.getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
		return false;
	}
	if (!isalpha(nickname[0]) && !isSpecial(nickname[0]))
	{
		cl.getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
		return false;
	}
	for (size_t i = 0; i < nickname.size(); i++)
	{
	   unsigned char c = nickname[i];
		if (!isdigit(c) && !isalpha(c) && !isSpecial(c))
		{
			cl.getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
			return false;
		}
	}// you need to check if there is another client with the same nickname
	if (sameName(nickname))
	{
		cl.getoutbuffer() += ERR_NICKINUSE(SERVER_NAME, nickname);
		return false;
	}
	std::string broadcast = BROADCAST_NICK(cl.getnickname(), cl.getusername(), cl.getIp(), nickname);
	broadcastToSharedChannels(cl, broadcast);
	cl.setnickname(nickname);

	return true;

}

void Server::broadcastToSharedChannels(Client &ignored, std::string & messages)
{
	chnmap::iterator it = _channels.begin();
	Channel *chn;

	while (it != _channels.end())
	{
		chn = &(it->second);
		if (chn->isMember(&ignored))
			chn->broadcastMessage(messages, &ignored);
		it++;
	}
}

void Server::processCommand(std::string line, int sock)
{
	try
	{
		Client &cl = getClient(sock);
		if (cl.getlevel(3) != REGISTRED)
			return ;

		std::string allCmd, cmd, token , prefix;
		allCmd = line;

		std::stringstream stream_me(allCmd);
		stream_me >> cmd;

	if (!cmd.empty() && cmd[0] == ':')
 	{
 		prefix = cmd;
		stream_me >> cmd;// The NEXT word is the actual command
		stream_me >> token;
	}
	else
		stream_me >> token;

	toUpper(cmd);
		if (cmd == "PONG")
		{
			cl.setLastActivity(time(NULL));
			cl.setping(false);
		}
		else if (cmd == "PING")
		{
			if (token.empty())
			{
				cl.getoutbuffer() += ERR_NOORIGIN(SERVER_NAME);
				return;
			}
			if (token[0] == ':')
				token.erase(0, 1);
			cl.getoutbuffer() += RPL_PONG(SERVER_NAME, token);
		}
		else if (cmd == "PRIVMSG")
		{
			if (!Privmsg(cl, allCmd))
				return ;
		}
		else if (cmd == "PASS" || cmd == "USER")
		{
			cl.getoutbuffer() += ERR_ALREADYREG(SERVER_NAME);
		}
		else if (cmd == "NICK")
		{
			if (!changeNICK(cl, token))
				return ;
		}
		// --- HYBRID QUIT COMMAND ---
		else if (cmd == "QUIT")
		{
			// 1. Player 2: Extract the reason
			std::string reason = "Client Quit";
			size_t colonPos = allCmd.find(':');
			if (colonPos != std::string::npos) {
				reason = allCmd.substr(colonPos + 1);
			}

			// 2. Player 2: Scrub them from all channels immediately
			removeClientFromAllChannels(&cl, reason);

			// 3. Player 1: Tell the engine to drop them safely
			cl.getoutbuffer() += ERR_QUIT(cl.getIp(), allCmd.substr(cmd.size()));
			cl.getTimeout() = true;
		}
// --- JOIN COMMAND ---
        else if (cmd == "JOIN")
        {
            std::stringstream ss(allCmd);
            std::string instruction, channelString, keyString;

            // Extract the instruction, the channels, and optionally the keys
            ss >> instruction >> channelString >> keyString;

            if (channelString.empty()) {
                cl.getoutbuffer() += ":ft_irc.2004.ma 461 JOIN :Not enough parameters\r\n";
                return;
            }

            // --- 1. HANDLE "JOIN 0" (Leave all channels) ---
            if (channelString == "0") {
                std::vector<std::string> channelsToLeave;
                // Collect all channels the user is currently in
                for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
                    if (it->second.isMember(&cl)) {
                        channelsToLeave.push_back(it->first);
                    }
                }
                // Leave them one by one (this prevents map iteration errors)
                for (size_t i = 0; i < channelsToLeave.size(); ++i) {
                    Channel* chan = getChannel(channelsToLeave[i]);
                    if (chan) {
                        std::string partMsg = ":" + cl.getnickname() + "!" + cl.getusername() + "@" + cl.getIp() + " PART " + channelsToLeave[i] + " :Left all channels\r\n";
                        chan->broadcastMessage(partMsg);
                        chan->removeMember(&cl);
                        if (chan->getMembers().size() == 0) {
                            _channels.erase(channelsToLeave[i]); // Empty room trap!
                        }
                    }
                }
                return; // Stop processing, we are done.
            }

            // --- 2. HANDLE MULTIPLE CHANNELS (e.g., JOIN #a,#b pass1,pass2) ---
            std::stringstream chanStream(channelString);
            std::stringstream keyStream(keyString);
            std::string singleChan, singleKey;

            // Loop through the channels separated by commas
            while (std::getline(chanStream, singleChan, ',')) {

                // Get the corresponding key if one was provided
                singleKey = "";
                if (!keyString.empty()) {
                    std::getline(keyStream, singleKey, ',');
                }

                // Slice 1: Server Memory
                Channel* chan = getChannel(singleChan);
                if (chan == NULL) {
                    createChannel(singleChan, cl);
                    chan = getChannel(singleChan);
                    chan->addOperator(&cl); // Grant the Crown
                } else {
                    chan->addMember(&cl);
                }

                //tal3i code------------------------------------------------------------------------------------------------------------------------------------------------------------------
                if (chan->getLimit() > 0 && chan->getMembers().size() > static_cast<size_t>(chan->getLimit())) {
                    chan->removeMember(&cl);
                    cl.getoutbuffer() += ":ft_irc.2004.ma 471 " + cl.getnickname() + " " + singleChan + " :Cannot join channel (+l)\r\n";
                    continue; // Replaced return with continue to process the next channel!
                }
                if (chan->hasKey()) {
                    // Replaced allCmd.find() with the exact singleKey we extracted above
                    // allCmd.find() would break if multiple passwords were provided!
                    if (singleKey != chan->getKey()) {
                        chan->removeMember(&cl);
                        cl.getoutbuffer() += ":ft_irc.2004.ma 475 " + cl.getnickname() + " " + singleChan + " :Cannot join channel (+k)\r\n";
                        continue; // Replaced return with continue!
                    }
                }
                //------------------------------------------------------------------------------SFIMX RAK 4IRE 9AWAD DYL DAHMANE-----------------------------------------------------------------------------------------------------

                // Slice 2: The Protocol Handshake
                std::string nick = cl.getnickname();
                std::string user = cl.getusername();
                std::string host = cl.getIp();

                std::string joinMsg = ":" + nick + "!" + user + "@" + host + " JOIN " + singleChan + "\r\n";
                chan->broadcastMessage(joinMsg);

                std::string topicMsg = ":ft_irc.2004.ma 332 " + nick + " " + singleChan + " :No topic set\r\n";
                cl.getoutbuffer() += topicMsg;

                std::string nameList = chan->getMemberListAsString();
                std::string namesMsg = ":ft_irc.2004.ma 353 " + nick + " = " + singleChan + " :" + nameList + "\r\n";
                cl.getoutbuffer() += namesMsg;

                std::string endNamesMsg = ":ft_irc.2004.ma 366 " + nick + " " + singleChan + " :End of /NAMES list.\r\n";
                cl.getoutbuffer() += endNamesMsg;
            }
        }
		// --- INVITE COMMAND ---
		else if (cmd == "INVITE")
		{
			std::stringstream ss(allCmd);
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

			Client* targetClient = getClientByNickname(targetNick);
			if (targetClient == NULL) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 401 " + cl.getnickname() + " " + targetNick + " :No such nick/channel\r\n";
				return;
			}

			chan->inviteUser(targetNick);

			cl.getoutbuffer() += ":ft_irc.2004.ma 341 " + cl.getnickname() + " " + targetNick + " " + channelName + "\r\n";

			std::string inviteMsg = ":" + cl.getnickname() + "!" + cl.getusername() + "@" + cl.getIp() + " INVITE " + targetNick + " :" + channelName + "\r\n";
			targetClient->getoutbuffer() += inviteMsg;
		}
		// --- KICK COMMAND ---
		else if (cmd == "KICK")
		{
			std::stringstream ss(allCmd);
			std::string instruction, channelName, targetNick;

			ss >> instruction >> channelName >> targetNick;

			if (channelName.empty() || targetNick.empty()) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " KICK :Not enough parameters\r\n";
				return;
			}

			Channel* chan = getChannel(channelName);

			if (chan == NULL) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 403 " + cl.getnickname() + " " + channelName + " :No such channel\r\n";
				return;
			}

			if (!chan->isMember(&cl)) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 442 " + cl.getnickname() + " " + channelName + " :You're not on that channel\r\n";
				return;
			}

			if (!chan->isOperator(&cl)) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 482 " + cl.getnickname() + " " + channelName + " :You're not channel operator\r\n";
				return;
			}

			Client* targetClient = getClientByNickname(targetNick);
			if (targetClient == NULL || !chan->isMember(targetClient)) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 441 " + cl.getnickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n";
				return;
			}

			std::string reason = "No reason given";
			size_t colonPos = allCmd.find(':', allCmd.find(targetNick));
			if (colonPos != std::string::npos) {
				reason = allCmd.substr(colonPos + 1);
			}

			std::string kickMsg = ":" + cl.getnickname() + "!" + cl.getusername() + "@" + cl.getIp() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
			chan->broadcastMessage(kickMsg);

			chan->removeMember(targetClient);
		}
		else if(cmd == "mode"|| cmd =="MODE")
		{
			std::stringstream ss(allCmd);
			std::string mode , channelName, modeChanges;
			ss >> mode >> channelName >> modeChanges;
			if (channelName.empty())
			{
				cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " MODE :Not enough parameters\r\n";
				return;
			}
			if (channelName[0] != '#')
			{
				cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " MODE :Not enough parameters\r\n";
				return;
			}
			Channel *chn = getChannel(channelName);
			if (chn == NULL)
			{
				cl.getoutbuffer() += ":ft_irc.2004.ma 403 "
					+ cl.getnickname() + " " + channelName
					+ " :No such channel\r\n";
				return;
			}
			if (modeChanges.empty())
			{
				// std::string modes = chn->getModesString(); // e.g. "+itk"

				// cl.getoutbuffer() += ":ft_irc.2004.ma 324 "
				// 	+ cl.getnickname() + " "
				// 	+ channelName + " "
				// 	+ modes + "\r\n";

				return;
			}
	std::vector<std::string> params;
	std::string tmp;
	while (ss >> tmp)
		params.push_back(tmp);
	size_t paramIndex = 0;
	char sign = '+';
	for (size_t i = 0; i < modeChanges.length(); ++i)
	{
		char c = modeChanges[i];

		if (c == '+' || c == '-')
		{
			sign = c;
			continue;
		}
		if (c == 'o' || c == 'k' || c == 'l')
		{
			if (paramIndex >= params.size())
			{
				cl.getoutbuffer() += ":ft_irc.2004.ma 461 "
					+ cl.getnickname() + " MODE :Not enough parameters\r\n";
				return;
			}
			std::string param = params[paramIndex++];
			if (c == 'o')
			{
				run_o(*chn, cl, param, sign);
			}
			else if (c == 'k')
			{
				run_k(*chn, cl, param, sign);
			}
			else if (c == 'l')
			{
				run_l(*chn, cl, param, sign);
			}
		}
		else if (c == 'i' || c == 't')
		{
			if (c == 'i')
				std:: cout << "basiiiiiiiiiiiiiite sahbi\n";
			else if (c == 't')
				std::cout << "bassiiiiiiiiiiiiiiiite my man\n";

		}
		else
		{
			cl.getoutbuffer() += ":ft_irc.2004.ma 472 "
				+ cl.getnickname() + " " + c + " :is unknown mode char to me\r\n";
		}
	}
		}
		else if (cmd == "topic" || cmd == "TOPIC") {
			std::stringstream ss(allCmd);
			std::string topic, channelName, newTopic;
			ss >> topic >> channelName >> newTopic;

			// check aerguments
			if (channelName.empty()) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " TOPIC :Not enough parameters\r\n";
				return ;
			}

			//  MONAKACHA M3A BASITE RAK 4IRE 9AWAD DAHMANE
			// if (channelName[0] != '#') {
			// 	cl.getoutbuffer() += ":ft_irc.2004.ma 461 " + cl.getnickname() + " TOPIC :Not enough parameters\r\n";
			// 	return;
			// }
			Channel *chn = getChannel(channelName);
			// Check if the channel exists
			if (chn == NULL) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 403 " + cl.getnickname() + " " + channelName + " :No such channel\r\n";
				return;
			}

			// Check if the client is a member of the channel
			if (!chn->isMember(&cl)) {
				cl.getoutbuffer() += ":ft_irc.2004.ma 442 " + cl.getnickname() + " " + channelName + " :You're not on that channel\r\n";
				return;
			}

			// If no new topic is provided, return the current topic
			if (newTopic.empty()) {
				std::string topicMsg, whoAndWhenMessage;

				// If no topic is set, inform the client
				if (chn->gettopic().empty()) {
					topicMsg = ":ft_irc.2004.ma 332 " + cl.getnickname() + " " + channelName + " :No topic set\r\n";
					cl.getoutbuffer() += topicMsg;
					return ;
				}

				// If a topic is set, return it along with the setter and timestamp
				topicMsg = ":ft_irc.2004.ma 332 " + cl.getnickname() + " " + channelName + " :" + chn->gettopic() + "\r\n";
				whoAndWhenMessage = ":ft_irc.2004.ma 333 " + cl.getnickname() + " " + channelName + " " + chn->getTopicSetter() + " " + timeToString(chn->getTopicSetTime()) + "\r\n";
				cl.getoutbuffer() += topicMsg;
				cl.getoutbuffer() += whoAndWhenMessage;
				return;
			} else {

				// If a new topic is provided, check if the client has permission to change it
				if (chn->gethistopic() == true && !chn->isOperator(&cl)) {
					cl.getoutbuffer() += ":ft_irc.2004.ma 482 " + cl.getnickname() + " " + channelName + " :You're not channel operator\r\n";
					return ;
					
				}

				chn->setTopic(newTopic);

				for (size_t i = 0; i < chn->getMembers().size(); ++i) {
					Client* member = chn->getMembers()[i];
					member->getoutbuffer() += ":ft_irc.2004.ma 332 " + member->getnickname() + " " + channelName + " :" + newTopic + "\r\n";
				}


			} 


		}
	}
	catch (const std::out_of_range& e)
	{
	   (void)e;
		std::cerr << "getClient() failed (at()) !" << std::endl;
	}
}

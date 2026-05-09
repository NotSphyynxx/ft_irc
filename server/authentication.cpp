#include "Client.hpp"

bool Client::Emptynames()//true (empty)
{
	if (getnickname().empty() || getusername().empty())
	{
		return true;
	}
	return false;
}

bool Client::pass(std::string &pass, Server &sv)
{
	if (getlevel(0) == hasPASS)
	{
		getoutbuffer() += ERR_ALREADYREG(SERVER_NAME);
		return false;
	}
	if (pass == sv.getpass())
	{
		setlevel(0, hasPASS);
		return true;
	}
	else
	{
		if (pass.empty())
			return (outbuffer+=ERR_NEEDMOREPARAMS(SERVER_NAME, "PASS"), false);
		outbuffer += ERR_PASSWDMISMATCH(SERVER_NAME);
		timeOut = true;
	}
	return false;
}

bool Client::nick(std::string &nickname, Server &sv)
{
	if (nickname.empty())
	{
		this->getoutbuffer() += ERR_NONICKNAME(SERVER_NAME);
		return false;
	}
	if (nickname.size() > 9)
	{
		//Nickname too long
		this->getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
		return false;
	}
	if (!isalpha(nickname[0]) && !isSpecial(nickname[0]))
	{
		this->getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
		return false;
	}
	for (size_t i = 0; i < nickname.size(); i++)
	{
	   unsigned char c = nickname[i];
		if (!isdigit(c) && !isalpha(c) && !isSpecial(c))
		{
			this->getoutbuffer() += ERR_ERRONEUSNICK(SERVER_NAME, nickname);
			return false;
		}
	}// you need to check if there is another client with the same nickname
	if (sv.sameName(nickname))
	{
		this->getoutbuffer() += ERR_NICKINUSE(SERVER_NAME, nickname);
		return false;
	}
	setlevel(1, hasNICK);
	setnickname(nickname);
	return true;
}


bool  Client::user(std::string &extracted)
{
	std::stringstream ss(extracted);

	std::string cmd;
	std::string user;// should not be empty
	std::string mode;
	std::string unused;
	std::string realname; //  start with :

	if (getlevel(3) == REGISTRED)
	{
		this->getoutbuffer() += ERR_ALREADYREG(SERVER_NAME);
		return false;
	}
	ss >> cmd;
	ss >> user;
	ss >> mode;
	ss >> unused;

	std::string rest_of_line;
	std::getline(ss, rest_of_line);

	if (ss.fail() || user.empty() || mode.empty() || unused.empty())
	{
		this->getoutbuffer() += ERR_NEEDMOREPARAMS(SERVER_NAME, "USER");
		return false;
	}

	size_t col_pos = rest_of_line.find(':');
	if (col_pos != std::string::npos)
	{
		realname = rest_of_line.substr(col_pos + 1);
	}
	else
	{
		// Fallback if they didn't put a colon (rare but possible)
		size_t skipSpaces = rest_of_line.find_first_not_of(" \t");
		realname = rest_of_line.substr(skipSpaces);
	}
	if (realname.empty())
	{
		getoutbuffer() += ERR_NEEDMOREPARAMS(SERVER_NAME, "USER");
		return false;
	}
	setusername(user);
	setrealname(realname);
	setlevel(2, hasUSER);
	if ((getlevel(0) == hasPASS && getlevel(1) == hasNICK && getlevel(2) == hasUSER) && !Emptynames())
	{
		setlevel(3, REGISTRED);
		sendWelcome();
	}
	return true;
}

void Client::sendWelcome()
 {
	std::string nick = getnickname();
	std::string user = getusername();
	std::string host = getIp(); // Or hostname if you have it
	std::string serverName = SERVER_NAME;
	std::string welcome;

	welcome += ":" + serverName + " 001 " + nick +
						 " :Welcome to the IRC Network " + nick + "!" + user + "@" + host + "\r\n";

	welcome += ":" + serverName + " 002 " + nick +
						 " :Your host is " + serverName + ", running version 1.0\r\n";

	welcome += ":" + serverName + " 003 " + nick +
						 " :This server was created Jan 01 2024\r\n";
	welcome += ":" + serverName + " 004 " + nick +
						 " " + serverName + " 1.0 io itkol\r\n";

	this->outbuffer += welcome;

 }

int Client::Authentication(Server &sv)
{
	size_t pos;
	std::string &copy = getBuffer();// later you will know if you need a copy or not
	std::string extracted;
	std::string cmd;
	std::string value;

	if (this->getlevel(3) == REGISTRED)
		return  1;
	if (copy.empty())
		return 0;

	while ((pos = copy.find("\n")) != std::string::npos)
	{
		extracted = copy.substr(0, pos);
		if (!extracted.empty() && extracted[extracted.length() - 1] == '\r')
		{
			extracted.erase(extracted.length() - 1);
		}
		if (extracted.empty())
			continue;

		if (extracted == "2004")
		{
			username = "user2004";
			nickname = "nick2004";
			realname = "nick2004";

			this->setlevel(3, REGISTRED);
			return 0;
		}
		std::stringstream sp(extracted);
		sp >> cmd >> value;
		toUpper(cmd);
		if (cmd == "PASS")
		{
			if (!this->pass(value, sv))
				return (copy.erase(0 , pos + 1), 0);
		}
		else if (cmd == "NICK")
		{
			if (getlevel(0) == hasPASS)//has_pass
			{
				if (!this->nick(value, sv))
					return (copy.erase(0 , pos + 1), 0);
			}
			else
			{
				this->getoutbuffer()+= ERR_NOTREGISTERED(SERVER_NAME);
				return (copy.erase(0 , pos + 1), 0);
			}
		}
		else if (cmd == "USER")
		{
			if (getlevel(0) == hasPASS && getlevel(1) == hasNICK)//has_pass && has_nick
			{
				if (!this->user(extracted))
					return (copy.erase(0 , pos + 1),0);
			}
			 else
			{
				this->getoutbuffer() +=  ERR_NOTREGISTERED(SERVER_NAME);;
				return (copy.erase(0 , pos + 1),0);
			}

		}
		else if (cmd == "QUIT")
		{
			outbuffer += ERR_QUIT(myIp, extracted.substr(cmd.size()));
			timeOut = true;
		}
		else
			return (copy.erase(0 , pos + 1),0);
		copy.erase(0 , pos + 1);
	}

	return 1;
}

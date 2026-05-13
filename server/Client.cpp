#include "Client.hpp"

Client::Client(int sock) : mysocket(sock) , connectTime(time(NULL)), lastActivity(time(NULL)), pingsent(false) , timeOut(false)
{
	for (int i = 0; i < 4 ; i++)
		setlevel(i , EMPTY);
}

std::string  &Client::appand(const std::string &buf)
{
	std::string &mybuffer = getBuffer();
	if (buf.empty())
		return (mybuffer);
	mybuffer += buf;
	return mybuffer;
}

/*                                            Getters & Setters                                                     */

int Client::getsock()
{
	return this->mysocket;
}

std::string &Client::getnickname()
{
	return this->nickname;
}

std::string &Client::getusername()
{
	return this->username;
}

std::string &Client::getrealname()
{
	return this->realname;
}

std::string &Client::getBuffer()
{
	return this->buffer;
}

void Client::setnickname(std::string s)
{
	std::string clean;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] != '\r' && s[i] != '\n')
			clean += s[i];
	}
	this->nickname = clean;
}

void Client::setusername(std::string s)
{
	std::string clean;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] != '\r' && s[i] != '\n')
			clean += s[i];
	}
	this->username = clean;
}

void Client::setrealname(std::string s)
{
	std::string clean;

	for (size_t i = 0; i < s.size(); i++)
	{
		if (s[i] != '\r' && s[i] != '\n')
			clean += s[i];
	}
	this->realname = clean;
}

void Client::setBuffer(std::string buf)
{
	this->buffer += buf;
}


Level Client::getlevel(unsigned int  index)
{
	return this->rank[index];
}

void Client::setlevel(unsigned int index, Level value)
{
	this->rank[index] = value;
}

std::string &Client::getIp()
{
	return this->myIp;
}

void Client::setIp(std::string ip)
{
	this->myIp = ip;
}

time_t &Client::getconnecttime()
{
	return this->connectTime;
}

void Client::setconnecttinme(time_t tm)
{
	this->connectTime = tm;
}

time_t &Client::getLastActivity()
{
	return this->lastActivity;
}

void Client::setLastActivity(time_t tm)
{
	this->lastActivity = tm;
}

std::string &Client::getoutbuffer()
{
	return this->outbuffer;
}

void Client::setoutbuffer(std::string outbuff) // 0 for erase
{
	 this->outbuffer = outbuff;
}

bool Client::pingissent()
{
	return this->pingsent;
}

void Client::setping(bool value)
{
	this->pingsent = value;
}

time_t &Client::getwhenpingsent()
{
	return whenpingsent;
}

// for broadcasting
std::string Client::getPrefix() {
	// This creates the standard IRC identity mask
	return nickname + "!" + username + "@" + myIp;
}

bool &Client::getTimeout()
{
	return this->timeOut;
}

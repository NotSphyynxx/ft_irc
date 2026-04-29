
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <cerrno>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <signal.h>
#include "../server/Server.hpp"


#define bot_CMD_PRIVMSG(cmd, target, text) ("PRIVMSG " + std::string(target) + " :" + std::string(text) + "\r\n")

struct socket_bot
{
	int socket_save;
};
int sending(std::string &message , int socket)
{
	ssize_t bytes_sent = 0;

	while (!message.empty())
	{
		bytes_sent = send(socket , message.c_str(), message.size(), 0);
		if (bytes_sent == -1)
		{
			throw std::runtime_error("send() failed !");
		}
		message.erase(0, bytes_sent);
	}
	return 1;
}

std::string extract(std::stringstream &ss)
{
	std::string what;

	ss >> what;
	if (what[0] == ':')
	{
		what = what.substr(1);
	}
	return what;
}

int main(int argc , char **argv)
{
	struct socket_bot save_me;
	try
	{
		signal(SIGPIPE, SIG_IGN);
	if (argc != 4)
	{
		std::cerr << "Usage: ./bot <server_ip> <port> <password>" << std::endl;
		return 1;
	}
	srand(time(0));
	myport(argv[2]);
	char *port = argv[2];
	struct addrinfo		*botaddress;
	struct addrinfo		*tmp;
	struct addrinfo		hints;

	int sockBot;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	if (getaddrinfo(argv[1], port , &hints, &botaddress) != 0)
	{
		throw std::runtime_error("getaddrinfo failed !");
	}
	for (tmp = botaddress; tmp != NULL ; tmp = tmp->ai_next)
	{
		if ((sockBot = socket(tmp->ai_family, tmp->ai_socktype , tmp->ai_protocol)) == -1)
		{
			continue;
		}
		if ((connect(sockBot, tmp->ai_addr, tmp->ai_addrlen)) == -1)
		{
			close(sockBot);
			//freeaddrinfo(botaddress);
			continue;
		}
		break;
	}
	if (tmp == NULL)
	{
		freeaddrinfo(botaddress);
		throw std::runtime_error("couldn't establish a connection !");
	}
	save_me.socket_save = sockBot;
	freeaddrinfo(botaddress);
	char	buff[1025];
	memset(buff, 0, sizeof(buff));
	std::string	fullBuff;
	ssize_t byte_recv ;
	ssize_t byte_sent;
	std::string Jokes[5] = {"I have a great joke about UDP, but I'm not sure you'll get it.",
							"Schrödinger’s cat walks into a bar. And doesn't.",
							"A C++ developer, a Java developer, and a Python developer walk into a cafe. The Java dev waits 5 minutes for the garbage collector to clear a table. The Python dev imports a table. The C++ dev builds a table from scratch, eats, and then accidentally destroys the entire cafe trying to free the memory.","Why did the database administrator leave his wife? She had one-to-many relationships." , "Why do programmers prefer dark mode? Because light attracts bugs."
							};

	std::string passCmd = "PASS " + std::string(argv[3]) + "\r\n";
	sending(passCmd, sockBot);

	// 2. Send the Nickname
	std::string nickCmd = "NICK DELYBOT\r\n";
	sending(nickCmd, sockBot);

	// 3. Send the User info
	std::string userCmd = "USER bot 0 * :Bot\r\n";
	sending(userCmd, sockBot);


	while (1)
	{
		byte_recv = recv(sockBot, buff, sizeof(buff) - 1, 0);
		if (byte_recv == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			std::cerr << "recv() failed !" << std::endl;
			close(sockBot);
			return -1; // check for -1 later
		}
		if (byte_recv >= 0)
			buff[byte_recv] = '\0';
		fullBuff += buff;
		size_t pos;

		while ((pos = fullBuff.find("\r\n")) != std::string::npos)
		{
			std::cout << "[SERVER] " << fullBuff << std::endl;
			std::string prefix, cmd, target, what;
			std::string line = fullBuff.substr(0, pos);
			std::stringstream ss(line);
			ss >> prefix >> cmd >> target;
			if (prefix == "PING")
			{
				std::string pong = RPL_PONG(SERVER_NAME , cmd);
				sending(pong,sockBot);
			}
			else if (cmd == "PONG")
				continue;
			else if (cmd == "PRIVMSG")//token here is the cmd
			{
				size_t mark = prefix.find("!");
				std::string sender = prefix.substr(1, mark - 1);
				what = extract(ss);
				if (what == "!roll")
				{
					long guess = rand() % 101;
					std::stringstream tostr;
					tostr << guess;
					std::string ReplyTarget = (target[0] == '#' || target[0] == '&') ? target : sender;
					std::string reply = bot_CMD_PRIVMSG("PRIVMSG", ReplyTarget, tostr.str());
					sending(reply , sockBot);
				}
				if (what == "!joke")
				{
					long random = rand() % 5;
					std::string ReplyTarget = (target[0] == '#' || target[0] == '&') ? target : sender;
					std::string reply = bot_CMD_PRIVMSG("PRIVMSG", ReplyTarget, Jokes[random]);
					sending(reply , sockBot);
				}
			}
			else if (cmd == "INVITE")
			{
				// The raw string looks like: :SABONA!user@127.0.0.1 INVITE DELYBOT :#1337
				std::string channel  = extract(ss);
    			std::cout << "[BOT] I was invited to " << channel<< "! Joining now..." << std::endl;

    			// Send the JOIN command back to the server
    			std::string joinReq = "JOIN " + channel + "\r\n";
    			sending(joinReq, sockBot);
			}
			fullBuff.erase(0, pos + 2);
		}
	}
	}
	catch (const std::exception &e)
	{
		std::cerr << "\n[CRITICAL] Connection lost: " << e.what() << std::endl;
		close(save_me.socket_save);
		return 1;
	}
}

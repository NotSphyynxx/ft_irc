
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
#include "../server/Server.hpp"


int main(int argc , char **argv)
{
	if (argc != 4)
		return 1;
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
		if ((sockBot = socket(tmp->ai_family, tmp->ai_socktype , tmp->ai_protocol)) != -1)
		{
			freeaddrinfo(botaddress);
			continue;
		}
		int truee = 1;
		if (setsockopt(sockBot, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(truee)) == -1)
		{
			 close(sockBot);
			 continue;
		}
		break;
	}

	if ((connect(sockBot, tmp->ai_addr, tmp->ai_addrlen)) == -1)
	{
		close(sockBot);
		freeaddrinfo(botaddress);
		throw "Failed to connect to the server !";
	}
	freeaddrinfo(botaddress);
	char	buff[1025];
	memset(buff, 0, sizeof(buff));
	std::string	fullBuff;
	size_t byte_recv ;
	size_t byte_sent;
	std::string Jokes[5] = {"I have a great joke about UDP, but I'm not sure you'll get it.",
							"Schrödinger’s cat walks into a bar. And doesn't.",
							"A C++ developer, a Java developer, and a Python developer walk into a cafe. The Java dev waits 5 minutes for the garbage collector to clear a table. The Python dev imports a table. The C++ dev builds a table from scratch, eats, and then accidentally destroys the entire cafe trying to free the memory.","Why did the database administrator leave his wife? She had one-to-many relationships." , "Why do programmers prefer dark mode? Because light attracts bugs."
							};

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
		if (byte_recv > 0)
			buff[byte_recv] = '\0';
		fullBuff += buff;
		size_t pos;
		while ((pos = fullBuff.find("\r\n")) != std::string::npos)
		{
			std::string prefix, cmd, target, what;
			std::string line = fullBuff.substr(0, pos);
			std::stringstream ss(line);
			ss >> prefix >> cmd >> target;
			if (prefix == "PING")
			{
				std::string pong = RPL_PONG(SERVER_NAME , cmd);
				while (!pong.empty())
				{
					byte_sent = send(sockBot, pong.c_str() , pong.size(), 0);
					if (byte_sent == -1)
					{
						throw std::runtime_error("send() failed !");
					}
					pong.erase(0, byte_sent);
				}
			}
			else if (cmd == "PRIVMSG")//token here is the cmd
			{
				size_t mark = prefix.find("!");
				std::string sender = prefix.substr(1, mark - 1);
				if (what == ":!roll")
				{
					long guess = rand() % 101;
					std::stringstream tostr;
					tostr << guess;
					std::string ReplyTarget = (target[0] == '#' || target[0] == '&') ? target : sender;
					std::string reply = CMD_PRIVMSG(prefix, ReplyTarget, tostr.str());
					while (!reply.empty())
					{
						byte_sent = send(sockBot, reply.c_str(), reply.size(), 0);
						if (byte_sent == -1)
						{
							throw std::runtime_error("send() failed !");
						}
						reply.erase(0, byte_sent);
					}
				}
				if (what == ":!joke")
				{
					long random = rand() % 5;
					std::string ReplyTarget = (target[0] == '#' || target[0] == '&') ? target : sender;
					std::string reply = CMD_PRIVMSG(prefix, ReplyTarget, Jokes[random]);
					while (!reply.empty())
					{
						byte_sent = send(sockBot, reply.c_str(), reply.size(), 0);
						if (byte_sent == -1)
						{
							throw std::runtime_error("send() failed !");
						}
						reply.erase(0, byte_sent);
					}
				}
			}
			fullBuff.erase(0, pos + 2);
		}
	}
}

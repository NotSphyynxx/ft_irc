
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
		return ;
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
	for (tmp = botaddress; tmp != NULL ;tmp =  tmp->next)
	{
		if ((sockBot = socket(tmp->ai_family, tmp->ai_socktype , tmp->ai_protocol)) != -1)
		{
			freeaddrinfo(botaddress);
			continue;
		}
		if (fcntl(sockBot, F_SETFL, O_NONBLOCK) == -1)
		{
			close(sockBot);
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

	while (1)
	{
		byte_recv = recv(sockBot, buff, sizeof(buff) - 1);
		if (byte_recv == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			std::cerr << "recv() failed !" << std::endl;
			close(sockBot);
			return -1; // check for -1 later
		}
		buff[byte_recv] = '\0';
		fullBuff += buff;
		if ((size_t pos = fullBuff.find("\r\n")) != std::string::npos)
		{
			std::string cmd , token;
			std::string line  = fullBuff.substr(0, pos);
			std::stringstream ss(line);
			ss >> cmd >> token;
			if (cmd == "PING" && token == ":" + std::string(SERVER_NAME))
			{
				std::string pong = RPL_PONG(SERVER_NAME , token);
				byte_sent = send(sockBot, pong , pong.size());
				if (byte_sent == -1)
				{
					if (errno == EWOULDBLOCK || errno == EAGAIN) // in a blocking socket the program would wait but since we set it to no blocking the func just return
					continue; // Just try again next time POLLOUT is ready
					throw std::runtime_error("send() failed !");
				}
				
			}

		}




	}



}

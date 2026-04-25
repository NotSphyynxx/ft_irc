#include "Server.hpp"
#include "Client.hpp"

Server::Server(char *pt, char *pass)
{
	struct addrinfo hints;
	struct addrinfo *p;
	struct addrinfo *server_info;
	int check  = -1;
	myport(pt);

	sockfd = -1;
	if (mypass(pass))
		this->password = pass;
	memset(&hints, 0 , sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // force the socket to listen to all net interface (ip is 0.0.0.0 )


	if ((check = getaddrinfo(NULL, pt, &hints, &server_info)) != 0)
	{
	   throw std::runtime_error("getaddrinfo failed !");
	}

	for (p = server_info; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;
		if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		{
			close(sockfd);
			continue;
		}
		int truee = 1;
		if  (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(truee)) == -1)
		 {
			 close(sockfd);
			 continue;
		 }
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		freeaddrinfo(server_info);
		throw std::runtime_error("Server failed to bind to any address");
	}
	//you may need to save server ip
	if (listen(sockfd, REQUEST) == -1)
	{
		freeaddrinfo(server_info);
		close(sockfd);
		throw std::runtime_error("listen failed !");
	}
	struct sockaddr_in *tmp = (struct sockaddr_in *) &p;
	char ipchar[IPV4LEN];
	inet_ntop(tmp->sin_family, &tmp->sin_addr, ipchar, sizeof(ipchar));
	setServerIp(ipchar);
	std::cout << "server: waiting for connections... " << std::endl;
	freeaddrinfo(server_info);
	server_info  = NULL;
}

int Server::run()
{
	pollvec &sockarray = getpollstruct();
	int server_sock = getsocket();

	//add the main socket (server) to our pollfd vector
	struct pollfd pfd;
	pfd.fd = server_sock;
	pfd.events = POLLIN;
	sockarray.push_back(pfd);

	int p = -1;

	while (1)
	{
		// check if there is data waiting in the client outputbuffer
		checkPollout(sockarray);

		p = poll(sockarray.data(), sockarray.size(), 3000); // so poll wait up to the time specified if there is no data flow it return 0

		if (p < 0)
		{
			if (errno == EINTR)
				continue; // Just a signal, go back to the top of the while(1)
		if (p == -1)
			throw std::runtime_error("poll failed");
		}
		if (p == 0)
		{
			checkTimeout(sockarray);
			continue ;
			//return 0;// nothing happened (timeout)  but if you said -1 then probably you need to rm this check
		}
		else
		{
			for (size_t i = 0; i < sockarray.size();)
			{
				if (sockarray[i].revents & POLLIN)
				{
					if (sockarray[i].fd == server_sock) // from listener (a new connection)
					{
						NewConnection(sockarray, server_sock);
					}
					else // a client
					{
						RecieveMessage(sockarray, sockarray[i].fd);//sock
					}
				}
				if (sockarray[i].revents & (POLLHUP | POLLERR)) // in case we lost the connection in specific sockets or it may be an error
				{
					closeSocket(sockarray, sockarray[i].fd);
					continue;
				}
				if (sockarray[i].revents & POLLOUT) // we have data to send
				{
					sendMessages(sockarray, i, sockarray[i].fd);
				}
				i++;
			}
		}
	}

}

int Server::NewConnection(std::vector <struct pollfd> &fds, int sock)
{
	struct sockaddr_storage st;
	socklen_t sz = sizeof(st);
	struct sockaddr_in *hp;
	struct sockaddr_in6 *s6;
	char    ipv4char[INET6_ADDRSTRLEN];

	int new_fd = -1;
	if ((new_fd = accept(sock,(sockaddr *) &st, &sz)) == -1)
	{
		return -1;
	}

	if (fcntl(new_fd, F_SETFD, O_NONBLOCK) == -1) // again set this socket as non-blocking
	{
		std::cerr << "fcntl() failed on new connection!" << std::endl;
		closeSocket(fds, new_fd);
		return -1;
	}

	// add the incoming connection to our pollfd
	struct pollfd tmp;
	tmp.fd = new_fd;
	tmp.events = POLLIN;
	fds.push_back(tmp);

	// we manipulate & point to the sockaddr_storage as sock..in or in6 according to the upcoming connection
	if (st.ss_family == AF_INET)
	{
		hp =  (struct sockaddr_in *) &st;
		inet_ntop(hp->sin_family, &hp->sin_addr, ipv4char, sizeof(ipv4char));
		 std::cout << "New connection from: " << ipv4char << " : "  << ntohs(hp->sin_port) << std::endl;
	}
	else if (st.ss_family == AF_INET6)
	{
		s6 =  (struct sockaddr_in6 *) &st;
		inet_ntop(s6->sin6_family, &s6->sin6_addr, ipv4char, sizeof(ipv4char));
		 std::cout << "New connection from: " << ipv4char << " : "  << ntohs(s6->sin6_port) << std::endl;
	}

	// inet_ntop(hp->sin_family, &hp->sin_addr, ipv4char, sizeof(ipv4char));

	try
	{
		addClient(new_fd);
		Client &cl = getClient(new_fd);
		cl.setIp(ipv4char);
		// hp->sin_port is in Network Byte Order, so we use ntohs()
		// std::cout << "New connection from: " << ipv4char << " : "  << ntohs(hp->sin_port) << std::endl;

		// cl.setconnecttinme(time(NULL));
		// cl.setLastActivity(time(NULL));
	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "getClient() failed (at()) !" << std::endl;
		closeSocket(fds, new_fd);
		return 0;
	}
	return 1;
}

int Server::RecieveMessage(std::vector <struct pollfd> &fds, int sock)
{
	char buff[BUFFER];
	ssize_t bytes_recv;

	memset(buff, 0, sizeof(buff));
	bytes_recv = recv(sock, buff, BUFFER, 0);
	if (bytes_recv == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return 0;
		std::cerr << "recv failed !" << std::endl;
		closeSocket(fds, sock);
		return -1;// check for -1 later
	}
	if (bytes_recv == 0)
	{
		std::cerr << "Client disconnected !" << std::endl;
		closeSocket(fds, sock);
		return -1;// check for -1 later
	}
	buff[bytes_recv] = '\0';
	try {
		Client &cl = getClient(sock);
		cl.appand(buff);
		cl.setLastActivity(time(NULL));
		if (cl.getBuffer().size() > 5120)
		{
			std::cerr << "Client " << sock << " is flooding. Disconnecting." << std::endl;
			closeSocket(fds, sock);
			return 0;
		}
		if (!cl.Authentication(*this) && cl.getlevel(3) != REGISTRED)
			return 0;
		processCommand(fds, buff, sock);
		std::cout << "client " << sock  << " : received " << buff << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "getClient() failed (at()) !" << std::endl;
		closeSocket(fds, sock); // i guess you should remove this
		return 0;
	}
	return 1;
}

int Server::sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock)
{
	try
	{
		ssize_t bytesent;
		Client &cl = getClient(sock);
		std::string &buf = cl.getoutbuffer();

		if (buf.empty())
			return (0);
			// fds[i].events |= POLLOUT;
			if ((bytesent = send(sock, buf.c_str(), buf.size(), 0)) == -1)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN) // in a blocking socket the program would wait but since we set it to no blocking the func just return
					return 0; // Just try again next time POLLOUT is ready
				throw std::runtime_error("send() failed !");
			}
			buf.erase(0, bytesent);
		if (!buf.empty())
			fds[i].events |= POLLOUT;
	   else
			fds[i].events &= ~POLLOUT;

		return 1;

	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "getClient() failed (at()) !" << std::endl;
		return -1;
	}
}

Server::~Server()
{
   // freeaddrinfo(getServerI());
   // close(sockfd);
}

bool Server::clientExists(int fd) const
{
	return _client.find(fd) != _client.end(); // false = we found end()
}

void Server::addClient(int fd) {
	// This creates a new Client object using the default constructor
	// and maps it to the file descriptor 'fd'
	// or overwrite the value
	std::map<int, Client>::iterator it = _client.find(fd);
	if (it != _client.end())
		it->second = Client(fd); // overwrite existing
	else
		_client.insert(std::make_pair(fd, Client(fd)));
}

void Server::removeClient(int fd)
{
	if (clientExists(fd))
	{
		_client.erase(fd);
	}
}

bool Server::sameName(std::string &nickname)
{
	cmaps tmp = this->getcmaps();
	cmaps::iterator it = tmp.begin();

	for (; it != tmp.end(); it++)
	{
		if (it->second.getnickname() == nickname)
			return true;
	}
	return false;
}

void Server::closeSocket(std::vector <struct pollfd> &fds, int sock)
{
	std::vector <struct pollfd>::iterator it = fds.begin();
	for (; it != fds.end(); it++)
	{
		if (it->fd == sock)
		{
			fds.erase(it);
			break;
		}
	}
	if (clientExists(sock)){
		Client &cl = getClient(sock);
		removeClientFromAllChannels(&cl, "Connection closed");
	}
	removeClient(sock);
	close(sock);
}

int Server::checkTimeout(pollvec &sockarray)
{
	time_t now = time(NULL);

	for (size_t i = 0; i < sockarray.size();)
	{
		if (i > 0)
		{
			try
			{
				Client &cl = getClient(sockarray[i].fd);
				if (cl.getlevel(3) != REGISTRED && (now - cl.getconnecttime()) > 60)
				{
					std::cout << "Timeout: Closing unregistered client " << sockarray[i].fd << std::endl;
					closeSocket(sockarray, sockarray[i].fd);
					continue;
				}
				else if (!cl.pingissent() &&  cl.getlevel(3) == REGISTRED && (now - cl.getLastActivity()) > 60)
				{
					std::string PING = "PING :" + std::string(SERVER_NAME) + "\r\n";
					cl.getoutbuffer() += PING;
					cl.setping(true);
					cl.getwhenpingsent() = now;
				}
				else if (cl.pingissent() &&  cl.getlevel(3) == REGISTRED && (now - cl.getwhenpingsent()) > 60)
				{
					std::cout << "Timeout: Closing client " +  cl.getrealname() + " Ip : " + cl.getIp() << " "<< std::endl;
					closeSocket(sockarray, sockarray[i].fd);
					continue;
				}
			}
			catch (const std::out_of_range& e)
			{
				(void)e;
				continue;
			}
		}
		i++;
	}
	return 1;
}

int Server::checkPollout(pollvec &fds)
{
	for (size_t i = 1; i < fds.size();)
	{
		try
		{

			Client &cl = getClient(fds[i].fd);
			if (!cl.getoutbuffer().empty())
			{
				fds[i].events |= POLLOUT;
			}
			else
			{
				fds[i].events &=  ~POLLOUT;
			}
			i++;
		}
		catch (const std::out_of_range& e)
		{
			(void)e;
			continue;
		}
	}
	return 1;
}



void Server::broadcast(pollvec &fds, std::string message)
{
	// fds here are the ones that live in a specific channel
	//When you finally builds the Channel
	//class, you will store FDs. If you close a
	//socket, you must make sure they know so they can
	//remove that FD from their channel lists.

	size_t i = 1;

	for (; i < fds.size();)
	{
		try
		{
			Client &cl = getClient(fds[i].fd);
			cl.getoutbuffer() += message;
			i++;
		}
		catch (const std::out_of_range& e)
		{
		   (void)e;
			continue;
		}
	}
}

long Server::getclientbyNick(const std::string &nick)
{
	cmaps &myClient = this->_client;
	cmaps::iterator start = myClient.begin();

	for (; start != myClient.end(); start++)
	{
		Client &ct = start->second;
		if (ct.getnickname() == nick)
			return start->first;
	}
	return -1;
}

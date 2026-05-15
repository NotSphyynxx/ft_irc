#include "Server.hpp"
#include "Client.hpp"

Server::Server(char *pt, char *pass)
{
	struct addrinfo hints;
	struct addrinfo *p;
	struct addrinfo *server_info;
	std::string		final_pass;
	int check  = -1;
	myport(pt);

	sockfd = -1;
	mypass(pass, final_pass);
	this->password = final_pass;
	memset(&hints, 0 , sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // force the socket to listen to all net interface (ip is 0.0.0.0 )


	if ((check = getaddrinfo(NULL, pt, &hints, &server_info)) != 0) // DNS LOOKUP
	{
	   throw std::runtime_error("getaddrinfo failed !");
	}

	for (p = server_info; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) // the os kernel allocates a block of ram & create the send buffer and recieve buffer
			continue;
		if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		{
			close(sockfd);
			continue;
		}
		int truee = 1;
		if  (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &truee, sizeof(truee)) == -1) // tell os to reuse the port even if it stuck on time wait
		 {
			 close(sockfd);
			 continue;
		 }
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		if (listen(sockfd, REQUEST) == -1) // listen is by default non blocking cuz it just switch the state form active socket to passive & [QUEUE]
		{
			close(sockfd);
			continue;
		}
		break;
	}
	if (p == NULL)
	{
		freeaddrinfo(server_info);
		throw std::runtime_error("[SERVER] : failed to bind to any address");
	}
	struct sockaddr_in *tmp = (struct sockaddr_in *) &p;
	char ipchar[IPV4LEN];
	inet_ntop(tmp->sin_family, &tmp->sin_addr, ipchar, sizeof(ipchar));
	setServerIp(ipchar);
	std::cout << "[SERVER] : waiting for connections... " << std::endl;
	freeaddrinfo(server_info);
	server_info  = NULL;
}

int Server::run()
{
	pollvec &sockarray = getpollstruct();
	int server_sock = this->sockfd;

	// add the main socket (server) to our pollfd vector
	struct pollfd pfd;
	pfd.fd = server_sock;
	pfd.events = POLLIN;
	sockarray.push_back(pfd);

	int p = -1;

	while (1)
	{
		// check if there is data waiting in the client outputbuffer
		checkPollout(sockarray);
		int did_we_reach_p = 0;

		p = poll(&sockarray[0], sockarray.size(), 3000); // so poll wait up to the time specified if there is no data flow it return 0 (so we can check timeout)

		if (p < 0)
		{
			if (errno == EINTR)
				continue; // Just a signal, go back to the top of the while(1)
		if (p == -1)
			throw std::runtime_error("poll failed");
		}
		if (p == 0)
		{
			checkClients(sockarray); // set the timeout if there is an inactive or unregistered client
			checkTimeout(sockarray);// close client with timeout flag
			continue ;
			// nothing happened (timeout)  but if you said -1 then probably you need to rm this check
		}
		else
		{
			did_we_reach_p = 0;
			for (size_t i = 0; i < sockarray.size();)
			{
				if (did_we_reach_p == p) // loop as long as there is an unchecked event ( index <= poll return value )
					break ;
				if (sockarray[i].revents & POLLIN)
				{
					did_we_reach_p++;
					if (sockarray[i].fd == server_sock) // from listener (a new connection)
					{
						if (NewConnection(sockarray, server_sock) == -1)
							continue;
					}
					else // a client
					{
						if (RecieveMessage(sockarray, sockarray[i].fd) == -1)
							continue ;
					}
				}
				if (sockarray[i].revents & (POLLHUP | POLLERR)) // in case we lost the connection in specific sockets or it may be an error
				{
					did_we_reach_p++;
					closeSocket(sockarray, sockarray[i].fd);
					continue;
				}
				if (sockarray[i].revents & POLLOUT) // we have data to send
				{
					did_we_reach_p++;
					if (sendMessages(sockarray, i, sockarray[i].fd) == -1)
						continue;
					if (checkTimeout(sockarray))
						continue;
				}
				i++;
			}
		}
	}

}

int Server::NewConnection(std::vector <struct pollfd> &fds, int sock)
{
	struct sockaddr_storage st; // this struct is big enough to hold ipv4 & ipv6
	socklen_t sz = sizeof(st);
	struct sockaddr_in *hp;
	struct sockaddr_in6 *s6;
	char    ipv4char[INET6_ADDRSTRLEN];

	int new_fd = -1;
	memset(&st, 0, sizeof(st));

	if ((new_fd = accept(sock,(sockaddr *) &st, &sz)) == -1)
	{
		if (errno == EMFILE || errno == ENFILE)
		{
			std::cerr << "[WARNING] Server is full. Dropping new connection." << std::endl;
			return 0;
		}
		std::cerr << "accept () failed on new connection!" << std::endl;
		return 0;
	}
	if (fcntl(new_fd, F_SETFL, O_NONBLOCK) == -1) // again set this socket as non-blocking
	{
		std::cerr << "fcntl() failed on new connection!" << std::endl;
		close(new_fd);
		return 0;
	}
	try
	{
		addClient(new_fd);
		Client &cl = getClient(new_fd);

		// we manipulate & point to the sockaddr_storage as sock..in or in6 according to the upcoming connection
		if (st.ss_family == AF_INET)
		{
			hp =  (struct sockaddr_in *) &st;
			inet_ntop(hp->sin_family, &hp->sin_addr, ipv4char, sizeof(ipv4char));
		 	std::cout << "[SERVER] : New connection from: " << ipv4char << " : "  << ntohs(hp->sin_port) << std::endl;
		}
		else if (st.ss_family == AF_INET6)
		{
			s6 =  (struct sockaddr_in6 *) &st;
			inet_ntop(s6->sin6_family, &s6->sin6_addr, ipv4char, sizeof(ipv4char));
			std::cout << "[SERVER] : New connection from: " << ipv4char << " : "  << ntohs(s6->sin6_port) << std::endl;
		}
		cl.setIp(ipv4char);

		// add the incoming connection to our pollfd
		struct pollfd tmp;
		tmp.fd = new_fd;
		tmp.events = POLLIN;
		fds.push_back(tmp);
	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "[SERVER] : getClient() failed (at()) !" << std::endl;
		closeSocket(fds, new_fd);
		return -1;
	}
	return 1;
}

int Server::RecieveMessage(std::vector <struct pollfd> &fds, int sock)
{
	char buff[BUFFER + 1];
	ssize_t bytes_recv;

	memset(buff, 0, sizeof(buff));
	bytes_recv = recv(sock, buff, BUFFER, 0);
	if (bytes_recv == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return 0;
		std::cerr << "[SERVER] : recv() failed !" << std::endl;
		closeSocket(fds, sock);
		return -1;// check for -1 later
	}
	if (bytes_recv == 0)
	{
		std::cerr << "[SERVER] : Client disconnected !" << std::endl;
		closeSocket(fds, sock);
		return -1;// check for -1 later
	}
	try {
		Client &cl = getClient(sock);
		cl.appand(buff);
		cl.setLastActivity(time(NULL));
		if (cl.getBuffer().size() > 5120)
		{
			std::cerr << "[SERVER] : Client " << sock << " is flooding. Disconnecting." << std::endl;
			closeSocket(fds, sock);
			return -1;
		}
		if (cl.getlevel(3) != REGISTRED)
		{
			cl.Authentication(*this);
		}
		processBuffer(cl);
		std::cout << "[CLIENT : ]" << sock  << " : Received " << buff << std::endl;
	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "[SERVER] : getClient() failed at Recieving messages !" << std::endl;
		closeSocket(fds, sock); // i guess you should remove this
		return -1;
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
		std::cout << "[DEBUG] Attempting to send " << buf.size() << " bytes to socket " << sock << std::endl;

		if (buf.empty())
		{
			cl.there_is_data_to_send() = false;
			return (fds[i].events &= ~POLLOUT, 0);
		}

			// fds[i].events |= POLLOUT;
			if ((bytesent = send(sock, buf.c_str(), buf.size(), 0)) == -1)
			{
				if (errno == EWOULDBLOCK || errno == EAGAIN) // in a blocking socket the program would wait but since we set it to no blocking the func just return
					return (std::cout << "[DEBUG] EAGAIN hit for socket " << sock << ". OS bucket is full!" << std::endl, 0); // Just try again next time POLLOUT is ready
				closeSocket(fds, sock);
				return -1;
			}
			std::cout << "[DEBUG] Partial Send: Sent " << bytesent << " bytes. " << (buf.size() - bytesent) << " bytes remain." << std::endl;
			buf.erase(0, bytesent);
		if (!buf.empty())
			fds[i].events |= POLLOUT;
	   else
	   {
			fds[i].events &= ~POLLOUT;
			cl.there_is_data_to_send() = false;
	   }

		return 1;

	}
	catch (const std::out_of_range& e)
	{
		(void)e;
		std::cerr << "[SERVER] : getClient() failed at sending messages !" << std::endl;
		fds[i].events &= ~POLLOUT;
		closeSocket(fds, sock);
		return -1;
	}
}

Server::~Server()
{
	Cleanup(sockarrayy);
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

void Server::closeSocket(pollvec &fds, int sock)
{
	std::vector <struct pollfd>::iterator it = fds.begin();
	for (; it != fds.end(); it++)
	{
		if (it->fd == sock)
		{
			*it = fds.back();
			fds.pop_back();
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

void Server::Cleanup(pollvec &fds)
{
	std::vector <struct pollfd>::iterator it = fds.begin();
	for (; it != fds.end(); it++)
	{
		if (clientExists(it->fd))
		{
			Client &cl = getClient(it->fd);
			removeClientFromAllChannels(&cl, "Connection closed");
		}
		close(it->fd);
		fds.erase(it);
	}
}


int Server::checkTimeout(pollvec &sockarray) // closing sockets
{
	int flag = 0;
	for (size_t i = 1; i < sockarray.size();)
	{
		try
		{
			Client &cl = getClient(sockarray[i].fd);
			if (cl.getTimeout() && !cl.there_is_data_to_send())
			{
				flag = 1;
				closeSocket(sockarray, sockarray[i].fd);
				continue;
			}
		}
		catch (const std::out_of_range& e)
		{
			flag = 1;
			close(sockarray[i].fd);
			sockarray.erase(sockarray.begin() + i);
			continue;
		}
		i++;
	}
	return flag;
}

int Server::checkClients(pollvec &sockarray)
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
					std::cout << "Timeout: Closing unregistered client ..." << sockarray[i].fd << std::endl;
					cl.getoutbuffer() += ERR_CLOSINGLINK(SERVER_NAME, "Authentication time has Passed !");
					cl.getTimeout() = true;
					cl.there_is_data_to_send() = true;
				}
				else if (!cl.pingissent() &&  cl.getlevel(3) == REGISTRED && (now - cl.getLastActivity()) > 60)
				{
					cl.getoutbuffer() += CMD_PING(SERVER_NAME);
					cl.setping(true);
					cl.getwhenpingsent() = now;
				}
				else if (cl.pingissent() &&  cl.getlevel(3) == REGISTRED && (now - cl.getwhenpingsent()) > 60)
				{
					std::cout << "Timeout: Closing client ..." << sockarray[i].fd << std::endl;
					cl.getoutbuffer() += ERR_PINGTIMEOUT(cl.getIp());
					cl.getTimeout() = true;
					cl.there_is_data_to_send() = true;
				}
			}
			catch (const std::out_of_range& e)
			{
				(void)e;
				i++;
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
			i++;
		}
	}
	return 1;
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

#include "Server.hpp"
#include "Client.hpp"

Server::Server(char *pt, char *pass)
{
    struct addrinfo hints;
    struct addrinfo *p;
    struct addrinfo *server_info = this->getServerI();
    int check  = -1;
    myport(pt);
    
    sockfd = -1;
    if (mypass(pass))
        this->password = pass;   
    memset(&hints, 0 , sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
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
    std::cout << "server: waiting for connections... " << std::endl;
    freeaddrinfo(server_info);
    server_info  = NULL;
}


int Server::getsocket()
{
    return (this->sockfd);
}

int Server::run()
{
    std::vector <struct pollfd> &sockarray = getpollstruct();
    int sock = getsocket();
    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN;
    sockarray.push_back(pfd);
    int p = -1;
    while (1)
    {
        checkPollout(sockarray);
        p = poll(sockarray.data(), sockarray.size(), 3000);
        if (p == -1)
            throw std::runtime_error("poll failed");
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
                    if (sockarray[i].fd == sock) // from listener (a new connection)
                    {
                        NewConnection(sockarray, sock);
                    }
                    else // a client
                    {
                        RecieveMessage(sockarray, sockarray[i].fd);//sock
                    }
                }
                if (sockarray[i].revents & (POLLHUP | POLLERR))
                {
                    closeSocket(sockarray, sockarray[i].fd);
                    continue;
                }
                if (sockarray[i].revents & POLLOUT)
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
    struct sockaddr_in *hp;
    socklen_t sz = sizeof(st);
    char    ipv4char[IPV4LEN];

    int new_fd = -1;
    if ((new_fd = accept(sock,(sockaddr *) &st, &sz)) == -1)
    {
        return -1;
    }
    struct pollfd tmp;
    tmp.fd = new_fd;
    tmp.events = POLLIN;
    fds.push_back(tmp);
    hp =  (struct sockaddr_in *) &st;
    std::cout << "new connection  from : "
    << inet_ntop(hp->sin_family, &hp->sin_addr, ipv4char, sizeof(ipv4char))
    << std::endl;
    addClient(new_fd);
    try
    {
        Client &cl = getClient(new_fd);
        cl.setconnecttinme(time(NULL));  
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
        std::cerr << "recv failed !" << std::cout;
        closeSocket(fds, sock);
        return -1;// check for -1 later
    }
    if (bytes_recv == 0)
    {
        std::cerr << "Client disconnected !" << std::cout;
        closeSocket(fds, sock);
        return -1;// check for -1 later
    }
    buff[bytes_recv] = '\0';
    try {
        Client &cl = getClient(sock);
        cl.appand(buff);
    }
    catch (const std::out_of_range& e)
    {
        (void)e;
        std::cerr << "getClient() failed (at()) !" << std::endl;
        closeSocket(fds, sock); // i guess you should remove this 
        return 0;   
    }
    if (getClient(sock).getBuffer().size() > 5120)
    {
        std::cerr << "Client " << sock << " is flooding. Disconnecting." << std::endl;
        closeSocket(fds, sock); 
        return 0;
    }
    if (!getClient(sock).Authentication(*this))
        return 0;
    std::cout << "client : received " << buff << std::cout;
    return 1;
}

int Server::sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock)
{
    try 
    {
        ssize_t bytesent;
        Client &cl = getClient(sock);
        std::string &buf = cl.getoutbuffer();
        if (cl.getBuffer().empty())
            return (0);
       // fds[i].events |= POLLOUT;
        if ((bytesent = send(sock, buf.c_str(), buf.size(), 0) )== -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
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

const std::string &Server::getpass() const
{
    return this->password;
}

Server::~Server()
{
   // freeaddrinfo(getServerI());
   // close(sockfd);
}

Client& Server::getClient(int fd)
{
    return _client.at(fd); 
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

const cmaps &Server::getcmaps()
{
    return this->_client;
}

pollvec &Server::getpollstruct()
{
    return this->sockarrayy;
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
    std::vector <struct pollfd>::iterator last = fds.end();
    for (; it != last; it++)
    {
        if (it->fd == sock)
        {
            fds.erase(it);
            break;
        }
    }
    removeClient(sock);
    close(sock);
}

int Server::checkTimeout(pollvec &sockarray)
{

    for (size_t i = 0; i < sockarray.size();)
    {
        if (i > 0)
        {
            try 
            {
                Client &cl = getClient(sockarray[i].fd);
                if (((time(NULL)) - cl.getconnecttime()) > 60)
                {
                    std::cout << "Timeout: Closing unregistered client " << sockarray[i].fd << std::endl;
                    closeSocket(sockarray, sockarray[i].fd);
                    continue;
                }
            }
            catch (const std::out_of_range& e)
            {
                (void)e;
                std::cerr << "getClient() failed (at()) !" << std::endl;
                return -1;   
            }
        }
        i++;
    }
    return 1;
}

int Server::checkPollout(pollvec &fds)
{
    try 
    {
    for (size_t i = 1; i < fds.size();)
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
    return 1;
    }
     catch (const std::out_of_range& e)
    {
       (void)e;
        std::cerr << "getClient() failed (at()) !" << std::endl;
        return -1;   
    }
    return 1;
}

struct addrinfo *Server::getServerI()
{
    return this->serverI;
}
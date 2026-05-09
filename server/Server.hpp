#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <sys/socket.h>
#include "Channel.hpp"

#define PORT 2020
#define REQUEST SOMAXCONN
#define IPV4LEN 16
#define BUFFER 1024
#define SERVER_NAME "ft_irc.2004.ma"
#define MAX_CHANNEL 10
#define MAX_CLIENT 10000

#define ERR_NOTREGISTERED(server) (":" + std::string(server) + " 451 * :You have not registered\r\n")
#define ERR_CLOSINGLINK(host, reason) ("ERROR :Closing Link: " + std::string(host) + " (" + std::string(reason) + ")\r\n")
#define ERR_PASSWDMISMATCH(s) (":" + std::string(s) + " 464 * :Password incorrect\r\n")
#define ERR_NONICKNAME(s) (":" + std::string(s) + " 431 * :No nickname given\r\n")
#define ERR_ERRONEUSNICK(s, n) (":" + std::string(s) + " 432 * " + std::string(n) + " :Erroneous nickname\r\n")
#define ERR_NICKINUSE(s, n) (":" + std::string(s) + " 433 * " + std::string(n) + " :Nickname is already in use\r\n")
#define ERR_ALREADYREG(s) (":" + std::string(s) + " 462 * :Unauthorized command (already registered)\r\n")
#define ERR_NEEDMOREPARAMS(s, c) (":" + std::string(s) + " 461 * " + std::string(c) + " :Not enough parameters\r\n")
#define bot_CMD_PRIVMSG(target, text) ("PRIVMSG " + std::string(target) + " :" + std::string(text) + "\r\n")

#define CMD_PING(server_name) ("PING :" + std::string(server_name) + "\r\n")

// When they send PING without a token

#define ERR_NOORIGIN(s) (":" + std::string(s) + " 409 * :No origin specified\r\n")

// The successful PONG reply
#define RPL_PONG(s, token) (":" + std::string(s) + " PONG " + std::string(s) + " :" + std::string(token) + "\r\n")

// This is what you send when the user has been quiet for too long.

//The Ping Timeout ERROR Macro
#define ERR_PINGTIMEOUT(ip) ("ERROR :Closing Link: " + std::string(ip) + " (Ping timeout)\r\n")

#define ERR_QUIT(ip, reason) ("ERROR :Closing Link: " + std::string(ip) + " (Quit: " + std::string(reason) + ")\r\n")
//QUIT but for broadcasting
#define CMD_QUIT(prefix, reason) (":" + std::string(prefix) + " QUIT :Quit: " + std::string(reason) + "\r\n")

//PRIVMSG
#define CMD_PRIVMSG(prefix, target, text) (":" + std::string(prefix) + " PRIVMSG " + std::string(target) + " :" + std::string(text) + "\r\n")

//(They tried to DM a user who is offline, or a channel that hasn't been created).
#define ERR_NOSUCHNICK(server, nickname, target) (":" + std::string(server) + " 401 " + std::string(nickname) + " " + std::string(target) + " :No such nick/channel\r\n")

//(They tried to send a message to #1337, but they haven't JOINed #1337 yet).
#define ERR_CANNOTSENDTOCHAN(server, nickname, channel) (":" + std::string(server) + " 404 " + std::string(nickname) + " " + std::string(channel) + " :Cannot send to channel\r\n")

//(They typed PRIVMSG but forgot to put a target name).
#define ERR_NORECIPIENT(server, nickname, command) (":" + std::string(server) + " 411 " + std::string(nickname) + " :No recipient given (" + std::string(command) + ")\r\n")

//(They typed PRIVMSG #1337 but forgot to actually type a message)
#define ERR_NOTEXTTOSEND(server, nickname) (":" + std::string(server) + " 412 " + std::string(nickname) + " :No text to send\r\n")


// --- NICK BROADCAST MACRO ---
#define BROADCAST_NICK(old_nick, username, ip, new_nick) (":" + old_nick + "!" + username + "@" + ip + " NICK :" + new_nick + "\r\n")

class Client;
typedef std::map <int , Client> cmaps;
typedef std::vector <struct pollfd> pollvec;
typedef std::map<std::string, Channel>  chnmap;

class Server
{

	private :
		int         sockfd;
		std::string password;
		Server &operator=(const Server  &other);
		Server(const Server &other);
		Server();
		cmaps _client;
		std::map<std::string, Channel> _channels; // Player 2 map
		pollvec sockarrayy;
		struct addrinfo *serverI;
		std::string serverIp;

	public :
		Server(char *port, char *password);
		~Server();
		int run();
		int getsocket();
		std::string getServerIp();
		void setServerIp(std::string ip);
		int NewConnection(std::vector <struct pollfd> &fds, int sock);
		int RecieveMessage(std::vector <struct pollfd> &fds, int sock);
		int sendMessages(std::vector <struct pollfd> &fds, unsigned int i, int sock);
		std::string getpass();
		void removeClient(int fd);
		bool clientExists(int fd) const;
		Client& getClient(int fd);
		const cmaps & getcmaps();
		pollvec &getpollstruct();
		bool	sameName(std::string &nickname);
		void	closeSocket(pollvec &fds, int sock);
		void	Cleanup(pollvec &fds);
		int		checkTimeout(pollvec &fds);
		int		checkPollout(pollvec &fds);
		int		checkClients(pollvec &sockarray);
		struct addrinfo *getServerI();
		void	addClient(int fd);
		void	processCommand(std::string line, int sock);
		void	processBuffer(Client &cl);
		bool	Privmsg(Client &cl, std::string allCmd);
		bool	changeNICK(Client &cl, std::string &nickname);
		void	broadcastToSharedChannels(Client &ignored, std::string &messages);

		// --- Player 2 Channel Methods ---
		Channel* getChannel(std::string name);
		void createChannel(std::string name, Client &cl);
		Client* getClientByNickname(std::string nickname);
		void removeClientFromAllChannels(Client* cl, std::string quitMsg);

		// --- Player 1 Methods ---
		long getclientbyNick(const std::string &nick);
		Client *getClientByUser(std::string &username);



};

int			myport(char *port);
bool		mypass(char *pass);
int 		parsechannel();
void		toUpper(std::string &s);
std::string	bot_ascii_trim_line(const std::string& target, const std::string& raw_text);
bool		isSpecial(char c);


//tal3i
void		run_o(Channel& chn, Client& cl, std::string& param, char sign);
void		run_k(Channel& chn, Client& cl, std::string& param, char sign);
void 		run_l(Channel& chn, Client& cl, std::string& param, char sign);
//tal3i 

#endif

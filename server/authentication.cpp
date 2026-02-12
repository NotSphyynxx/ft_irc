#include "Client.hpp"

bool isSpecial(char c)
{
    if (c == '_' || c == '-' || c == '\\' || c == '[' || c == ']'
        || c == '{' || c == '}' || c == '^' || c == '|')
    {
        return true;
    }
    return false;
}

bool Client::Emptynames()
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
        return true;
    if (pass == sv.getpass())
    {
        setlevel(0, hasPASS);
        return true;
    }
    else
    {
        std::string error = ":" + std::string(SERVER_NAME) + " 464 :Password incorrect\nClosing unregistered client...\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
        sv.closeSocket(sv.getpollstruct(), getsock());
    }
    return false;
}

bool Client::nick(std::string &nickname, Server &sv)
{
    if (nickname.empty())
    {
        std::string error = ":ft_irc.2004.ma 431 :No nickname given\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
        return false;
    }
    if (nickname.size() > 9)
    {
        std::string error = ":ft_irc.2004.ma 432 :Nickname too long\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
        return false;
    }
    if (!isalpha(nickname[0]) && !isSpecial(nickname[0]))
    {
        std::string error = ":ft_irc.2004.ma 432 :Erroneous nickname\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
        return false;
    }
    for (size_t i = 0; i < nickname.size(); i++)
    {
       unsigned char c = nickname[i];
       if (c == '\r' && i + 1 < nickname.size() && nickname[i + 1] == '\n')
            break;
        if (!isdigit(c) && !isalpha(c) && !isSpecial(c))
        {
            std::string error = ":ft_irc.2004.ma 432 :Erroneous nickname\r\n";
            send(getsock(), error.c_str(), error.size(), 0);
            return false;
        }
    }// you need to check if there is another client with the same nickname
    if (sv.sameName(nickname))
    {
        std::string error = ":" + std::string(SERVER_NAME) + " 433 Nickname is already in use\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
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
        std::string error = ":server 462 :You may not reregister\r\n";
        send(getsock(), error.c_str(), error.size(), 0);
        return false;  
    }
    ss >> cmd;
    ss >> user;
    ss >> mode;
    ss >> unused;
    std::string rest_of_line;
    std::getline(ss, rest_of_line);
    if (user.empty() || mode.empty() || unused.empty()) {
        std::string error = "461 USER :Not enough parameters\r\n";
        send(getsock(), error.c_str(), error.size() , 0);
        return false;
    }
    size_t col_pos = rest_of_line.find(':');
    if (col_pos != std::string::npos) {
        realname = rest_of_line.substr(col_pos + 1);
    } else {
        // Fallback if they didn't put a colon (rare but possible)
        realname = rest_of_line; 
    }
    this->setusername(user);
    this->setrealname(realname);
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
    std::string serverName = "ft_irc.2004.ma";
    
    std::string rpl001 = ":" + serverName + " 001 " + nick + 
                         " :Welcome to the IRC Network " + nick + "!" + user + "@" + host + "\r\n";
    send(getsock(), rpl001.c_str(), rpl001.size(), 0);

    std::string rpl002 = ":" + serverName + " 002 " + nick + 
                         " :Your host is " + serverName + ", running version 1.0\r\n";
    send(getsock(), rpl002.c_str(), rpl002.size(), 0);
    
    std::string rpl003 = ":" + serverName + " 003 " + nick + 
                         " :This server was created Jan 01 2024\r\n";
    send(getsock(), rpl003.c_str(), rpl003.size(), 0);

    std::string rpl004 = ":" + serverName + " 004 " + nick + 
                         " " + serverName + " 1.0 io itkol\r\n";
    send(getsock(), rpl004.c_str(), rpl004.size(), 0);

 }
int Client::Authentication(Server &sv)
{
    size_t pos;
    std::string &copy = getBuffer();// later you will know if you need a copy or not 
    std::string extracted;
    std::string cmd;
    std::string value;
    std::stringstream sp;
    
    while ((pos = copy.find("\r\n")) != std::string::npos)
    {
        extracted = copy.substr(0, pos);
        sp.str(extracted);
        sp >> cmd;
        sp >> value;

        if (cmd == "NICK")
        {
            if (getlevel(0) == hasPASS)//has_pass 
            {
                if (!this->nick(value, sv))
                    return (copy.erase(0 , pos + 2), 0);
            }
            else
            {
                std::string err = ":ft_irc.2004.ma 451 * :You have not registered\r\n";
                send(getsock(), err.c_str(), err.size(), 0);
                return (copy.erase(0 , pos + 2), 0);
            }
        }
        if (cmd == "USER")
        {
            if (getlevel(0) == hasPASS && getlevel(1) == hasNICK)//has_pass && has_nick
            {
                if (!this->user(extracted))
                    return (copy.erase(0 , pos + 2),0);
            }
             else
            {
                std::string err = ":ft_irc.2004.ma 451 * :You have not registered\r\n";
                send(getsock(), err.c_str(), err.size(), 0);
                return (copy.erase(0 , pos + 2),0);
            }
            
        }
        if (cmd == "PASS")
        {
            if (!this->pass(value, sv))
                return (copy.erase(0 , pos + 2), 0);
            if (getlevel(0) == hasPASS && ((time(NULL)) - getconnecttime() > 40))
            {
                std::cout << "Timeout: Closing unregistered client " << getsock() << std::endl;
                sv.closeSocket(sv.getpollstruct(), getsock());
            }
        }
        copy.erase(0 , pos + 2);
        sp.clear();
        sp.str(extracted);
    }
    return 1;
}
#include "Client.hpp"

std::string  Client::appand(std::string buf)
{
    if (buf.empty())
        return (this->buffer);
    this->buffer += buf;
    return this->buffer;
}

//getters and setters
const int Client::getsock()
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
void Client::setnickname(std::string &s)
{
    this->nickname = s;
}
void Client::setusername(std::string &s)
{
    this->username = s;
}
void Client::setrealname(std::string &s)
{
    this->realname = s;
}


Client::Client(int sock) : mysocket(sock) , rank{EMPTY, EMPTY, EMPTY, EMPTY}
{
    
}
// bool Client::pass(std::string &pass, const Server &sv)
// {
//     if (pass == sv.getpass())
//     {
//         getlevel[1] = hasPASS;
//         return true;
//     }
//     else
//     {
//         std::string error = ":server_name 464 :Password incorrect\r\n";
//         send(getsocket(), error.c_str(), error.size());
//     }
//     return false;
// }

// bool Client::nick(std::string &nickname, const Server &sv)
// {
//     if (nickname.empty())
//     {
//         std::string error = ":server_name 431 :No nickname given\r\n";
//         send(getsocket(), error.c_str(), error.size());
//         return false;
//     }
//     if (nickname.size > 9)
//     {
//         std::string error = ":server_name 432 :Nickname too long\r\n";
//         send(getsocket(), error.c_str(), error.size());
//         return false;
//     }
//     if (!isalpha(nickname[0]) || !isSpecial(nickname[0]))
//     {
//         std::string error = ":server_name 432 :Erroneous nickname\r\n";
//         send(getsocket(), error.c_str(), error.size());
//         return false;
//     }
//     for (size_t i = 0; i <= nickname.size(); i++)
//     {
//         if (!isdigit(nickname[i]) && !isalpha(nickname[i]) && !isSpecial(nickname[i]))
//         {
//             std::string error = ":server_name 432 :Erroneous nickname\r\n";
//             send(getsocket(), error.c_str(), error.size());
//             return false;
//         }
//     }// you need to check if there is another client with the same nickname 
//     return true;
// }


// bool  Client::user(std::string &extracted, const Server &sv)
// {
//     std::stringstream ss(extracted);
//     std::string user;// should not be empty
//     std::string mode;
//     std::string unused;
//     std::string realname; //  start with :
  
//     if (getlevel(4) == REGISTRED)
//     {
//         std::string error = ":server 462 :You may not reregister\r\n";
//         send(getsocket(), error.c_str(), error.size());
//         return false;  
//     }
//     ss >> user;
//     ss >> mode;
//     ss >> unused;
//     std::string rest_of_line;
//     std::getline(ss, rest_of_line);
//     if (username.empty() || mode.empty() || unused.empty()) {
//         std::string error = "461 USER :Not enough parameters\r\n";
//         send(getsocket(), error.c_str(), error.size());
//         return false;
//     }
//     size_t col_pos = rest_of_line.find(':');
//     if (col_pos != std::string::npos) {
//         realname = rest_of_line.substr(col_pos + 1);
//     } else {
//         // Fallback if they didn't put a colon (rare but possible)
//         realname = rest_of_line; 
//     }
//     this->setusername(username);
//     this->setrealname(realname);
//     return true;
// }

// int Client::Authentication(const Server &sv)
// {
//     size_t pos;
//     bool    has_name = false;
//     bool    has_nick = false;
//     bool    has_pass = false;

//     std::string &copy = buffer;// later you will know if you need a copy or not 
//     std::string extracted;
//     std::string cmd;
//     std::string value;
//     std::stringstream sp;
    

//     while ((pos = copy.find("\r\n")) != std::string::npos)
//     {
//         extracted = copy.substr(0, pos);
//         sp(extracted);
//         sp >> cmd;
//         sp >> value;

//         if (cmd == "NICK")
//         {
//             has_nick = true;
//             if (has_pass)
//             {
//                 if (!this->nick(value, sv))
//                     return 0;
//             }
//             else
//             {
//                 std::cerr << "you need a password before to register !" << std::cout;
//                 return 0;
//             }
//         }
//         if (cmd == "USER")
//         {
//             has_name = true;
//             if (has_pass && has_nick)
//             {
//                 if (!this->user(extracted, sv))
//                     return 0;
//             }
//              else
//             {
//                 std::cerr << "you need a password and a nickname to register !" << std::cout;
//                 return 0;
//             }
            
//         }
//         if (cmd == "PASS")
//         {
//             if (has_name || has_nick)
//              {
//                  std::cerr << "you need a password before registration !" << std::cout;
//                  return 0;
//             }
//             else
//             {
//                 if (!this->pass(value, sv))
//                     return 0;
//                 has_pass = true;
//             }
//         }
//         copy.erase(0 , pos + 2);
//     }
//     return 1;
// }


// void Client::process_buffer()
// {
//     size_t pos;
//     Level lv[3];
//     std::string copy = buffer;
//     std::string extracted;
//     size_t pos;

//     while ((pos = copy.find("\r\n")) != std::string::npos)
//     {
//         extracted = copy.substr(0, pos);
//         if (extracted == "NICK" || extracted == "PASS")
//         {
//             if (rank[1] == EMPTY)
//                 {
//                     std::cerr << "you need a password !" << std::cout;
//                     continue
//                 }
//         }
        
//         copy.erase(0 , pos + 2);
//     }

// }


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
    return this->myIP;
}
void Client::myIp(std::string ip)
{
    this->myIp = ip;
}


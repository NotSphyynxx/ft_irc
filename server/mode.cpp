#include "Client.hpp"
#include "Server.hpp"
#include <sstream>



void run_o(Channel& chn, Client& cl, std::string& param, char sign)
{
    if (!chn.isOperator(&cl))
    {
        cl.getoutbuffer() += ":ft_irc.2004.ma 482 "
            + cl.getnickname() + " " + chn.getName() + " :You're not channel operator\r\n";
        return;
    }

    Client* target = chn.getClientByNickname(param);
    if (target == NULL)
    {
        cl.getoutbuffer() += ":ft_irc.2004.ma 441 "
            + cl.getnickname() + " " + param + " " + chn.getName() + " :They aren't on that channel\r\n";
        return;
    }

    if (sign == '+')  
    {
        if (!chn.isOperator(target))
            chn.addOperator(target);
    }
    else  
    {
        chn.removeOperator(target);
    }

    std::string modeStr = sign ? "+o" : "-o";
    std::string msg = ":" + cl.getnickname() + " MODE " + chn.getName()
        + " " + modeStr + " " + param + "\r\n";
    chn.broadcastMessage(msg);
}

void run_k(Channel& chn, Client& cl, std::string& param, char sign)
{
    if (!chn.isOperator(&cl))
    {
        cl.getoutbuffer() += ":ft_irc.2004.ma 482 "
            + cl.getnickname() + " " + chn.getName() + " :You're not channel operator\r\n";
        return;
    }

    if (sign == '+')  
    {
        chn.setKey(param);
        std::string msg = ":" + cl.getnickname() + " MODE " + chn.getName()
            + " +k " + param + "\r\n";
        chn.broadcastMessage(msg);
    }
    else  
    {
        if (chn.getKey() == param)
            chn.removeKey();
        else
                cl.getoutbuffer() += ":ft_irc.2004.ma 461 "
                + cl.getnickname() + " MODE :Key parameter does not match\r\n";
        std::string msg = ":" + cl.getnickname() + " MODE " + chn.getName()
            + " -k *\r\n";
        chn.broadcastMessage(msg);
    }
}
void run_l(Channel& chn, Client& cl, std::string& param, char sign)
{
    if (!chn.isOperator(&cl))
    {
        cl.getoutbuffer() += ":ft_irc.2004.ma 482 "
            + cl.getnickname() + " " + chn.getName() + " :You're not channel operator\r\n";
        return;
    }

    if (sign == '+') 
    {
        int limit = std::atoi(param.c_str());
        if (limit <= 0)
        {
            cl.getoutbuffer() += ":ft_irc.2004.ma 461 "
                + cl.getnickname() + " MODE :Invalid limit parameter\r\n";
            return;
        }
        chn.setLimit(limit);
        std::string msg = ":" + cl.getnickname() + " MODE " + chn.getName()
            + " +l " + param + "\r\n";
        chn.broadcastMessage(msg);
    }
    else  
    {
        chn.removeLimit();
        std::string msg = ":" + cl.getnickname() + " MODE " + chn.getName()
            + " -l\r\n";
        chn.broadcastMessage(msg);
    }
}
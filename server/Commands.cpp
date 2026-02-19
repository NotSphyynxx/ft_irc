#include "Client.hpp"

bool removeCRLF(std::string &str)// true there is crlf
{
    std::string cleaned;
    bool flag = false;
    size_t i = 0;

    for (; i < str.size(); ++i)
    {
        if (str[i] != '\r' && str[i] != '\n')
            cleaned += str[i];
        if (str[i] == '\r' || str[i] == '\n')
            flag = true;
    }

    str = cleaned;
    return flag;
}

void Server::processCommand(pollvec &fds, std::string line, int sock)
{
    try
    {
        std::string cmd = line;
        removeCRLF(cmd);
        Client &cl = getClient(sock);

        if (cmd.find("PONG") != std::string::npos)
        {
            cl.setLastActivity(time(NULL));
            cl.setping(false);
        }
    }
    catch (const std::out_of_range& e)
    {
       (void)e;
        std::cerr << "getClient() failed (at()) !" << std::endl;
    }

}

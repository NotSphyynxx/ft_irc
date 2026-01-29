#include "Server.cpp"

int myport(char *port)
{
    std::string  pt = port;

    if (pt.empty())
        throw std::runtime_error("Port cannot be empty!");
    for (size_t i = 0; i < pt.size(); i++)
    {
        if (!isdigit(pt[i]))
            throw std::runtime_error("Invalid input ! \n");   
    }
    long to_nb = atol(port);

    if (to_nb > 65535 || to_nb < 0)
        throw std::runtime_error("Overflow occured ! \n"); 

    if (to_nb < 1024)
        throw std::runtime_error("that port is reserved ! \n");

   return (static_cast<int> (to_nb)); 
}

bool mypass(char *pass)
{
   std::string pw = pass;
   if (pw.empty())
        throw std::runtime_error("Empty password ! \n");
    return true;
}
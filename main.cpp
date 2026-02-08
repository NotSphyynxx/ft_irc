#include "Server.hpp"

int main(int argc, char **argv)
{

    try
    {
        if (argc != 3)
        {
            throw std::runtime_error("missing arguments !\n");
        }
        Server sv(argv[1], argv[2]);
        sv.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
    
}
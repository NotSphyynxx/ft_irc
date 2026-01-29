#include "Server.hpp"

int main(int argc, char **argv)
{

    try
    {
        if (argc != 3)
        {
            throw std::runtime_error("missing arguments !\n");
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }
    
}
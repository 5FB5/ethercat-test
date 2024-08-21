#include <iostream>
#include "ethercat.h"

int main()
{
    std::string interfaceName = "enp3s0";

    if (ec_init(interfaceName.c_str()) == 0)
    {
        std::cout << "ERROR!" << std::endl;
        return -1;
    }

    std::cout << "Network initialized at " << interfaceName << std::endl;

    ec_close();

    return 0;
}

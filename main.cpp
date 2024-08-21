#include <iostream>
#include "ethercat.h"

void readPdo()
{

}

int main()
{
    std::string interfaceName = "enp3s0";

    if (ec_init(interfaceName.c_str()) == 0)
    {
        std::cout << "ERROR with init network. Start app with root permission!" << std::endl;
        return -1;
    }

    std::cout << "Network initialized at " << interfaceName << std::endl;

    ec_config_init(FALSE);

    if (ec_slavecount == 0)
    {
        std::cout << "No connected slaves found" << std::endl;
        ec_close();
        return -1;
    }

    std::cout << "Found " << ec_slavecount << " slave(s):" << std::endl;

    // Отсчёт slaves идёт от 1
    for (int i = 1; i <= ec_slavecount; i++)
    {
        std::cout << "\tSlave[" << i << "]: " << ec_slave[i].name << std::endl;
    }

    char ioMap[128];
    int usedmem;

    usedmem = ec_config_map(ioMap);

    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

    if (ec_slave[0].state == EC_STATE_SAFE_OP)
        std::cout << "All slaves are in SAFE OP" << std::endl;
    else
        std::cout << "Not all slaves in SAFE OP" << std::endl;

    ec_readstate();

    ec_close();

    return 0;
}

#include <iostream>
#include "ethercat.h"

void printObjectDictionary(uint16_t slave)
{
    std::cout << std::endl;
    std::cout << "Object dictionary for slave: " << slave << " [" << ec_slave[slave].name << "]" << std::endl;

    ec_ODlistt objectDictionary;

    objectDictionary.Entries = 0;

    if (!ec_readODlist(slave, &objectDictionary))
    {
        std::cout << "Can't read object dictionary" << std::endl;
        return;
    }

    std::cout << "Found " << objectDictionary.Entries << " entries" << std::endl;

    for (int i = 0; i < objectDictionary.Entries; i++)
    {
        uint16_t index = objectDictionary.Index[i];

        ec_readODdescription(i, &objectDictionary);

        std::cout << "Index: " << "0x" << std::hex << index << " [" << objectDictionary.Name[i] << "]" << std::endl;
    }
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

    int wkc;
    char ioMap[4096];

    std::cout << "Found " << ec_slavecount << " slave(s)" << std::endl;

    ec_config_map(ioMap);

    ec_slave[0].state = EC_STATE_SAFE_OP;
    ec_writestate(0);
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

    std::cout << "Slave(s) info:" << std::endl;

    // Отсчёт slaves идёт от 1
    for (int i = 1; i <= ec_slavecount; i++)
    {
        std::cout << "\tSlave[" << i << "]: " << ec_slave[i].name << std::endl;
        std::cout << "\tVendorID: " << ec_slave[i].eep_id << std::endl;
        std::cout << "\tEtherCAT addr: 0x" << ec_slave[i].configadr << std::endl;
        std::cout << "\tManufacturer: 0x" << ec_slave[i].eep_man << std::endl;
    }

    if (ec_slave[0].state != EC_STATE_SAFE_OP)
    {
        std::cout << "Not all slaves in SAFE OP" << std::endl;
        ec_close();
        return -1;
    }

    std::cout << "All slaves are in SAFE OP state" << std::endl;

    std::cout << "Set slaves to OP mode..." << std::endl;

    ec_slave[0].state = EC_STATE_OPERATIONAL;

    // Так как эта штука посылает 1 Read/Write запрос, то WorkingCounter должен быть +3
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);

    ec_writestate(0);
    ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);

    std::cout <<"Wkc: " << wkc << std::endl;

    if (ec_slave[0].state != EC_STATE_OPERATIONAL)
        std::cout << "Not all slaves in OP state" << std::endl;
    else
        std::cout << "All slaves are in OP state" << std::endl;

    for (int i = 1; i <= ec_slavecount; i++)
    {
        printObjectDictionary(1);
    }

    ec_close();

    return 0;
}

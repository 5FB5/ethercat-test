#include <iostream>
#include "ethercat.h"

#define OTYPE_VAR               0x0007
#define OTYPE_ARRAY             0x0008
#define OTYPE_RECORD            0x0009

#define ATYPE_Rpre              0x01
#define ATYPE_Rsafe             0x02
#define ATYPE_Rop               0x04
#define ATYPE_Wpre              0x08
#define ATYPE_Wsafe             0x10
#define ATYPE_Wop               0x20

char* otype2string(uint16 otype)
{
    static char str[32] = { 0 };

    switch(otype)
    {
    case OTYPE_VAR:
        sprintf(str, "VAR");
        break;
    case OTYPE_ARRAY:
        sprintf(str, "ARRAY");
        break;
    case OTYPE_RECORD:
        sprintf(str, "RECORD");
        break;
    default:
        sprintf(str, "ot:0x%4.4X", otype);
    }
    return str;
}

char* dtype2string(uint16 dtype, uint16 bitlen)
{
    static char str[32] = { 0 };

    switch(dtype)
    {
    case ECT_BOOLEAN:
        sprintf(str, "BOOLEAN");
        break;
    case ECT_INTEGER8:
        sprintf(str, "INTEGER8");
        break;
    case ECT_INTEGER16:
        sprintf(str, "INTEGER16");
        break;
    case ECT_INTEGER32:
        sprintf(str, "INTEGER32");
        break;
    case ECT_INTEGER24:
        sprintf(str, "INTEGER24");
        break;
    case ECT_INTEGER64:
        sprintf(str, "INTEGER64");
        break;
    case ECT_UNSIGNED8:
        sprintf(str, "UNSIGNED8");
        break;
    case ECT_UNSIGNED16:
        sprintf(str, "UNSIGNED16");
        break;
    case ECT_UNSIGNED32:
        sprintf(str, "UNSIGNED32");
        break;
    case ECT_UNSIGNED24:
        sprintf(str, "UNSIGNED24");
        break;
    case ECT_UNSIGNED64:
        sprintf(str, "UNSIGNED64");
        break;
    case ECT_REAL32:
        sprintf(str, "REAL32");
        break;
    case ECT_REAL64:
        sprintf(str, "REAL64");
        break;
    case ECT_BIT1:
        sprintf(str, "BIT1");
        break;
    case ECT_BIT2:
        sprintf(str, "BIT2");
        break;
    case ECT_BIT3:
        sprintf(str, "BIT3");
        break;
    case ECT_BIT4:
        sprintf(str, "BIT4");
        break;
    case ECT_BIT5:
        sprintf(str, "BIT5");
        break;
    case ECT_BIT6:
        sprintf(str, "BIT6");
        break;
    case ECT_BIT7:
        sprintf(str, "BIT7");
        break;
    case ECT_BIT8:
        sprintf(str, "BIT8");
        break;
    case ECT_VISIBLE_STRING:
        sprintf(str, "VISIBLE_STR(%d)", bitlen);
        break;
    case ECT_OCTET_STRING:
        sprintf(str, "OCTET_STR(%d)", bitlen);
        break;
    default:
        sprintf(str, "dt:0x%4.4X (%d)", dtype, bitlen);
    }
    return str;
}

char* access2string(uint16 access)
{
    static char str[32] = { 0 };

    sprintf(str, "%s%s%s%s%s%s",
            ((access & ATYPE_Rpre) != 0 ? "R" : "_"),
            ((access & ATYPE_Wpre) != 0 ? "W" : "_"),
            ((access & ATYPE_Rsafe) != 0 ? "R" : "_"),
            ((access & ATYPE_Wsafe) != 0 ? "W" : "_"),
            ((access & ATYPE_Rop) != 0 ? "R" : "_"),
            ((access & ATYPE_Wop) != 0 ? "W" : "_"));
    return str;
}

void printObjectDescription(uint16_t slave)
{
    std::cout << std::endl;
    std::cout << "Object dictionary for slave: " << slave << " [" << ec_slave[slave].name << "]" << std::endl;

    ec_ODlistt objectDescriptionList;
    ec_OElistt objectEntryInformationList;

    objectDescriptionList.Entries = 0;

    if (!ec_readODlist(slave, &objectDescriptionList))
    {
        std::cout << "Can't read object dictionary" << std::endl;
        return;
    }

    std::cout << "Found " << objectDescriptionList.Entries << " entries" << std::endl;

    for (int i = 0; i < objectDescriptionList.Entries; i++)
    {
        uint16_t index = objectDescriptionList.Index[i];
        uint8_t maxSubindexes = 0;

        ec_readODdescription(i, &objectDescriptionList);

        std::cout << "Index: " << "0x" << std::hex << index << " [" << objectDescriptionList.Name[i] << "] " << "[" <<  otype2string(objectDescriptionList.ObjectCode[i]) << "]" << std::endl;

        ec_readOE(i, &objectDescriptionList, &objectEntryInformationList);

        if (objectDescriptionList.ObjectCode[i] != OTYPE_VAR)
        {
            int l = sizeof(maxSubindexes);
            int wkc = ec_SDOread(slave, objectDescriptionList.Index[i], 0, FALSE, &l, &maxSubindexes, EC_TIMEOUTRXM);

            if (wkc == 1 )
                std::cout << "Good read (wkc: " << wkc << ")" << std::endl;
            else
                std::cout << "Bad read (wkc: " << wkc << ")" << std::endl;
        }
        else
        {
            maxSubindexes = objectDescriptionList.MaxSub[i];
        }

        for (int j = 0; j < maxSubindexes; j++)
        {
            std::cout << "\t" << j << ": " << objectEntryInformationList.Name[j] <<
                    " [" << dtype2string(objectEntryInformationList.DataType[j], objectEntryInformationList.BitLength[j]) << "] = "
                      << " [" << access2string(objectEntryInformationList.ObjAccess[j]) << "] " << std::endl;
        }
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

    for (int i = 1; i <= ec_slavecount; i++)
    {
        printObjectDescription(1);
    }

    ec_close();

    return 0;
}

#include <iostream>
#include <vector>
#include <memory.h>
#include <unistd.h>
#include <inttypes.h>
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

ec_ODlistt objectDescriptionList;
ec_OElistt objectEntryInformationList;

char *ioMap = nullptr;

struct currentPdoSubindexInfo
{
    uint32_t subindex;
    std::string name;
    std::string type;
};

struct readData_t
{
    int32_t posActualValue;
    uint32_t digitalInputs;
    uint16_t statusWord;
} __attribute__((packed));


struct writeData_t
{
    int32_t position;
    uint32_t subindex01;
    uint16_t controlword;
} __attribute__((packed));

struct registert
{
    uint8_t subindex;
    std::string name;
};

struct record_t
{
    uint16_t index;
    std::string name;

    std::vector<registert> registers;
};

struct pdoMap_t
{
    uint16_t smIndex;
    std::vector<record_t> records;
};

pdoMap_t pdoInput;
pdoMap_t pdoOutput;

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
    std::cout << "________________" << std::endl;
    std::cout << "Object dictionary for slave: " << slave << " [" << ec_slave[slave].name << "]" << std::endl;

    objectDescriptionList.Entries = 0;

    if (!ec_readODlist(slave, &objectDescriptionList))
    {
        std::cout << "Can't read object dictionary" << std::endl;
        return;
    }

    std::cout << "Found " << std::dec << objectDescriptionList.Entries << " entries" << std::endl;

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

        for (int j = 0; j <= maxSubindexes; j++)
        {
            std::cout << "\t" << std::dec << j << ": " << objectEntryInformationList.Name[j] <<
                    " [" << dtype2string(objectEntryInformationList.DataType[j], objectEntryInformationList.BitLength[j]) << "] = "
                      << " [" << access2string(objectEntryInformationList.ObjAccess[j]) << "] " << std::endl;
        }

        std::cout << std::endl;
    }
}

int readInputPdoMapping(uint16_t slave, uint16_t pdoAssign)
{
    uint16_t pdo_number = 0;    // Число PDO объектов
    uint16_t pdo_cnt = 0;
    uint32_t pdo_data = 0;
    uint16_t current_io_addr = 0;   // Используется для отображения PDO_description в iomap
    uint8_t bitlen = 0;

    int bsize = 0;
    int wkc = 0;

    int data_size = sizeof(pdo_number);

    // Получаем число PDO объектов с нулевого сабиндекса
    wkc = ec_SDOread(slave, pdoAssign, 0x00, FALSE, &data_size, &pdo_number, EC_TIMEOUTRXM);
    pdo_number = etohs(pdo_number); // Конвертация в зависимости от платформы

    if (wkc <= 0 || pdo_number <= 0)
    {
        std::cout << "Can't get TxPDO mapping (wkc:" << wkc << ", pdo_number:" << pdo_number << ")" << std::endl;
        return -1;
    }

    for (int pdoCnt = 1; pdoCnt <= pdo_number; pdoCnt++)
    {
        uint16_t currentIndex = 0;

        data_size = sizeof(pdo_number);

        wkc = ec_SDOread(slave, pdoAssign, (uint8_t)pdoCnt, FALSE, &data_size, &currentIndex, EC_TIMEOUTRXM);

        currentIndex = etohs(currentIndex);

        if (currentIndex <= 0)
        {
            std::cout << "Can't get PDO from index" << currentIndex << std::endl;
            return -1;
        }

        std::vector<currentPdoSubindexInfo> subindexesList;

        std::cout << std::endl;
        std::cout << "PDO index: 0x" << std::hex << currentIndex << std::endl;

        uint8_t subindexNumber = 0;

        data_size = sizeof(subindexNumber);

        std::cout << std::endl;
        std::cout << "Reading PDO subindex count..." << std::endl;

        wkc = ec_SDOread(slave, currentIndex, 0x00, FALSE, &data_size, &subindexNumber, EC_TIMEOUTRXM);

        if (wkc <= 0)
        {
            std::cout << "Bad read (wkc:" << wkc << ")" << std::endl;
            return -1;
        }

        std::cout << "Good read (wkc: " << wkc << ")" << std::endl;
        std::cout << std::endl;

        data_size = sizeof(pdo_data);

        for (int iSubidx = 1; iSubidx <= subindexNumber; iSubidx++)
        {
            ec_SDOread(slave, currentIndex, iSubidx, FALSE, &data_size, &pdo_data, EC_TIMEOUTRXM);

            uint16_t index = (uint16_t) (pdo_data >> 16);
            uint8_t subindex = (uint8_t) ( (pdo_data >> 8) & 0x000000ff);

            uint8_t size = (pdo_data & 0x000000ff) / 8;

            bitlen = LO_BYTE(pdo_data);
            bsize += bitlen;

            objectDescriptionList.Slave = slave;
            objectDescriptionList.Index[0] = index;

            objectEntryInformationList.Entries = 0;

            if (index || subindex)
            {
                std::cout << "Reading OE info..." << std::endl;
                wkc = ec_readOEsingle(0, subindex, &objectDescriptionList, &objectEntryInformationList);
            }

            if (wkc <= 0)
            {
                std::cout << "Can't read OE (wkc: " << wkc << ")" << std::endl;
                return -1;
            }

            if (objectEntryInformationList.Entries == 0)
            {
                std::cout << "No entries in OE list" << std::endl;
                return -1;
            }

            std::cout << "Good read (wkc: " << wkc << ")" << std::endl;
            std::cout << std::endl;

            currentPdoSubindexInfo newInfo;

            newInfo.subindex = static_cast<int>(subindex);
            newInfo.name = objectEntryInformationList.Name[subindex];
            newInfo.type = dtype2string(objectEntryInformationList.DataType[subindex], bitlen);

            subindexesList.push_back(newInfo);
        }

        std::cout << std::endl;

        std::cout << "0x" << std::hex << currentIndex << std::endl;

        for (const auto &it : subindexesList)
        {
            std::cout << "\t" << std::dec << it.subindex << ": " << it.name << " " << it.type << std::endl;
        }
    }

    return bsize;
}

int readOutputPdoMapping(uint16_t slave, uint16_t pdoAssign)
{
    uint16_t pdo_number = 0;    // Число PDO объектов
    uint16_t pdo_cnt = 0;
    uint32_t pdo_data = 0;
    uint16_t current_io_addr = 0;   // Используется для отображения PDO_description в iomap
    uint8_t bitlen = 0;

    int bsize = 0;
    int wkc = 0;

    int data_size = sizeof(pdo_number);

    // Получаем число PDO объектов с нулевого сабиндекса
    wkc = ec_SDOread(slave, pdoAssign, 0x00, FALSE, &data_size, &pdo_number, EC_TIMEOUTRXM);
    pdo_number = etohs(pdo_number); // Конвертация в зависимости от платформы

    if (wkc <= 0 || pdo_number <= 0)
    {
        std::cout << "Can't get RxPDO mapping (wkc:" << wkc << ", pdo_number:" << pdo_number << ")" << std::endl;
        return -1;
    }

    for (int pdoCnt = 1; pdoCnt <= pdo_number; pdoCnt++)
    {
        uint16_t currentIndex = 0;

        data_size = sizeof(pdo_number);

        wkc = ec_SDOread(slave, pdoAssign, (uint8_t)pdoCnt, FALSE, &data_size, &currentIndex, EC_TIMEOUTRXM);

        currentIndex = etohs(currentIndex);

        if (currentIndex <= 0)
        {
            std::cout << "Can't get PDO from index" << currentIndex << std::endl;
            return -1;
        }

        std::vector<currentPdoSubindexInfo> subindexesList;

        std::cout << std::endl;
        std::cout << "PDO index: 0x" << std::hex << currentIndex << std::endl;

        uint8_t subindexNumber = 0;

        data_size = sizeof(subindexNumber);

        std::cout << std::endl;
        std::cout << "Reading PDO subindex count..." << std::endl;

        wkc = ec_SDOread(slave, currentIndex, 0x00, FALSE, &data_size, &subindexNumber, EC_TIMEOUTRXM);

        if (wkc <= 0)
        {
            std::cout << "Bad read (wkc:" << wkc << ")" << std::endl;
            return -1;
        }

        std::cout << "Good read (wkc: " << wkc << ")" << std::endl;
        std::cout << std::endl;

        data_size = sizeof(pdo_data);

        for (int iSubidx = 1; iSubidx <= subindexNumber; iSubidx++)
        {
            ec_SDOread(slave, currentIndex, iSubidx, FALSE, &data_size, &pdo_data, EC_TIMEOUTRXM);

            pdo_data = etohl(pdo_data);

            uint16_t index = (uint16_t) (pdo_data >> 16);
            uint8_t subindex = (uint8_t) ( (pdo_data >> 8) & 0x000000ff);
            uint8_t size = (pdo_data & 0x000000ff) / 8;

            bitlen = LO_BYTE(pdo_data);
            bsize += bitlen;

            objectDescriptionList.Slave = slave;
            objectDescriptionList.Index[0] = index;

            objectEntryInformationList.Entries = 0;

            if (index || subindex)
            {
                std::cout << "Reading OE info..." << std::endl;
                wkc = ec_readOEsingle(0, subindex, &objectDescriptionList, &objectEntryInformationList);
            }

            if (wkc <= 0)
            {
                std::cout << "Can't read OE (wkc: " << wkc << ")" << std::endl;
                return -1;
            }

            if (objectEntryInformationList.Entries == 0)
            {
                std::cout << "No entries in OE list" << std::endl;
                return -1;
            }

            std::cout << "Good read (wkc: " << wkc << ")" << std::endl;
            std::cout << std::endl;

            currentPdoSubindexInfo newInfo;

            newInfo.subindex = subindex;
            newInfo.name = objectEntryInformationList.Name[subindex];
            newInfo.type = dtype2string(objectEntryInformationList.DataType[subindex], bitlen);

            subindexesList.push_back(newInfo);
        }

        std::cout << std::endl;

        std::cout << "0x" << std::hex << currentIndex << std::endl;

        for (const auto &it : subindexesList)
        {
            std::cout << "\t" << std::dec << it.subindex << ": " << it.name << " " << it.type << std::endl;
        }
    }

    return bsize;
}

void printPdoMapping(uint16_t slave)
{
    if (ioMap == nullptr)
    {
        std::cout << "Can't get PDO, 'cause iomap not initialized" << std::endl;
        return;
    }

    std::cout << std::endl;
    std::cout << "_____________________" << std::endl;
    std::cout << "Reading PDOs..." << std::endl;

    uint8_t syncManagerNumber, currentSyncManager = 0;

    int wkc = 0;
    int syncManagerNumberSize = 0;
    int smBugAdd = 0;

    int outputsNum = 0;
    int inputsNum = 0;

    syncManagerNumberSize = sizeof(syncManagerNumber);

    wkc = ec_SDOread(slave, ECT_SDO_SMCOMMTYPE, 0x00, FALSE, &syncManagerNumberSize, &syncManagerNumber, EC_TIMEOUTRXM);

    if (wkc <= 0 || syncManagerNumber <= 2)
    {
        std::cout << "Can't start read PDO (wkc =" << wkc << ", syncManagerNumber =" << syncManagerNumber << ")" << std::endl;
        return;
    }

    // make syncManagerNumber equal to number of defined SM
    syncManagerNumber--;

    if (syncManagerNumber > EC_MAXSM)
        syncManagerNumber = EC_MAXSM;

    for (int iSm = 2; iSm <= syncManagerNumber; iSm++)
    {
        syncManagerNumberSize = sizeof(currentSyncManager);

        std::cout << "Reading current SM..." << std::endl;

        wkc = ec_SDOread(slave, ECT_SDO_SMCOMMTYPE, iSm + 1, FALSE, &syncManagerNumberSize, &currentSyncManager, EC_TIMEOUTRXM);

        if (wkc <= 0)
        {
            std::cout << "Bad read (wkc: " << wkc << ")" << std::endl;
            return;
        }

        std::cout << "Good read (wkc: " << wkc << ")" << std::endl;

        if ((iSm == 2) && (currentSyncManager == 2)) // SM2 has type 2 == mailbox out, this is a bug in the slave!
        {
            smBugAdd = 1; // try to correct, this works if the types are 0 1 2 3 and should be 1 2 3 4
            std::cout << "Activated SM type workaround, possible incorrect mapping" << std::endl;
        }

        if (currentSyncManager)
            currentSyncManager += smBugAdd; // only add if SMt > 0

        std::cout << std::endl;
        std::cout << "Reading SM" << static_cast<unsigned>(currentSyncManager) << " (";

        if (currentSyncManager == 3) // RxPDO
        {
            std::cout << "RxPDO)..." << std::endl;
            outputsNum = readOutputPdoMapping(slave, ECT_SDO_PDOASSIGN + iSm);
        }
        else if (currentSyncManager == 4) // TxPDO
        {
            std::cout << "TxPDO)..." << std::endl;
            inputsNum = readInputPdoMapping(slave, ECT_SDO_PDOASSIGN + iSm);
        }

        std::cout << std::endl;
    }
}

/*
 * Control word (0x6040 RxPDO)
 * Status word (0x60401 RxPDO)
 * Modes of operation (0x6060 RxPDO)
 * Mode of operation display (0x6061 TxPDO)
 * Target torque (0x6071 RxPDO)
 * Torque actual value (0x6077 TxPDO)
 */

// PreOP to SafeOP state hook
int po2soHook(uint16_t slave)
{
    std::cout << std::endl;
    std::cout << "Set custom PDO map..." << std::endl;

    // Очищаем SM
    uint8_t pdoCounter = 0;
    int wkc = 0;

    std::cout << "\tClear input SM (0x1c13)...";
    // Очищаем сначала Input SM
    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1c13), 0x00, FALSE, sizeof(pdoCounter), &pdoCounter, EC_TIMEOUTRXM);

    if (wkc <= 0)
        std::cout << "Bad write (wkc: " << wkc << ")" << std::endl;
    else
        std::cout << "Good write (wkc: " << wkc << ")" << std::endl;

    std::cout << "\tClear output SM (0x1c12)...";

    wkc = 0;

    // Очищаем Output SM
    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1c12), 0x00, FALSE, sizeof(pdoCounter), &pdoCounter, EC_TIMEOUTRXM);

    if (wkc <= 0)
        std::cout << "Bad write (wkc: " << wkc << ")" << std::endl;
    else
        std::cout << "Good write (wkc: " << wkc << ")" << std::endl;

    // RxPDO index = 1607
    // TxPDO index = 1A07

    // Очищаем текущие PDO

    std::cout << "\tReset RxPDO...";
    wkc = 0;

    uint8_t clearObj = 0;

    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 00, FALSE, sizeof(clearObj), &clearObj, EC_TIMEOUTRXM);

    if (wkc == 1)
        std::cout << "Good writing (wkc: 1)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    std::cout << "\tReset TxPDO...";
    wkc = 0;

    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1A07), 00, FALSE, sizeof(clearObj), &clearObj, EC_TIMEOUTRXM);

    if (wkc == 1)
        std::cout << "Good writing (wkc: 1)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    // RxPDO converted objects
    uint32_t controlWordObj = (0x6040 << 16) | (0 << 8) | (sizeof(uint16_t) * 8);
    uint32_t statusWordObj = (0x6041 << 16) | (0 << 8) | (sizeof(uint16_t) * 8);
    uint32_t modesOfOpObj = (0x6060 << 16) | (0 << 8) | (sizeof(int8_t) * 8);
    uint32_t targetTorqueObj = (0x6071 << 16) | (0 << 8) | (sizeof(int16_t) * 8);

    // TxPDO converted objects
    uint32_t modesOfOpDisplayObj = (0x6061 << 16) | (0 << 8) | (sizeof(int8_t) * 8);
    uint32_t torqueActualValueObj = (0x6077 << 16) | (0 << 8) | (sizeof(int16_t) * 8);

    // Разметка RxPDO
    std::cout << "\tMapping RxPDO...";
    wkc = 0;

    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 1, FALSE, sizeof(controlWordObj), &controlWordObj, EC_TIMEOUTRXM);
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 2, FALSE, sizeof(statusWordObj), &statusWordObj, EC_TIMEOUTRXM);
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 3, FALSE, sizeof(modesOfOpObj), &modesOfOpObj, EC_TIMEOUTRXM);
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 4, FALSE, sizeof(targetTorqueObj), &targetTorqueObj, EC_TIMEOUTRXM);

    if (wkc == 4)
        std::cout << "Good writing (wkc: 4)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    // Разметка TxPDO
    std::cout << "\tMapping RxPDO...";
    wkc = 0;

    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1A07), 1, FALSE, sizeof(modesOfOpDisplayObj), &modesOfOpDisplayObj, EC_TIMEOUTRXM);
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1A07), 2, FALSE, sizeof(torqueActualValueObj), &torqueActualValueObj, EC_TIMEOUTRXM);

    if (wkc == 2)
        std::cout << "Good writing (wkc: 2)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    uint8_t rxPdoContentSize = 4;
    uint8_t txPdoContentSize = 2;

    std::cout << "\tSet Rx and Tx PDOs sizes...";
    wkc = 0;

    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1607), 0, FALSE, sizeof(uint8_t), &rxPdoContentSize, EC_TIMEOUTRXM);
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1A07), 0, FALSE, sizeof(uint8_t), &txPdoContentSize, EC_TIMEOUTRXM);

    if (wkc == 2)
        std::cout << "Good writing (wkc: 2)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    // Настраиваем Sync Managar'ы

    std::cout << "\tSet Sync Managers..." << std::endl;

    // Задаём объекты PDO в SyncManager
    std::cout << "\tSet RxPDO to SyncManager 0x1c13...";
    wkc = 0;

    uint16_t rxPdoObj = static_cast<uint16_t>(0x1607);
    uint16_t txPdoObj = static_cast<uint16_t>(0x1A07);

    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1c13), 1, FALSE, sizeof(uint16_t), &rxPdoObj, EC_TIMEOUTRXM);

    if (wkc == 1)
        std::cout << "Good writing (wkc: 1)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    std::cout << "\tSet TxPDO to SyncManager 0x1c12...";
    wkc = 0;

    wkc = ec_SDOwrite(slave, static_cast<uint16_t>(0x1c12), 1, FALSE, sizeof(uint16_t), &txPdoObj, EC_TIMEOUTRXM);

    if (wkc == 1)
        std::cout << "Good writing (wkc: 1)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    std::cout << "\tUpdate SyncManager's PDO counter...";

    wkc = 0;

    // RxPDO
    uint8_t rxPdoCounter = 1;
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1c13), 0, FALSE, sizeof(rxPdoCounter), &rxPdoCounter, EC_TIMEOUTRXM);

    // TxPDO
    uint8_t txPdoCounter = 1;
    wkc += ec_SDOwrite(slave, static_cast<uint16_t>(0x1c12), 0, FALSE, sizeof(txPdoCounter), &txPdoCounter, EC_TIMEOUTRXM);

    if (wkc == 2)
        std::cout << "Good writing (wkc: 2)" << std::endl;
    else
        std::cout << "Bad writing (wkc: " << wkc << ")" << std::endl;

    std::cout << std::endl;

    return 1;
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

    std::cout << "Found " << ec_slavecount << " slave(s)" << std::endl;

    // Прикрепляем коллбек при переходе из PreOP в SafeOP для маппинга PDO
    for (int i = 1; i <= ec_slavecount; i++)
    {
        ec_slave[i].PO2SOconfig = po2soHook;
    }

    ioMap = new char[4096];
    ec_config_map(ioMap);

    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

    std::cout << "Slave(s) info:" << std::endl;

    // Отсчёт slaves идёт от 1
    for (int i = 1; i <= ec_slavecount; i++)
    {
        std::cout << "\tSlave[" << i << "]: " << ec_slave[i].name << std::endl;
        std::cout << "\tVendorID: " << ec_slave[i].eep_id << std::endl;
        std::cout << "\tEtherCAT addr: 0x" << ec_slave[i].configadr << std::endl;
        std::cout << "\tManufacturer: 0x" << ec_slave[i].eep_man << std::endl;
        std::cout << std::endl;
    }

    if (ec_slave[0].state != EC_STATE_SAFE_OP)
    {
        std::cout << "Not all slaves in SAFE OP" << std::endl;

        ec_close();
        return -1;
    }

    std::cout << "All slaves are in SAFE OP state" << std::endl;

    // readData_t *readData = (readData_t*)ec_slave[1].inputs;
    // writeData_t *writeData = (writeData_t*)ec_slave[1].outputs;

    // std::cout << "Set slaves to OP state..." << std::endl;
    // // Перед переводом в OP режим надо отправить пакет
    // ec_send_processdata();
    // ec_receive_processdata(EC_TIMEOUTRET);

    // ec_slave[0].state = EC_STATE_OPERATIONAL;

    // ec_writestate(0);

    // ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);

    // if (ec_slave[0].state != EC_STATE_OPERATIONAL)
    // {
    //     std::cout << "Not all slaves are in OP state" << std::endl;

    //     ec_close();
    //     return -1;
    // }

    // std::cout << "All slaves are in OP state" << std::endl;

    // int counter = 0;

    // while (counter != 15)
    // {
    //     sleep(1);

    //     ec_send_processdata();
    //     wkc = ec_receive_processdata(EC_TIMEOUTRET);

    //     std::cout << "controlWord: " << readData->statusWord << std::endl;

    //     counter++;
    // }

    for (int i = 1; i <= ec_slavecount; i++)
    {
        // printObjectDescription(i);
        printPdoMapping(i);
    }

    ec_close();

    return 0;
}

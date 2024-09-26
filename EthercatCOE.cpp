//
// Created by Artem Gusev on 8/9/23.
//

#include "EthercatCOE.h"

/**
 * @brief Функция очистки SyncManager
 * @details Используется перед разметкой PDO
 * @param slave
 * @return
 */
int EthercatCOE::clearSM(uint16_t slave, uint16_t smIndex)
{
    uint8_t pdoCounter = 0;
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(smIndex), 00, FALSE, sizeof(pdoCounter), &pdoCounter, EC_TIMEOUTRXM);
    return wc;
}

/**
 * @brief Функция очистки разметки PDO
 * @param slave
 * @param pdoMappingIndex
 * @return
 */
int EthercatCOE::clearPDOMapping(uint16_t slave, uint16_t pdoMappingIndex)
{
    uint32_t obj32 = 0x656C3F00;    // Неизвестная константа, используемая в TwinCAT для очистки PDO
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(pdoMappingIndex), 00, FALSE, sizeof(obj32), &obj32, EC_TIMEOUTRXM);
    return wc;
}

/**
 * @brief Функция добавления объекта в PDO разметку
 * @param slave
 * @param pdoMappingIndex - индекс PDO, в который добавляется объект
 * @param objectIndex - индекс добавляемого объекта
 * @param objectSubindex - сабиндекс добавляемого объекта
 * @param objectSize - размер объекта в байтах(типа данных)
 * @param position - позиция в PDO, в которую добавляется объект(PDO mapping subindex), должна быть > 0
 * @return
 */
int EthercatCOE::addObjectToPDOMapping(uint16_t slave, uint16_t pdoMappingIndex, uint16_t objectIndex,
                                   uint8_t objectSubindex, uint8_t objectSize, uint8_t position)
{
    if(position == 0)
        return -1;
    uint32_t obj32 = (objectIndex << 16) | (objectSubindex << 8) | (objectSize * 8);    // *8 так как размер передается в битах
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(pdoMappingIndex), position, FALSE, sizeof(obj32), &obj32, EC_TIMEOUTRXM);
    return wc;
}

/**
 * @brief Функция задания количество размапленных объектов в PDO разметке
 * @param slave
 * @param pdoMappingIndex
 * @param size - Число размапленных объектов
 * @return
 */
int EthercatCOE::setPDOMappingSize(uint16_t slave, uint16_t pdoMappingIndex, uint8_t size)
{
    uint32_t obj32 = 0x776F4000 + size; // 0x776F4000 - неизвестная константа, взятая из PDO разметки в TwinCAT
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(pdoMappingIndex), 0, FALSE, sizeof(obj32), &obj32, EC_TIMEOUTRXM);
    return wc;
}

/**
 * @brief Функция добавления размапленного PDO в Sync Manager
 * @param slave
 * @param pdoMappingIndex
 * @param smIndex
 * @param position
 * @return
 */
int EthercatCOE::addPDOMappingToSyncManager(uint16_t slave, uint16_t pdoMappingIndex, uint16_t smIndex, uint8_t position)
{
    uint16_t obj16 = static_cast<uint16_t>(pdoMappingIndex);
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(smIndex), position, FALSE, sizeof(obj16), &obj16, EC_TIMEOUTRXM);
    return wc;
}

/**
 * @brief Функция задания количества PDO разметок в Sync Manager
 * @param slave
 * @param smIndex
 * @param pdoNumber
 * @return
 */
int EthercatCOE::setSMPDONumber(uint16_t slave, uint16_t smIndex, uint8_t pdoNumber)
{
    int wc = ec_SDOwrite(slave, static_cast<uint16_t>(smIndex), 00, FALSE, sizeof(pdoNumber), &pdoNumber, EC_TIMEOUTRXM);
    return 0;
}


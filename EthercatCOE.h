//
// Created by Artem Gusev on 8/24/23.
//

#ifndef ETHERCATCOE_H
#define ETHERCATCOE_H

#include <stdint.h>
#include "ethercat.h"

/**
 * @brief Функции для работы с CanOpen Over Ethercat
 */
namespace EthercatCOE
{
    enum class SMIndex : uint16_t
    {
        SM_RPDO = 0x1C12,       ///< INPUT sync manager  ex. ControlWord
        SM_TPDO = 0x1C13        ///< OUTPUT sync manager ex. StatusWord
    };

    /**
     * @defgroup PDOMapping
     * @brief Функции разметки PDO в EtherCAT Slave
     * @details Порядок вызова
     * Важно, чтобы данные функции вызывались
     * Последовательность вызова следующая:
     * ОЧИСТКА
     * 1. - clearSM - очистка SyncManager для TPDO и RPDO
     * 2. - clearPDOMapping - очистка всех необходимых разметок PDO или тех, которые планируется использовать
     * РАЗМЕТКА PDO
     * 3. - addObjectToPDOMapping - добавление объекта в разметку PDO, делается для каждого регистра в RPDO и TPDO
     * 4. - setPDOMappingSize - задание числа размеченных объектов в PDO разметке, делается для каждой разметки
     * РЕГИСТРАЦИЯ PDO В SYNC MANAGER
     * 5. - addPDOMappingToSyncManager - добавление PDO разметки в SyncManager
     * 6. - setSMPDONumber - задание числа разметок PDO в SyncManager
     * 7. Готово
     *
     * @{
     */
    int clearSM(uint16_t slave, uint16_t smIndex);
    int clearPDOMapping(uint16_t slave, uint16_t pdoMappingIndex);
    int addObjectToPDOMapping(uint16_t slave, uint16_t pdoMappingIndex, uint16_t objectIndex,
                              uint8_t objectSubindex, uint8_t objectSize, uint8_t position);
    int setPDOMappingSize(uint16_t slave, uint16_t pdoMappingIndex, uint8_t size);
    int addPDOMappingToSyncManager(uint16_t slave, uint16_t pdoMappingIndex, uint16_t smIndex, uint8_t position);
    int setSMPDONumber(uint16_t slave, uint16_t smIndex, uint8_t pdoNumber);

    /**
     * @}
     */
}

#endif //ETHERCATCOE_H

/*
 * alarms.h
 *
 *  Created on: Sep 24, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_INCLUDE_ALARMS_H_
#define OS_INCLUDE_ALARMS_H_

#include <stdint.h>

#include "os.h"

typedef enum _alarm_id_e {
    ALARM_A,
    ALARM_B,
    ALARM_C,
    ALARM_D,
    ALARM_E,
    ALARM_F,
    ALARM_MAX,
} alarm_id_e;

/**
 * @brief Activa una alarma relativa. Cuando la alarma relativa expira, el OS activa la tarea especificada.
 *
 * La alarma puede ser de activaci�n �nica ("one-shot") o de activaci�n m�ltiple ("auto-reload").
 *
 * @param id Identificador �nico de la alarma que ser� activada.
 * @param ticks N�mero de ticks para que expire la alarma, relativos al tick actual.
 * @param task_to_activate Identificador �nico de la tarea que ser� activada cuando la alarma expire.
 * @param autoreload Si es distinto de 0, el OS recarga la cuenta de la alarma y la vuelve a iniciar.
 *
 * @return OS_OK si el OS activ� la alarma.
 *         OS_ERROR_INVALID_ARGUMENT si id, ticks o task_to_activate tienen valores inv�lidos.
 */
error_id_e os_alarm_set_rel(alarm_id_e id, uint16_t ticks, uint8_t task_to_activate, uint8_t autoreload);

/**
 * @brief Cancela una alarma, evitando que expire y active una tarea.
 *
 * @param id Identificador �nico de la alarma que ser� cancelada.
 *
 * @return OS_OK si el OS cancel� la alarma.
 *         OS_ERROR_INVALID_ARGUMENT si id no es alguno de ALARM_*.
 */
error_id_e os_alarm_cancel(alarm_id_e id);

#endif /* OS_INCLUDE_ALARMS_H_ */

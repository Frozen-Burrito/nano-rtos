/*
 * os.h
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_H_
#define OS_H_

#include <stdint.h>

#include <msp430.h>

#include "hal_timer.h"

#define SYSTICK_BASE_TA0_0  (1)
//#define SYSTICK_BASE_TA0_1  (1)
//#define SYSTICK_BASE_TA0_2  (1)
//#define SYSTICK_BASE_TA1_0  (1)
//#define SYSTICK_BASE_TA1_1  (1)
//#define SYSTICK_BASE_TA1_2  (1)

#define SYSTICK_PERIOD      (8000u)

typedef enum _error_id_e {
    OS_OK,                          /* Ning�n error, funci�n complet� con �xito. */
    OS_ERROR_INVALID_ARGUMENT,      /* Uno o varios de los argumentos de una funci�n son no-v�lidos. */
    OS_ERROR_MAX_CREATED_TASKS,     /* El OS alcanz� el l�mite de tareas creadas. No se puede crear una nueva tarea. */
    OS_ERROR_MAX_ACTIVE_TASKS,      /* El OS ya tiene el m�ximo n�mero de tareas activas. */
} error_id_e;

/**
 * @brief Inicializa el sistema operativo y ejecuta el scheduler por primera vez.
 *
 * @return OS_OK si inicializ� OS sin errores.
 */
error_id_e os_init(void);

/**
 * @brief Comienza la ejecuci�n del scheduler del OS. Esta funci�n no retorna.
 */
void scheduler_run(void);

#endif /* OS_H_ */

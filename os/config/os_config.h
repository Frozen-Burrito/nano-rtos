/*
 * os_config.h
 *
 *  Created on: Oct 13, 2024
 *      Author: Fernando Mendoza v.
 */
#ifndef OS_CONFIG_H_
#define OS_CONFIG_H_

/*
 * Número máximo de tareas que puede ejecutar el sistema operativo. Usar os_task_create cuando ya hay este
 * número de tareas ejecutándose resulta en un error.
 */
#define NUM_TASK_MAX    ((uint8_t) 4u)

/*
 * ID numérico máximo para una tarea.
 */
#define OS_TASK_ID_MAX  ((uint8_t) 0xFFu)

/*
 * Tamaño en palabras del stack dedicado para una tarea. Debe ser al menos de 16 palabras (32 bytes) para almacenar
 * los 16 registros en los cambios de contexto.
 */
#define TASK_STACK_SIZE ((uint8_t) 16u)

/* Timer de hardware usado como base para systick. */
#define SYSTICK_BASE_TA0_0  (1)
//#define SYSTICK_BASE_TA0_1  (1)
//#define SYSTICK_BASE_TA0_2  (1)
//#define SYSTICK_BASE_TA1_0  (1)
//#define SYSTICK_BASE_TA1_1  (1)
//#define SYSTICK_BASE_TA1_2  (1)

/* Número de cuentas del timer base por cada tick (segundos por tick / clock Hz). */
#define SYSTICK_PERIOD      (8000u)

/* Habilitar soporte para queues. */
#define OS_QUEUES           (1)

/* Número máximo de queues que pueden crearse. */
#ifdef OS_QUEUES
#define OS_QUEUE_COUNT_MAX  (2)
#endif /* OS_QUEUES */

/* Número máximo de elementos que puede contener una queue. */
#ifdef OS_QUEUES
#define OS_QUEUE_LEN_MAX    (5)
#endif /* OS_QUEUES */

#endif /* OS_CONFIG_H_ */

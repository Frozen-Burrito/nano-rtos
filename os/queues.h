/*
 * queues.h
 *
 *  Created on: Oct 13, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_QUEUES_H_
#define OS_QUEUES_H_

#include <stdint.h>

#include "os.h"
#include "os_config.h"

typedef uint8_t queue_id_t;

/**
 * @brief Registra una nueva queue en el sistema operativo.
 *
 * @param id Identificador �nico de la queue.
 * @param length N�mero de elementos almacenados en la queue. Debe ser menor que OS_QUEUE_LEN_MAX.
 * @param access Campo de bits con permisos de acceso para cada tarea. Una tarea tiene permiso si su bit est� en 1.
 *
 * @return OS_OK si el OS inicializ� la queue.
 *         OS_INVALID_ARGUMENT si uno de los par�metros es incorrecto.
 */
error_id_e os_queue_init(queue_id_t id, uint8_t length, uint8_t access);

/**
 * @brief Env�a un elemento al final de la queue.
 *
 * @param id Identificador �nico de la queue.
 * @param item Elemento que ser� enviado a la queue.
 * @param ticks_to_wait Ticks que espera la tarea si la queue est� llena. Asignar 0 hace que
 * el servicio regrese inmediatamente.
 *
 * @return OS_OK si envi� el elemento a la queue.
 *         OS_INVALID_ARGUMENT si uno de los par�metros es incorrecto.
 *         OS_QUEUE_FULL si la queue est� llena y no envi� el elemento. Tarea puede haber esperado ticks_to_wait.
 */
error_id_e os_queue_send(queue_id_t id, const void * item, tick_type_t ticks_to_wait);

/**
 * @brief Recibe un elemento del comienzo de la queue.
 *
 * @param id Identificador �nico de la queue.
 * @param item Espacio para recibir el elemento desde la queue.
 * @param ticks_to_wait Ticks que espera la tarea si la queue est� vac�a. Asignar 0 hace que
 * el servicio regrese inmediatamente.
 *
 * @return OS_OK si recibi� un elemento de la queue.
 *         OS_INVALID_ARGUMENT si uno de los par�metros es incorrecto.
 *         OS_QUEUE_FULL si la queue est� vac�a y no recibi� el elemento. Tarea puede haber esperado ticks_to_wait.
 */
error_id_e os_queue_receive(queue_id_t id, void * out_item, tick_type_t ticks_to_wait);

#endif /* OS_QUEUES_H_ */

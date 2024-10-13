/*
 * os_private.h
 *
 *  Created on: Oct 13, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_PRIVATEINCLUDE_OS_PRIVATE_H_
#define OS_PRIVATEINCLUDE_OS_PRIVATE_H_

#include <stdint.h>

#define NUM_TASK_MAX    ((uint8_t) 7u)
#define OS_TASK_ID_MAX  ((uint8_t) 0xFFu)
#define TASK_STACK_SIZE ((uint8_t) 16u)

// @brief Guarda todos los registros del contexto de la tarea actual, excepto R4.
// Recuperar R0 (PC) del stack.
// Mover puntero de stack de la tarea a R4.
// Almacenar valor original de R0 en stack de la tarea.
// Almacenar R1 -> R15 en stack de la tarea.
#define SAVE_CONTEXT() ({\
    __asm volatile (" MOV &current_task_stack, R4");\
    __asm volatile (" MOV 0(SP), 0(R4)");\
    __asm volatile (" ADD #2, SP");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV SP, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R2, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R3, 0(R4)");\
    __asm volatile (" SUB #4, R4");\
    __asm volatile (" MOV R5, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R6, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R7, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R8, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R9, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R10, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R11, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R12, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R13, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R14, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R15, 0(R4)");\
    __asm volatile (" MOV R4, &current_task_stack");\
})


// @brief Recupera todos los registros de la tarea, excepto R4.
// Mover puntero de stack de la tarea a R4.
// Recuperar valor original de R15 -> R5.
// Ignorar valor original de R4.
// Recuperar valor original de R3 -> R0 (PC).
#define RESTORE_CONTEXT() ({\
    __asm volatile (" MOV &current_task_stack, R4");\
    __asm volatile (" MOV @R4+, R15");\
    __asm volatile (" MOV @R4+, R14");\
    __asm volatile (" MOV @R4+, R13");\
    __asm volatile (" MOV @R4+, R12");\
    __asm volatile (" MOV @R4+, R11");\
    __asm volatile (" MOV @R4+, R10");\
    __asm volatile (" MOV @R4+, R9");\
    __asm volatile (" MOV @R4+, R8");\
    __asm volatile (" MOV @R4+, R7");\
    __asm volatile (" MOV @R4+, R6");\
    __asm volatile (" MOV @R4+, R5");\
    __asm volatile (" ADD #2, R4");\
    __asm volatile (" MOV @R4+, R3");\
    __asm volatile (" ADD #2, R4");\
    __asm volatile (" BIC #16, 0(R4)");\
    __asm volatile (" MOV @R4+, SR");\
    __asm volatile (" MOV @R4+, SP");\
    __asm volatile (" MOV R4, &current_task_stack");\
    __asm volatile (" EINT");\
    __asm volatile (" MOV @R4, PC");\
})

typedef enum _task_state_e {
    OS_TASK_STATE_EMPTY,
    OS_TASK_STATE_SUSPENDED,
    OS_TASK_STATE_WAIT,
    OS_TASK_STATE_READY,
    OS_TASK_STATE_RUN,
} task_state_e;

typedef uint8_t task_id_t;
typedef void (*task_function_t)(void);

typedef struct _task_t {
    task_state_e state;                 /* Estado actual de la tarea. */
    task_function_t task_function;      /* Dirección de inicio de la tarea. */
    uint8_t priority;                   /* Prioridad, en rango 0-255. */
    uint8_t autostart;                  /* Si es TRUE, la inicialización del sistema activa la tarea automáticamente. */
    uint16_t stack[TASK_STACK_SIZE];    /* Memoria para guardar el contexto de la tarea (R0-R15). */
} task_t;

extern volatile task_t tasks[];

extern volatile task_id_t current_task;
extern volatile uint8_t num_active_tasks;

volatile uint16_t * current_task_stack;

void scheduler_run(void);

#endif /* OS_PRIVATEINCLUDE_OS_PRIVATE_H_ */

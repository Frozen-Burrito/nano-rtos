# Nano-RTOS

Sistema operativo en tiempo real para MSP430.

## Características

El proyecto está basado en FreeRTOS y OSEK, combinando algunas de las características de ambos:

- Scheduler "preemptive".
- Tareas con distintas prioridades.
- Alarmas relativas (equivalentes a software timers de FreeRTOS).
- Queues.

## Comenzar

Para comenzar a trabajar en el proyecto, seguir estos pasos:

1. Clonar el repositorio usando `git clone https://github.com/Frozen-Burrito/nano-rtos.git`.
2. En CCS, navegar a **Project** > **Import CCS Projects**.
3. En el menú para importar el proyecto, checar **Select search-directory** y presionar en **Browse**.
4. Seleccionar la carpeta clonada con el repositorio.
5. Asegurar que **Copy projects into workspace** está desactivado, para poder contribuir los cambios al repositorio.
6. Presionar **Finish**.

## Estructura de archivos

El repositorio tiene la siguiente estructura de archivos:

```
├── os
│   ├── config
│   │   ├── os_config.h
│   ├── privateInclude
│   │   ├── os_private.h
│   ├── os.h
│   ├── os.c
│   ├── tasks.h
│   ├── tasks.c
│   ├── alarms.h
│   └── alarms.c
├── src
│   ├── hal
│   │   ├── ... (varios archivos de HAL)
├── main.c
├── README.md
└── .gitignore
```

Algunos archivos importantes son:

- `main.c`: El punto de entrada de la aplicación, contiene una demo de los servicios del sistema operativo.
- `os/`: La carpeta incluye el archivo principal del sistema operativo y otros archivos para los servicios, como `alarms.h`.
- `os/config/os_config.h`: Contiene la configuración del sistema operativo.
- `src/hal/`: Incluye archivos para funciones básicas del MSP430, como GPIO, timers y UART.

Por ejemplo, hace falta agregar los siguientes `#include` para usar el sistema operativo desde `main.c`:

```c
#include "os.h"
#include "os/tasks.h"
#include "os/<otro componente>.h"
```

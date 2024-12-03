#include "metric_handler.h"
#include "JSON_handler.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

status s = NOT_STARTED;

// IDs de procesos para el monitor y el wrapper
pid_t monitor_pid = -1;
pid_t wrapper_pid = -1;

/**
 * @brief Manejador de señales para terminar los procesos hijos.
 *
 * Este manejador se encarga de matar los procesos hijos cuando el proceso
 * padre recibe una señal de terminación.
 */
void signal_handler(int signo)
{
    (void)signo;
    // Matar los procesos hijos antes de que el padre termine
    if (monitor_pid > 0)
    {
        kill(monitor_pid, SIGKILL); // O SIGKILL, dependiendo de lo que necesites
    }
    if (wrapper_pid > 0)
    {
        kill(wrapper_pid, SIGKILL); // O SIGKILL
    }

    // Salir del proceso padre
    exit(EXIT_SUCCESS);
}

/**
 * @brief Convierte el estado a una cadena de caracteres.
 *
 * @return Cadena de caracteres que representa el estado.
 */
const char* status_to_string(status s)
{
    switch (s)
    {
    case RUN:
        return "RUN";
    case STOP:
        return "STOP";
    case NOT_STARTED:
        return "NOT_STARTED";
    default:
        return "UNKNOWN"; // Manejo de casos no definidos
    }
}

/**
 * @brief Inicia el proceso de monitorización.
 *
 * Crea procesos para ejecutar los binarios de monitorización y envoltura en
 * segundo plano. Maneja la señalización para permitir la terminación adecuada
 * de estos procesos.
 */
void comando_start_monitoring()
{
    if (s == RUN)
    {
        printf("Monitorización ya en ejecución.\n");
        return;
    }

    if (s == NOT_STARTED)
    {
        // Crear proceso para ejecutar el binario "monitor"
        monitor_pid = fork();
        if (monitor_pid == 0)
        {
            setsid();                                           // Hacer que el proceso hijo se ejecute en segundo plano
            execl("./bin/monitoring_project", "monitor", NULL); // Reemplaza con la ruta a tu binario monitor
            perror("Error al ejecutar monitor");
            exit(EXIT_FAILURE);
        }

        // Crear proceso para ejecutar el binario "wrapper"
        wrapper_pid = fork();
        if (wrapper_pid == 0)
        {
            setsid(); // Hacer que el proceso hijo se ejecute en segundo plano
            if (chdir("jsonconfig") != 0)
            {
                perror("Error al cambiar al directorio bin");
                exit(EXIT_FAILURE);
            }
            execl("../bin/wrapper", "wrapper", NULL); // Reemplaza con la ruta a tu binario wrapper
            perror("Error al ejecutar wrapper");
            exit(EXIT_FAILURE);
        }

        signal(SIGTERM, signal_handler);
        signal(SIGHUP, signal_handler);

        if (monitor_pid > 0 && wrapper_pid > 0)
        {
            s = RUN;
            printf("Monitorización iniciada. Procesos creados en segundo plano.\n");
        }
        else
        {
            perror("Error al crear procesos");
            exit(EXIT_FAILURE);
        }
    }
    else if (s == STOP)
    {
        // Reanudar procesos si están en STOP
        kill(monitor_pid, SIGCONT);
        kill(wrapper_pid, SIGCONT);
        s = RUN;
        printf("Monitorización reanudada.\n");
    }
}

/**
 * @brief Detiene el proceso de monitorización.
 *
 * Pausa los procesos de monitorización y envoltura si están en ejecución.
 */
void comando_stop_monitoring()
{
    if (s == RUN)
    {
        // Pausar procesos
        kill(monitor_pid, SIGSTOP);
        kill(wrapper_pid, SIGSTOP);
        s = STOP;
        printf("Monitorización detenida.\n");
    }
    else
    {
        printf("Monitorización ya está detenida o no ha comenzado.\n");
    }
}

/**
 * @brief Obtiene el estado actual de la monitorización.
 *
 * @return Estado de la monitorización.
 */
status status_monitor()
{
    return s;
}

#ifndef MONITOR_CONTROL_H
#define MONITOR_CONTROL_H

#include <signal.h> ///< Header para manejo de señales.
#include <stdio.h>  ///< Header para operaciones de entrada/salida estándar.
#include <stdlib.h> ///< Header para funciones de memoria dinámica y conversión.
#include <unistd.h> ///< Header para llamadas al sistema POSIX.

/**
 * @enum status
 * @brief Enum para representar los estados de la monitorización.
 */
typedef enum
{
    NOT_STARTED, ///< Estado inicial, monitorización no iniciada.
    RUN,         ///< Estado de ejecución, monitorización en curso.
    STOP         ///< Estado detenido, monitorización detenida.
} status;

/**
 * @brief Manejador de señales para la monitorización.
 *
 * Define el comportamiento del programa cuando recibe una señal específica.
 *
 * @param signo Número de la señal recibida.
 */
void signal_handler(int signo);

/**
 * @brief Convierte el estado de monitorización a una cadena de texto.
 *
 * Retorna una representación en texto del estado actual.
 *
 * @param current_status El estado actual de la monitorización.
 * @return const char* Cadena de caracteres que representa el estado.
 */
const char* status_to_string(status current_status);

/**
 * @brief Comando para iniciar la monitorización.
 *
 * Realiza las acciones necesarias para comenzar la monitorización.
 */
void comando_start_monitoring();

/**
 * @brief Comando para detener la monitorización.
 *
 * Realiza las acciones necesarias para detener la monitorización.
 */
void comando_stop_monitoring();

/**
 * @brief Obtiene el estado actual de la monitorización.
 *
 * @return status Estado actual de la monitorización.
 */
status status_monitor();

#endif // MONITOR_CONTROL_H

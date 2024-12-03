#ifndef COMMANDS_H
#define COMMANDS_H

#include <signal.h>    ///< Header para el manejo de señales.
#include <stdio.h>     ///< Header para operaciones de entrada/salida estándar.
#include <stdlib.h>    ///< Header para funciones de memoria dinámica, control de procesos y conversión.
#include <string.h>    ///< Header para manipulación de cadenas.
#include <sys/types.h> ///< Header para el tipo pid_t.
#include <sys/wait.h>  ///< Header para la función wait y macros relacionadas.
#include <unistd.h>    ///< Header para llamadas al sistema POSIX.

#define COMMAND_BUFFER_SIZE 1024 // Tamaño del buffer para leer comandos
#define FILE_PERMISSIONS 0666

/**
 * @brief Ejecuta un comando dado.
 *
 * Esta función procesa y ejecuta el comando especificado.
 *
 * @param command Cadena de caracteres que representa el comando a ejecutar.
 */
void execute_command(char* command);

/**
 * @brief Ejecuta un comando externo al proceso principal.
 *
 * Esta función maneja la ejecución de comandos externos y realiza las llamadas al sistema necesarias.
 *
 * @param command Cadena de caracteres que representa el comando a ejecutar externamente.
 */
void external_command(char* command);

#endif // COMMANDS_H

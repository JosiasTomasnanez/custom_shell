#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdio.h>  ///< Header para operaciones de entrada/salida estándar.
#include <stdlib.h> ///< Header para funciones de gestión de memoria y conversión.
#include <string.h> ///< Header para manipulación de cadenas.
#include <unistd.h> ///< Header para llamadas al sistema POSIX.

// Colores para el terminal. Ten en cuenta que pueden no funcionar en todos los terminales.
#define RED "\033[1;91m"      ///< Color rojo para el texto en terminal.
#define YELLOW "\033[1;93m"   ///< Color amarillo para el texto en terminal.
#define GREEN "\033[1;32m"    ///< Color verde para el texto en terminal.
#define BLUE "\033[1;34m"     ///< Color azul para el texto en terminal.
#define RESET "\033[38;5;87m" ///< Resetea el color del texto al valor por defecto.

#define HOSTNAME_SIZE 32 // Tamaño máximo para hostname
#define COMMAND_BUFFER_SIZE_2 64

/**
 * @brief Inicializa el terminal para la aplicación.
 *
 * Configura el entorno del terminal con las características necesarias
 * para la ejecución del programa.
 */
void init_terminal();

/**
 * @brief Obtiene el comando ingresado por el usuario.
 *
 * Lee la entrada del usuario desde el terminal y devuelve el comando como una cadena de caracteres.
 *
 * @return Cadena de caracteres que contiene el comando ingresado por el usuario.
 */
char* get_command();

#endif // COMMAND_LINE_H

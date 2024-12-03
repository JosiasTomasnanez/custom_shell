#include "command_processor.h"
#include "input_interface.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Programa principal para el procesamiento de comandos.
 *
 * Este programa inicializa la interfaz de terminal, ejecuta comandos
 * desde un archivo si se proporciona como argumento, y permite al usuario
 * introducir comandos en modo interactivo.
 *
 * @return int Código de salida del programa (0 si es exitoso).
 */
int main(int argc, char const* argv[])
{

    int unused = system("clear"); ///< Limpia la pantalla de la terminal al inicio.
    (void)unused;                 // O simplemente no usar "unused" en ninguna otra parte

    /* Initialize variables for command line prompt and print header. */
    init_terminal();

    /**
     * Si se proporciona un archivo batch como argumento, ejecuta los comandos dentro de él.
     */
    if (argc >= 2)
    {
        FILE* batchfile = fopen(argv[1], "r");
        if (batchfile == NULL)
        {
            fprintf(stderr, "Error opening file\n");
            exit(EXIT_FAILURE);
        }

        char* command = (char*)malloc(sizeof(char) * COMMAND_BUFFER_SIZE);
        while (fgets(command, COMMAND_BUFFER_SIZE, batchfile) != NULL)
            execute_command(command);
        fclose(batchfile);
        free(command);
        exit(EXIT_SUCCESS);
    }

    /**
     * En modo interactivo, toma un comando cada vez que el usuario lo ingresa y lo ejecuta.
     */
    while (1)
    {
        char* command = get_command(); // La funcion get_comand() termina el programa en caso de error
        execute_command(command);      // la funcion execute_command() termina el programa si recibe "quit"
        // por lo tanto si bien es un bucle infinito, sigue habiendo control del proceso ante circunstancias
    }

    return 0;
}

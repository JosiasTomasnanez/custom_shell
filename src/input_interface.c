#include "input_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* username;
char hostname[HOSTNAME_SIZE]; // Buffer para hostname
char* current_working_directory;

/**
 * @brief Imprime el encabezado del terminal con un arte ASCII y ayuda de comandos personalizados.
 *
 * Esta función muestra el arte ASCII en la consola junto con información
 * sobre los comandos personalizados disponibles en el programa, como la ayuda
 * para la configuración y la exposición de métricas.
 */
static void print_header()
{
    printf(RED "%-8s %-33s \n", "", "   _____ _          _ _ ");
    printf("%-8s %-33s \n", "", "  / ____| |        | | |");
    printf("%-8s %-33s \n", "", " | (___ | |__   ___| | |");
    printf("%-8s %-33s \n", "", "  \\___ \\| '_ \\ / _ \\ | |");
    printf("%-8s %-33s \n", "", "  ____) | | | |  __/ | |");
    printf("%-8s %-33s \n", "", " |_____/|_| |_|\\___|_|_|");
    printf(BLUE);
    printf("%-8s %-33s \n", "", "       / \\__            _        ");
    printf("%-8s %-33s \n", "", "      (    @\\_       | |       ");
    printf("%-8s %-33s \n", "", "       /         O      | |       ");
    printf("%-8s %-33s \n", "", "      /   (____/       | |      ");
    printf("%-8s %-33s \n", "", "     /__ /    U        ||     ");

    printf(YELLOW " A continuacion le daremos informacion acerca de los comandos perosnalizados:\n");
    printf(" config help: Nos brinda ayuda para la configuracion del archivo JSON.\n");
    printf(" metrics help: Nos brinda ayuda para exponer nuestras metricas.\n");

    printf("\n" RESET);
}

/**
 * @brief Imprime la línea del comando con el formato `user@hostname: <current path>$`.
 *
 * Esta función actualiza las variables necesarias para mostrar el nombre de
 * usuario, el hostname y el directorio de trabajo actual. Si el directorio
 * actual es el directorio home del usuario, imprime `~` en su lugar.
 */
static void print_line()
{
    /* Refresh variables */
    username = getenv("USER");                 // Obtiene el nombre de usuario
    gethostname(hostname, sizeof(hostname));   // Almacena el hostname
    current_working_directory = getenv("PWD"); // Obtiene el directorio de trabajo actual

    printf("╭─");
    printf(GREEN "%s@%s" RESET ":", username, hostname);

    /* If cwd is /home/user print "~" instead of "cwd" */
    if (strcmp(current_working_directory, getenv("HOME")) == 0)
        printf(RESET "~/\n");
    else
        printf(RESET "%s\n", current_working_directory);

    printf("╰─$ ");
}

/**
 * @brief Inicializa las variables necesarias para imprimir en el terminal.
 *
 * Esta función obtiene el nombre de usuario, el hostname y el directorio
 * de trabajo actual al iniciar el terminal. Además, imprime el encabezado
 * del terminal.
 */
void init_terminal()
{
    /* No es necesario malloc para username */
    username = getenv("USER");

    /* hostname ya está preasignado */
    gethostname(hostname, sizeof(hostname));

    /* No es necesario malloc para current_working_directory */
    current_working_directory = getenv("PWD");

    print_header();
}

/**
 * @brief Obtiene el comando ingresado por el usuario.
 *
 * Esta función imprime la línea del comando y espera a que el usuario ingrese
 * un comando. El comando se lee y se devuelve como una cadena.
 *
 * @return char* La cadena que contiene el comando ingresado por el usuario.
 */
char* get_command()
{
    print_line();

    char* command = (char*)malloc(sizeof(char) * COMMAND_BUFFER_SIZE_2);
    if (fgets(command, COMMAND_BUFFER_SIZE_2, stdin) == NULL)
        exit(EXIT_FAILURE);

    return command;
}

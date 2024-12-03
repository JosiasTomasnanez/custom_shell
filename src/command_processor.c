#include "command_processor.h"
#include "JSON_handler.h"
#include "metric_handler.h"
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKGROUND 4
#define BUFFER_SIZE 1024

int flags[BACKGROUND] = {0};
int job_bg[BACKGROUND] = {0};
int pid_bg[BACKGROUND] = {0};
int job_ID = 0;
int background_flag = 0;
volatile bool realtime = false;
pid_t foreground_pid = -1;
Config* conf = NULL; // Inicializado a NULL

/**
 * @enum command_type
 * @brief Enumeración de los tipos de comandos principales soportados.
 *
 * Define los tipos de comandos reconocidos por la función `execute_command`.
 */
typedef enum
{
    CMD_QUIT,     /**< Comando para salir del programa. */
    CMD_CD,       /**< Comando para cambiar el directorio actual. */
    CMD_CLR,      /**< Comando para limpiar la pantalla. */
    CMD_ECHO,     /**< Comando para imprimir texto o valores. */
    CMD_EXTERNAL, /**< Comando externo no reconocido internamente. */
    CMD_SCAN      /**< Comando para buscar configuraciones recursivamente. */
} command_type;

/**
 * @brief Mapea un comando base a su tipo correspondiente.
 *
 * Esta función analiza el comando base proporcionado como cadena y lo mapea
 * al tipo definido en la enumeración @ref command_type.
 *
 * @return El tipo de comando correspondiente de la enumeración @ref command_type.
 *         Si el comando no coincide con ninguno de los tipos predefinidos,
 *         devuelve CMD_EXTERNAL.
 */
command_type parse_command(const char* cmd)
{
    if (strcmp(cmd, "quit") == 0)
        return CMD_QUIT;
    if (strcmp(cmd, "cd") == 0)
        return CMD_CD;
    if (strcmp(cmd, "clr") == 0)
        return CMD_CLR;
    if (strcmp(cmd, "echo") == 0)
        return CMD_ECHO;
    if (strcmp(cmd, "scan") == 0)
        return CMD_SCAN;
    return CMD_EXTERNAL; // Por defecto, cualquier otro comando se trata como externo
}

/**
 * @brief Obtiene la configuración desde un archivo JSON.
 *
 * Si la configuración no ha sido inicializada, se lee desde el archivo "jsonconfig/config.json".
 *
 * @return Un puntero a la estructura Config con la configuración leída, o NULL si ocurre un error.
 */
Config* get_config()
{
    // Verifica si conf es NULL y lo inicializa si es necesario
    if (conf == NULL)
    {
        conf = read_config("jsonconfig/config.json");
        if (conf == NULL)
        {
            fprintf(stderr, "Error al leer la configuración.\n");
        }
    }
    return conf; // Devuelve la configuración
}

/**
 * @brief Determina si el comando debe ejecutarse en segundo plano.
 *
 * Si el comando contiene '&' al final, se establece el indicador de segundo plano.
 *
 */
static void get_flag(char* command)
{
    if (strrchr(command, '&'))
    {
        background_flag = 1;
        command[strcspn(command, "&")] = '\0';
    }
}

/**
 * @brief Manejador de la señal SIGINT (Ctrl-C).
 *
 * Si existe un proceso en primer plano, envía SIGINT a ese proceso. Si el modo en tiempo real está activo, lo
 * desactiva.
 *
 */
void ctrl_c_Handler(int sig)
{
    (void)sig;
    if (foreground_pid > 0)
    {
        kill(foreground_pid, SIGINT); // Envía SIGINT solo al proceso en primer plano
        foreground_pid = -1;          // Reinicia foreground_pid después de terminar el proceso
    }
    else if (realtime)
    {
        realtime = false;
    }
}

/**
 * @brief Manejador de la señal SIGTSTP (Ctrl-Z).
 *
 * Envía la señal SIGTSTP al proceso en primer plano.
 *
 */
void ctrl_z_Handler(int sig)
{
    (void)sig;

    if (foreground_pid > 0)
    {
        // Enviar la señal al proceso en primer plano
        kill(foreground_pid, SIGTSTP);
        foreground_pid = -1;
    }
}

/**
 * @brief Manejador de la señal SIGQUIT (Ctrl-\).
 *
 * Ignora la señal SIGQUIT.
 *
 */
void sigquit_Handler(int sig)
{
    (void)sig;
    // ignora la señal
}

/**
 * @brief Manejador para la señal SIGCHLD.
 *
 * Verifica si alguno de los trabajos en segundo plano ha finalizado y actualiza sus indicadores.
 */
static void CHLD_Handler()
{
    pid_t pid;
    for (int i = 0; i < BACKGROUND; i++)
    {
        if ((pid = waitpid(-1, NULL, WNOHANG)) == pid_bg[i] && pid_bg[i] != 0)
        {
            flags[i] = 1;
            break;
        }
    }
}

/**
 * @brief Obtiene el directorio de trabajo actual.
 *
 * Esta función asigna memoria para un buffer y utiliza la función `getcwd` para obtener el directorio de trabajo
 * actual en el que se encuentra el programa. Si ocurre algún error durante la asignación de memoria o la obtención
 * del directorio, se imprime un mensaje de error y se retorna `NULL`.
 *
 * @return Un puntero al buffer que contiene la ruta del directorio actual, o `NULL` si ocurrió un error.
 */
char* get_cwd()
{
    // Asignamos un buffer del tamaño definido
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    if (buffer == NULL)
    {
        perror("Error al asignar memoria");
        return NULL;
    }

    // Obtener el directorio actual
    if (getcwd(buffer, BUFFER_SIZE) == NULL)
    {
        perror("Error al obtener el directorio actual");
        free(buffer);
        return NULL;
    }

    return buffer;
}
/**
 * @brief Ejecuta un comando externo con soporte para redirección y tuberías.
 *
 * Esta función maneja la ejecución de comandos con redirección de entrada/salida, ejecución en segundo plano y
 * tuberías.
 *
 */
void external_command(char* command)
{

    get_flag(command);

    int pipe_flag = 0;
    char* pipe_token = strchr(command, '|');
    if (pipe_token != NULL)
    {
        pipe_flag = 1;
        *pipe_token = '\0'; // Split command into two parts at the pipe symbol
    }

    int input_fd = -1;
    int output_fd = -1;

    // Handle input redirection
    char* input_token = strchr(command, '<');
    if (input_token != NULL)
    {
        *input_token = '\0';
        input_fd = open(input_token + 1, O_RDONLY);
        if (input_fd == -1)
        {
            perror("Error opening input file");
            exit(EXIT_FAILURE);
        }
    }

    // Handle output redirection
    char* output_token = strchr(command, '>');
    if (output_token != NULL)
    {
        *output_token = '\0';
        char* output_file_path = output_token + 1;

        char full_output_path[COMMAND_BUFFER_SIZE];
        if (output_file_path[0] == '/')
        {
            // Absolute path
            snprintf(full_output_path, sizeof(full_output_path), "%s", output_file_path);
        }
        else
        {
            // Relative path, construct full path based on current working directory
            char cwd[COMMAND_BUFFER_SIZE];
            if (getcwd(cwd, sizeof(cwd)) == NULL)
            {
                perror("getcwd");
                exit(EXIT_FAILURE);
            }

            // Ensure that the destination buffer has enough space for both strings
            if ((size_t)snprintf(full_output_path, sizeof(full_output_path), "%s/%s", cwd, output_file_path) >=
                sizeof(full_output_path))
            {
                fprintf(stderr, "Error: Output path is too long\n");
                exit(EXIT_FAILURE);
            }
        }

        output_fd = open(full_output_path, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMISSIONS);
        if (output_fd == -1)
        {
            perror("Error opening output file");
            exit(EXIT_FAILURE);
        }
    }
    pid_t fork_ID = fork();

    switch (fork_ID)
    {
    case -1:
        perror("Fork failed.\n");
        exit(EXIT_FAILURE);
        break;

    case 0:

        // Set up input redirection

        if (input_fd != -1)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        // Set up output redirection
        if (output_fd != -1)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        // Handle pipe
        if (pipe_flag)
        {
            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pipe_pid = fork();

            if (pipe_pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if (pipe_pid == 0) // Child process for the first command
            {
                close(pipefd[0]); // Close unused read end

                // Redirect stdout to the pipe
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]); // Close the write end of the pipe

                execl("/bin/sh", "sh", "-c", command, NULL);

                perror("execl");
                exit(EXIT_FAILURE);
            }
            else // Parent process
            {
                close(pipefd[1]); // Close the write end of the pipe

                // Redirect stdin to read from the pipe
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]); // Close the read end of the pipe

                execl("/bin/sh", "sh", "-c", pipe_token + 1, NULL);

                perror("execl");
                exit(EXIT_FAILURE);
            }
        }
        else // No pipe
        {
            execl("/bin/sh", "sh", "-c", command, NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
        break;

    default:
        foreground_pid = fork_ID; // Asigna el proceso en primer plano

        if (background_flag == 0)
            waitpid(fork_ID, 0, 0); // Wait for the child process to finish in the foreground
        else
        {
            job_ID++;
            printf("[%d] %d\n", job_ID, fork_ID);

            for (int i = 0; i < BACKGROUND; i++)
            {
                if (pid_bg[i] == 0)
                {
                    pid_bg[i] = fork_ID;
                    job_bg[i] = job_ID;
                    break;
                }
            }

            // Non-blocking wait for background processes
            waitpid(-1, NULL, WNOHANG);
        }

        // Close file descriptors in the parent process
        if (input_fd != -1)
            close(input_fd);
        if (output_fd != -1)
            close(output_fd);

        for (int i = 0; i < BACKGROUND; i++)
        {
            if (flags[i] == 1)
            {
                printf("[%d]+ Done \n", job_bg[i]);
                flags[i] = 0;
                pid_bg[i] = 0;
            }
        }
        background_flag = 0;

        break;
    }
}

/**
 * @brief Ejecuta el comando `echo`, permitiendo expandir variables de entorno.
 *
 */
static void echo(char* comment)
{
    /* comments contains the full length command i.e "echo hello" so we grab only the arg. */
    comment = strtok(NULL, " ");

    if (comment == NULL)
    {
        printf("\n");
    }
    else if (comment[0] == '$')
    {
        char* env_value = getenv(comment + 1);
        if (env_value != NULL)
        {
            printf("%s\n", env_value);
        }
        else
        {
            fprintf(stderr, "Error: La variable de entorno '%s' no está definida.\n", comment + 1);
        }
    }
    else
    {
        printf("%s\n", comment);
    }
}

/**
 * @brief Cambia el directorio de trabajo actual.
 *
 * La función interpreta el argumento después del comando `cd` como
 * el nuevo directorio de trabajo. Si no se proporciona ningún argumento,
 * cambia al directorio de inicio del usuario.
 *
 * @return 0 si el cambio de directorio es exitoso, -1 si ocurre un error.
 */
static void cd(char* directory)
{

    directory = strtok(NULL, " ");

    if (directory == NULL)
        directory = getenv("HOME");

    if (!strcmp(directory, "-"))
        directory = getenv("OLDPWD");

    if (chdir(directory) == -1)
        printf("cd: %s: No such file or directory\n", directory);

    char* cwd = malloc(sizeof(char) * COMMAND_BUFFER_SIZE);
    if (getcwd(cwd, COMMAND_BUFFER_SIZE) == NULL)
    {
        perror("Error al obtener el directorio actual");
        return;
    }
    setenv("OLDPWD", getenv("PWD"), 1);
    setenv("PWD", cwd, 1);
    free(cwd);
}

bool handle_start_monitor(void)
{
    comando_start_monitoring();
    return true;
}

bool handle_stop_monitor(void)
{
    comando_stop_monitoring();
    return true;
}

bool handle_status_monitor(void)
{
    status current_status = status_monitor();
    printf("Estado actual del monitor: %s\n", status_to_string(current_status));
    return true;
}

bool handle_expose_metrics(void)
{
    status current_status = status_monitor();
    if (current_status != RUN)
    {
        printf("El monitor no está corriendo, inserte el comando: start_monitor \n");
        return true;
    }
    int fd = open("/tmp/metrics_pipe", O_RDONLY);
    if (fd == -1)
    {
        perror("Error al abrir la FIFO");
        return true;
    }
    char buffer[COMMAND_BUFFER_SIZE];
    char acumulador[BUFFER_SIZE * 4] = ""; // Buffer acumulativo
    size_t acumulado_len = 0;
    bool found_end_marker = false;
    // Bucle para leer de la FIFO hasta encontrar el marcador "<END_OF_METRICS>"
    while (!found_end_marker)
    {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0)
        {
            perror("Error al leer de la FIFO");
            close(fd);
            return true;
        }
        else if (bytes_read == 0)
        {
            // Si no se leyeron bytes, la FIFO está vacía
            continue;
        }
        buffer[bytes_read] = '\0'; // Aseguramos que el buffer está terminado en '\0'
        // Acumular los datos leídos
        strncat(acumulador, buffer, sizeof(acumulador) - acumulado_len - 1);
        acumulado_len += bytes_read;
        // Buscar el marcador de fin de las métricas
        char* delimitador_pos = strstr(acumulador, "<END_OF_METRICS>");
        if (delimitador_pos)
        {
            *delimitador_pos = '\0'; // Termina el bloque de métricas en el delimitador
            found_end_marker = true;
        }
    }
    printf("\n%s\n", acumulador);
    close(fd);
    return true;
}

bool handle_expose_metrics_realtime(void)
{
    status current_status = status_monitor();
    if (current_status != RUN)
    {
        printf("El monitor no está corriendo, inserte el comando: start_monitor \n");
        return true;
    }
    realtime = true;

    int fd = open("/tmp/metrics_pipe", O_RDONLY);
    if (fd == -1)
    {
        perror("Error al abrir la FIFO");
        return false;
    }

    char buffer[COMMAND_BUFFER_SIZE];
    char acumulador[BUFFER_SIZE * 4] = "";
    size_t acumulado_len = 0;

    while (realtime)
    {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0)
        {
            perror("Error al leer de la FIFO");
            close(fd);
            return false;
        }
        else if (bytes_read == 0)
        {
            continue;
        }

        buffer[bytes_read] = '\0';

        strncat(acumulador, buffer, sizeof(acumulador) - acumulado_len - 1);
        acumulado_len += bytes_read;

        char* delimitador_pos = strstr(acumulador, "<END_OF_METRICS>");
        if (delimitador_pos)
        {
            int unused = system("clear");
            (void)unused;
            usleep(20000);
            *delimitador_pos = '\0';
            printf("\n%s\n", acumulador);
            printf("\n------------------------------------------------------------------\n control c para cerrar");
            memset(acumulador, 0, sizeof(acumulador));
            acumulado_len = 0;
            acumulador[0] = '\0';
        }
    }

    close(fd);
    realtime = false;
    return true;
}

bool handle_metrics_help(void)
{
    printf("Comandos disponibles:\n");
    printf(" - expose metrics: Lee métricas una sola vez.\n");
    printf(" - expose metrics realtime: Lee métricas en tiempo real.\n");
    printf(" - metrics help: Muestra esta ayuda sobre los comandos de métricas.\n");
    printf(" - start_monitor: Inicia o reanuda el monitor que expone las metricas. \n");
    printf(" - stop_monitor: Suspende el monitor.\n");
    printf(" - status_monitor: Muestra el estado del monitor. \n");
    return true;
}
bool monitor_comand(char* comand)
{
    struct command
    {
        const char* name;
        bool (*handler)(void);
    };

    struct command commands[] = {{"start_monitor", handle_start_monitor},
                                 {"stop_monitor", handle_stop_monitor},
                                 {"status_monitor", handle_status_monitor},
                                 {"expose metrics", handle_expose_metrics},
                                 {"expose metrics realtime", handle_expose_metrics_realtime},
                                 {"metrics help", handle_metrics_help}};

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strcmp(comand, commands[i].name) == 0)
        {
            return commands[i].handler();
        }
    }

    return false; // Si el comando no es reconocido
}

/**
 * @brief Verifica si un archivo es un archivo de configuración.
 *
 * Esta función recibe un nombre de archivo y verifica si su extensión es ".config" o ".json".
 * Si el archivo tiene una de estas extensiones, se considera un archivo de configuración.
 *
 * @return 1 si es un archivo de configuración, 0 si no lo es.
 */
int is_config_file(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    if (ext != NULL)
    {
        if (strcmp(ext, ".config") == 0 || strcmp(ext, ".json") == 0)
        {
            return 1; // Es un archivo de configuración
        }
    }
    return 0; // No es un archivo de configuración
}

/**
 * @brief Lee y muestra el contenido de un archivo de configuración.
 *
 * Esta función abre el archivo especificado en modo lectura y muestra su contenido línea por línea en la consola.
 * Si el archivo no se puede abrir, se imprime un mensaje de error.
 */
void read_config_file(const char* file_path)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL)
    {
        perror("No se pudo abrir el archivo");
        return;
    }
    char line[BUFFER_SIZE];
    printf("\033[1;31mArchivo de configuracion encontrado: %s\ncontenido de %s:\033[38;5;87m\n", file_path, file_path);
    while (fgets(line, sizeof(line), file) != NULL)
    {
        printf("%s", line);
    }
    printf("\n");
    fclose(file);
}

/**
 * @brief Explora recursivamente el directorio actual y subdirectorios en busca de archivos de configuración.
 *
 * Esta función recursiva explora el directorio en el que se encuentra, buscando archivos de configuración con las
 * extensiones ".config" y ".json". Cuando se encuentra un archivo de este tipo, se lee y muestra su contenido.
 * Además, la función realiza llamadas recursivas en los subdirectorios.
 */
void explore_recursive_config()
{
    char* cwd = get_cwd(); // Usar la función get_cwd
    if (cwd == NULL)
    {
        return; // Salir si no se pudo obtener el directorio actual
    }
    struct dirent* entry;
    struct stat statbuf;
    DIR* dp = opendir(cwd);

    if (dp == NULL)
    {
        perror("No se pudo abrir el directorio");
        free(cwd);
        return;
    }

    // Recorrer los elementos del directorio
    while ((entry = readdir(dp)) != NULL)
    {
        char path[BUFFER_SIZE];
        snprintf(path, sizeof(path), "%s/%s", cwd, entry->d_name);

        // Obtener información sobre el archivo
        if (stat(path, &statbuf) == -1)
        {
            perror("No se pudo obtener información sobre el archivo");
            continue;
        }

        // Si es un directorio, hacer llamada recursiva
        if (S_ISDIR(statbuf.st_mode))
        {
            // Ignorar los directorios "." y ".."
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                chdir(path);                // Cambiar al nuevo directorio
                explore_recursive_config(); // Llamada recursiva
                chdir(cwd);                 // Regresar al directorio original
            }
        }
        else if (S_ISREG(statbuf.st_mode) && is_config_file(entry->d_name))
        {
            // Si es un archivo .config o .json, leerlo
            read_config_file(path);
        }
    }

    closedir(dp);
    free(cwd); // Liberar la memoria asignada para cwd
}

/**
 * @brief Ejecuta un comando ingresado por el usuario.
 *
 * Esta función configura los manejadores de señales y limpia el
 * comando ingresado. Separa el comando de sus argumentos y llama
 * a la función correspondiente para ejecutarlo, ya sea un comando interno
 * o externo.
 *
 */
void execute_command(char* command)
{
    /* Configuración de señales */
    signal(SIGINT, ctrl_c_Handler);
    signal(SIGCHLD, CHLD_Handler);
    signal(SIGTSTP, ctrl_z_Handler);
    signal(SIGQUIT, sigquit_Handler);
    /* Elimina el salto de línea al final del comando */
    command[strcspn(command, "\n")] = '\0';
    /* Comprueba si es un comando JSON */
    if (JSON_command(command, get_config()))
    {
        return; // Si es un comando JSON y se ejecutó, termina la función
    }
    /* Comprueba si es un comando monitor */
    if (monitor_comand(command))
    {
        return; // Si es un comando monitor y se ejecutó, termina la función
    }
    /* Procesa comandos principales */
    char* cwd;
    char* cmd = strdup(command);
    strtok(cmd, " "); // Extrae el comando base (antes de los argumentos)

    if (cmd != NULL)
    {
        command_type cmd_type = parse_command(cmd);
        free(cmd); // Liberamos memoria después de analizar

        switch (cmd_type)
        {
        case CMD_QUIT:
            signal_handler(SIGTERM);
            exit(EXIT_SUCCESS);
            break;
        case CMD_CD:
            strtok(command, " "); // Extrae y descarta la palabra "cd"
            cd(NULL);             // Llama a `cd` con la siguiente parte del comando
            break;
        case CMD_CLR:
            printf("\033[2J\033[1;1H");
            fflush(stdout);
            break;
        case CMD_ECHO:
            if (strchr(command, '$') != NULL)
                echo(command); // Usa la función personalizada
            else
                system(command); // Usa el `echo` del sistema
            break;
        case CMD_SCAN:
            cwd = get_cwd();
            printf("Explorando el directorio: %s en busca de archivos '.config' o '.json'\n", cwd);
            explore_recursive_config();
            break;
        case CMD_EXTERNAL:
            external_command(command); // Maneja cualquier otro comando como externo
            break;
        }
    }
}

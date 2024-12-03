#include "JSON_handler.h"
#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_METRICAS 50
#define MAX_METRICA_LEN 256 // Longitud máxima de cada métrica

char* filen = NULL;
char metricas[NUM_METRICAS][MAX_METRICA_LEN];

/**
 * @enum json_command_type
 * @brief Enumeración de los tipos de comandos JSON soportados.
 *
 * Define los tipos de comandos reconocidos por la función JSON_command.
 */
typedef enum
{
    CMD_SET_INTERVAL,  /**< Comando para establecer el intervalo de muestreo. */
    CMD_PRINT_CONFIG,  /**< Comando para imprimir la configuración actual. */
    CMD_ADD_METRIC,    /**< Comando para agregar métricas. */
    CMD_GET_LIST,      /**< Comando para obtener una lista de configuraciones. */
    CMD_HELP,          /**< Comando para mostrar la ayuda. */
    CMD_REMOVE_METRIC, /**< Comando para eliminar métricas. */
    CMD_INVALID        /**< Comando inválido o no reconocido. */
} json_command_type;

/**
 * @brief Mapea un comando JSON a su tipo correspondiente.
 *
 * Esta función analiza el comando JSON proporcionado y determina a qué tipo
 * corresponde según las palabras clave definidas.
 *
 * @return El tipo de comando correspondiente de la enumeración @ref json_command_type.
 *         Si el comando no es reconocido, devuelve CMD_INVALID.
 */
json_command_type parse_json_command(const char* command)
{
    if (strncmp(command, "config set intervalo_muestreo ", 30) == 0)
        return CMD_SET_INTERVAL;
    if (strcmp(command, "config print") == 0)
        return CMD_PRINT_CONFIG;
    if (strncmp(command, "config add metric ", 17) == 0)
        return CMD_ADD_METRIC;
    if (strcmp(command, "config get list") == 0)
        return CMD_GET_LIST;
    if (strcmp(command, "config help") == 0)
        return CMD_HELP;
    if (strncmp(command, "config rm metric ", 16) == 0)
        return CMD_REMOVE_METRIC;

    return CMD_INVALID; // Si no coincide con ningún comando
}
/**
 * @brief Genera una lista de métricas predefinidas y las copia al arreglo global `metricas`.
 *
 * Esta función inicializa el arreglo global `metricas` con un conjunto de métricas predefinidas.
 * Asegura que cada métrica copiada esté correctamente terminada con un carácter nulo y respeta
 * el tamaño máximo de cada métrica definido por `MAX_METRICA_LEN`.
 *
 * Las métricas predefinidas incluyen:
 * - "cpu_usage"
 * - "memory_usage"
 * - "disk_usage"
 * - "network_usage"
 * - "bandwidth_usage"
 * - "major_page_faults"
 * - "minor_page_faults"
 * - "change_context"
 * - "total_processes"
 * - "memory_total"
 * - "memory_available"
 * - "memory_usage_2"
 *
 * @note Si el número de métricas predefinidas excede el valor de `NUM_METRICAS`,
 *       solo las primeras `NUM_METRICAS` serán copiadas.
 */
void list_generator()
{
    // Lista de métricas predefinidas
    const char* METRICAS_PREDEFINIDAS[] = {"cpu_usage",         "memory_usage",     "disk_usage",
                                           "network_usage",     "bandwidth_usage",  "major_page_faults",
                                           "minor_page_faults", "change_context",   "total_processes",
                                           "memory_total",      "memory_available", "memory_usage_2"};

    size_t num_predefinidas = sizeof(METRICAS_PREDEFINIDAS) / sizeof(METRICAS_PREDEFINIDAS[0]);

    // Copiar las métricas predefinidas al arreglo `metricas`
    for (size_t i = 0; i < num_predefinidas && i < NUM_METRICAS; i++)
    {
        strncpy(metricas[i], METRICAS_PREDEFINIDAS[i], MAX_METRICA_LEN - 1);
        metricas[i][MAX_METRICA_LEN - 1] = '\0'; // Asegura terminación nula
    }
}

/**
 * @brief Lee la configuración desde un archivo JSON.
 *
 * @return Config* Puntero a la estructura de configuración o NULL si ocurre un error.
 */
Config* read_config(const char* filename)
{
    list_generator();
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("No se pudo abrir config.json");
        return NULL;
    }
    filen = strdup(filename); // Esto crea una copia en memoria dinámica de `filename`

    // Leer el archivo completo en memoria
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    size_t read_count = fread(data, 1, length, file);
    (void)read_count;
    fclose(file);

    // Parsear el JSON
    cJSON* json = cJSON_Parse(data);
    if (json == NULL)
    {
        fprintf(stderr, "Error al parsear config.json\n");
        free(data);
        return NULL;
    }

    // Crear la estructura de configuración
    Config* config = (Config*)malloc(sizeof(Config));
    config->intervalo_muestreo = cJSON_GetObjectItem(json, "intervalo_muestreo")->valueint;

    // Obtener la lista de métricas
    cJSON* metricas_json = cJSON_GetObjectItem(json, "metricas");
    if (metricas_json == NULL || !cJSON_IsArray(metricas_json))
    {
        fprintf(stderr, "No se encontró la lista de métricas o no es un arreglo.\n");
        cJSON_Delete(json);
        free(data);
        return NULL;
    }
    int num_metricas = cJSON_GetArraySize(metricas_json);
    config->metricas = (char**)malloc(num_metricas * sizeof(char*));
    config->num_metricas = num_metricas;

    for (int i = 0; i < num_metricas; i++)
    {
        config->metricas[i] = strdup(cJSON_GetArrayItem(metricas_json, i)->valuestring);
    }

    cJSON_Delete(json);
    free(data);

    return config;
}

/**
 * @brief Imprime la lista de métricas disponibles.
 */
void get_list()
{
    for (int i = 0; i < NUM_METRICAS; i++)
    {
        if (strlen(metricas[i]) > 0)
        { // Solo imprime si la métrica no está vacía
            printf("Métrica %d: %s\n", i + 1, metricas[i]);
        }
    }
    printf("\n");
}

/**
 * @brief Actualiza la configuración en el archivo JSON.
 */
void update_config(const char* filename, Config* config)
{
    // Crear el objeto JSON
    cJSON* json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "intervalo_muestreo", config->intervalo_muestreo);

    cJSON* metricas_json = cJSON_CreateArray();
    for (int i = 0; i < config->num_metricas; i++)
    {
        cJSON_AddItemToArray(metricas_json, cJSON_CreateString(config->metricas[i]));
    }
    cJSON_AddItemToObject(json, "metricas", metricas_json);

    // Guardar el JSON en el archivo
    FILE* file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("No se pudo abrir config.json para escritura");
        cJSON_Delete(json);
        return;
    }

    char* string = cJSON_Print(json);
    fprintf(file, "%s\n", string);
    fclose(file);

    free(string);
    cJSON_Delete(json);
}

/**
 * @brief Establece el intervalo de muestreo.
 */
void command_set_interval(int new_interval, Config* config)
{
    config->intervalo_muestreo = new_interval;
    update_config(filen, config);
    printf("Intervalo de muestreo actualizado a %d\n", new_interval);
}

/**
 * @brief Agrega una métrica a la lista de métricas.
 */
void command_add_metric(char* new_metric, Config* config)
{
    config->metricas = realloc(config->metricas, (config->num_metricas + 1) * sizeof(char*));
    config->metricas[config->num_metricas] = strdup(new_metric);
    config->num_metricas++;

    update_config(filen, config);
    printf("Métrica '%s' añadida.\n", new_metric);
}

/**
 * @brief Elimina una métrica de la lista de métricas.
 *
 * @return true si la métrica fue encontrada y eliminada, false en caso contrario.
 */
void command_remove_metric(char* metric_to_remove, Config* config)
{
    for (int i = 0; i < config->num_metricas; i++)
    {
        if (strcmp(config->metricas[i], metric_to_remove) == 0)
        {
            free(config->metricas[i]);
            for (int j = i; j < config->num_metricas - 1; j++)
            {
                config->metricas[j] = config->metricas[j + 1];
            }
            config->num_metricas--;
            config->metricas = realloc(config->metricas, config->num_metricas * sizeof(char*));
            update_config(filen, config);
            printf("Métrica '%s' eliminada.\n", metric_to_remove);
            return; // Termina la función una vez que se elimina la métrica
        }
    }
    printf("Métrica '%s' no encontrada.\n", metric_to_remove);
}

/**
 * @brief Agrega múltiples métricas a la lista de configuración.
 */
void command_add_multiple_metrics(char** new_metrics, int num_new_metrics, Config* config)
{
    for (int i = 0; i < num_new_metrics; i++)
    {
        command_add_metric(new_metrics[i], config);
    }
    printf("%d métricas añadidas.\n", num_new_metrics);
}

/**
 * @brief Elimina múltiples métricas de la lista de configuración.
 */
void command_remove_multiple_metrics(char** metrics_to_remove, int num_metrics_to_remove, Config* config)
{
    int removed_count = 0;
    for (int i = 0; i < num_metrics_to_remove; i++)
    {
        command_remove_metric(metrics_to_remove[i], config);
        removed_count++;
    }

    printf("%d métricas eliminadas.\n", removed_count);
}

/**
 * @brief Imprime la ayuda sobre los comandos de configuración.
 */
void config_help()
{
    printf("Comandos de configuración disponibles:\n");
    printf("1. config set intervalo_muestreo <valor>\n");
    printf("   - Establece el intervalo de muestreo en segundos.\n");
    printf("2. config add metric <nombre_metrica>\n");
    printf("   - Agrega una nueva métrica a la lista de métricas a monitorear.\n");
    printf("3. config add metric <nombre_metrica1> <nombre_metrica2> ...\n");
    printf("   - Agrega múltiples métricas a la vez.\n");
    printf("4. config rm metric <nombre_metrica>\n");
    printf("   - Elimina una métrica de la lista de métricas a monitorear.\n");
    printf("5. config rm metric <nombre_metrica1> <nombre_metrica2> ...\n");
    printf("   - Elimina múltiples métricas a la vez.\n");
    printf("6. config help\n");
    printf("   - Muestra esta ayuda.\n");
    printf("7. config print\n");
    printf("   - Imprime las configuraciones del JSON.\n");
    printf("8. config get list\n");
    printf("   - Imprime las metricas disponibles en nuestro monitor.\n");
}

/**
 * @brief Imprime la configuración actual.
 *
 * @param config Puntero a la estructura de configuración.
 */
void print_config(const Config* config)
{
    printf("Configuración actual:\n");
    printf("Intervalo de muestreo: %d segundos\n", config->intervalo_muestreo);
    printf("Métricas monitoreadas (%d):\n", config->num_metricas);

    for (int i = 0; i < config->num_metricas; i++)
    {
        printf(" - %s\n", config->metricas[i]);
    }
}

/**
 * @brief Procesa comandos de configuración para la aplicación JSON.
 *
 * Esta función interpreta diferentes comandos relacionados con la configuración del
 * sistema y realiza las acciones correspondientes, como establecer el intervalo de
 * muestreo, imprimir la configuración actual, agregar o eliminar métricas y
 * mostrar la lista de métricas disponibles.
 *
 * @return true Si el comando fue reconocido y procesado correctamente.
 * @return false Si el comando no es de configuración.
 */
bool JSON_command(char* command, Config* config)
{
    json_command_type cmd_type = parse_json_command(command);

    switch (cmd_type)
    {
    case CMD_SET_INTERVAL: {
        int new_interval = atoi(command + 30);
        command_set_interval(new_interval, config); // Usa la función definida
        return true;
    }
    case CMD_PRINT_CONFIG:
        print_config(config); // Llama a la función que imprime la configuración
        return true;

    case CMD_ADD_METRIC: {
        char* metrics_list = command + 17;
        char* token = strtok(metrics_list, " ");
        char** new_metrics = NULL;
        int num_new_metrics = 0;

        while (token != NULL)
        {
            new_metrics = realloc(new_metrics, (num_new_metrics + 1) * sizeof(char*));
            new_metrics[num_new_metrics++] = strdup(token);
            token = strtok(NULL, " ");
        }

        command_add_multiple_metrics(new_metrics, num_new_metrics, config);

        // Liberar memoria
        for (int i = 0; i < num_new_metrics; i++)
        {
            free(new_metrics[i]);
        }
        free(new_metrics);

        return true;
    }
    case CMD_GET_LIST:
        get_list();
        return true;

    case CMD_HELP:
        config_help();
        return true;

    case CMD_REMOVE_METRIC: {
        char* metrics_list = command + 16;
        char* token = strtok(metrics_list, " ");
        char** metrics_to_remove = NULL;
        int num_metrics_to_remove = 0;

        while (token != NULL)
        {
            metrics_to_remove = realloc(metrics_to_remove, (num_metrics_to_remove + 1) * sizeof(char*));
            metrics_to_remove[num_metrics_to_remove++] = strdup(token);
            token = strtok(NULL, " ");
        }

        command_remove_multiple_metrics(metrics_to_remove, num_metrics_to_remove, config);

        // Liberar memoria
        for (int i = 0; i < num_metrics_to_remove; i++)
        {
            free(metrics_to_remove[i]);
        }
        free(metrics_to_remove);

        return true;
    }
    case CMD_INVALID:
    default:
        return false; // Si no coincide con ningún comando válido
    }
}

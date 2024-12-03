#ifndef CONFIG_H
#define CONFIG_H

#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @struct Config
 * @brief Estructura que almacena la configuración del sistema.
 */
typedef struct
{
    int intervalo_muestreo; /**< Intervalo de muestreo en segundos */
    char** metricas;        /**< Lista de métricas a monitorear */
    int num_metricas;       /**< Número de métricas en la lista */
} Config;

/**
 * @brief Lee la configuración desde un archivo JSON.
 *
 * Abre el archivo especificado y lee los valores de configuración
 * para inicializar la estructura Config.
 *
 * @param filename Nombre del archivo JSON a leer.
 * @return Config* Puntero a la estructura de configuración o NULL si ocurre un error.
 */
Config* read_config(const char* filename);

/**
 * @brief Actualiza la configuración en el archivo JSON.
 *
 * Escribe los valores actuales de la estructura Config en el archivo JSON especificado.
 *
 * @param filename Nombre del archivo JSON a actualizar.
 * @param config Puntero a la estructura de configuración.
 */
void update_config(const char* filename, Config* config);

/**
 * @brief Establece el intervalo de muestreo.
 *
 * Actualiza el intervalo de muestreo en la estructura Config.
 *
 * @param new_interval El nuevo intervalo de muestreo.
 * @param config Puntero a la estructura de configuración.
 */
void command_set_interval(int new_interval, Config* config);

/**
 * @brief Agrega una métrica a la lista de métricas.
 *
 * Añade una nueva métrica a la lista de métricas en la estructura Config.
 *
 * @param new_metric La nueva métrica a agregar.
 * @param config Puntero a la estructura de configuración.
 */
void command_add_metric(char* new_metric, Config* config);

/**
 * @brief Elimina una métrica de la lista de métricas.
 *
 * Elimina una métrica específica de la lista en la estructura Config.
 *
 * @param metric_to_remove La métrica a eliminar.
 * @param config Puntero a la estructura de configuración.
 */
void command_remove_metric(char* metric_to_remove, Config* config);

/**
 * @brief Agrega múltiples métricas a la lista de configuración.
 *
 * Añade varias métricas nuevas a la lista en la estructura Config.
 *
 * @param new_metrics Array de cadenas de métricas a agregar.
 * @param num_new_metrics Número de métricas en el array.
 * @param config Puntero a la estructura de configuración.
 */
void command_add_multiple_metrics(char** new_metrics, int num_new_metrics, Config* config);

/**
 * @brief Elimina múltiples métricas de la lista de configuración.
 *
 * Quita varias métricas de la lista en la estructura Config.
 *
 * @param metrics_to_remove Array de cadenas de métricas a eliminar.
 * @param num_metrics_to_remove Número de métricas en el array.
 * @param config Puntero a la estructura de configuración.
 */
void command_remove_multiple_metrics(char** metrics_to_remove, int num_metrics_to_remove, Config* config);

/**
 * @brief Imprime la ayuda sobre los comandos de configuración.
 *
 * Muestra en pantalla una lista de comandos disponibles para modificar la configuración.
 */
void config_help();

/**
 * @brief Imprime la configuración actual.
 *
 * Muestra en pantalla los valores actuales de la configuración.
 *
 * @param config Puntero a la estructura de configuración.
 */
void print_config(const Config* config);

/**
 * @brief Ejecuta un comando para modificar la configuración.
 *
 * Interpreta y ejecuta un comando específico para cambiar la configuración.
 *
 * @param command El comando a ejecutar.
 * @param config Puntero a la estructura de configuración.
 * @return true si el comando fue exitoso, false en caso contrario.
 */
bool JSON_command(char* command, Config* config);

#endif // CONFIG_H

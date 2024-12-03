#ifndef METRICS_PROCESSOR_H
#define METRICS_PROCESSOR_H

#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <fcntl.h> // Para open y O_TRUNC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> // Para mkfifo
#include <unistd.h>    // Para access y sleep

#define PIPE_PATH "/tmp/metrics_pipe"
#define CONFIG_PATH "config.json"
#define DELIMITADOR "<END_OF_METRICS>\n"
#define DEFAULT_INTERVAL 10

/**
 * @struct memory_struct
 * @brief Estructura para almacenar la respuesta de libcurl.
 *
 * Esta estructura se utiliza para almacenar la memoria de la respuesta recibida
 * al realizar solicitudes HTTP mediante la biblioteca cURL.
 */
struct memory_struct
{
    char* memory; /**< Puntero a la memoria que almacena la respuesta. */
    size_t size;  /**< Tamaño de la memoria almacenada. */
};

/**
 * @brief Callback para escribir datos en un buffer.
 *
 * Esta función se llama para manejar los datos recibidos de cURL y
 * almacenarlos en un buffer dinámico.
 *
 * @return Tamaño real de los datos escritos.
 */
static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp);

/**
 * @brief Lee el archivo de configuración.
 *
 * Esta función carga y analiza el archivo de configuración JSON para
 * obtener el intervalo de muestreo.
 *
 * @return Puntero a un objeto cJSON que representa la configuración leída, o NULL en caso de error.
 */
cJSON* leer_configuracion(int* intervalo_muestreo);

/**
 * @brief Filtra métricas según el archivo de configuración.
 *
 * Esta función verifica si las métricas proporcionadas están incluidas en
 * el conjunto de métricas especificadas en la configuración.
 *
 * @return 1 si hay métricas filtradas, 0 de lo contrario.
 */
int metricas_filtradas(const char* metricas, cJSON* config);

/**
 * @brief Función principal para obtener, filtrar y escribir métricas.
 *
 * Esta función realiza una solicitud HTTP para obtener métricas,
 * las filtra según la configuración y las escribe en una pipe nombrada.
 */
void procesar_metricas();

/**
 * @brief Función principal del programa.
 *
 * Esta función crea una pipe nombrada si no existe y ejecuta el
 * procesamiento de métricas en un bucle continuo, usando el intervalo
 * de muestreo definido en el archivo de configuración.
 *
 * @return 0 si el programa se ejecuta con éxito.
 */
int main();

#endif // METRICS_PROCESSOR_H

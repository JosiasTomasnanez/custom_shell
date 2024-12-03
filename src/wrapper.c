#include "wrapper.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <fcntl.h> // Para open y O_TRUNC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> // Para mkfifo
#include <unistd.h>    // Para access y sleep

/**
 * @brief Callback para escribir datos en un buffer.
 *
 * Esta función se llama para manejar los datos recibidos de cURL y
 * almacenarlos en un buffer dinámico.
 *
 * @return Tamaño real de los datos escritos.
 */
static size_t write_memory_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct memory_struct* mem = (struct memory_struct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; // Sin memoria

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/**
 * @brief Lee el archivo de configuración.
 *
 * Esta función carga y analiza el archivo de configuración JSON para
 * obtener el intervalo de muestreo.
 *
 * @return Puntero a un objeto cJSON que representa la configuración leída, o NULL en caso de error.
 */
cJSON* leer_configuracion(int* intervalo_muestreo)
{
    FILE* file = fopen(CONFIG_PATH, "r");
    if (!file)
    {
        perror("No se puede abrir config.json");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = malloc(length + 1);
    size_t read_count = fread(data, 1, length, file);
    (void)read_count;
    fclose(file);

    cJSON* json = cJSON_Parse(data);
    free(data);
    if (!json)
        return NULL;

    // Leer el intervalo de muestreo
    cJSON* intervalo = cJSON_GetObjectItem(json, "intervalo_muestreo");
    if (cJSON_IsNumber(intervalo))
    {
        *intervalo_muestreo = intervalo->valueint;
    }
    else
    {
        *intervalo_muestreo = DEFAULT_INTERVAL; // Valor predeterminado si no está definido en el JSON
    }
    return json;
}

/**
 * @brief Filtra métricas según el archivo de configuración.
 *
 * Esta función verifica si las métricas proporcionadas están incluidas en
 * el conjunto de métricas especificadas en la configuración.
 *
 * @return 1 si hay métricas filtradas, 0 de lo contrario.
 */
int metricas_filtradas(const char* metricas, cJSON* config)
{
    cJSON* metricas_array = cJSON_GetObjectItem(config, "metricas");
    if (!metricas_array)
        return 0;

    cJSON* metrica;
    cJSON_ArrayForEach(metrica, metricas_array)
    {
        const char* nombre_metrica = cJSON_GetStringValue(metrica);
        if (strstr(metricas, nombre_metrica))
            return 1;
    }
    return 0;
}

/**
 * @brief Función principal para obtener, filtrar y escribir métricas.
 *
 * Esta función realiza una solicitud HTTP para obtener métricas,
 * las filtra según la configuración y las escribe en una pipe nombrada.
 */
void procesar_metricas()
{
    CURL* curl;
    CURLcode res;
    struct memory_struct chunk = {0};

    // Inicializar cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl)
        return;

    // Configurar URL del programa de monitoreo
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/metrics");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Error de cURL: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return;
    }

    // Leer configuración
    int intervalo_muestreo = DEFAULT_INTERVAL;
    cJSON* config = leer_configuracion(&intervalo_muestreo);
    if (!config)
    {
        fprintf(stderr, "Error al cargar la configuración\n");
        return;
    }

    // Abrir pipe para escribir (usar open en lugar de fopen)
    int pipe_fd = open(PIPE_PATH, O_WRONLY | O_TRUNC);
    if (pipe_fd == -1)
    {
        perror("Error al abrir la pipe");
        return;
    }

    // Filtrar y escribir métricas
    char* linea = strtok(chunk.memory, "\n");
    while (linea != NULL)
    {
        if (metricas_filtradas(linea, config))
        {
            ssize_t bytes_written = write(pipe_fd, linea, strlen(linea));
            if (bytes_written == -1)
            {
                perror("Error al escribir en el pipe");
                continue;
            }
            bytes_written = write(pipe_fd, "\n", 1); // Agregar nueva línea
            (void)bytes_written;
        }
        linea = strtok(NULL, "\n");
    }

    // Escribir el delimitador al final de todas las métricas
    size_t unus = write(pipe_fd, DELIMITADOR, strlen(DELIMITADOR));
    (void)unus;

    close(pipe_fd); // Cerrar el descriptor de archivo
    cJSON_Delete(config);
    curl_easy_cleanup(curl);
    free(chunk.memory);
}

/**
 * @brief Función principal del programa.
 *
 * Esta función crea una pipe nombrada si no existe y ejecuta el
 * procesamiento de métricas en un bucle continuo, usando el intervalo
 * de muestreo definido en el archivo de configuración.
 *
 * @return 0 si el programa se ejecuta con éxito.
 */
int main()
{
    // Crear la pipe nombrada si no existe
    if (access(PIPE_PATH, F_OK) == -1)
    {
        if (mkfifo(PIPE_PATH, 0666) != 0)
        {
            perror("Error al crear la pipe");
            return 1;
        }
    }

    // Ejecutar el wrapper continuamente
    while (1)
    {
        int intervalo_muestreo = DEFAULT_INTERVAL;
        cJSON* config = leer_configuracion(&intervalo_muestreo);
        if (!config)
        {
            fprintf(stderr, "Error al cargar la configuración\n");
            return 1;
        }
        procesar_metricas();
        cJSON_Delete(config);
        sleep(intervalo_muestreo); // Usar el intervalo de muestreo definido en el JSON
    }

    return 0;
}

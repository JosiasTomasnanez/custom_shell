/**
 * @file expose_metrics.c
 * @brief Funciones para exponer métricas de Prometheus y actualizar las
 * métricas relacionadas con el rendimiento del sistema.
 *
 * Este archivo contiene la implementación de funciones que inicializan
 * métricas, actualizan su valor y exponen estas métricas a través de un
 * servidor HTTP utilizando la biblioteca de Prometheus.
 */

#include "expose_metrics.h"

/**
 * @brief Mutex para sincronización de hilos.
 */
pthread_mutex_t lock;

/**
 * @brief Métrica de Prometheus para el uso de CPU.
 */
static prom_gauge_t* cpu_usage_metric;

/**
 * @brief Métrica de Prometheus para el uso de memoria.
 */
static prom_gauge_t* memory_usage_metric;

/**
 * @brief Métrica de Prometheus para el uso de disco.
 */
static prom_gauge_t* disk_usage_metric;

/**
 * @brief Métrica de Prometheus para el uso de red.
 */
static prom_gauge_t* network_usage_metric;

/**
 * @brief Métrica de Prometheus que representa el uso de ancho de banda en la
 * red.
 */
static prom_gauge_t* bandwidth_usage_metric;

/**
 * @brief Métrica de Prometheus para los fallos de página mayores.
 */
static prom_gauge_t* major_page_faults_metric;

/**
 * @brief Métrica de Prometheus para los fallos de página menores.
 */
static prom_gauge_t* minor_page_faults_metric;

/**
 * @brief Métrica de Prometheus para los cambios de contexto.
 */
static prom_gauge_t* change_context_metric;

/**
 * @brief Métrica de Prometheus para el total de procesos.
 */
static prom_gauge_t* total_processes_metric;

/**
 * @brief Métrica de Prometheus para estadísticas del disco.
 */
static prom_gauge_t* disk_stats_metric;

/**
 * @brief Métrica de Prometheus para la memoria total.
 */
static prom_gauge_t* memory_total_metric;

/**
 * @brief Métrica de Prometheus para la memoria disponible.
 */
static prom_gauge_t* memory_avalible_metric;

/**
 * @brief Métrica de Prometheus para el uso alternativo de memoria.
 */
static prom_gauge_t* memory_usage_2_metric;

/**
 * @brief Actualiza la métrica de memoria disponible.
 */
void update_memory_avalible_gauge()
{
    double usage = get_memory_avalible();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_avalible_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la memoria disponible\n");
    }
}

/**
 * @brief Actualiza la métrica de memoria total.
 */
void update_memory_total_gauge()
{
    double usage = get_memory_total();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_total_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la memoria total\n");
    }
}

/**
 * @brief Actualiza la métrica alternativa de uso de memoria.
 */
void update_memory_2_gauge()
{
    double usage = get_memory_usage_2();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_2_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria (métrica alternativa)\n");
    }
}

/**
 * @brief Actualiza la métrica de estadísticas del disco.
 */
void update_disk_stats_gauge()
{
    double usage = get_disk_stats();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_stats_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener las estadísticas del disco\n");
    }
}

/**
 * @brief Actualiza la métrica del número total de procesos.
 */
void update_total_processes_gauge()
{
    double usage = get_total_processes();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(total_processes_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el número total de procesos\n");
    }
}

/**
 * @brief Actualiza la métrica de cambios de contexto.
 */
void update_change_context_gauge()
{
    double usage = get_change_context();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(change_context_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el número de cambios de contexto\n");
    }
}

/**
 * @brief Actualiza la métrica de uso de CPU.
 */
void update_cpu_gauge()
{
    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

/**
 * @brief Actualiza la métrica de uso de memoria.
 */
void update_memory_gauge()
{
    double usage = get_memory_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

/**
 * @brief Actualiza la métrica de uso del disco.
 */
void update_disk_gauge()
{
    double usage = get_disk_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(disk_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso del disco\n");
    }
}

/**
 * @brief Actualiza la métrica de uso de la red.
 */
void update_network_gauge()
{
    double usage = get_network_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(network_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de la red\n");
    }
}

/**
 * @brief Actualiza la métrica de uso de ancho de banda.
 */
void update_bandwidth_gauge()
{
    double usage = get_average_bandwidth();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(bandwidth_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el ancho de banda en uso\n");
    }
}

/**
 * @brief Actualiza la métrica de fallos de página mayores.
 */
void update_major_page_faults_gauge()
{
    int faults = get_major_page_faults();
    if (faults >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(major_page_faults_metric, faults, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener los fallos de página mayores\n");
    }
}

/**
 * @brief Actualiza la métrica de fallos de página menores.
 */
void update_minor_page_faults_gauge()
{
    int faults = get_minor_page_faults();
    if (faults >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(minor_page_faults_metric, faults, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener los fallos de página menores\n");
    }
}

/**
 * @brief Función que expone las métricas en el servidor HTTP.
 */

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
}

/**
 * @brief Inicializa las métricas del sistema y registra las métricas de
 * Prometheus.
 *
 * Esta función inicializa el mutex utilizado para proteger el acceso a las
 * métricas, configura el registro de coleccionistas de Prometheus, y crea
 * métricas para el uso de CPU, memoria, disco y red, así como otros indicadores
 * de rendimiento.
 *
 * @note Si ocurre un error en la inicialización de cualquiera de las métricas,
 * se imprimirá un mensaje de error en stderr.
 */
void init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
        // return EXIT_FAILURE;
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de disco
    disk_usage_metric = prom_gauge_new("disk_usage_percentage", "Porcentaje de uso de disco", 0, NULL);
    if (disk_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de disco\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de la red
    network_usage_metric = prom_gauge_new("network_usage", "Uso de la red", 0, NULL);
    if (network_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de la red\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para el ancho de banda de la red
    bandwidth_usage_metric = prom_gauge_new("bandwidth_usage", "Ancho de banda en uso", 0, NULL);
    if (bandwidth_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de ancho de banda\n");
        // return EXIT_FAILURE;
    }

    // Creamos la métrica para los fallos de página mayores
    major_page_faults_metric = prom_gauge_new("major_page_faults", "Número de fallos de página mayores", 0, NULL);
    if (major_page_faults_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de fallos de página mayores\n");
    }

    // Creamos la métrica para los fallos de página menores
    minor_page_faults_metric = prom_gauge_new("minor_page_faults", "Número de fallos de página menores", 0, NULL);
    if (minor_page_faults_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de fallos de página menores\n");
    }

    // Ejemplo de métricas adicionales (asegúrate de definirlas en tu código)
    change_context_metric = prom_gauge_new("change_contexts", "Número de cambios de contexto", 0, NULL);
    if (change_context_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cambios de contexto\n");
        return; // Salimos si hay un error
    }

    total_processes_metric = prom_gauge_new("total_processes", "Número total de procesos", 0, NULL);
    if (total_processes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de total de procesos\n");
        return; // Salimos si hay un error
    }

    memory_total_metric = prom_gauge_new("memory_total", "Memoria total del sistema", 0, NULL);
    if (memory_total_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria total\n");
        return; // Salimos si hay un error
    }

    memory_avalible_metric = prom_gauge_new("memory_available", "Memoria disponible del sistema", 0, NULL);
    if (memory_avalible_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria disponible\n");
        return; // Salimos si hay un error
    }

    memory_usage_2_metric = prom_gauge_new("memory_usage_2", "Uso de memoria (otra métrica)", 0, NULL);
    if (memory_usage_2_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria (otra)\n");
        return; // Salimos si hay un error
    }

    // Agregamos la métrica para los disk stats
    disk_stats_metric = prom_gauge_new("disk_stats", "Estadísticas del disco", 0, NULL);
    if (disk_stats_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de estadísticas del disco\n");
        return; // Manejo de errores
    }

    // Registramos las métricas en el registro por defecto
    if (prom_collector_registry_must_register_metric(cpu_usage_metric) == NULL ||
        prom_collector_registry_must_register_metric(memory_usage_metric) == NULL ||
        prom_collector_registry_must_register_metric(disk_usage_metric) == NULL ||
        prom_collector_registry_must_register_metric(network_usage_metric) == NULL ||
        prom_collector_registry_must_register_metric(bandwidth_usage_metric) == NULL ||
        prom_collector_registry_must_register_metric(major_page_faults_metric) == NULL ||
        prom_collector_registry_must_register_metric(minor_page_faults_metric) == NULL ||
        prom_collector_registry_must_register_metric(change_context_metric) == NULL ||
        prom_collector_registry_must_register_metric(total_processes_metric) == NULL ||
        prom_collector_registry_must_register_metric(memory_total_metric) == NULL ||
        prom_collector_registry_must_register_metric(memory_avalible_metric) == NULL ||
        prom_collector_registry_must_register_metric(memory_usage_2_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar las métricas\n");
        // return EXIT_FAILURE;
    }
}

/**
 * @brief Destruye el mutex utilizado para proteger el acceso a las métricas.
 *
 * Esta función libera los recursos asociados al mutex creado en la función
 * @ref init_metrics.
 */
void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}

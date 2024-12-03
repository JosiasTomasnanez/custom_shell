/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU y memoria y exponerlos como métricas
 * de Prometheus.
 *
 * Este archivo contiene las declaraciones de las funciones que permiten leer
 * varias métricas del sistema, como el uso de CPU, memoria, disco, red y más.
 * Las métricas se exponen vía HTTP utilizando Prometheus.
 */

#include "metrics.h"
// #include "read_cpu_usage.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep

/**
 * @brief Tamaño del buffer utilizado para leer archivos o datos.
 */
#define BUFFER_SIZE 256

/**
 * @brief Actualiza la métrica de memoria disponible.
 *
 * Lee el valor de memoria disponible desde /proc/meminfo y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_memory_avalible_gauge();

/**
 * @brief Actualiza la métrica de memoria total.
 *
 * Lee el valor de memoria total desde /proc/meminfo y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_memory_total_gauge();

/**
 * @brief Actualiza la métrica de memoria en uso (no expresada como porcentaje).
 *
 * Lee el valor de memoria en uso desde /proc/meminfo y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_memory_2_gauge();

/**
 * @brief Actualiza la métrica de estadística de disco.
 *
 * Lee el valor total de lecturas y escrituras de disco desde /proc/diskstats y
 * actualiza el gauge correspondiente de Prometheus.
 */
void update_disk_stats_gauge();

/**
 * @brief Actualiza la métrica de la cantidad total de procesos del sistema.
 *
 * Lee el valor de procesos totales desde /proc/stat y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_total_processes_gauge();

/**
 * @brief Actualiza la métrica de cambios de contexto del sistema.
 *
 * Lee el valor de cambios de contexto desde /proc/stat y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_change_context_gauge();

/**
 * @brief Actualiza la métrica de uso de CPU.
 *
 * Lee el porcentaje de uso de CPU desde /proc/stat y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 *
 * Lee el porcentaje de uso de memoria desde /proc/meminfo y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_memory_gauge();

/**
 * @brief Actualiza la métrica de uso de disco.
 *
 * Lee el porcentaje de uso de disco desde /proc/diskstats y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_disk_gauge();

/**
 * @brief Actualiza la métrica de uso de red.
 *
 * Lee las estadísticas de red desde /proc/net/dev y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_network_gauge();

/**
 * @brief Actualiza la métrica de ancho de banda promedio.
 *
 * Calcula el ancho de banda en uso basado en /proc/net/dev y actualiza el gauge
 * correspondiente de Prometheus.
 */
void update_bandwidth_gauge();

/**
 * @brief Actualiza la métrica de fallos de página mayores.
 *
 * Lee el número de fallos de página mayores desde /proc/vmstat y actualiza el
 * gauge correspondiente de Prometheus.
 */
void update_major_page_faults_gauge();

/**
 * @brief Actualiza la métrica de fallos de página menores.
 *
 * Lee el número de fallos de página menores desde /proc/vmstat y actualiza el
 * gauge correspondiente de Prometheus.
 */
void update_minor_page_faults_gauge();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 *
 * Esta función es ejecutada por un hilo separado para exponer las métricas a
 * través de HTTP en el puerto 8000.
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializa los mutex y las métricas de Prometheus.
 *
 * Esta función se encarga de inicializar los mutex necesarios y configurar las
 * métricas de Prometheus.
 */
void init_metrics();

/**
 * @brief Destructor de mutex.
 *
 * Esta función destruye los mutex que fueron inicializados para evitar fugas de
 * memoria.
 */
void destroy_mutex();

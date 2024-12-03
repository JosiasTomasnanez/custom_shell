/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU, memoria, disco y otros recursos
 * del sistema desde el sistema de archivos /proc.
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer en bytes.
 */
#define BUFFER_SIZE 256

/**
 * @brief Tamaño de un sector en bytes.
 */
#define SECTOR_SIZE 512 // para el get_disk

/**
 * @brief Obtiene la cantidad de cambios de contexto del sistema desde
 * /proc/stat.
 *
 * @return Cantidad de cambios de contexto, o -1 en caso de error.
 */
unsigned long long get_change_context();

/**
 * @brief Obtiene la cantidad total de procesos del sistema desde /proc/stat.
 *
 * @return Cantidad de procesos del sistema, o -1 en caso de error.
 */
unsigned long long get_total_processes();

/**
 * @brief Obtiene la suma total de lecturas y escrituras en el disco desde
 * /proc/diskstats.
 *
 * @return Suma de lecturas y escrituras del disco en bytes, o -1.0 en caso de
 * error.
 */
double get_disk_stats();

/**
 * @brief Obtiene la memoria total del sistema desde /proc/meminfo.
 *
 * @return Memoria total del sistema en bytes, o -1.0 en caso de error.
 */
double get_memory_total();

/**
 * @brief Obtiene la memoria disponible del sistema desde /proc/meminfo.
 *
 * @return Memoria disponible del sistema en bytes, o -1.0 en caso de error.
 */
double get_memory_avalible();

/**
 * @brief Obtiene la memoria en uso del sistema desde /proc/meminfo, no
 * expresada como porcentaje.
 *
 * @return Memoria en uso del sistema en bytes, o -1.0 en caso de error.
 */
double get_memory_usage_2();

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de
 * error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtiene el porcentaje de uso de disco desde /proc/diskstats.
 *
 * Esta función lee el estado del uso de discos desde /proc/diskstats, que
 * proporciona estadísticas detalladas de los discos en el sistema, y calcula el
 * uso total de lectura y escritura.
 *
 * @return Porcentaje de uso de disco, o -1.0 en caso de error.
 */
double get_disk_usage();

/**
 * @brief Obtiene el tráfico de red desde /proc/net/dev.
 *
 * Lee las estadísticas de las interfaces de red desde /proc/net/dev y calcula
 * el uso de red.
 *
 * @return Porcentaje de uso de red, o -1.0 en caso de error.
 */
double get_network_usage();

/**
 * @brief Obtiene el ancho de banda promedio en uso desde /proc/net/dev.
 *
 * Lee los bytes transmitidos y recibidos desde /proc/net/dev y calcula el
 * ancho de banda promedio en uso basado en el intervalo de tiempo de muestreo.
 *
 * @return Ancho de banda en uso en Megabytes por segundo, o -1.0 en caso de
 * error.
 */
double get_average_bandwidth();

/**
 * @brief Obtiene el número de fallos de página mayores desde /proc/vmstat.
 *
 * @return Número de fallos de página mayores, o -1 en caso de error.
 */
unsigned long long get_major_page_faults();

/**
 * @brief Obtiene el número de fallos de página menores desde /proc/vmstat.
 *
 * @return Número de fallos de página menores, o -1 en caso de error.
 */
unsigned long long get_minor_page_faults();

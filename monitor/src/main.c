/**
 * @file main.c
 * @brief Punto de entrada del sistema.
 *
 * Este archivo contiene la función principal que inicializa las métricas y
 * comienza un hilo para exponer las métricas a través de HTTP. Actualiza
 * continuamente varios indicadores relacionados con el rendimiento del sistema
 * cada segundo.
 */

#include "expose_metrics.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SLEEP_TIME 1 /**< Tiempo de espera en segundos. */

/**
 * @brief Función principal de la aplicación.
 *
 * Esta función inicializa la recolección de métricas, crea un hilo para exponer
 * las métricas a través de HTTP y entra en un bucle para actualizar varios
 * indicadores del sistema cada segundo.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Array de cadenas de argumentos de la línea de comandos.
 *
 * @return EXIT_SUCCESS si la aplicación termina con éxito, EXIT_FAILURE en caso
 * contrario.
 */
int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    init_metrics(); /**< Inicializa la recolección de métricas. */

    // Creamos un hilo para exponer las métricas vía HTTP
    pthread_t tid; /**< Identificador del hilo del servidor HTTP. */
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE; /**< Retorna fallo si la creación del hilo falla. */
    }

    // Bucle principal para actualizar las métricas cada segundo
    while (true)
    {
        update_cpu_gauge();               /**< Actualiza el indicador de uso de CPU. */
        update_memory_gauge();            /**< Actualiza el indicador de uso de memoria. */
        update_disk_gauge();              /**< Actualiza el indicador de uso de disco. */
        update_network_gauge();           /**< Actualiza el indicador de uso de red. */
        update_bandwidth_gauge();         /**< Actualiza el indicador de ancho de banda. */
        update_major_page_faults_gauge(); /**< Actualiza el indicador de fallos de
                                             página mayores. */
        update_minor_page_faults_gauge(); /**< Actualiza el indicador de fallos de
                                             página menores. */
        update_memory_avalible_gauge();   /**< Actualiza el indicador de memoria
                                             disponible. */
        update_memory_total_gauge();      /**< Actualiza el indicador de memoria total.
                                           */
        update_memory_2_gauge();          /**< Actualiza el segundo indicador de memoria. */
        update_disk_stats_gauge();        /**< Actualiza el indicador de estadísticas del
                                             disco. */
        update_total_processes_gauge();   /**< Actualiza el indicador de procesos
                                             totales. */
        update_change_context_gauge();    /**< Actualiza el indicador de cambios de
                                             contexto. */

        sleep(SLEEP_TIME); /**< Duerme durante un periodo definido antes de
                              actualizar nuevamente. */
    }

    return EXIT_SUCCESS; /**< Retorna éxito si la aplicación finaliza. */
}

#include "JSON_handler.h"
#include "command_processor.h"
#include "input_interface.h"
#include "metric_handler.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unity/unity.h>

#define OUTPUT_BUFFER_SIZE 1024

// Mock para la salida estándar
static char output_buffer[OUTPUT_BUFFER_SIZE]; // Buffer donde almacenamos la salida
static FILE* output_stream;

void setUp(void)
{
    // Inicialización antes de cada prueba, si es necesario
    memset(output_buffer, 0, sizeof(output_buffer)); // Limpiar el buffer de salida
    output_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
}
void tearDown(void)
{
    // Limpieza después de cada prueba
    if (output_stream)
    {
        fclose(output_stream);
        output_stream = NULL;
    }
}

void test_cd_to_home()
{
    char* command = strdup("cd");
    execute_command(command);

    char* current_dir = getenv("PWD");
    char* home_dir = getenv("HOME");

    TEST_ASSERT_EQUAL_STRING(home_dir, current_dir);
    free(command);
}

void test_cd_to_previous_directory()
{
    setenv("OLDPWD", "/tmp", 1);
    char* command = strdup("cd -");
    execute_command(command);

    char* current_dir = getenv("PWD");
    TEST_ASSERT_EQUAL_STRING("/tmp", current_dir);
    free(command);
}

void test_output_redirection()
{
    char* command = strdup("echo test > output.txt");
    execute_command(command);

    FILE* file = fopen("output.txt", "r");
    TEST_ASSERT_NOT_NULL(file);

    char output[10];
    fgets(output, sizeof(output), file);
    fclose(file);

    TEST_ASSERT_EQUAL_STRING("test\n", output);

    remove("output.txt");
    free(command);
}

void test_get_command()
{
    const char* input = "test_command\n";
    FILE* temp_input = fmemopen((void*)input, strlen(input), "r");
    TEST_ASSERT_NOT_NULL(temp_input);

    // Redirigir stdin temporalmente a temp_input
    FILE* stdin_backup = stdin; // Guardar el puntero original de stdin
    stdin = temp_input;

    char* result = get_command();
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("test_command\n", result);

    free(result);

    // Restaurar stdin a su estado original y liberar temp_input
    stdin = stdin_backup;
    fclose(temp_input);
}

void test_JSON_command_print()
{
    Config config;
    config.intervalo_muestreo = 10;
    config.num_metricas = 2;
    config.metricas = malloc(config.num_metricas * sizeof(char*));
    config.metricas[0] = strdup("CPU"); // Usa strdup para cadenas
    config.metricas[1] = strdup("Memoria");

    char output_buffer[OUTPUT_BUFFER_SIZE];
    FILE* stream = fmemopen(output_buffer, sizeof(output_buffer), "w");
    TEST_ASSERT_NOT_NULL(stream);

    FILE* stdout_backup = stdout;
    stdout = stream;

    print_config(&config);

    fflush(stream);
    fclose(stream);
    stdout = stdout_backup;

    const char* expected_output = "Configuración actual:\n"
                                  "Intervalo de muestreo: 10 segundos\n"
                                  "Métricas monitoreadas (2):\n"
                                  " - CPU\n"
                                  " - Memoria\n";

    TEST_ASSERT_EQUAL_STRING(expected_output, output_buffer);

    free(config.metricas[0]);
    free(config.metricas[1]);
    free(config.metricas);
}

void test_config_help()
{
    FILE* stdout_backup = stdout;
    stdout = fmemopen(output_buffer, OUTPUT_BUFFER_SIZE, "w");
    TEST_ASSERT_NOT_NULL(stdout);

    config_help();

    fflush(stdout);
    fclose(stdout);
    stdout = stdout_backup;

    const char* expected_output = "Comandos de configuración disponibles:\n"
                                  "1. config set intervalo_muestreo <valor>\n"
                                  "   - Establece el intervalo de muestreo en segundos.\n"
                                  "2. config add metric <nombre_metrica>\n"
                                  "   - Agrega una nueva métrica a la lista de métricas a monitorear.\n"
                                  "3. config add metric <nombre_metrica1> <nombre_metrica2> ...\n"
                                  "   - Agrega múltiples métricas a la vez.\n"
                                  "4. config rm metric <nombre_metrica>\n"
                                  "   - Elimina una métrica de la lista de métricas a monitorear.\n"
                                  "5. config rm metric <nombre_metrica1> <nombre_metrica2> ...\n"
                                  "   - Elimina múltiples métricas a la vez.\n"
                                  "6. config help\n"
                                  "   - Muestra esta ayuda.\n"
                                  "7. config print\n"
                                  "   - Imprime las configuraciones del JSON.\n"
                                  "8. config get list\n"
                                  "   - Imprime las metricas disponibles en nuestro monitor.\n";

    TEST_ASSERT_EQUAL_STRING(expected_output, output_buffer);
}
extern status s; // Declaración de la variable s

void test_status_monitor(void)
{
    // Configura el estado de la monitorización a un valor conocido
    s = NOT_STARTED;
    TEST_ASSERT_EQUAL(NOT_STARTED, status_monitor());

    // Cambia el estado a RUN y verifica
    s = RUN;
    TEST_ASSERT_EQUAL(RUN, status_monitor());

    // Cambia el estado a STOPPED y verifica
    s = STOP;
    TEST_ASSERT_EQUAL(STOP, status_monitor());
}
void test_status_to_string(void)
{
    // Verifica que cada estado se convierte correctamente en texto

    // Asumiendo que estos son los valores posibles de los estados
    TEST_ASSERT_EQUAL_STRING("NOT_STARTED", status_to_string(NOT_STARTED));
    TEST_ASSERT_EQUAL_STRING("RUN", status_to_string(RUN));
    TEST_ASSERT_EQUAL_STRING("STOP", status_to_string(STOP));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cd_to_home);
    RUN_TEST(test_cd_to_previous_directory);
    RUN_TEST(test_output_redirection);
    RUN_TEST(test_get_command);
    RUN_TEST(test_JSON_command_print);
    RUN_TEST(test_config_help);
    RUN_TEST(test_status_monitor);
    RUN_TEST(test_status_to_string);
    return UNITY_END();
}

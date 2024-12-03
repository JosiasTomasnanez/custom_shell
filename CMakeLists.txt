cmake_minimum_required(VERSION 3.10)
project(IntegratedProject)


# Habilitar la política CMP0079
if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

# Añadir advertencias (incluyendo -Werror)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)  # Añade tus advertencias
endif()

# Eliminar -Werror si está presente
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()



if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Crear un entorno virtual para Conan usando python3
execute_process(
    COMMAND python3 -m venv ${CMAKE_BINARY_DIR}/conan_env
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE venv_result
    OUTPUT_VARIABLE venv_output
    ERROR_VARIABLE venv_error
)

if(venv_result EQUAL 0)
    message(STATUS "Virtual environment created successfully.")
else()
    message(WARNING "Failed to create virtual environment: ${venv_error}")
endif()

# Instalar Conan en el entorno virtual
execute_process(
    COMMAND ${CMAKE_BINARY_DIR}/conan_env/bin/pip install conan
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    RESULT_VARIABLE pip_result
    OUTPUT_VARIABLE pip_output
    ERROR_VARIABLE pip_error
)

if(pip_result EQUAL 0)
    message(STATUS "Conan installed successfully.")
else()
    message(WARNING "Failed to install Conan: ${pip_error}")
endif()

# Comprobar si el perfil predeterminado de Conan existe y crearlo si no
execute_process(
    COMMAND ${CMAKE_BINARY_DIR}/conan_env/bin/conan profile list
    OUTPUT_VARIABLE profile_list
)

if(NOT profile_list MATCHES "default")
    message(STATUS "Conan default profile not found. Creating default profile...")
    execute_process(
        COMMAND ${CMAKE_BINARY_DIR}/conan_env/bin/conan profile detect
        RESULT_VARIABLE profile_detect_result
    )
    if(NOT profile_detect_result EQUAL 0)
        message(FATAL_ERROR "Failed to create Conan default profile. Error code: ${profile_detect_result}")
    endif()
else()
    message(STATUS "Conan default profile found.")
endif()

# Ejecutar conan install
message(STATUS "Conan dependencies not found. Running conan install...")
execute_process(
    COMMAND ${CMAKE_BINARY_DIR}/conan_env/bin/conan install . --output . --build=missing --settings=build_type=${CMAKE_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE conan_result
)

if(NOT conan_result EQUAL 0)
    message(FATAL_ERROR "Conan install failed with error code: ${conan_result}")
endif()

# Incluir el archivo de Conan Toolchain
include(${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators/conan_toolchain.cmake)

# Directorio de salida para el ejecutable principal en la carpeta principal
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Directorio de salida para los ejecutables en la carpeta bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_BIN ${CMAKE_SOURCE_DIR}/bin)

# Incluir directorios de encabezados
include_directories(include)                      # Encabezados del proyecto principal
include_directories(monitor/include)              # Encabezados del subproyecto monitor


# Encontrar y vincular bibliotecas cJSON, Unity, libcurl y libmicrohttpd proporcionadas por Conan
find_package(cJSON REQUIRED)
find_package(unity REQUIRED)
find_package(CURL REQUIRED)
find_package(libmicrohttpd REQUIRED)

# Añadir subdirectorios para Prometheus después de encontrar todas las dependencias necesarias
add_subdirectory(prometheus-client-c/prom)
add_subdirectory(prometheus-client-c/promhttp)

# Asegurarte de que promhttp use libmicrohttpd a través de Conan
target_link_libraries(promhttp PRIVATE libmicrohttpd::libmicrohttpd)
target_include_directories(promhttp PRIVATE ${libmicrohttpd_INCLUDE_DIRS})

# Proyecto principal: shell
add_executable(shell
    src/baash.c
    src/metric_handler.c
    src/input_interface.c
    src/command_processor.c
    src/JSON_handler.c
)

# Vincular bibliotecas al ejecutable principal
target_link_libraries(shell
    cjson::cjson
    CURL::libcurl  # Usar la biblioteca de libcurl proporcionada por Conan
)

# Subproyecto: monitoring_project
add_executable(monitoring_project
    monitor/src/main.c
    monitor/src/metrics.c
    monitor/src/expose_metrics.c
)

# Vincular las bibliotecas de Prometheus
target_link_libraries(monitoring_project
    prom  # Biblioteca principal de Prometheus
    promhttp  # Biblioteca HTTP de Prometheus
    cjson::cjson   # Biblioteca de cJSON proporcionada por Conan
    CURL::libcurl  # Biblioteca de libcurl proporcionada por Conan
   libmicrohttpd::libmicrohttpd  # Biblioteca de microhttpd por Conan
)

# Configurar salida de monitoring_project en el directorio bin
set_target_properties(monitoring_project PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_BIN}
)

# Añadir el ejecutable wrapper
add_executable(wrapper
    src/wrapper.c
)

# Configurar la salida del ejecutable wrapper en el directorio bin
set_target_properties(wrapper PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_BIN}
)

# Vincular las bibliotecas necesarias para el ejecutable wrapper
target_link_libraries(wrapper
    CURL::libcurl  # Usar la biblioteca de libcurl proporcionada por Conan
    cjson::cjson
)
enable_testing()
# Ejecutable de pruebas
add_executable(mytest
    src/metric_handler.c
    src/input_interface.c
    src/command_processor.c
    src/JSON_handler.c
    test/test_command_processor.c
)

# Aplica los flags de cobertura solo al ejecutable de prueba
target_compile_options(mytest PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(mytest PRIVATE -fprofile-arcs -ftest-coverage) 

# Configurar la salida del ejecutable de pruebas en el directorio build/tests
set_target_properties(mytest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

target_link_libraries(mytest
    cjson::cjson
    unity::unity  # Asegúrate de que Unity esté vinculado aquí
)
# Registrar la prueba para CTest
add_test(NAME test_command_processor COMMAND mytest)


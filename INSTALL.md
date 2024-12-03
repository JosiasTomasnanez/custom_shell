# Instalación del Proyecto

Este documento describe los pasos necesarios para configurar y compilar el proyecto. Asegúrate de tener instalado Python 3 y las herramientas necesarias para compilar el proyecto. Tambien es necesario para la funcion de monitoreo de nuestra shell que se tenga instalado las librerias de prometheus_client_c

## Requisitos Previos

Se requiere instalar lo siguiente:

- **Python 3**: Puedes instalarlo desde [python.org](https://www.python.org/downloads/).
- **CMake**: Para la construcción del proyecto. Instálalo utilizando: `sudo apt-get install cmake`.
- **Conan**: Un gestor de paquetes para C++.

## Pasos de Instalación

1. **Crear un entorno virtual de Python**: `python3 -m venv venv`.

2. **Activar el entorno virtual**: `source venv/bin/activate`.

3. **Instalar Conan**: `pip install conan`.

4. **Detectar el perfil de Conan**: `conan profile detect --force`.

5. **Instalar la biblioteca `libcjson-dev`**: `sudo apt-get install libcjson-dev`.

6. **Crear un perfil de depuración en Conan**:
   - `cp ~/.conan2/profiles/default ~/.conan2/profiles/debug`
   - `echo "build_type=Debug" >> ~/.conan2/profiles/debug`

7. **Crear el directorio build** `mkdir build`

8. **Irnos al directorio que se creo "build"**: `cd build`

9. **Crear el MakeFile de acuerdo al CMakeLists.txt**: `cmake ..`

10. **Compilar el proyecto usando cmake**: `make`.

Esto nos creara los binarios necesarios para que funcione nuestra shell, podemos correrla usando el comando `./shell` en la terminal o corriendo el script `run.sh`.

## Instalacion de prometheus

vease el archivo INSTALL.md en el repositorio del programa de monitoreo.

## Notas Adicionales

Si deseas salir del entorno virtual, puedes usar el comando: `source venv/bin/activate` en el lugar donde hayas creado tu entorno.

Asegúrate de seguir estos pasos en el orden indicado para evitar problemas de configuración.


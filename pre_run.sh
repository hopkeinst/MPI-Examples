#!/bin/bash

# Cargar las variables de intel oneAPi

# Posible ruta donde está el script de variables de entorno
SETVARS_PATH="/opt/intel/oneapi/setvars.sh"

# Obtener la carpeta lib64 para agregar a la variable de entorno de librería de intel
ACTUAL_PATH=$(pwd)

# Ubicación predeterminada de setvars.sh
if [ -f "$SETVARS_PATH" ]; then
    echo "El archivo setvars.sh está en => $SETVARS_PATH"
    source "$SETVARS_PATH"
else
    SETVARS_PATH=$(find / -name "setvars.sh" 2>/dev/null | grep "/oneapi/")
    if [ -z "$SETVARS_PATH" ]; then
        echo "No se encontró el archivo setvars.sh, por lo cual no se puede continuar. Pruebe instalando el kit de oneAPI."
        exit 1
    else
        echo "El archivo setvars.sh está en => $SETVARS_PATH"
        source "$SETVARS_PATH"
    fi
fi

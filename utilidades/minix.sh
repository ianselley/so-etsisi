#!/bin/bash

# minix.sh

# Leer el ID desde id.txt y eliminar espacios
ID=$(tr -d ' ' < ./id.txt)

# Verificar si ID está definido y no es "TODO"
if [[ -z "$ID" || "$ID" == "TODO" ]]; then
    echo "ERROR: Debes modificar id.txt con tu identificador"
    exit 1
fi

echo "El identificador establecido es: $ID"

# Definir nombres de archivos de imagen
IMAGENAME="minix3hd.plot"
IMAGEZIP="${IMAGENAME}.7z"
IMAGEUNZIP="${IMAGENAME}.qcow2"
IMAGEFILE="${IMAGENAME}.${ID}.qcow2"

# Verificar si QEMU existe
if [[ ! -f "./utilidades/qemu/qemu-system-x86_64w.exe" ]]; then
    echo "ERROR: No se encuentra QEMU para arrancar la máquina virtual"
    echo "Por favor, asegúrate de que QEMU está instalado correctamente."
    exit 1
fi

# Verificar si la imagen de Minix existe
if [[ ! -f "./imagen/${IMAGEUNZIP}" ]]; then
    echo "ERROR: No se encuentra la imagen de Minix para arrancar la máquina virtual"
    echo "Por favor, ejecuta el script de descompresión:"
    echo "./descomprimir.sh"
    exit 1
fi

echo "Herramientas verificadas: QEMU y la imagen de Minix existen."

# Configurar directorio de trabajo
WORK_DIR="$(pwd)/trabajo"
OUT_DIR="$(pwd)/out"

# Crear el directorio de salida si no existe
if [[ ! -d "$OUT_DIR" ]]; then
    mkdir -p "$OUT_DIR"
    echo "Creado el directorio de salida: $OUT_DIR"
fi

# Sincronizar el directorio 'trabajo' con Minix usando rsync
# Asumimos que Minix tiene SSH activo y está accesible en localhost:5522
echo "Sincronizando el directorio 'trabajo' con Minix usando rsync..."
rsync -avz -e "ssh -p 5522" "$WORK_DIR/" usuario@localhost:/home/usuario/trabajo/

if [[ $? -ne 0 ]]; then
    echo "Falló la sincronización con Minix. Vamos a ejecutar Minix"
    # Iniciar QEMU con la imagen de Minix
    echo ""
    echo ""
    echo "Arrancando la máquina virtual Minix con QEMU..."

    qemu-system-x86_64 \
        -cpu "pentium3" \
        -m 512 \
        -name "$ID" \
        -rtc base=localtime \
        -hda "./imagen/${IMAGEFILE}" \
        -drive file=fat:rw:$OUT_DIR,format=raw,media=disk,cache=none \
        -netdev user,id=n1,ipv6=off,restrict=off,hostfwd=tcp:127.0.0.1:5522-:22 \
        -device ne2k_pci,netdev=n1,mac=52:54:98:76:54:32 \
        -debugcon file:../log_e9.bin &

    QEMU_PID=$!
    echo "QEMU iniciado con PID: $QEMU_PID"

    # Esperar a que QEMU inicie (puedes ajustar el tiempo según tu sistema)
    sleep 15
fi


echo "Realizado el arranque de Minix en QEMU."

# # Opcional: Sincronizar nuevamente después de iniciar QEMU
# rsync -avz -e "ssh -p 5522" "$WORK_DIR/" usuario@localhost:/home/usuario/trabajo/

# if [[ $? -ne 0 ]]; then
#     echo "ERROR: Falló la sincronización inicial con Minix después de arrancar QEMU."
#     kill $QEMU_PID
#     exit 1
# fi

# echo "Máquina virtual Minix arrancada y lista para usar."

# echo "Para apagar Minix, entra a la consola de Minix y ejecuta 'halt'. Luego, detén QEMU con 'kill $QEMU_PID'."

exit 0

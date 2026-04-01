#!/bin/bash

echo "🔨 Compilando pak_tool v2.0..."
gcc -Wall -Wextra -std=c99 -O2 -o pak_tool pak_tool_v2.c crc32.c -lm

if [ $? -eq 0 ]; then
    echo "✓ Compilación exitosa"
    echo ""
    echo "EJEMPLOS DE USO:"
    echo ""
    echo "1. Crear PAK CON subdirectorios (por defecto):"
    echo "   ./pak_tool create ./resources ./SUPAPLEX.pak"
    echo ""
    echo "2. Crear PAK SOLO root (sin subdirectorios):"
    echo "   ./pak_tool create -f ./resources ./SUPAPLEX.pak"
    echo ""
    echo "3. Crear PAK con output detallado:"
    echo "   ./pak_tool create -v ./resources ./SUPAPLEX.pak"
    echo ""
    echo "4. Listar con estructura de directorios:"
    echo "   ./pak_tool list ./SUPAPLEX.pak"
    echo ""
    echo "5. Ver estadísticas por carpeta:"
    echo "   ./pak_tool info ./SUPAPLEX.pak"
    echo ""
    echo "6. Extraer preservando estructura:"
    echo "   ./pak_tool extract ./SUPAPLEX.pak ./output"
else
    echo "❌ Error de compilación"
    exit 1
fi
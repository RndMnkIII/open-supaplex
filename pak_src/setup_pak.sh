#!/bin/bash

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║  SUPAPLEX PAK SETUP - Integración Completa                    ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# 1. Compilar herramienta PAK
echo "1️⃣  Compilando herramienta PAK..."
gcc -Wall -Wextra -std=c99 -O2 -o pak_tool pak_tool_v2.c crc32.c -lm
if [ $? -eq 0 ]; then
    echo "   ✅ pak_tool compilado"
else
    echo "   ❌ Error compilando pak_tool"
    exit 1
fi

# 2. Crear PAK desde recursos
echo ""
echo "2️⃣  Creando SUPAPLEX.pak..."
if [ -d "resources" ]; then
    ./pak_tool create ./resources ./SUPAPLEX.pak
    if [ $? -eq 0 ]; then
        echo "   ✅ SUPAPLEX.pak creado"
    else
        echo "   ❌ Error creando PAK"
        exit 1
    fi
else
    echo "   ⚠️  Directorio 'resources' no encontrado"
    echo "   Asegúrate de estar en el directorio raíz del proyecto"
    exit 1
fi

# 3. Verificar contenido
echo ""
echo "3️⃣  Verificando contenido del PAK..."
./pak_tool list ./SUPAPLEX.pak | head -20
echo "   ... (use 'pak_tool list ./SUPAPLEX.pak' para lista completa)"

# 4. Compilar juego
echo ""
echo "4️⃣  Compilando juego con soporte PAK..."
make clean
make
if [ $? -eq 0 ]; then
    echo "   ✅ Juego compilado"
else
    echo "   ❌ Error compilando juego"
    exit 1
fi

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║ ✅ SETUP COMPLETADO                                            ║"
echo "╠════════════════════════════════════════════════════════════════╣"
echo "║ Archivos generados:                                            ║"
echo "║   • pak_tool         - Herramienta de empaquetado              ║"
echo "║   • SUPAPLEX.pak     - Archivo de recursos comprimido          ║"
echo "║   • supaplex         - Juego ejecutable                        ║"
echo "╠════════════════════════════════════════════════════════════════╣"
echo "║ Próximos pasos:                                                ║"
echo "║   1. ./supaplex              - Ejecutar juego                  ║"
echo "║   2. pak_tool list *.pak     - Verificar recursos              ║"
echo "║   3. pak_tool extract *.pak  - Extraer recursos si necesario   ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
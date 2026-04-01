#!/bin/bash

# ============================================================================
# BUILD TEST - Compilar test_pak_loader
# ============================================================================

set -e

CC=${CC:-gcc}
CFLAGS="-Wall -Wextra -std=c99 -O2"
LDFLAGS="-lm"

PAK_DIR="."
TARGET="test_pak_loader"

# Colores
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}▶ Compilando test_pak_loader...${NC}\n"

SOURCES="$PAK_DIR/test_pak_loader.c $PAK_DIR/resource_loader.c $PAK_DIR/supaplex_pak.c $PAK_DIR/crc32.c"

if $CC $CFLAGS -I"$PAK_DIR" -o "$TARGET" $SOURCES $LDFLAGS; then
    SIZE=$(du -h "$TARGET" | cut -f1)
    echo -e "\n${GREEN}✓ $TARGET compilado ($SIZE)${NC}\n"
    echo "Uso:"
    echo "  ./$TARGET              # Usa supaplex.pak por defecto"
    echo "  ./$TARGET otro.pak     # Especificar otro PAK"
else
    echo -e "\n${RED}✗ Error compilando${NC}\n"
    exit 1
fi
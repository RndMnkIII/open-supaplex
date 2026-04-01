#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pak_format.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <pak_file>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("❌ Error: no se puede abrir %s\n", argv[1]);
        return 1;
    }
    
    // Leer header
    SpaplexResourceHeader h;
    fread(&h, PAK_HEADER_SIZE, 1, f);
    
    printf("=== ANÁLISIS DEL PAK ===\n\n");
    printf("Header:\n");
    printf("  Magic: 0x%08x\n", h.magic);
    printf("  Version: %u\n", h.version);
    printf("  Num entries: %u\n", h.num_entries);
    printf("  Dir offset: %u (0x%x)\n", h.dir_offset, h.dir_offset);
    printf("  Total size: %u\n\n", h.total_size);
    
    // Validar tamaños
    printf("=== VALIDACIÓN DE TAMAÑOS ===\n");
    printf("  PAK_HEADER_SIZE: %zu bytes\n", PAK_HEADER_SIZE);
    printf("  PAK_ENTRY_SIZE: %zu bytes\n", PAK_ENTRY_SIZE);
    printf("  PAK_FILENAME_SIZE: %u bytes\n\n", PAK_FILENAME_SIZE);
    
    // Calcular estructura
    uint32_t expected_dir_size = h.num_entries * (uint32_t)PAK_ENTRY_SIZE;
    uint32_t expected_data_start = h.dir_offset + expected_dir_size;
    
    printf("Estructura esperada:\n");
    printf("  Header: 0 - %zu (size: %zu)\n", PAK_HEADER_SIZE - 1, PAK_HEADER_SIZE);
    printf("  Directorio: %u - %u (size: %u)\n", 
           h.dir_offset, h.dir_offset + expected_dir_size - 1, expected_dir_size);
    printf("  Datos: %u - %u\n", expected_data_start, h.total_size - 1);
    printf("\n");
    
    // Validaciones
    printf("=== VALIDACIONES ===\n");
    if (h.magic != PAK_MAGIC) {
        printf("  ❌ Magic inválido: 0x%08x (esperado 0x%08x)\n", h.magic, PAK_MAGIC);
        fclose(f);
        return 1;
    }
    printf("  ✓ Magic válido\n");
    
    if (h.dir_offset != PAK_HEADER_SIZE) {
        printf("  ⚠ Dir offset diferente de PAK_HEADER_SIZE\n");
        printf("    Esperado: %zu, Actual: %u\n", PAK_HEADER_SIZE, h.dir_offset);
    } else {
        printf("  ✓ Dir offset correcto\n");
    }
    
    if (expected_data_start != (uint32_t)(PAK_HEADER_SIZE + expected_dir_size)) {
        printf("  ⚠ Datos no comienzan donde se espera\n");
    } else {
        printf("  ✓ Estructura de offsets correcta\n");
    }
    printf("\n");
    
    // Leer primeras 5 entradas
    printf("=== PRIMERAS 5 ENTRADAS ===\n\n");
    for (int i = 0; i < 5 && i < (int)h.num_entries; i++) {
        SpaplexResourceEntry e;
        uint32_t entry_offset = h.dir_offset + (i * (uint32_t)PAK_ENTRY_SIZE);
        
        fseek(f, entry_offset, SEEK_SET);
        if (fread(&e, PAK_ENTRY_SIZE, 1, f) != 1) {
            printf("❌ Error leyendo entrada %d\n", i);
            continue;
        }
        
        printf("Entrada %d (offset 0x%x):\n", i, entry_offset);
        printf("  Filename: '%.60s'\n", e.filename);
        printf("  Offset: %u\n", e.offset);
        printf("  Size: %u\n", e.size);
        printf("  Compressed size: %u\n", e.compressed_size);
        printf("  CRC32: 0x%08x\n", e.crc32);
        printf("\n");
    }
    
    // Últimas 3 entradas
    printf("=== ÚLTIMAS 3 ENTRADAS ===\n\n");
    for (int i = h.num_entries - 3; i < (int)h.num_entries && i >= 0; i++) {
        SpaplexResourceEntry e;
        uint32_t entry_offset = h.dir_offset + (i * (uint32_t)PAK_ENTRY_SIZE);
        
        fseek(f, entry_offset, SEEK_SET);
        if (fread(&e, PAK_ENTRY_SIZE, 1, f) != 1) {
            printf("❌ Error leyendo entrada %d\n", i);
            continue;
        }
        
        printf("Entrada %d (offset 0x%x):\n", i, entry_offset);
        printf("  Filename: '%.60s'\n", e.filename);
        printf("  Offset: %u\n", e.offset);
        printf("  Size: %u\n", e.size);
        printf("  CRC32: 0x%08x\n", e.crc32);
        printf("\n");
    }
    
    fclose(f);
    return 0;
}
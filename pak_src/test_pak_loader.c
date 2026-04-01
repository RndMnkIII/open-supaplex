#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "pak_format.h"
#include "crc32.h"

int main(int argc, char *argv[]) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║       SUPAPLEX PAK LOADER TEST - VERIFICACIÓN REAL             ║\n");
    printf("║       Lee y verifica CADA archivo físicamente                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
    
    const char *pak_file = (argc > 1) ? argv[1] : "supaplex.pak";
    
    printf("📦 Archivo PAK: %s\n\n", pak_file);
    
    // ========================================================================
    // PASO 1: ABRIR Y LEER HEADER
    // ========================================================================
    
    printf("▶ PASO 1: Validando archivo PAK...\n");
    
    FILE *f = fopen(pak_file, "rb");
    if (!f) {
        fprintf(stderr, "❌ No se puede abrir: %s\n", pak_file);
        return 1;
    }
    
    // Obtener tamaño total del archivo
    fseek(f, 0, SEEK_END);
    uint32_t file_size = (uint32_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Leer header
    SpaplexResourceHeader header;
    if (fread(&header, PAK_HEADER_SIZE, 1, f) != 1) {
        fprintf(stderr, "❌ Error leyendo header\n");
        fclose(f);
        return 1;
    }
    
    printf("  ✓ Header leído\n");
    printf("    Magic: 0x%08x\n", header.magic);
    printf("    Version: %u\n", header.version);
    printf("    Entradas: %u\n", header.num_entries);
    printf("    Dir offset: %u\n", header.dir_offset);
    printf("    Total size: %u bytes (%.2f MB)\n",
           header.total_size, header.total_size / (1024.0 * 1024.0));
    
    // Validar magic
    if (header.magic != PAK_MAGIC) {
        fprintf(stderr, "❌ Magic inválido: 0x%08x (esperado 0x%08x)\n",
                header.magic, PAK_MAGIC);
        fclose(f);
        return 1;
    }
    
    printf("  ✓ Header válido\n\n");
    
    // ========================================================================
    // PASO 2: LEER Y VERIFICAR CADA ARCHIVO
    // ========================================================================
    
    printf("▶ PASO 2: Leyendo y verificando cada recurso...\n\n");
    
    printf("┌─────────────────────────────────┬────────────┬──────────┬──────────┐\n");
    printf("│ Recurso                         │ Tamaño     │ CRC32    │ Estado   │\n");
    printf("├─────────────────────────────────┼────────────┼──────────┼──────────┤\n");
    
    uint32_t successful = 0;
    uint32_t failed = 0;
    uint32_t total_loaded = 0;
    
    for (uint32_t i = 0; i < header.num_entries; i++) {
        // Leer entrada del directorio
        fseek(f, header.dir_offset + (i * PAK_ENTRY_SIZE), SEEK_SET);
        
        SpaplexResourceEntry entry;
        if (fread(&entry, PAK_ENTRY_SIZE, 1, f) != 1) {
            printf("│ %-33s │ %10s │ %8s │ ✗ READ   │\n", "???", "---", "---");
            failed++;
            continue;
        }
        
        // Validar entrada
        if (entry.size == 0 || entry.offset == 0) {
            printf("│ %-33s │ %10u │ %08x │ ✗ EMPTY  │\n",
                   entry.filename, entry.size, entry.crc32);
            failed++;
            continue;
        }
        
        // Validar que los datos están dentro del archivo
        if (entry.offset + entry.size > file_size) {
            printf("│ %-33s │ %10u │ %08x │ ✗ OOB    │\n",
                   entry.filename, entry.size, entry.crc32);
            failed++;
            continue;
        }
        
        // Leer datos del archivo
        uint8_t *data = malloc(entry.size);
        if (!data) {
            printf("│ %-33s │ %10u │ %08x │ ✗ MEM    │\n",
                   entry.filename, entry.size, entry.crc32);
            failed++;
            continue;
        }
        
        fseek(f, entry.offset, SEEK_SET);
        if (fread(data, entry.size, 1, f) != 1) {
            printf("│ %-33s │ %10u │ %08x │ ✗ READ   │\n",
                   entry.filename, entry.size, entry.crc32);
            failed++;
            free(data);
            continue;
        }
        
        // Calcular CRC32
        uint32_t calc_crc = crc32_calculate(data, entry.size);
        
        // Verificar
        const char *status = "✗ CRC!";
        if (calc_crc == entry.crc32) {
            status = "✓ OK";
            successful++;
            total_loaded += entry.size;
        } else {
            failed++;
        }
        
        printf("│ %-33s │ %10u │ %08x │ %s │\n",
               entry.filename, entry.size, entry.crc32, status);
        
        free(data);
    }
    
    printf("└─────────────────────────────────┴────────────┴──────────┴──────────┘\n");
    
    fclose(f);
    
    // ========================================================================
    // PASO 3: RESUMEN
    // ========================================================================
    
    printf("\n▶ RESUMEN FINAL:\n\n");
    printf("  Recursos totales: %u\n", header.num_entries);
    printf("  ✓ Verificados OK: %u\n", successful);
    if (failed > 0) {
        printf("  ✗ Fallos: %u\n", failed);
    }
    printf("  Datos leídos: %u bytes (%.2f MB)\n\n",
           total_loaded, total_loaded / (1024.0 * 1024.0));
    
    if (failed == 0 && successful == header.num_entries) {
        printf("  ✅ TODAS LAS PRUEBAS PASARON - PAK VÁLIDO\n\n");
        return 0;
    } else {
        printf("  ❌ ALGUNAS PRUEBAS FALLARON\n\n");
        return 1;
    }
}
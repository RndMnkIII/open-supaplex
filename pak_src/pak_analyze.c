#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PAK_MAGIC 0x4B415053
#define PAK_ENTRY_SIZE 148
#define PAK_FILENAME_SIZE 128

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t num_entries;
    uint32_t dir_offset;
    uint32_t total_size;
    uint32_t reserved[3];
} __attribute__((packed)) Header;

typedef struct {
    char filename[PAK_FILENAME_SIZE];
    uint32_t offset;
    uint32_t size;
    uint32_t compressed_size;
    uint32_t crc32;
    uint8_t compression_type;
    uint8_t reserved[3];
} __attribute__((packed)) Entry;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <pak_file>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Error: no se puede abrir %s\n", argv[1]);
        return 1;
    }
    
    // Leer header
    Header h;
    fread(&h, sizeof(h), 1, f);
    
    printf("=== ANÁLISIS DEL PAK ===\n\n");
    printf("Header:\n");
    printf("  Magic: 0x%08x\n", h.magic);
    printf("  Num entries: %u\n", h.num_entries);
    printf("  Dir offset: %u (0x%x)\n", h.dir_offset, h.dir_offset);
    printf("  Total size: %u\n\n", h.total_size);
    
    // Calcular dónde DEBERÍA estar el directorio
    uint32_t expected_data_start = sizeof(h);  // 32
    uint32_t expected_dir_start = h.dir_offset;
    uint32_t expected_dir_size = h.num_entries * PAK_ENTRY_SIZE;
    
    printf("Estructura esperada:\n");
    printf("  Header: 0 - %u (size: %zu)\n", sizeof(h)-1, sizeof(h));
    printf("  Datos: %u - %u\n", expected_data_start, expected_dir_start - 1);
    printf("  Directorio: %u - %u (size: %u)\n", 
           expected_dir_start, expected_dir_start + expected_dir_size - 1, expected_dir_size);
    printf("\n");
    
    // Leer primeras 3 entradas del directorio declarado
    printf("=== ENTRADAS DESDE OFFSET %u ===\n\n", h.dir_offset);
    
    for (int i = 0; i < 3 && i < (int)h.num_entries; i++) {
        fseek(f, h.dir_offset + (i * PAK_ENTRY_SIZE), SEEK_SET);
        Entry e;
        fread(&e, sizeof(e), 1, f);
        
        printf("Entrada %d (offset %u):\n", i, h.dir_offset + (i * PAK_ENTRY_SIZE));
        printf("  Filename: '%.50s'\n", e.filename);
        printf("  Offset: %u\n", e.offset);
        printf("  Size: %u\n", e.size);
        printf("  CRC32: 0x%08x\n", e.crc32);
        printf("\n");
    }
    
    // Buscar PATRONES de nombres válidos
    printf("=== BÚSQUEDA DE PATRONES ===\n\n");
    printf("Buscando strings que parezcan nombres de archivo...\n\n");
    
    // Leer todo el archivo
    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = malloc(file_size);
    fread(data, file_size, 1, f);
    
    // Buscar ".DAT" que es patrón común
    int found_count = 0;
    for (uint32_t i = 0; i < file_size - 4 && found_count < 10; i++) {
        if (data[i] == 'D' && data[i+1] == 'A' && data[i+2] == 'T') {
            // Retroceder para encontrar el inicio del nombre
            uint32_t start = i;
            while (start > 0 && data[start-1] >= 32 && data[start-1] < 127) {
                start--;
            }
            
            // Imprimir nombre encontrado
            printf("Encontrado en offset %u: '", start);
            for (uint32_t j = start; j <= i + 3 && j < file_size && data[j] >= 32 && data[j] < 127; j++) {
                printf("%c", data[j]);
            }
            printf("'\n");
            
            found_count++;
        }
    }
    
    printf("\n");
    
    // Verificar si el directorio está después de los datos
    printf("=== BÚSQUEDA ALTERNATIVA ===\n\n");
    printf("El directorio podría estar escrito DESPUÉS de todos los datos.\n");
    printf("Total size declarado: %u\n", h.total_size);
    printf("Si el directorio está al final:\n");
    printf("  Directorio comenzaría en: %u\n", h.total_size - expected_dir_size);
    printf("\n");
    
    // Leer desde esa posición
    uint32_t alt_dir_offset = h.total_size - expected_dir_size;
    printf("Leyendo entradas desde offset %u:\n\n", alt_dir_offset);
    
    for (int i = 0; i < 3 && i < (int)h.num_entries; i++) {
        if (alt_dir_offset + (i * PAK_ENTRY_SIZE) < file_size) {
            fseek(f, alt_dir_offset + (i * PAK_ENTRY_SIZE), SEEK_SET);
            Entry e;
            fread(&e, sizeof(e), 1, f);
            
            printf("Entrada %d (offset %u):\n", i, alt_dir_offset + (i * PAK_ENTRY_SIZE));
            printf("  Filename: '%.50s'\n", e.filename);
            printf("  Offset: %u\n", e.offset);
            printf("  Size: %u\n", e.size);
            printf("  CRC32: 0x%08x\n", e.crc32);
            printf("\n");
        }
    }
    
    free(data);
    fclose(f);
    return 0;
}

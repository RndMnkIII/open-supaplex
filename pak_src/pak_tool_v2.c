#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include "pak_format.h"
#include "crc32.h"

#ifdef _WIN32
    #include <windows.h>
    #define PATH_SEPARATOR "\\"
#else
    #include <unistd.h>
    #define PATH_SEPARATOR "/"
#endif

// ============================================================================
// ESTRUCTURAS
// ============================================================================

typedef struct {
    char filename[PAK_FILENAME_SIZE];
    char filepath[512];
    uint32_t size;
    uint32_t offset;
    uint32_t crc32;
} FileEntry;

typedef struct {
    FileEntry *entries;
    int count;
    int capacity;
} FileList;

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void print_header(void) {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║           SUPAPLEX RESOURCE PACKER v2.3 (CORREGIDO)            ║\n");
    printf("║           Pack/List/Extract PAK Archives                       ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n\n");
}

void print_usage(const char *program) {
    printf("USAGE:\n");
    printf("  %s create <source_dir> <output.pak>       - Crear PAK\n", program);
    printf("  %s list <input.pak>                       - Listar contenido\n", program);
    printf("  %s extract <input.pak> <output_dir>       - Extraer PAK\n", program);
    printf("  %s info <input.pak>                       - Información detallada\n\n", program);
}

// ============================================================================
// GESTIÓN DE LISTA DE ARCHIVOS
// ============================================================================

FileList* filelist_create(int capacity) {
    FileList *list = malloc(sizeof(FileList));
    if (!list) return NULL;
    list->entries = malloc(sizeof(FileEntry) * capacity);
    if (!list->entries) {
        free(list);
        return NULL;
    }
    list->count = 0;
    list->capacity = capacity;
    return list;
}

void filelist_destroy(FileList *list) {
    if (list) {
        free(list->entries);
        free(list);
    }
}

void build_relative_path(const char *base_dir, const char *full_path, char *relative) {
    if (!base_dir || !full_path || !relative) return;
    
    size_t base_len = strlen(base_dir);
    
    if (base_len > 0 && (base_dir[base_len - 1] == '/' || base_dir[base_len - 1] == '\\')) {
        base_len--;
    }
    
    const char *rel_start = full_path + base_len;
    if (*rel_start == '/' || *rel_start == '\\') {
        rel_start++;
    }
    
    strcpy(relative, rel_start);
    
    for (int i = 0; relative[i]; i++) {
        if (relative[i] == '\\') {
            relative[i] = '/';
        }
    }
}

int filelist_add(FileList *list, const char *relative_name, const char *filepath) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->entries = realloc(list->entries, sizeof(FileEntry) * list->capacity);
        if (!list->entries) {
            fprintf(stderr, "❌ Error: no hay memoria\n");
            return -1;
        }
    }
    
    FileEntry *entry = &list->entries[list->count];
    
    if (strlen(relative_name) >= PAK_FILENAME_SIZE) {
        fprintf(stderr, "⚠️  Advertencia: nombre demasiado largo: %s\n", relative_name);
        return -1;
    }
    
    strcpy(entry->filename, relative_name);
    strcpy(entry->filepath, filepath);
    
    struct stat stat_buf;
    if (stat(filepath, &stat_buf) != 0) {
        fprintf(stderr, "❌ Error stat() en %s: %s\n", filepath, strerror(errno));
        return -1;
    }
    
    if (stat_buf.st_size == 0) {
        fprintf(stderr, "⚠️  Archivo vacío: %s\n", relative_name);
    }
    
    if (stat_buf.st_size > 100000000) {
        fprintf(stderr, "⚠️  Archivo muy grande (%lu bytes): %s\n", 
                (unsigned long)stat_buf.st_size, relative_name);
    }
    
    entry->size = (uint32_t)stat_buf.st_size;
    entry->offset = 0;
    entry->crc32 = 0;
    
    list->count++;
    return 0;
}

int filelist_compare(const void *a, const void *b) {
    return strcmp(((FileEntry*)a)->filename, ((FileEntry*)b)->filename);
}

void filelist_sort(FileList *list) {
    qsort(list->entries, list->count, sizeof(FileEntry), filelist_compare);
}

// ============================================================================
// ESCANEO RECURSIVO DE DIRECTORIOS
// ============================================================================

int scan_directory_recursive(const char *dir_path, const char *base_dir, FileList *list) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        fprintf(stderr, "⚠️  No se puede abrir: %s\n", dir_path);
        return 0;
    }
    
    struct dirent *entry;
    int total_size = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s%s%s", dir_path, PATH_SEPARATOR, entry->d_name);
        
        struct stat stat_buf;
        if (stat(full_path, &stat_buf) != 0) {
            continue;
        }
        
        char relative_path[PAK_FILENAME_SIZE];
        build_relative_path(base_dir, full_path, relative_path);
        
        if (S_ISREG(stat_buf.st_mode)) {
            if (filelist_add(list, relative_path, full_path) == 0) {
                total_size += (int)stat_buf.st_size;
                printf("    ✓ %s (%u bytes)\n", relative_path, (unsigned)stat_buf.st_size);
            }
        }
        else if (S_ISDIR(stat_buf.st_mode)) {
            total_size += scan_directory_recursive(full_path, base_dir, list);
        }
    }
    
    closedir(dir);
    return total_size;
}

// ============================================================================
// LECTURA DE ARCHIVOS Y CÁLCULO DE CRC32
// ============================================================================

uint8_t* read_file_data(const char *filepath, uint32_t *out_size, uint32_t *out_crc32) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "❌ Error abriendo: %s (%s)\n", filepath, strerror(errno));
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fprintf(stderr, "❌ Archivo vacío o inválido: %s\n", filepath);
        fclose(file);
        return NULL;
    }
    
    if (file_size > 100000000) {
        fprintf(stderr, "❌ Archivo demasiado grande: %s (%ld bytes)\n", filepath, file_size);
        fclose(file);
        return NULL;
    }
    
    uint8_t *data = malloc(file_size);
    if (!data) {
        fprintf(stderr, "❌ Error de memoria al leer: %s\n", filepath);
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(data, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        fprintf(stderr, "❌ Error leyendo: %s (leídos %zu/%ld bytes)\n", 
                filepath, bytes_read, file_size);
        free(data);
        fclose(file);
        return NULL;
    }
    
    *out_crc32 = crc32_calculate(data, file_size);
    *out_size = (uint32_t)file_size;
    
    fclose(file);
    return data;
}

// ============================================================================
// CREAR PAK - VERSIÓN CORREGIDA
// ============================================================================
//
// ESTRUCTURA CORRECTA:
// [HEADER: 32 bytes] → [DIRECTORIO: n*148 bytes] → [DATOS: archivos]
// Offset 0-31          Offset 32 - (32+n*148)     Offset (32+n*148) - fin
//

int pak_create(const char *source_dir, const char *output_file) {
    print_header();
    
    printf("📂 Origen: %s\n", source_dir);
    printf("📦 Salida: %s\n\n", output_file);
    
    FileList *files = filelist_create(512);
    if (!files) {
        fprintf(stderr, "❌ Error creando lista\n");
        return -1;
    }
    
    printf("Escaneando directorio...\n");
    int total_content_size = scan_directory_recursive(source_dir, source_dir, files);
    
    if (files->count == 0) {
        fprintf(stderr, "❌ No se encontraron archivos\n");
        filelist_destroy(files);
        return -1;
    }
    
    printf("\n📊 Resumen:\n");
    printf("   Archivos: %d\n", files->count);
    printf("   Tamaño total: %d bytes (%.2f MB)\n\n", 
           total_content_size, total_content_size / (1024.0 * 1024.0));
    
    filelist_sort(files);
    
    FILE *pak = fopen(output_file, "wb");
    if (!pak) {
        fprintf(stderr, "❌ No se puede crear: %s\n", output_file);
        filelist_destroy(files);
        return -1;
    }
    
    // ✅ CORRECCIÓN: El directorio comienza DESPUÉS del header
    SpaplexResourceHeader header = {
        .magic = PAK_MAGIC,
        .version = PAK_VERSION,
        .num_entries = (uint32_t)files->count,
        .dir_offset = PAK_HEADER_SIZE,  // ✅ Directorio en offset 32
    };
    
    // Escribir placeholder del header
    fwrite(&header, PAK_HEADER_SIZE, 1, pak);
    if (ferror(pak)) {
        fprintf(stderr, "❌ Error escribiendo header\n");
        fclose(pak);
        filelist_destroy(files);
        return -1;
    }
    
    // Crear tabla de entradas
    SpaplexResourceEntry *entries = malloc(sizeof(SpaplexResourceEntry) * files->count);
    if (!entries) {
        fprintf(stderr, "❌ Error de memoria\n");
        fclose(pak);
        filelist_destroy(files);
        return -1;
    }
    
    printf("📝 Escribiendo estructura...\n\n");
    
    // ✅ CORRECCIÓN: Los datos comienzan DESPUÉS del header Y del directorio
    uint32_t current_offset = PAK_HEADER_SIZE + (files->count * PAK_ENTRY_SIZE);
    int files_written = 0;
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ Archivo                      │ Tamaño     │ CRC32        ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < files->count; i++) {
        FileEntry *file_entry = &files->entries[i];
        SpaplexResourceEntry *pak_entry = &entries[i];
        
        uint32_t size, crc32;
        uint8_t *data = read_file_data(file_entry->filepath, &size, &crc32);
        
        if (!data) {
            printf("║ ❌ ERROR: %-20s SKIPPED                            ║\n", 
                   file_entry->filename);
            memset(pak_entry, 0, sizeof(*pak_entry));
            strcpy(pak_entry->filename, file_entry->filename);
            continue;
        }
        
        // Escribir datos al PAK
        fseek(pak, current_offset, SEEK_SET);
        size_t bytes_written = fwrite(data, 1, size, pak);
        
        if (bytes_written != size) {
            fprintf(stderr, "❌ Error escribiendo datos: %s\n", file_entry->filename);
            free(data);
            continue;
        }
        
        // Crear entrada en directorio
        memset(pak_entry, 0, sizeof(*pak_entry));
        strcpy(pak_entry->filename, file_entry->filename);
        pak_entry->offset = current_offset;
        pak_entry->size = size;
        pak_entry->compressed_size = 0;
        pak_entry->crc32 = crc32;
        pak_entry->compression_type = 0;
        
        printf("║ %-29s │ %10u │ %08x     ║\n",
               file_entry->filename, size, crc32);
        
        current_offset += size;
        files_written++;
        free(data);
    }
    
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // ✅ Actualizar header con tamaño total real
    header.total_size = current_offset;
    
    // ✅ CORRECCIÓN: Escribir directorio DESPUÉS del header
    fseek(pak, PAK_HEADER_SIZE, SEEK_SET);
    for (int i = 0; i < files->count; i++) {
        size_t written = fwrite(&entries[i], PAK_ENTRY_SIZE, 1, pak);
        if (written != 1) {
            fprintf(stderr, "❌ Error escribiendo directorio\n");
            fclose(pak);
            free(entries);
            filelist_destroy(files);
            return -1;
        }
    }
    
    // Reescribir header actualizado
    fseek(pak, 0, SEEK_SET);
    fwrite(&header, PAK_HEADER_SIZE, 1, pak);
    
    fclose(pak);
    
    printf("✅ PAK creado exitosamente\n");
    printf("   Archivo: %s\n", output_file);
    printf("   Tamaño: %u bytes (%.2f MB)\n", header.total_size, header.total_size / (1024.0 * 1024.0));
    printf("   Estructura:\n");
    printf("     Header: 0 - 31 (%zu bytes)\n", PAK_HEADER_SIZE);
    printf("     Directorio: %u - %u (%u bytes)\n", 
           PAK_HEADER_SIZE, 
           PAK_HEADER_SIZE + (files->count * PAK_ENTRY_SIZE) - 1,
           files->count * PAK_ENTRY_SIZE);
    printf("     Datos: %u - %u\n", 
           PAK_HEADER_SIZE + (files->count * PAK_ENTRY_SIZE),
           header.total_size - 1);
    printf("   Archivos empaquetados: %d/%d\n\n", files_written, files->count);
    
    free(entries);
    filelist_destroy(files);
    return 0;
}

// ============================================================================
// LISTAR CONTENIDO
// ============================================================================

int pak_list(const char *pak_file) {
    print_header();
    printf("📦 Contenido: %s\n\n", pak_file);
    
    FILE *pak = fopen(pak_file, "rb");
    if (!pak) {
        perror("Error al abrir PAK");
        return -1;
    }
    
    SpaplexResourceHeader header;
    fread(&header, PAK_HEADER_SIZE, 1, pak);
    
    if (header.magic != PAK_MAGIC) {
        fprintf(stderr, "❌ Archivo no es un PAK válido\n");
        fclose(pak);
        return -1;
    }
    
    printf("📊 Info:\n");
    printf("   Archivos: %u\n", header.num_entries);
    printf("   Tama��o: %u bytes (%.2f MB)\n\n", 
           header.total_size, header.total_size / (1024.0 * 1024.0));
    
    fseek(pak, PAK_HEADER_SIZE, SEEK_SET);
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ Archivo                      │ Offset     │ Tamaño     ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    
    uint32_t total_data = 0;
    
    for (uint32_t i = 0; i < header.num_entries; i++) {
        SpaplexResourceEntry entry;
        fread(&entry, PAK_ENTRY_SIZE, 1, pak);
        
        printf("║ %-29s │ %10u │ %10u ║\n",
               entry.filename, entry.offset, entry.size);
        total_data += entry.size;
    }
    
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ Total: %u bytes (%.2f MB) %47s ║\n",
           total_data, total_data / (1024.0 * 1024.0), "");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    fclose(pak);
    return 0;
}

// ============================================================================
// EXTRAER PAK
// ============================================================================

int create_directories_recursive(const char *path) {
    char temp_path[512];
    strncpy(temp_path, path, sizeof(temp_path) - 1);
    temp_path[sizeof(temp_path) - 1] = '\0';
    
    for (char *p = temp_path + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char sep = *p;
            *p = '\0';
            
            #ifdef _WIN32
                mkdir(temp_path);
            #else
                mkdir(temp_path, 0755);
            #endif
            
            *p = sep;
        }
    }
    
    #ifdef _WIN32
        mkdir(temp_path);
    #else
        mkdir(temp_path, 0755);
    #endif
    
    return 0;
}

int pak_extract(const char *pak_file, const char *output_dir) {
    print_header();
    printf("📦 Extrayendo: %s\n", pak_file);
    printf("📁 Destino: %s\n\n", output_dir);
    
    FILE *pak = fopen(pak_file, "rb");
    if (!pak) {
        perror("Error al abrir PAK");
        return -1;
    }
    
    #ifdef _WIN32
        mkdir(output_dir);
    #else
        mkdir(output_dir, 0755);
    #endif
    
    SpaplexResourceHeader header;
    fread(&header, PAK_HEADER_SIZE, 1, pak);
    
    if (header.magic != PAK_MAGIC) {
        fprintf(stderr, "❌ No es un PAK válido\n");
        fclose(pak);
        return -1;
    }
    
    printf("📊 Extrayendo %u archivos...\n\n", header.num_entries);
    
    // Guardar TODAS las entradas del directorio
    SpaplexResourceEntry *entries = malloc(sizeof(SpaplexResourceEntry) * header.num_entries);
    if (!entries) {
        fprintf(stderr, "❌ Error de memoria\n");
        fclose(pak);
        return -1;
    }
    
    fseek(pak, PAK_HEADER_SIZE, SEEK_SET);
    for (uint32_t i = 0; i < header.num_entries; i++) {
        if (fread(&entries[i], PAK_ENTRY_SIZE, 1, pak) != 1) {
            fprintf(stderr, "❌ Error leyendo directorio en entrada %u\n", i);
            free(entries);
            fclose(pak);
            return -1;
        }
    }
    
    uint32_t extracted = 0, failed = 0;
    uint32_t total_extracted = 0;
    
    for (uint32_t i = 0; i < header.num_entries; i++) {
        SpaplexResourceEntry *entry = &entries[i];
        
        char output_path[512];
        snprintf(output_path, sizeof(output_path), "%s%s%s",
                 output_dir, PATH_SEPARATOR, entry->filename);
        
        #ifdef _WIN32
            for (int j = 0; output_path[j]; j++) {
                if (output_path[j] == '/') output_path[j] = '\\';
            }
        #else
            for (int j = 0; output_path[j]; j++) {
                if (output_path[j] == '\\') output_path[j] = '/';
            }
        #endif
        
        char dir_path[512];
        strcpy(dir_path, output_path);
        
        char *last_sep = NULL;
        for (int j = strlen(dir_path) - 1; j >= 0; j--) {
            if (dir_path[j] == '/' || dir_path[j] == '\\') {
                last_sep = &dir_path[j];
                break;
            }
        }
        
        if (last_sep) {
            *last_sep = '\0';
            create_directories_recursive(dir_path);
            *last_sep = PATH_SEPARATOR[0];
        }
        
        uint8_t *data = malloc(entry->size);
        if (!data) {
            fprintf(stderr, "❌ Error memoria: %s\n", entry->filename);
            failed++;
            continue;
        }
        
        fseek(pak, entry->offset, SEEK_SET);
        size_t bytes_read = fread(data, 1, entry->size, pak);
        
        if (bytes_read != entry->size) {
            fprintf(stderr, "❌ Error leyendo: %s (%zu/%u)\n",
                   entry->filename, bytes_read, entry->size);
            free(data);
            failed++;
            continue;
        }
        
        FILE *out = fopen(output_path, "wb");
        if (out) {
            size_t bytes_written = fwrite(data, 1, entry->size, out);
            fclose(out);
            
            if (bytes_written == entry->size) {
                printf("  ✓ %-40s (%u bytes)\n", entry->filename, entry->size);
                extracted++;
                total_extracted += entry->size;
            } else {
                fprintf(stderr, "❌ Error escribiendo: %s\n", entry->filename);
                failed++;
            }
        } else {
            fprintf(stderr, "❌ No se puede crear: %s\n", output_path);
            failed++;
        }
        
        free(data);
    }
    
    printf("\n✅ Completado:\n");
    printf("   Extraídos: %u\n", extracted);
    printf("   Bytes: %u (%.2f MB)\n", total_extracted, total_extracted / (1024.0 * 1024.0));
    if (failed > 0) {
        printf("   ⚠️  Errores: %u\n", failed);
    }
    
    free(entries);
    fclose(pak);
    return (failed == 0) ? 0 : -1;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_header();
        print_usage(argv[0]);
        return 1;
    }
    
    const char *command = argv[1];
    
    if (strcmp(command, "create") == 0) {
        if (argc < 4) {
            printf("❌ Uso: %s create <source_dir> <output.pak>\n", argv[0]);
            return 1;
        }
        return pak_create(argv[2], argv[3]);
    }
    else if (strcmp(command, "list") == 0) {
        if (argc < 3) {
            printf("❌ Uso: %s list <input.pak>\n", argv[0]);
            return 1;
        }
        return pak_list(argv[2]);
    }
    else if (strcmp(command, "extract") == 0) {
        if (argc < 4) {
            printf("❌ Uso: %s extract <input.pak> <output_dir>\n", argv[0]);
            return 1;
        }
        return pak_extract(argv[2], argv[3]);
    }
    else {
        printf("❌ Comando desconocido: %s\n\n", command);
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}
#include "resource_compat.h"
#include "supaplex_pak.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PAK_FILE* pak_fopen(const char *filename, const char *mode) {
    PAK_FILE *file = malloc(sizeof(PAK_FILE));
    if (!file) return NULL;
    
    // Intentar cargar del PAK
    uint32_t size;
    const void *data = pak_load_resource(filename, &size);
    
    if (data) {
        // Encontrado en PAK
        file->data = data;
        file->position = 0;
        file->size = size;
        file->is_pak = 1;
        return file;
    }
    
    // Fallback: intentar archivo tradicional
    FILE *traditional_file = fopen(filename, mode);
    if (traditional_file) {
        // Leer archivo completo en memoria
        fseek(traditional_file, 0, SEEK_END);
        size = (uint32_t)ftell(traditional_file);
        fseek(traditional_file, 0, SEEK_SET);
        
        void *data_copy = malloc(size);
        if (fread(data_copy, 1, size, traditional_file) != size) {
            free(data_copy);
            free(file);
            fclose(traditional_file);
            return NULL;
        }
        fclose(traditional_file);
        
        file->data = data_copy;
        file->position = 0;
        file->size = size;
        file->is_pak = 0;  // Fue leído de archivo tradicional
        return file;
    }
    
    free(file);
    return NULL;
}

size_t pak_fread(void *ptr, size_t size, size_t nmemb, PAK_FILE *stream) {
    if (!stream || !stream->data) {
        return 0;
    }
    
    size_t bytes_to_read = size * nmemb;
    size_t bytes_available = stream->size - stream->position;
    
    if (bytes_to_read > bytes_available) {
        bytes_to_read = bytes_available;
    }
    
    memcpy(ptr, (const char*)stream->data + stream->position, bytes_to_read);
    stream->position += bytes_to_read;
    
    return bytes_to_read / size;  // Retornar número de items leídos
}

int pak_fseek(PAK_FILE *stream, long offset, int whence) {
    if (!stream) {
        return -1;
    }
    
    uint32_t new_position = 0;
    
    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = stream->position + offset;
            break;
        case SEEK_END:
            new_position = stream->size + offset;
            break;
        default:
            return -1;
    }
    
    if (new_position > stream->size) {
        return -1;
    }
    
    stream->position = new_position;
    return 0;
}

long pak_ftell(PAK_FILE *stream) {
    if (!stream) {
        return -1L;
    }
    return (long)stream->position;
}

int pak_fclose(PAK_FILE *stream) {
    if (!stream) {
        return -1;
    }
    
    // Solo liberar memoria si no es PAK (porque PAK está en memoria global)
    if (!stream->is_pak) {
        free((void*)stream->data);
    }
    
    free(stream);
    return 0;
}
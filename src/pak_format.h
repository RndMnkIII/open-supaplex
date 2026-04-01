#ifndef PAK_FORMAT_H
#define PAK_FORMAT_H

#include <stdint.h>

#define PAK_MAGIC           0x4B415053  // "SPAK" little-endian
#define PAK_VERSION         2           // Version 2 with path support
#define PAK_MAX_ENTRIES     512         // Increased for subdirectories
#define PAK_FILENAME_SIZE   128         // Increased for full paths

#pragma pack(1)

typedef struct {
    uint32_t magic;           // "SPAK" (0x4B415053)
    uint32_t version;         // 2 (with paths)
    uint32_t num_entries;     // Number of files
    uint32_t dir_offset;      // Offset to directory table
    uint32_t total_size;      // Total PAK size
    uint32_t reserved[3];     // Future use
} SpaplexResourceHeader;

typedef struct {
    char filename[PAK_FILENAME_SIZE];  // Relative path: "audio/music.ogg"
    uint32_t offset;
    uint32_t size;
    uint32_t compressed_size;
    uint32_t crc32;
    uint8_t  compression_type;
    uint8_t  reserved[3];
} SpaplexResourceEntry;

#pragma pack()

#define PAK_HEADER_SIZE     sizeof(SpaplexResourceHeader)
#define PAK_ENTRY_SIZE      sizeof(SpaplexResourceEntry)

#endif

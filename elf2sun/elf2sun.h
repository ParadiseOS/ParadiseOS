#pragma once

#include <stdint.h>

typedef struct {
    uint32_t entry_point; // Address to Entry Point

    uint32_t text_size;       // Program text size
    unsigned char *text_data; // Program text data

    uint32_t data_size;       // Program data section size
    unsigned char *data_data; // Program data section data

    uint32_t rodata_size;       // Program read-only size
    unsigned char *rodata_data; // Program read-only data

    uint32_t bss_size; // Program bss size
} Program;

typedef struct {
    char name[16];        // Null Terminated string
    uint32_t offset;      // Offset from start of file
    uint32_t entry_point; // Address of Entry Point
    uint32_t text_size;   // Size of text section
    uint32_t data_size;   // Size of data section
    uint32_t rodata_size; // Size of read-only section
    uint32_t bss_size;    // Size of bss section
} TableEntry;
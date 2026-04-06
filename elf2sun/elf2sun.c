#define _XOPEN_SOURCE 500 // For pread
#include "elf2sun.h"
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Checks if a file is a valid elf exe
 * @param filename The name of the file
 * @return int 1 if valid, 0 if not
 */
int is_valid_elf_file(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror(filename);
        return 0;
    }

    unsigned char e_ident[EI_NIDENT];
    if (read(fd, e_ident, EI_NIDENT) != EI_NIDENT) {
        close(fd);
        fprintf(
            stderr, "%s: Not a valid ELF file (could not read header)\n",
            filename
        );
        return 0;
    }

    Elf32_Half e_type;
    if (read(fd, &e_type, sizeof(Elf32_Half)) != sizeof(Elf32_Half)) {
        close(fd);
        fprintf(
            stderr, "%s: Not a valid ELF file (could not read type)\n", filename
        );
        return 0;
    }

    Elf32_Half e_machine;
    if (read(fd, &e_machine, sizeof(Elf32_Half)) != sizeof(Elf32_Half)) {
        close(fd);
        fprintf(
            stderr, "%s: Not a valid ELF file (could not read machine)\n",
            filename
        );
        return 0;
    }

    close(fd);

    if (e_ident[EI_MAG0] != ELFMAG0 || e_ident[EI_MAG1] != ELFMAG1 ||
        e_ident[EI_MAG2] != ELFMAG2 || e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(
            stderr, "%s: Not a valid ELF file (incrrect magic bytes)\n",
            filename
        );
        return 0;
    }

    if (e_ident[EI_CLASS] != ELFCLASS32) {
        fprintf(stderr, "%s: Not a valid ELF file (not 32-bit)\n", filename);
        return 0;
    }

    if (e_machine != EM_386) {
        fprintf(stderr, "%s: Not a valid ELF file (not Intel x86)\n", filename);
        return 0;
    }

    return 1;
}

/**
 * @brief Create a program struct of a file
 * @param filename The name of the file
 * @return Program*
 */
Program *parse_program(const char *filename) {
    Program *program = (Program *) malloc(sizeof(Program));

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror(filename);
        return 0;
    }

    // Read header
    Elf32_Ehdr elf_header;
    pread(fd, &elf_header, sizeof(Elf32_Ehdr), 0); // Reads header

    // Read section headers
    Elf32_Shdr section_headers[elf_header.e_shnum];
    pread(
        fd, section_headers, sizeof(Elf32_Shdr) * elf_header.e_shnum,
        elf_header.e_shoff
    );

    // Read program headers
    Elf32_Phdr program_headers[elf_header.e_phnum];
    pread(
        fd, program_headers, sizeof(Elf32_Phdr) * elf_header.e_phnum,
        elf_header.e_phoff
    );

    // Read section header string table
    Elf32_Shdr sh_strtab = section_headers[elf_header.e_shstrndx];
    char *sh_strs = malloc(sh_strtab.sh_size);
    pread(fd, sh_strs, sh_strtab.sh_size, sh_strtab.sh_offset);

    // Set section sizes to 0 (incase program doesn't contain data)
    program->entry_point = elf_header.e_entry; // Expected to be _start
    program->text_size = 0;
    program->data_size = 0;
    program->rodata_size = 0;
    program->bss_size = 0;

    for (int i = 0; i < elf_header.e_shnum; i++) {
        const char *name = &sh_strs[section_headers[i].sh_name];

        if (strcmp(name, ".text") == 0) { // Text Section
            program->text_size = section_headers[i].sh_size;
            program->text_data = malloc(program->text_size);
            Elf32_Off text_offset = section_headers[i].sh_offset;
            pread(fd, program->text_data, program->text_size, text_offset);
        }
        else if (strcmp(name, ".data") == 0) { // Data Section
            program->data_size = section_headers[i].sh_size;
            program->data_data = malloc(program->data_size);
            Elf32_Off data_offset = section_headers[i].sh_offset;
            pread(fd, program->data_data, program->data_size, data_offset);
        }
        else if (strcmp(name, ".rodata") == 0) { // Read-only Section
            program->rodata_size = section_headers[i].sh_size;
            program->rodata_data = malloc(program->rodata_size);
            Elf32_Off rodata_offset = section_headers[i].sh_offset;
            pread(
                fd, program->rodata_data, program->rodata_size, rodata_offset
            );
        }
        else if (strcmp(name, ".bss") == 0) { // BSS Section
            program->bss_size = section_headers[i].sh_size;
        }
    }

    free(sh_strs);
    close(fd);

    return program;
}

int main(int argc, char **argv) {

    // Argument checking
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file1> [elf_file2] ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 257) {
        fprintf(stderr, "Too many arguments...\n");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        if (!is_valid_elf_file(argv[i])) {
            fprintf(stderr, "Error: %s is not a valid ELF file.\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    printf("Valid ELF Files...\n");

    int executable_count = argc - 1;
    unsigned char executable_count_byte = (unsigned char) executable_count;
    Program *program[executable_count];

    for (int i = 1; i < argc; i++) {
        printf("Collecting data on %s...\n", argv[i]);
        program[i - 1] = parse_program(argv[i]);
    }

#ifdef TESTING
    for (int i = 0; i < executable_count; i++) {
        printf("Info on file: %s\n", argv[i + 1]);
        printf("Entry Point- %x\n", program[i]->entry_point);
        printf("Text Size-   %x\n", program[i]->text_size);
        printf("ROdata Size- %x\n", program[i]->rodata_size);
        printf("Data Size-   %x\n", program[i]->data_size);
        printf("BSS Size-    %x\n", program[i]->bss_size);
        printf(".text section data");
        for (int j = 0; j < program[i]->text_size; j++) {
            if (j % 4 == 0)
                printf(" ");
            if (j % 16 == 0)
                printf("\n");
            printf("%02x", program[i]->text_data[j]);
        }
        printf("\n.rodata section data");
        for (int j = 0; j < program[i]->rodata_size; j++) {
            if (j % 4 == 0)
                printf(" ");
            if (j % 16 == 0)
                printf("\n");
            printf("%02x", program[i]->rodata_data[j]);
        }
        printf("\n.data section data");
        for (int j = 0; j < program[i]->data_size; j++) {
            if (j % 4 == 0)
                printf(" ");
            if (j % 16 == 0)
                printf("\n");
            printf("%02x", program[i]->data_data[j]);
        }
        printf("\n\n");
    }
#endif

    // Construct Table Entries
    uint32_t current_offset = 4 + (executable_count * sizeof(TableEntry));
    TableEntry *table_entry[executable_count];
    for (int i = 0; i < executable_count; i++) {
        table_entry[i] = (TableEntry *) malloc(sizeof(TableEntry));
        strncpy(
            table_entry[i]->name,
            strrchr(argv[i + 1], '/') ? strrchr(argv[i + 1], '/') + 1
                                      : argv[i + 1],
            15
        );
        table_entry[i]->name[15] = '\0';
        table_entry[i]->offset = current_offset;
        table_entry[i]->entry_point = program[i]->entry_point;
        table_entry[i]->text_size = program[i]->text_size;
        table_entry[i]->rodata_size = program[i]->rodata_size;
        table_entry[i]->data_size = program[i]->data_size;
        table_entry[i]->bss_size = program[i]->bss_size;
        current_offset += table_entry[i]->text_size +
                          table_entry[i]->rodata_size +
                          table_entry[i]->data_size;
    }

    printf("Constructing sun executable\n");
    int sun_fd = open("binary.sun", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    write(sun_fd, "SUN", 3);                     // Magic bytes
    write(sun_fd, &executable_count_byte, 1);    // Exe count
    for (int i = 0; i < executable_count; i++) { // Table Entry info
        write(sun_fd, table_entry[i]->name, 16);
        write(sun_fd, &table_entry[i]->offset, 4);
        write(sun_fd, &table_entry[i]->entry_point, 4);
        write(sun_fd, &table_entry[i]->text_size, 4);
        write(sun_fd, &table_entry[i]->rodata_size, 4);
        write(sun_fd, &table_entry[i]->data_size, 4);
        write(sun_fd, &table_entry[i]->bss_size, 4);
    }

    for (int i = 0; i < executable_count; i++) { // Section data
        if (program[i]->text_size > 0)
            write(sun_fd, program[i]->text_data, program[i]->text_size);
        if (program[i]->rodata_size > 0)
            write(sun_fd, program[i]->rodata_data, program[i]->rodata_size);
        if (program[i]->data_size > 0)
            write(sun_fd, program[i]->data_data, program[i]->data_size);
    }

    close(sun_fd);

    for (int i = 0; i < executable_count; i++) {
        if (program[i]->text_size > 0)
            free(program[i]->text_data);
        if (program[i]->rodata_size > 0)
            free(program[i]->rodata_data);
        if (program[i]->data_size > 0)
            free(program[i]->data_data);
        free(program[i]);
        free(table_entry[i]);
    }

    return EXIT_SUCCESS;
}
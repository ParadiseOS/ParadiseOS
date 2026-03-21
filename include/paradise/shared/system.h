typedef struct {
    u32 pid;
    u32 page_size;
    void *stack_top;
    u32 stack_size;
    void *heap_start; // Start of "Free" / Unmapped vaddr space
    u32 heap_pages;   // End of "Free" / Unmapped vaddr space
    void *sunfile;
} SystemInfo;

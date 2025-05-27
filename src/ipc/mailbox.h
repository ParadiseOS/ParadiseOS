#include "lib/types.h"

// The mailbox messaging IPC is described in ParadiseDocs
// Because memory is still being worked on these functions may change drastically
// All unused functions are marked as current development doesn't support them
// They are still implemented to showcase what we hope the expected end behavior to be

typedef struct {
    void* head;
    void* tail;
    u16 size;
    u16 capacity;
    void* next_page;
    void* prev_page;
    char data[4076];
} MailboxHead;

typedef struct {
    char reserved[12];
    void* next_page;
    void* prev_page;
    char data[4076];
} MailboxPage;

typedef struct {
    u16 pid;
    u8 message_size;
    char data[255];
} Mailbox_Message;

typedef MailboxHead Mailbox;

// Temporary mailbox initialization. Treats an address like a mailbox.
void Mailbox_Init_Temp(Mailbox* mailbox);

// DO NOT USE - Initializes a mailbox
Mailbox* Mailbox_Init();

// DO NOT USE - Frees a mailbox
void Mailbox_Free(MailboxHead* mailbox);

// DO NOT USE - Increases the size of the mailbox
int Mailbox_Grow(MailboxHead* mailbox);

// DO NOT USE - Adds a message to a mailbox
int receive_message(Mailbox* mailbox, Mailbox_Message* message);

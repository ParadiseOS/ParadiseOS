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
void mailbox_init_temp(Mailbox* mailbox);

// Sends a message to a mailbox
int send_message(MailboxHead* mailbox, u16 sender_pid, u8 data_size, const char* data);

/** Reads a message from a mailbox into str[258]
 *  Returns True if a message was read, otherwise FALSE
 */
bool read_message(MailboxHead* mailbox, char* str);
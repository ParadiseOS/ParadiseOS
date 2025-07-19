#include "lib/error.h"
#include "lib/types.h"
#include "memory/mem.h"

// The mailbox messaging IPC is described in ParadiseDocs
// Because memory is still being worked on these functions may change
// drastically All unused functions are marked as current development doesn't
// support them They are still implemented to showcase what we hope the expected
// end behavior to be

#define MAILBOX_DATA_SIZE 4076 // 4076 to keep Mailbox page aligned
#define MAX_MESSAGE_SIZE  255  // max u8 value

typedef struct {
    void *head;
    void *tail;
    u16 size;
    u16 capacity;
    void *next_page;
    void *prev_page;
    char data[MAILBOX_DATA_SIZE];
} MailboxHead;

typedef struct {
    char reserved[12];
    void *next_page;
    void *prev_page;
    char data[MAILBOX_DATA_SIZE];
} MailboxPage;

typedef struct {
    u16 pid;
    u8 message_size;
    char data[MAX_MESSAGE_SIZE];
} MailboxMessage;

typedef MailboxHead Mailbox;

_Static_assert(sizeof(MailboxHead) == sizeof(MailboxPage), "Mailbox size");
_Static_assert(sizeof(Mailbox) == PAGE_SIZE, "Mailbox size");

// Temporary mailbox initialization. Treats an address like a mailbox.
void mailbox_init_temp(Mailbox *mailbox);

// Sends a message to a mailbox
int send_message(Mailbox *mailbox, u16 sender_pid, u8 size, const char *data);

/** Reads a message from a mailbox into str[258]
 *  Returns True if a message was read, otherwise FALSE
 */
bool read_message(MailboxHead *mailbox, char *str);

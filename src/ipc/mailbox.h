#ifndef MAILBOX_H_
#define MAILBOX_H_

#include "lib/error.h"
#include "lib/types.h"
#include "memory/mem.h"

// The mailbox messaging IPC is described in ParadiseDocs
// Because ipc is still being worked on these functions may change drastically

#define MAILBOX_DATA_SIZE PAGE_SIZE
#define MAX_MESSAGE_SIZE  255

// Send Message Syscall Flags
#define IPC_SIGNAL (1 << 0)

// Read Message Syscall Flags
#define IPC_BLOCKING (1 << 0)

typedef struct {
    void *head;
    void *tail;
    u16 unread_size;
    u16 used_size;
    u16 capacity;
    void *first_page;
    void *last_page;
    void *copy_page;
    // MailboxPage Structure should always be contigious.
} MailboxHeader;

typedef struct {
    char data[MAILBOX_DATA_SIZE];
} MailboxPage;

typedef struct __attribute__((packed)) {
    u32 sender_pid;
    u32 reader_pid;
    u8 data_size;
} MailboxMessageHeader;

typedef struct __attribute__((packed)) {
    MailboxMessageHeader header;
    char data[MAX_MESSAGE_SIZE];
} MailboxMessage;

_Static_assert(sizeof(MailboxPage) == PAGE_SIZE, "Mailbox size");

void mailbox_init(
    MailboxHeader *mailbox, void *mailbox_start_addr, u16 page_flags
);

void mailbox_del(MailboxHeader *mailbox);

// Sends a message to a mailbox
int mailbox_send_message(
    MailboxHeader *mailbox, u32 sender_pid, u32 reader_pid, u8 data_size,
    const void *data
);

// Reads messages from sender & reader of mailbox
int mailbox_read_message(
    MailboxHeader *mailbox, u32 sender_pid, u32 reader_pid,
    MailboxMessage *message
);

// sends a signal to a process
int send_signal(u8 signal_num);

#endif // MAILBOX_H_

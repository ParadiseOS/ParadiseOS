#ifndef PMAIL_H_
#define PMAIL_H_

#include "types.h"

#define MAX_MESSAGE_SIZE 255

// Send Message Syscall Flags
#define IPC_SIGNAL (1 << 0)

// Read Message Syscall Flags
#define IPC_BLOCKING (1 << 0)

typedef struct __attribute__((packed)) {
    u32 sender_pid;
    u32 reader_pid;
    u8 data_size;
} MailboxMessageHeader;

typedef struct __attribute__((packed)) {
    MailboxMessageHeader header;
    char data[MAX_MESSAGE_SIZE];
} MailboxMessage;

// Sends a message to another processes mailbox
void send_message(u32 reader_pid, u32 data_size, const char *data, u32 flags);

// Reads a message from own mailbox.
// Returns 1 if a message is present, otherwise 0
u32 read_message(
    u32 sender_pid, u32 reader_pid, MailboxMessage *message, u32 flags
);

#endif // PMAIL_H_
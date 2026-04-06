#include "pmail.h"
#include "psyscall.h"

// Sends a message to another processes mailbox
void send_message(u32 reader_pid, u32 data_size, const char *data, u32 flags) {
    psyscall(SYSCALL_SEND_MESSAGE, reader_pid, data_size, (u32) data, flags, 0);
}

// Reads a message from own mailbox.
// Returns 1 if a message is present, otherwise 0
u32 read_message(
    u32 sender_pid, u32 reader_pid, MailboxMessage *message, u32 flags
) {
    return psyscall(
        SYSCALL_READ_MESSAGE, sender_pid, reader_pid, (u32) message, flags, 0
    ).ret;
}

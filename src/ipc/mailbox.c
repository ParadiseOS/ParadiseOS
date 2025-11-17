#include "ipc/mailbox.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "lib/logging.h"
#include "lib/util.h"
#include "mailbox.h"
#include "memory/mem.h"

void *page_ptr(void *addr) {
    return (void *) (((u32) addr >> 12) << 12 /* Page Size Bits */);
}

// Links tgt page to src physical frame
void link_page(void *tgt, void *src) {
    KERNEL_ASSERT(
        !map_page(tgt, get_paddr(get_entry(src)), get_flags(get_entry(src)))
    );
}

static void bytes_to_message_header(
    MailboxMessageHeader *message_header, const void *data
) {
    pmemcpy(message_header, data, sizeof(MailboxMessageHeader));
}

static void bytes_to_message(MailboxMessage *message, const char *data) {
    pmemcpy(&message->header, data, sizeof(MailboxMessageHeader));
    pmemcpy(
        &message->data, data + sizeof(MailboxMessageHeader),
        message->header.data_size
    );
}

// Cleans up read messages from mailbox
u16 message_cleanup(MailboxHeader *mailbox) {
    u16 cleaned_up_bytes = 0;
    u16 seen_size = 0;
    MailboxMessageHeader header;
    void *tmp_header = mailbox->head;
    bytes_to_message_header(&header, tmp_header);
    while (header.sender_pid == 0 && header.reader_pid == 0 &&
           seen_size < mailbox->used_size) {
        cleaned_up_bytes += sizeof(MailboxMessageHeader) + header.data_size;
        seen_size += cleaned_up_bytes;
        tmp_header += cleaned_up_bytes;
        bytes_to_message_header(&header, tmp_header);
    }
    return cleaned_up_bytes;
}

void mailbox_init(
    MailboxHeader *mailbox, void *mailbox_start_addr, u16 page_flags
) {
    mailbox->capacity = MAILBOX_DATA_SIZE;
    mailbox->unread_size = 0;
    mailbox->used_size = 0;
    mailbox->head = mailbox_start_addr;
    mailbox->tail = mailbox_start_addr;
    mailbox->first_page = mailbox_start_addr;
    mailbox->last_page = mailbox_start_addr;
    mailbox->copy_page = mailbox->last_page + PAGE_SIZE;
    alloc_page(mailbox_start_addr, page_flags);
    link_page(mailbox->copy_page, mailbox->first_page);
}

// todo implement
void mailbox_del(MailboxHeader *mailbox) {
    (void) mailbox;
    // Should unmap and deallocate all mailbox pages for the given mailbox.
}

// todo implement
bool mailbox_grow(MailboxHeader *mailbox) {
    (void) mailbox;

    // potential implementation
    // unmap copy page
    // alloc new page at that address
    // map next copy page to the following page
    // increase capacities and such

    return false; //* no-op
}

int mailbox_send_message(
    MailboxHeader *mailbox, u32 sender_pid, u32 reader_pid, u8 data_size,
    const void *data
) {
    MailboxMessageHeader message_header;
    message_header.sender_pid = sender_pid;
    message_header.reader_pid = reader_pid;
    message_header.data_size = data_size;
    u32 message_header_size = sizeof(message_header);

    // Resize mailbox if message is too large
    if (mailbox->used_size + message_header_size + data_size >
        mailbox->capacity) {
        bool mailbox_grew = mailbox_grow(mailbox);
        if (!mailbox_grew)
            return false; // Mailbox at max capacity.
    }

    pmemcpy(mailbox->tail, &message_header, message_header_size);
    mailbox->tail += message_header_size;
    pmemcpy(mailbox->tail, data, data_size);
    mailbox->tail += data_size;

    // Ensure tail is never on copypage
    if (page_ptr(mailbox->tail) == mailbox->copy_page) {
        u32 page_offset = ((u32) mailbox->tail << 20) >> 20;
        mailbox->tail = mailbox->first_page + page_offset;
    }
    mailbox->used_size += message_header_size + data_size;
    return true;
}

bool match_pid(u32 field_pid, u32 target_pid) {
    if (field_pid == 0)
        return false;
    if (target_pid == 0)
        return true;
    if (get_pid_tid(field_pid) == 0)
        return get_pid_aid(field_pid) == get_pid_aid(target_pid);
    return field_pid == target_pid;
}

int mailbox_read_message(
    MailboxHeader *mailbox, u32 sender_pid, u32 reader_pid,
    MailboxMessage *message
) {
    // Search for message
    u16 seen_size = 0;
    char *current_message = mailbox->head;
    u16 total_message_size = 0;
    while (seen_size < mailbox->used_size) {
        bytes_to_message_header(&message->header, current_message);
        if (match_pid(message->header.sender_pid, sender_pid) &&
            match_pid(message->header.reader_pid, reader_pid))
            break;
        total_message_size =
            sizeof(MailboxMessageHeader) + message->header.data_size;
        seen_size += total_message_size;
        current_message += total_message_size;
    }

    if (seen_size >= mailbox->used_size)
        return 0;

    // Search read the message into message
    bytes_to_message(message, current_message);
    mailbox->unread_size -= total_message_size;
    pmemset(current_message, 0, 8);

    // Cleanup read messages
    u16 clean_up_size = message_cleanup(mailbox);
    mailbox->used_size -= clean_up_size;
    mailbox->head += clean_up_size;
    printk(DEBUG, "Cleanup size - %u\n", clean_up_size);

    // Ensure head is never on copy page
    if (page_ptr(mailbox->head) == mailbox->copy_page) {
        u32 page_offset = ((u32) mailbox->head << 20) >> 20;
        mailbox->head = mailbox->first_page + page_offset;
    }

    return 1;
}

int send_signal(u8 signal_num) {
    printk(DEBUG, "signal %u received...\n", signal_num);
    return 0; // No-op for now
}
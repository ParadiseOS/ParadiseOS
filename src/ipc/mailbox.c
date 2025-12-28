#include <paradise/error.h>
#include <paradise/libp.h>
#include <paradise/logging.h>
#include <paradise/mailbox.h>
#include <paradise/mem.h>
#include <paradise/util.h>

void *page_ptr(void *addr) {
    return (void *) ((u32) addr & 0xFFFFF000); // Mask out Page Offset
}

// Links tgt page to src physical frame
void link_page(void *tgt, void *src) {
    KERNEL_ASSERT(
        !map_page(tgt, get_paddr(get_entry(src)), get_flags(get_entry(src)))
    );
}

// Returns pointer to next page of page_ptr (for readability)
static void *next_page(void *page_ptr) {
    return page_ptr + PAGE_SIZE;
}

// Returns pointer to next n pages of page_ptr
static void *next_n_pages(void *page_ptr, u32 n) {
    return page_ptr + (n * PAGE_SIZE);
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
        u16 read_bytes = sizeof(MailboxMessageHeader) + header.data_size;
        cleaned_up_bytes += read_bytes;
        seen_size += read_bytes;
        tmp_header += read_bytes;
        // Ensure tmp header is never on link page
        if (page_ptr(tmp_header) == mailbox->link_page) {
            u32 page_offset = (u32) tmp_header % PAGE_SIZE;
            tmp_header = mailbox->first_page + page_offset;
        }
        bytes_to_message_header(&header, tmp_header);
    }
    return cleaned_up_bytes;
}

void mailbox_init(
    MailboxHeader *mailbox, void *mailbox_start_addr, u16 page_flags
) {
    mailbox->capacity = MAILBOX_DATA_SIZE - 1; // Account for overflow
    mailbox->unread_size = 0;
    mailbox->used_size = 0;
    mailbox->head = mailbox_start_addr;
    mailbox->tail = mailbox_start_addr;
    mailbox->first_page = mailbox_start_addr;
    mailbox->last_page = mailbox_start_addr;
    mailbox->link_page = mailbox->last_page + PAGE_SIZE;
    alloc_page(mailbox_start_addr, page_flags);
    link_page(mailbox->link_page, mailbox->first_page);
}

void mailbox_del(MailboxHeader *mailbox) {
    // Unmap link Page
    KERNEL_ASSERT(!unmap_page(mailbox->link_page, NULL));

    // Free frames of data pages
    u32 mailbox_page_count = mailbox->capacity / PAGE_SIZE;
    free_pages(mailbox->first_page, mailbox_page_count);
}

// Grows the mailbox by inserting a new page after tail.
bool mailbox_grow(MailboxHeader *mailbox) {
    if (mailbox->capacity == U16_MAX)
        return false;

    // Allocate new page
    u16 page_flags = get_flags(get_entry(mailbox->first_page));
    alloc_page(next_page(mailbox->link_page), page_flags);

    u32 pages_shifted =
        (page_ptr(mailbox->link_page) - page_ptr(mailbox->tail)) / PAGE_SIZE;
    while (pages_shifted) {
        void *prev = next_n_pages(mailbox->tail, pages_shifted);
        void *new = next_n_pages(mailbox->tail, pages_shifted + 1);
        swap_page_frames(prev, new);
        pages_shifted--;
    }

    // Shift over pointers
    mailbox->link_page = next_page(mailbox->link_page);
    mailbox->last_page = next_page(mailbox->last_page);

    // Head is shifted over
    if (mailbox->head > mailbox->tail) {
        if (page_ptr(mailbox->head) == page_ptr(mailbox->tail)) {
            // If head and tail are on the same page, copy head data to new page
            u32 head_page_offset = (u32) mailbox->head % PAGE_SIZE;
            pmemcpy(
                next_page(mailbox->head), mailbox->head,
                PAGE_SIZE - head_page_offset
            );
        }
        mailbox->head = next_page(mailbox->head);
    }

    mailbox->capacity += MAILBOX_DATA_SIZE;
    return true;
}

i32 mailbox_send_message(
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
        if (!mailbox_grow(mailbox))
            return ERR_FULL_MAILBOX; // Mailbox at max capacity.
    }

    pmemcpy(mailbox->tail, &message_header, message_header_size);
    mailbox->tail += message_header_size;
    pmemcpy(mailbox->tail, data, data_size);
    mailbox->tail += data_size;

    // Ensure tail is never on copypage
    if (page_ptr(mailbox->tail) == mailbox->link_page) {
        u32 page_offset = (u32) mailbox->tail % PAGE_SIZE;
        mailbox->tail = mailbox->first_page + page_offset;
    }
    mailbox->used_size += message_header_size + data_size;
    mailbox->unread_size += message_header_size + data_size;
    return data_size;
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

i32 mailbox_read_message(
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
        return ERR_NO_MESSAGE;

    // Pass found message to pointer
    bytes_to_message(message, current_message);
    mailbox->unread_size -= total_message_size;
    pmemset(current_message, 0, 8); // clear pids to set as read

    // Cleanup read messages
    u16 clean_up_size = message_cleanup(mailbox);
    mailbox->used_size -= clean_up_size;
    mailbox->head += clean_up_size;

    // Ensure head is never on copy page
    if (page_ptr(mailbox->head) == mailbox->link_page) {
        u32 page_offset = (u32) mailbox->head % PAGE_SIZE;
        mailbox->head = mailbox->first_page + page_offset;
    }

    return message->header.data_size;
}

i32 send_signal(u8 signal_num) {
    printk(DEBUG, "signal %u received...\n", signal_num);
    return 0; // No-op for now
}

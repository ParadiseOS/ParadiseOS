#include "ipc/mailbox.h"
#include "lib/error.h"
#include "lib/libp.h"
#include "memory/mem.h"

void *page_ptr(void *addr) {
    return (void *) ((u32) addr / PAGE_SIZE * PAGE_SIZE);
}

void mailbox_init_temp(Mailbox *mailbox) {
    KERNEL_ASSERT(is_page_aligned(mailbox));
    mailbox->head = &mailbox->data;
    mailbox->tail = &mailbox->data;
    mailbox->size = 0;
    mailbox->capacity = sizeof(mailbox->data);
    mailbox->next_page = mailbox;
    mailbox->prev_page = mailbox;
}

Mailbox *mailbox_init() {
    Mailbox *mailbox = kernel_alloc(1);
    mailbox->head = &mailbox->data;
    mailbox->tail = &mailbox->data;
    mailbox->size = 0;
    mailbox->capacity = sizeof(mailbox->data);
    mailbox->next_page = mailbox;
    mailbox->prev_page = mailbox;
    return mailbox;
}

void mailbox_free(MailboxHead *mailbox) {
    MailboxPage *page = mailbox->next_page;
    MailboxPage *next_page;
    while (page != (MailboxPage *) mailbox) {
        next_page = page->next_page;
        kernel_free(page);
        page = next_page;
    }
    kernel_free(mailbox);
}

// No-op for now
int mailbox_grow(MailboxHead *mailbox) {
    return 1;
    if (mailbox->capacity >= (PAGE_SIZE - 20) * 16)
        return 1; // Mailbox is at max size

    Mailbox *current_mailbox = (Mailbox *) mailbox->tail;
    Mailbox *old_next_mailbox = (Mailbox *) current_mailbox->next_page;
    Mailbox *new_mailbox = (Mailbox *) kernel_alloc(1);

    // Copy over data from head to bottom of page
    if ((page_ptr(mailbox->head) == page_ptr(mailbox->tail)) &&
        (mailbox->tail <= mailbox->head)) {
        u32 head_size_on_page =
            ((u32) current_mailbox + PAGE_SIZE) - ((u32) mailbox->head);
        void *head_on_new_page =
            (void *) (((u32) new_mailbox + PAGE_SIZE) - head_size_on_page);
        pmemcpy(head_on_new_page, mailbox->head, head_size_on_page);
        mailbox->head = head_on_new_page;
    }
    else { // When tail isn't behind head
        current_mailbox = (Mailbox *) page_ptr(mailbox->tail);
        old_next_mailbox = (Mailbox *) current_mailbox->next_page;
    }

    current_mailbox->next_page = new_mailbox;
    old_next_mailbox->prev_page = new_mailbox;
    new_mailbox->prev_page = current_mailbox;
    new_mailbox->next_page = old_next_mailbox;

    mailbox->capacity += (PAGE_SIZE - 20);
    return 0;
}

int receive_message(Mailbox *mailbox, MailboxMessage *message) {
    u16 message_pid = message->pid;
    u8 message_size = message->message_size;

    // Resize mailbox if message is too large
    if (mailbox->size + message_size + 3 > mailbox->capacity) {
        terminal_printf("mailbox size - %i\n", mailbox->size);
        int growth_status = mailbox_grow(mailbox);
        if (growth_status)
            return 1; // Mailbox is at max capacity. This will eventually be
                      // propagated through the syscall
    }

    Mailbox *current_mailbox = (Mailbox *) page_ptr(mailbox->tail);
    u32 space_on_page =
        ((u32) current_mailbox + PAGE_SIZE) - ((u32) mailbox->tail);

    if (space_on_page <
        3) { // Ensure that PID and data size are always on the same page
        mailbox->size += space_on_page;
        current_mailbox = (Mailbox *) current_mailbox->next_page;
        mailbox->tail = &current_mailbox->data;
        space_on_page = 258;
    }

    *((char *) mailbox->tail++) = message_pid & 0xFF;
    *((char *) mailbox->tail++) = (message_pid >> 8) & 0xFF;
    *((char *) mailbox->tail++) = message_size;
    space_on_page -= 3;

    if (space_on_page < message_size) { // If entire message can't fit on page
        pmemcpy(mailbox->tail, message->data, space_on_page);
        mailbox->tail = &((MailboxPage *) current_mailbox->next_page)->data;
        u32 remaining_space = message_size - space_on_page;
        pmemcpy(
            mailbox->tail, (message + space_on_page)->data, remaining_space
        );
        mailbox->tail += remaining_space;
    }
    else { // If whole message can fit on page
        pmemcpy(mailbox->tail, message->data, message_size);
        mailbox->tail += message_size;
    }

    mailbox->size += message_size + 3;
    return 0;
}

int send_message(Mailbox *mailbox, u16 sender_pid, u8 size, const char *data) {
    MailboxMessage message;
    message.pid = sender_pid;
    message.message_size = size;
    pmemcpy(message.data, data, size);
    int status = receive_message(mailbox, &message);
    return status;
}

bool read_message(MailboxHead *mailbox, char *str) {
    if (mailbox->size == 0) {
        return false; // Empty mailbox
    }

    Mailbox *current_mailbox = (Mailbox *) page_ptr(mailbox->head);
    u32 space_on_page =
        ((u32) current_mailbox + PAGE_SIZE) - ((u32) mailbox->head);

    if (space_on_page < 3) { // Ensure that PID and size are always on same page
        mailbox->size -= space_on_page;
        current_mailbox = (Mailbox *) current_mailbox->next_page;
        mailbox->head = &current_mailbox->data;
        space_on_page = 258;
    }

    u16 pid = *(u16 *) (mailbox->head);
    mailbox->head += 2;
    u8 message_size = *(u8 *) (mailbox->head);
    mailbox->head += 1;

    *(str++) = pid & 0xFF;
    *(str++) = (pid >> 8) & 0xFF;
    *(str++) = message_size;

    space_on_page -= 3;

    if (space_on_page < message_size) { // If entire message isn't on page
        pmemcpy(str, mailbox->head, space_on_page);
        mailbox->head = &((MailboxPage *) current_mailbox->next_page)->data;
        u32 remaining_space = message_size - space_on_page;
        pmemcpy(str, mailbox->head, remaining_space);
        mailbox->head += remaining_space;
    }
    else { // If entire message is on the same page
        pmemcpy(str, mailbox->head, message_size);
        mailbox->head += message_size;
    }

    mailbox->size -= (message_size + 3);
    return true;
}

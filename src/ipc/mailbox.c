#include "ipc/mailbox.h"
#include "memory/mem.h"
#include "lib/error.h"

#define PAGE_SIZE 4096

void* page_ptr(void* addr) {
    return (void*)((((u32)addr + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE - PAGE_SIZE);
}

void Mailbox_Init_Temp(Mailbox* mailbox) {
    KERNEL_ASSERT(is_page_aligned(mailbox));
    mailbox = kernel_alloc(1);
    mailbox->head = &mailbox->data;
    mailbox->tail = &mailbox->data;
    mailbox->size = 0;
    mailbox->capacity = (PAGE_SIZE-20);
    mailbox->next_page = mailbox;
    mailbox->prev_page = mailbox;
}

Mailbox* Mailbox_Init() {
    Mailbox* mailbox = kernel_alloc(1);
    mailbox->head = &mailbox->data;
    mailbox->tail = &mailbox->data;
    mailbox->size = 0;
    mailbox->capacity = (PAGE_SIZE-20);
    mailbox->next_page = mailbox;
    mailbox->prev_page = mailbox;
    return mailbox;
}

void Mailbox_Free(MailboxHead* mailbox) {
    MailboxPage* page = mailbox->next_page;
    MailboxPage* next_page;
    while (page != (MailboxPage*) mailbox) {
        next_page = page->next_page;
        kernel_free(page);
        page = next_page;
    }
    kernel_free(mailbox);
}

int Mailbox_Grow(MailboxHead* mailbox) {
    if (mailbox->capacity >= (PAGE_SIZE-20) * 16)
        return 1; // Mailbox is at max size

    Mailbox* current_mailbox = (Mailbox*)mailbox->tail;
    Mailbox* old_next_mailbox = (Mailbox*)current_mailbox->next_page;
    Mailbox* new_mailbox = (Mailbox*)kernel_alloc(1);

    // Copy over data from head to bottom of page
    if ((page_ptr(mailbox->head) == page_ptr(mailbox->tail)) && (mailbox->tail <= mailbox->head)) {
        u32 head_size_on_page = ((u32)current_mailbox + PAGE_SIZE) - ((u32)mailbox->head);
        void* head_on_new_page = (void*)(((u32)new_mailbox + PAGE_SIZE) - head_size_on_page);
        for (u32 i = 0; i < head_size_on_page; i++) {
            ((char*)head_on_new_page)[i] = ((char*)mailbox->head)[i];
        }
        mailbox->head = head_on_new_page;
    } else { // When tail isn't behind head
        current_mailbox = (Mailbox*)page_ptr(mailbox->tail);
        old_next_mailbox = (Mailbox*)current_mailbox->next_page;
    }

    current_mailbox->next_page = new_mailbox;
    old_next_mailbox->prev_page = new_mailbox;
    new_mailbox->prev_page = current_mailbox;
    new_mailbox->next_page = old_next_mailbox;

    mailbox->capacity += (PAGE_SIZE-20);
    return 0;
}

int receive_message(Mailbox* mailbox, Mailbox_Message message) {
    u16 message_pid = message.pid;
    u8 message_size = message.message_size;

    // Resize mailbox if message is too large
    if (mailbox->size + message_size + 3 > mailbox->capacity) { // See below space on page conditional for explanation of "+3"
        int growth_status = Mailbox_Grow(mailbox);
        if (growth_status)
            return 1; // Mailbox is at max capacity. This will eventually be propagated through the syscall
    }

    Mailbox* current_mailbox = (Mailbox*)page_ptr(mailbox->tail);
    u32 space_on_page = ((u32)current_mailbox + PAGE_SIZE) - ((u32)mailbox->tail);

    if (space_on_page < 3) { // Ensure that PID and data size are always on the same page
        mailbox->size += space_on_page;
        current_mailbox = (Mailbox*)current_mailbox->next_page;
        mailbox->tail = &current_mailbox->data;
        space_on_page = 258;
    }

    *((char*)mailbox->tail++) = message_pid & 0xFF;
    *((char*)mailbox->tail++) = (message_pid >> 8) & 0xFF;
    *((char*)mailbox->tail++) = message_size;
    space_on_page -= 3;

    if (space_on_page < message_size) { // If entire message can't fit on page
        for (u8 i = 0; i < space_on_page; i++) {
            *((char*)mailbox->tail++) = message.data[i];
        }
        mailbox->tail = &((MailboxPage*)current_mailbox->next_page)->data;
        u32 remaining_space = message_size - space_on_page;
        for (u8 i = 0; i < remaining_space; i++) {
            *((char*)mailbox->tail++) = message.data[space_on_page + i];
        }
    } else { // If whole message can fit on page
        for (u8 i = 0; i < message_size; i++) {
            *((char*)mailbox->tail++) = message.data[i];
        }
    }

    mailbox->size += message_size + 3;
    return 0;
}

int send_message(MailboxHead* mailbox, u16 sender_pid, u8 data_size, const char* data) {
    Mailbox_Message message;
    message.pid = sender_pid;
    message.message_size = data_size;
    for (int i = 0; i < data_size; i++) {
        message.data[i] = data[i];
    }
    int status = receive_message(mailbox, message);
    return status;
}

// Need to implement send message and read message next.
// Both of these have some code but i'll need to refer to the notion on how to alter them so they use the syscalls and stuff
// I also need to figure out blocking...
#import "template.typ": *

#show: project.with(
  title: "Paradise OS",
  authors: (
    "Anthony Fabius",
    "Lenny Kinsman",
    "Joyal Mathew",
  ),
  date:  $bold("Spring 2024 - Version 0.1.0")$,
)

= Overview

== Vision

ParadiseOS is an x86, 32-bit, multi-core, general-purpose operating system. ParadiseOS is designed to redefine the developer experience in the digital realm. Imagine a seamless fusion of performance and elegance, while navigating through tasks feels like a stroll through a virtual paradise. With a sleek and intuitive VGA terminal interface which boasts 16 colors and 256 extended ascii characters. Harnessing the power of Docker allowing for effortless set up and unparalleled portability, ParadiseOS aims to be a beacon of innovation, transforming the current mundane developer experience into a captivating one. ParadiseOS envisions a future where the operating system isn't just a tool but an immersive gateway to a developers utopia.

== Stack

- C
- x86 Assembly
- QEMU
- Shell
- Docker
- GNU Make

= Goals

+ A functioning, bootable OS
+ A working terminal
+ A lottery scheduler
+ Memory Management (Virtual Memory and Paging)
+ Semi-Working File System (Definitely not)

= Milestones

#set enum(numbering: "I.a.")
+ January
  - Get the OS to boot
    - Basic Kernel
  - Handling Interupts
    + Initialize a simple Global Descriptor Table and Interrupt Descriptor Table
  - Research
    + Designing Memory, Scheduler, and File System
  
+ February
  - Functional Terminal
    + Implement basic I/O
    + Have very basic functionality working
  - Kernel Enhancements
    + Expand kernel functionality for improved system support
  - Memory Management
    + Developing initial page tables and handling basic page faults
    + Begin implementing virtual memory and paging
  - Research
 
+ March
  - Begin work on File System
    + Define the basic structure
    + Groundwork of essential file operations
  - Begin Implementing a basic lottery scheduler
    + Develop task scheduling algorithms to allocate CPU time among processes
  - Research
  
+ April
  - Begin working on final presentations/poster
  - Wrap up any unfinished work
  - Documentation
  - Release version 0.1
  

= Team
Anthony Fabius - Taking RCOS for credit, Coordinator

Joyal Matthew - Taking RCOS for credit, Mentor

Lenny Kinsman - Taking RCOS for credit, Member

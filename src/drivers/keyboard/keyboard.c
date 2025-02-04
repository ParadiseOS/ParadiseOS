#include "keyboard.h"
#include "terminal/terminal.h"
#include "lib/util.h"

bool capsOn;
bool capsLock;

#define     UNKNOWN     0xFF          //
#define     ESC         (0xFF - 1)    //
#define     CTRL        (0xFF - 2)    //
#define     LSHFT       (0xFF - 3)    //
#define     RSHFT       (0xFF - 4)    //
#define     ALT         (0xFF - 5)    //
#define     F1          (0xFF - 6)    //
#define     F2          (0xFF - 7)    //
#define     F3          (0xFF - 8)    //
#define     F4          (0xFF - 9)    //
#define     F5          (0xFF - 10)   //
#define     F6          (0xFF - 11)   //
#define     F7          (0xFF - 12)   //
#define     F8          (0xFF - 13)   //
#define     F9          (0xFF - 14)   //
#define     F10         (0xFF - 15)   //
#define     F11         (0xFF - 16)   //
#define     F12         (0xFF - 17)   //
#define     SCRLCK      (0xFF - 18)   //
#define     HOME        (0xFF - 19)   //
#define     UP          (0xFF - 20)   //
#define     LEFT        (0xFF - 21)   //
#define     RIGHT       (0xFF - 22)   //
#define     DOWN        (0xFF - 23)   //
#define     PGUP        (0xFF - 24)   //
#define     PGDOWN      (0xFF - 25)   //
#define     END         (0xFF - 26)   //
#define     INS         (0xFF - 27)   //
#define     DEL         (0xFF - 28)   //
#define     CAPS        (0xFF - 29)   //
#define     NONE        (0xFF - 30)   //
#define     ALTGR       (0xFF - 31)   //
#define     NUMLCK      (0xFF - 32)   //


const u8  lowercase[128] = {
UNKNOWN,ESC,'1','2','3','4','5','6','7','8',
'9','0','-','=','\b','\t','q','w','e','r',
't','y','u','i','o','p','[',']','\n',CTRL,
'a','s','d','f','g','h','j','k','l',';',
'\'','`',LSHFT,'\\','z','x','c','v','b','n','m',',',
'.','/',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',LEFT,UNKNOWN,RIGHT,
'+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};

const u8  uppercase[128] = {
UNKNOWN,ESC,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R',
'T','Y','U','I','O','P','{','}','\n',CTRL,'A','S','D','F','G','H','J','K','L',':','"','~',LSHFT,'|','Z','X','C',
'V','B','N','M','<','>','?',RSHFT,'*',ALT,' ',CAPS,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,NUMLCK,SCRLCK,HOME,UP,PGUP,'-',
LEFT,UNKNOWN,RIGHT,'+',END,DOWN,PGDOWN,INS,DEL,UNKNOWN,UNKNOWN,UNKNOWN,F11,F12,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,
UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN,UNKNOWN
};

void keyboardHandler(InterruptRegisters* regs) {
    (void) regs;

    // Using Scan Code Set 1 - https://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
    u8 scanCode = inb(SCAN_PORT) & 0x7F; // Key
    u8 press = inb(SCAN_PORT) & 0x80; // Press down or release
    
    switch (scanCode) {
        case 0X01:  // ESC
        case 0x1D:  // CTRL
        case 0x38:  // ALT
        case 0x3B:  // F1
        case 0x3C:  // F2
        case 0X3D:  // F3
        case 0X3E:  // F4
        case 0X3F:  // F5
        case 0X40:  // F6
        case 0X41:  // F7
        case 0X42:  // F8
        case 0X43:  // F9
        case 0X44:  // F19
        case 0X57:  // F11
        case 0X58:  // F12
            break;
        case 0x2A:  //LSHFT
            if (press == 0) {
                capsOn = TRUE;
            } else {
                capsOn = FALSE;
            }
            break;
        case 0x36:  //RSHFT
            if (press == 0) {
                capsOn = TRUE;
            } else {
                capsOn = FALSE;
            }
            break;
        case 0x3A:  //CAPS
            if (!capsLock && press == 0) {
                capsLock = TRUE;
            } else if (capsLock && press == 0) {
                capsLock = FALSE;
            }
            break;
        default:
            if (press == 0){
                if (capsOn || capsLock) {
                    terminal_putchar(uppercase[scanCode]);
                } else {
                    terminal_putchar(lowercase[scanCode]);
                }
            }

    }
}

void init_keyboard() {
    capsOn = FALSE;
    capsLock = FALSE;
    irq_install_handler(1, &keyboardHandler);
}

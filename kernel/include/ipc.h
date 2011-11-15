#ifndef __IPC_H
#define __IPC_H

#define KEYBOARD_PORT 0x21
#define MAX_PORT_COUNT 0x100

void
send (unsigned int dest, unsigned long msg);

#endif

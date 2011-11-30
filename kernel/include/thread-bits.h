#ifndef __ESPERANZA_THREAD_BIT_H
#define __ESPERANZA_THREAD_BIT_H

/* Thread IDs. */
#define NIL_THREAD 0

#define UTCB_VIRTUAL_ADDRESS 0x200000

/** Thread states. */
#define THREAD_STOPPED 0
#define THREAD_RUNNING 1
#define THREAD_SENDING 2
#define THREAD_RECEIVING 3
#define THREAD_DEAD 4
#define THREAD_IPC_RCV 5

#endif

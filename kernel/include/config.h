/**
 * \file config.h Configurable options of the Kicker.
 *
 * You may want to edit this file by removing (commenting) some
 * configuration lines.
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define KICKER_MAJOR_VERSION 0
#define KICKER_MINOR_VERSION 0

/** Kicker stack size. */
#define STACK_SIZE 0x1000
/** Kicker heap size. */
#define HEAP_SIZE 0x1000

/* IA-32 optional improvements */
/** Enable the page size extension to use 4-MByte pages. */
/*#define CONFIG_IA32_PSE*/
/** Use the new SYSENTER/SYSEXIT way to do IPC (You still can use int $0x82). */
/*#define CONFIG_IA32_SYSENTER*/


/* Debugging facilities */

/** Print debug info on the bochs 0xe9 port. */
#define CONFIG_BOCHS_DEBUG

/** Print debug info on the screen. */
#define CONFIG_SCREEN_DEBUG

/** Print debug info on the seria console (disabled by default) */
/*#define CONFIG_COM1_DEBUG*/

#endif

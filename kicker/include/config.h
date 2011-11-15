/**
 * \file config.h Configurable options of the Kicker.
 *
 * You may want to edit this file by removing (commenting) some
 * configuration lines.
 */
#ifndef __CONFIG_H
#define __CONFIG_H


/*#define CPU_IA32*/
#define CPU_AMD64

#ifdef CPU_IA32
#define CONFIG_IA32_PSE
#endif

/** Print debug on 0xe9 port. */
#define CONFIG_BOCHS_DEBUG
/** Print debug on screen. */
#define CONFIG_SCREEN_DEBUG
/** Print debug info on the serial console (disabled by default) */
/*#define CONFIG_COM1_DEBUG*/

#define KICKER_MAJOR_VERSION 0
#define KICKER_MINOR_VERSION 0

/** Kicker stack size. */
#define STACK_SIZE 0x1000
/** Kicker heap size. */
#define HEAP_SIZE 0x1000

#endif

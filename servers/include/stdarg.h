#ifndef __STDARG_H
#define __STDARG_H

/* We rely on GCC built-in stdarg */

typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)

    
#endif /* __STDARG_H */
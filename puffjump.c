// v1.1 no MSVCRT setjmp/longjmp dependency
#ifdef __HAVE_BUILTIN_SETJMP__
#include <setjmp.h>
#undef setjmp
#define setjmp __builtin_setjmp
#undef longjmp
#define longjmp __builtin_longjmp
#endif

#include "zlibpuff/puff.c"

#ifndef ZZ_INOUT_H
#define ZZ_INOUT_H

#include <sys/types.h>

ssize_t     zzio_getline(   char** line,        size_t* linecap );
const char* zzio_out(       const char* fmt,    ...             );
int         zzio_print(     const char* fmt,    ...             );
int         zzio_getch(     void                                );

#endif /* ZZ_INOUT_H */

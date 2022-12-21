#ifndef ZZ_ARGUMENTS_H
#define ZZ_ARGUMENTS_H

#include <zzsys.h>

#define ZZ_ARG_SIZE     0xfff
#define ZZ_ARG_COUNT    0xff

int     zzgetargs(  char* tok,  char** args,    const char* div );
int     zzdirargs(  const ZZDIR* dir, char** args     );
void    zzsortargs( char** args,                const int count );
char*   zzstruntok( char* tok, const int count, const char div  );

#endif /* ZZ_ARGUMENTS_H */

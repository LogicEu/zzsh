#ifndef ZZ_SYSTEM_H
#define ZZ_SYSTEM_H

#include <zztypes.h>

#ifndef ZZ_SYSTEM_C
typedef void ZZDIR;
#endif

void    zzsys_init(     void                                );
int     zzsys_exit(     void                                );

void*   zzdir_data(     const ZZDIR* dir                    );
void    zzdir_set(      ZZDIR* dir                          );
ZZDIR*  zzdir_root(     void                                );
ZZDIR*  zzdir_curr(     void                                );
ZZDIR*  zzdir_search(   const char* name,   const int off   );
int     zzdir_childcount(const ZZDIR* dir                   );

const
char*   zzdir_name(     const ZZDIR* dir                    );
void    zzdir_rename(   ZZDIR* dir,         const char* name);

int     zzsys_mkfile(   const char* arg                     );
int     zzsys_mkdir(    const char* arg                     );
int     zzsys_rm(       const char* arg,    const int rec   );
int     zzsys_move(     const char* from,   const char* to  );
int     zzsys_copy(     const char* from,   const char* to,
                        const int rec                       );
int     zzsys_chdir(    const char* arg                     );

char*   zzpath(         const ZZDIR* dir                    );
char*   zzpath_name(    const char* path                    );

#endif /* ZZ_SYSTEM_H */

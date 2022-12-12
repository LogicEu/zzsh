#ifndef ZZ_SHELL_H
#define ZZ_SHELL_H

#include <stdarg.h>
#include <sys/types.h>
#include <tree.h>

#define ZZ_ERR_NOT_PERMITED 1
#define ZZ_ERR_NO_SUCH_FD 2
#define ZZ_ERR_NO_SUCH_PID 3
#define ZZ_ERR_SYS_INTERRUPT 4
#define ZZ_ERR_IO_ERROR 5
#define ZZ_ERR_NO_SUCH_DEV 6
#define ZZ_ERR_ARG_LIST_TO_LONG 7
#define ZZ_ERR_EXEC_FMT_ERROR 8
#define ZZ_ERR_BAD_FILE_NUM 9
#define ZZ_ERR_NO_CHILD_PID 10
#define ZZ_ERR_NOT_A_DIR 11
#define ZZ_ERR_IS_A_DIR 12
#define ZZ_ERR_DIR_EXISTS 13
#define ZZ_ERR_FILE_EXISTS 14
#define ZZ_ERR_CMD_NOT_FOUND 15
#define ZZ_ERR_ILLEGAL_OPTION 16

#define ZZ_ARG_COUNT 0xff

/* zzio */
ssize_t zzio_getline(char** line, size_t* linecap);
const char* zzio_vout(const char* fmt, va_list ap);
const char* zzio_out(const char* fmt, ...);
int zzio_vprint(const char* fmt, va_list ap);
int zzio_print(const char* fmt, ...);

/* zzsys */
void zzinit(void);
int zzexit(void);
void zzsys_set(struct tnode_t* node);
struct tnode_t* zzsys_root(void);
struct tnode_t* zzsys_curr(void);
struct tnode_t* zzsys_search(char* arg, const int off);
char* zzsys_path(struct tnode_t* node);
char* zzsys_path_name(const char* path);

/* zzargs */
int zzgetargs(char* tok, char** args, const char* div);
int zzdirargs(const struct tnode_t* node, char** args);
void zzsortargs(char** args, const int count);
char* zzstruntok(char* tok, const int count, const char div);

/* zzcmd */
int zzls(int argc, char** argv);
int zzecho(int argc, char** argv);
int zzpwd(int argc, char** argv);
int zzcp(int argc, char** argv);
int zzmv(int argc, char** argv);
int zzcd(int argc, char** argv);
int zzrm(int argc, char** argv);
int zzmkdir(int argc, char** argv);
int zztouch(int argc, char** argv);

/* zzsh */
int zzsh(int argc, char** argv);

/* zzcore */
void zzinit(void);
int zzexit(void);

/* zzerr */
int zzerr_get(void);
int zzerr_set(int error, const char* arg);
int zzerr_print(int error, const char* exe);

#endif /* ZZ_SHELL_H */

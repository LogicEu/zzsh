#ifndef ZZ_ERROR_H
#define ZZ_ERROR_H

#define ZZ_ERR_NOT_PERMITED     1
#define ZZ_ERR_NO_SUCH_FD       2
#define ZZ_ERR_NO_SUCH_PID      3
#define ZZ_ERR_SYS_INTERRUPT    4
#define ZZ_ERR_IO_ERROR         5
#define ZZ_ERR_NO_SUCH_DEV      6
#define ZZ_ERR_ARG_LIST_TO_LONG 7
#define ZZ_ERR_EXEC_FMT_ERROR   8
#define ZZ_ERR_BAD_FILE_NUM     9
#define ZZ_ERR_NO_CHILD_PID    10
#define ZZ_ERR_NOT_A_DIR       11
#define ZZ_ERR_IS_A_DIR        12
#define ZZ_ERR_DIR_EXISTS      13
#define ZZ_ERR_FILE_EXISTS     14
#define ZZ_ERR_CMD_NOT_FOUND   15
#define ZZ_ERR_ILLEGAL_OPTION  16
#define ZZ_ERR_TOO_MANY_ARGS   17

int zzerr_get(  void                        );
int zzerr_set(  int error,  const char* arg );
int zzerr_print(int error,  const char* exe );

#endif /* ZZ_ERROR_H */

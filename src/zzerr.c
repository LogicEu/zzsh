#include <zzerr.h>
#include <zzio.h>

static int errno = 0;
static const char* errarg = ((void*)0);

int zzerr_get(void)
{
    int error = errno;
    errno = 0;
    return error;
}

int zzerr_set(int error, const char* arg)
{
    errno = error;
    errarg = arg;
    return error;
}

int zzerr_print(int error, const char* exe)
{
    static const char* errmsg[] = {
        "operation not permited",
        "no such file or directory",
        "no such process",
        "interrupted system call",
        "input / output error",
        "no such device or address",
        "argument list too long",
        "exec format error",
        "bad file number",
        "no child process",
        "is not a directory",
        "is a directory",
        "directory exists",
        "file exists",
        "command not found",
        "illegal option",
        "too many arguments"
    };

    if (error < 1 || (unsigned long)error >= sizeof(errmsg) / sizeof(errmsg[0])) {
        return error;
    } 
    
    return !zzio_print("%s: %s: %s\n", exe, errmsg[error - 1], errarg ? errarg : "");
}

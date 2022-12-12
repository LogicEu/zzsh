#include <zzsh.h>
#include <stdio.h>
#include <stdarg.h>

#define ZBUFFER_SIZE 0xffff

const char* zzio_vout(const char* fmt, va_list ap)
{
    /* Handle Overflow */
    static char zzbuffer[ZBUFFER_SIZE];
    vsnprintf(zzbuffer, ZBUFFER_SIZE, fmt, ap);
    return zzbuffer;
}

const char* zzio_out(const char* fmt, ...)
{
    const char* buf;
    va_list ap;
    va_start(ap, fmt);
    buf = zzio_vout(fmt, ap);
    va_end(ap);
    return buf;
}

int zzio_vprint(const char* fmt, va_list ap)
{
    const char* buf = zzio_vout(fmt, ap);
    return fprintf(stdout, "%s", buf);
}

int zzio_print(const char* fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = zzio_vprint(fmt, ap);
    va_end(ap);
    return ret;
}

ssize_t zzio_getline(char** line, size_t* linecap)
{
    return getline(line, linecap, stdin);
}

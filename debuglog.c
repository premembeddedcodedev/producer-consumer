#include <stdio.h>
#include <stdarg.h>

#include "debuglog.h"

FILE *hndl;
char *savedFilename;

void debuglog_init(char *filename)
{
    savedFilename = filename;
    hndl = fopen(savedFilename, "wt");
    fclose(hndl);
}

void debuglog_close(void)
{
    fclose(hndl);
}

void debuglog_log(char* format, ...)
{
    hndl = fopen(savedFilename,"at");
    va_list argptr;
    va_start(argptr, format);
    vfprintf(hndl, format, argptr);
    va_end(argptr);
    fputc('\n',hndl);
    fclose(hndl);
}

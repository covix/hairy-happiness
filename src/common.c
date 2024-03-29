/*
	common.c
*/
#include "common.h"


/*
    return a formatted string
*/
char * toString ( const char * format, ... )
{
    char buffer[MAX_BUF];
    va_list args;
    va_start (args, format);
    vsprintf (buffer,format, args);
    va_end (args);
    return strdup(buffer);
}



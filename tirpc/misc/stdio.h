
#ifndef NTIRPC_STDIO_H
#define NTIRPC_STDIO_H

#if defined(__MINGW32__)
/* XXX fix -- another attraction MSVC */
#undef fprintf
#undef printf
#undef sprintf
#undef vfprintf
#undef vprintf
#undef fscanf
#undef scanf
#undef sscanf
#undef fclose
#undef fflush
#undef fgetc
#undef fgetpos
#undef fgets
#undef fopen
#undef fputc
#undef fputs
#undef fread
#undef freopen
#undef fsetpos
#undef fseek
#undef fseeko
#undef ftell
#undef ftello
#undef fwrite
#undef getc
#undef getchar
#undef gets
#undef popen
#undef pclose
#undef putc
#undef putchar
#undef puts
#undef remove
#undef rename
#undef unlink
#undef rewind
#undef tmpfile
#undef tmpnam
#undef ungetc
#undef fwscanf
#undef wscanf
#undef fwprintf
#undef wprintf
#undef vfwprintf
#undef vwprintf
#undef fgetwc
#undef fputwc
#undef getwc
#undef getwchar
#undef putwc
#undef putwchar
#undef ungetwc
#undef fgetws
#undef fputws
#endif
#include <stdio.h>
#endif /* NTIRPC_STDIO_H */
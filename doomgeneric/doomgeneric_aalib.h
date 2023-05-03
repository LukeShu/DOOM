#ifndef __DOOMGENERIC_AALIB_H__
#define __DOOMGENERIC_AALIB_H__

#define printf(...) fprintf(stderr, __VA_ARGS__)
#define puts(str) fputs(str, stderr)
#define putchar(c) putc(c, stderr)

#endif

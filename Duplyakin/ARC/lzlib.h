#ifndef __LZLIB_H__
#define __LZLIB_H__
#include<stdio.h>
int lzencode(FILE *fo, FILE *fi);
int lzdecode(FILE *fo, FILE *fi);
#endif

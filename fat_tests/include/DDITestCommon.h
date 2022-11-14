#ifndef __DDI_TEST_COMMON_H__
#define __DDI_TEST_COMMON_H__

#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

#endif
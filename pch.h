#define _CRT_SECURE_NO_WARNINGS
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#  define WIN32_MEAN_AND_LEAN
#  include <windows.h>
#  include <windowsx.h>
#else
#  include <sys/time.h>
#endif

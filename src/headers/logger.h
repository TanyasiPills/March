#pragma once

#include <stdio.h>

#define Log(x) printf("%s\n", x);
#define LogError(type,line,text) fprintf(stderr, "%d error at line %d:\n%s\n", type, line, text);

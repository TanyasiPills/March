#include "lexer.h"
#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("hello world\n");
	if(argc != 2) {
		printf("wrong input file\n");
		return 1;
	}
	printf("%s\n", argv[1]);
	return ProcessFile(argv[1]);
}

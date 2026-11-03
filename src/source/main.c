#include "lexer.h"
#include <stdio.h>
#include <parser.h>

int main(int argc, char* argv[])
{
	printf("hello world\n");
	if(argc != 2) {
		printf("wrong input file\n");
		return 1;
	}
	printf("%s\n", argv[1]);
	int error = ProcessFile(argv[1]);

	ProcessTokens();
}

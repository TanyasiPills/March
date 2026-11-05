#include "lexer.h"
#include <stdio.h>
#include <parser.h>

void Pad(){
	printf("\n\n\n\n");
}

int main(int argc, char* argv[])
{
	Pad();

	if(argc != 2) {
		printf("wrong input file\n");
		return 1;
	}

	int error = ProcessFile(argv[1]);
	if(error){
		Pad();
		return error;
	} 
	printf("File successfully parsed to tokens\n");

	error = ProcessTokens();
	if(error) {
		Pad();
		return error;
	}
	printf("Tokens successfully parsed to AST nodes\n");

	Pad();

	return 0;
}

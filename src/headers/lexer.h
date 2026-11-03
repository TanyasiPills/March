#pragma once

#include <bare.h>
#include <stdlib.h>
#include <string.h>
#include <TokenType.h>
#include <vector.h>

typedef struct {
    TokenType_t type;
    const char *start;
    int length;
    int line;
} Token_t;


extern Vector tokens; 

int ProcessFile(const char* file);

extern bool tokenTextValid;
extern char* tokenText;

static inline void GetText(Token_t* token){
	if(tokenTextValid) free(tokenText);
	tokenText = malloc(token->length+1);
	tokenText[token->length] = '\0';
	memcpy(tokenText, token->start, token->length);
	tokenTextValid = true;
}

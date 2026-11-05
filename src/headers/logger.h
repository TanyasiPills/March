#pragma once


#define Log(x) printf("%s\n", x);

#define LogLexerError(line,text) fprintf(stderr, "\nError at line %d: %s\n\n", line, text);

#define LogError(line,text) {fprintf(stderr, "\nError at line %d: %s\n", line, text); \
							GetText(&currentToken); \
							printf("Current invalid token: '%s'\n\n", tokenText);}

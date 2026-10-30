#include <stdlib.h>
#include <lexer.h>
#include <TokenType.h>
#include <string.h>

typedef struct {
    int type;
    const char *start;
    int length;
    int line;
} Token_t;

typedef struct {
    Token_t *tokens;
    int count;
    int capacity;
} TokenList;


static char* fileData;
static uint32_t currentPos = 0;
static uint32_t line = 0;

static TokenList tokens; 


void AddToken(TokenList *list, int type, const char *start, int length, int line) {
    if (list->count >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 128 : list->capacity * 2;
        list->tokens = realloc(list->tokens, list->capacity * sizeof(Token_t));
    }
    
    list->tokens[list->count++] = (Token_t){
        .type = type,
        .start = start,
        .length = length,
        .line = line
    };
}

static char Peek(){
	return fileData[currentPos+1];
}

static bool IsAlphabetic(char charIn){
	return (charIn >= 'a' && charIn <= 'z') || (charIn >= 'A' && charIn <= 'Z') || charIn == '_';
}

static bool IsDigit(char charIn) {
	return charIn >= '0' && charIn <= '9';
}

static inline bool IsAlphaNumeric(char charIn){
	return IsAlphabetic(charIn) || IsDigit(charIn);
}

static int ReadFile(const char* path){
	FILE* file = fopen(path, "rb");
	if(!file){
		LogError(404, line, "Coudn't open file");
		return 404;
	}

	fseek(file, 0, SEEK_END);
	uint64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileData = malloc(size+1);
	if(!fileData){
		LogError(1, line, "Malloc failed");
		return 1;
	}

	fread(fileData, 1, size, file);
	fileData[size] = '\0';

	fclose(file);

	return 0;
}

int ScanLong(){
	uint32_t startPos = currentPos;

	if(fileData[currentPos] == '"'){
		currentPos++;
		
		while (fileData[currentPos] != '"' && fileData[currentPos] != '\0') {
			if (fileData[currentPos] == '\n') {
				line++;
			}
			
			currentPos++;
		}

		if (fileData[currentPos] == '\0') {
			LogError(2, line, "End of file"); 
			return 2;
		}

		currentPos++; 

		AddToken(&tokens, StringType, fileData + startPos, currentPos - startPos, line);
		return 0;
	}

	if(IsDigit(fileData[currentPos])){
		while (IsDigit(Peek())) {
			currentPos++;
		}

		AddToken(&tokens, NumberType, fileData+startPos, currentPos - startPos + 1, line);
		return 0;
	}

	if(IsAlphabetic(fileData[currentPos])){

		while (IsAlphaNumeric(Peek())) {
			currentPos++;
		}

		AddToken(&tokens, SymbolType, fileData+startPos, currentPos - startPos + 1, line);
		return 0;
	}

	LogError(3, line, "Error: Cannot lexer char");
	return 999;
}

int ScanFile(){
	if(!fileData){
		LogError(1, line, "Malloc failed");
		return 1;
	}

	tokens = (TokenList){.tokens = malloc(sizeof(Token_t)), .count = 0, .capacity = 1};
	if(!tokens.tokens) {
		LogError(1, line, "Malloc failed");
		return 1;
	}

	while(fileData[currentPos] != '\0'){
		switch (fileData[currentPos]) {
			case ' ':
				break;
			case '\n':
				line++;
				break;
			case ';':
				AddToken(&tokens, LineType, fileData + currentPos, 1, line);
				break;
			case ':':
				AddToken(&tokens, PtrType, fileData + currentPos, 1, line);
				break;
			case '{':
				AddToken(&tokens, CurlyLeftType, fileData + currentPos, 1, line);
				break;
			case '}':
				AddToken(&tokens, CurlyRightType, fileData + currentPos, 1, line);
				break;
			case '(':
				AddToken(&tokens, BraceLeftType, fileData + currentPos, 1, line);
				break;
			case ')':
				AddToken(&tokens, BraceRightType, fileData + currentPos, 1, line);
				break;
			case ',':
				AddToken(&tokens, SeparatorType, fileData + currentPos, 1, line);
				break;
			case '.':
				AddToken(&tokens, ContainType, fileData + currentPos, 1, line);
				break;
			case '=': {
				if(Peek() == '='){
					AddToken(&tokens, EqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, AssingType, fileData + currentPos, 1, line);
				break;
			}
			case '!': {
				if(Peek() == '='){
					AddToken(&tokens, NotEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, NotType, fileData + currentPos, 1, line);
				break;
			}
			case '>': {
				if(Peek() == '='){
					AddToken(&tokens, MoreOrEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, MoreType, fileData + currentPos, 1, line);
				break;
			}
			case '<': {
				if(Peek() == '='){
					AddToken(&tokens, LessOrEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, LessType, fileData + currentPos, 1, line);
				break;
			}
			case '*': {
				if(Peek() == '='){
					AddToken(&tokens, MultiplicationByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, MultiplicationType, fileData + currentPos, 1, line);
				break;
			}
			case '/': {
				if(Peek() == '/'){
					AddToken(&tokens, DivisionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(&tokens, DivisionType, fileData + currentPos, 1, line);
				break;
			}
			case '+': {
				char peeked = Peek();
				if(peeked == '='){
					AddToken(&tokens, AdditionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				} else if(peeked == '+'){
					AddToken(&tokens, IncrementType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}

				AddToken(&tokens, AdditionType, fileData + currentPos, 1, line);
				break;
			}
			case '-': {
				char peeked = Peek();
				if(peeked == '='){
					AddToken(&tokens, SubtractionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				} else if(peeked == '+'){
					AddToken(&tokens, DecrementType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}

				AddToken(&tokens, SubtractionType, fileData + currentPos, 1, line);
				break;
			}
			default: {
				int error = ScanLong();
				if(error) return error;
				break;
			}
		}
		currentPos++;
	}

	return 0;
}

int ProcessFile(const char* path){
	int error;

	error = ReadFile(path);
	if(error) return error;

	error = ScanFile();
	if(error) return error;

	printf("sup\n");

	for (int i = 0; i < tokens.count; i++) {
		Token_t token = tokens.tokens[i];
		char* text = malloc(token.length+1);
		text[token.length] = '\0';
		memcpy(text, token.start, token.length);
		printf("line %d; %d; Token: %s\n", token.line, token.type, text);
		free(text);
	}

	return 0;
}

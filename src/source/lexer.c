#include <lexer.h>
#include <vector.h>

static char* fileData;
static uint32_t currentPos = 0;
static uint32_t line = 1;

Vector tokens;

bool tokenTextValid = false;
char* tokenText;

void AddToken(TokenType_t type, const char* nameStart, uint32_t length, uint32_t line) {
	Token_t token = {type, nameStart, length, line};
	VectorPush(&tokens, &token);
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
		LogLexerError(line, "Coudn't open file");
		return 404;
	}

	fseek(file, 0, SEEK_END);
	uint64_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	fileData = malloc(size+1);
	if(!fileData){
		LogLexerError(line, "Malloc failed");
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
			LogLexerError(line, "End of file"); 
			return 2;
		}

		currentPos++; 

		AddToken(StringType, fileData + startPos, currentPos - startPos, line);
		return 0;
	}

	if(IsDigit(fileData[currentPos])){
		while (IsDigit(Peek())) {
			currentPos++;
		}

		AddToken(NumberType, fileData+startPos, currentPos - startPos + 1, line);
		return 0;
	}

	if(IsAlphabetic(fileData[currentPos])){

		while (IsAlphaNumeric(Peek())) {
			currentPos++;
		}

		AddToken(SymbolType, fileData+startPos, currentPos - startPos + 1, line);
		return 0;
	}

	LogLexerError(line, "Error: Cannot lexer char");
	char* text = malloc(sizeof(char)*2);
	text[1] = '\0';
	text[0] = fileData[startPos];
	printf("yo '%s' yo\n", text);

	return 1;
}

int ScanFile(){
	if(!fileData){
		LogLexerError(line, "Malloc failed");
		return 1;
	}

	AddToken(CurlyLeftType, fileData + currentPos, 0, 0);


	while(fileData[currentPos] != '\0'){
		switch (fileData[currentPos]) {
			case ' ':
				break;
			case '\t':
				break;
			case '\n':
				line++;
				break;
			case ';':
				AddToken(LineType, fileData + currentPos, 1, line);
				break;
			case ':':
				AddToken(PtrType, fileData + currentPos, 1, line);
				break;
			case '{':
				AddToken(CurlyLeftType, fileData + currentPos, 1, line);
				break;
			case '}':
				AddToken(CurlyRightType, fileData + currentPos, 1, line);
				break;
			case '(':
				AddToken(BraceLeftType, fileData + currentPos, 1, line);
				break;
			case ')':
				AddToken(BraceRightType, fileData + currentPos, 1, line);
				break;
			case ',':
				AddToken(SeparatorType, fileData + currentPos, 1, line);
				break;
			case '.':
				AddToken(ContainType, fileData + currentPos, 1, line);
				break;
			case '=': {
				if(Peek() == '='){
					AddToken(EqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(AssignType, fileData + currentPos, 1, line);
				break;
			}
			case '!': {
				if(Peek() == '='){
					AddToken(NotEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(NotType, fileData + currentPos, 1, line);
				break;
			}
			case '>': {
				if(Peek() == '='){
					AddToken(MoreOrEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(MoreType, fileData + currentPos, 1, line);
				break;
			}
			case '<': {
				if(Peek() == '='){
					AddToken(LessOrEqualType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(LessType, fileData + currentPos, 1, line);
				break;
			}
			case '*': {
				if(Peek() == '='){
					AddToken(MultiplicationByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(MultiplicationType, fileData + currentPos, 1, line);
				break;
			}
			case '/': {
				if(Peek() == '/'){
					AddToken(DivisionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}
				AddToken(DivisionType, fileData + currentPos, 1, line);
				break;
			}
			case '+': {
				char peeked = Peek();
				if(peeked == '='){
					AddToken(AdditionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				} else if(peeked == '+'){
					AddToken(IncrementType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}

				AddToken(AdditionType, fileData + currentPos, 1, line);
				break;
			}
			case '-': {
				char peeked = Peek();
				if(peeked == '='){
					AddToken(SubtractionByType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				} else if(peeked == '+'){
					AddToken(DecrementType, fileData + currentPos, 2, line);
					currentPos++;
					break;
				}

				AddToken(SubtractionType, fileData + currentPos, 1, line);
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

	AddToken(CurlyRightType, fileData + currentPos, 0, line);
	AddToken(EOFType, fileData+currentPos, 0, line);

	return 0;
}

int ProcessFile(const char* path){
	VectorInit(&tokens, 10, sizeof(Token_t));

	int error;

	error = ReadFile(path);
	if(error) return error;

	error = ScanFile();
	if(error) return error;

	return 0;
}

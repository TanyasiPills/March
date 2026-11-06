#include "TokenType.h"
#include "logger.h"
#include <parser.h>
#include <stdio.h>

typedef struct {
	const char* text;
	ControlType_t type;
} ControlMapping;

typedef struct {
    const char *text;
    VariableType_t type;
} VariableMapping;

static const VariableMapping varTypes[] = {
    {"u8",	u8Type},
    {"u16", u16Type},
    {"u32", u32Type},
    {"u64", u64Type},
    {"s8",  s8Type},
    {"s16", s16Type},
    {"s32", s32Type},
    {"s64", s64Type},
    {"f32", f32Type},
    {"f64", f64Type},
};

static const ControlMapping conTypes[] = {
	{"return", ReturnType},
	{"while", WhileType},
	{"for", ForType}
};

uint64_t tokenPos = 0;
Token_t currentToken;
ASTNode_t* astTree;

bool isVariableControl(const char* text, ControlType_t* controlDst){
	int count = sizeof(conTypes) / sizeof(conTypes[0]);

	for (int i = 0; i < count; i++) {
		if(strcmp(conTypes[i].text, text) == 0) {
			if(controlDst)
				*controlDst = conTypes[i].type;
			return true;
		}
	}

	return false;
}

bool isVariableType(const char* text, VariableType_t* typeDst){
	int count = sizeof(varTypes) / sizeof(varTypes[0]);
    
    for (int i = 0; i < count; i++) {
        if (strcmp(varTypes[i].text, text) == 0) {
			if(typeDst)
				*typeDst = varTypes[i].type;
			return true;
        }
    }
	 
	return false;
}

void ConsumeToken() {
	if(tokenPos >= tokens.size)
		LogError(0, "Cant extract any more tokens from the lexer (ConsumeToken)");

	currentToken = ((Token_t*)tokens.data)[++tokenPos];
}

static inline bool Check (TokenType_t type) {
	return currentToken.type == type;
}

static bool Match(TokenType_t type) {
	if(Check(type)) {
		ConsumeToken();
		return true;
	}
	return false;
}

static Token_t Expect(TokenType_t type, const char* msg) {
	if(!Check(type)) {
		LogError(currentToken.line, msg);
		Token_t invalid = {InvalidTokenType, 0, 0, 0}; 
		return invalid; 
	}

	Token_t toky = currentToken;
	ConsumeToken();
	return toky;
}

static TypeInfo_t Type() {
	TypeInfo_t info = {-1, 0};

	while (Match(PtrType)) info.pointerDepth++;

	GetText(&currentToken);
	VariableType_t type = -1;
	if(!isVariableType(tokenText, &type)) {
		LogError(currentToken.line, "Type expected (Type)");
		info.type = invalidVariableType;
		info.pointerDepth = 0;
		return info;
	}

	info.type = type;
	ConsumeToken();
	return info;
}

ASTNode_t* Statement();
ASTNode_t* Expression();

ASTNode_t* Primary() {
	if(Check(NumberType)) {
		GetText(&currentToken);
		int value = atoi(tokenText);
		ConsumeToken();
		ASTNode_t* data = malloc(sizeof(ASTNode_t));
		data->line = currentToken.line;
		data->kind = AST_LITERAL;
		data->intBlock.value = value;
		return data;
	}
	if(Check(SymbolType)) {
		GetText(&currentToken);

		if(isVariableType(tokenText, NULL) || isVariableControl(tokenText, NULL)) {
			LogError(currentToken.line, "Expected an identifier (Primary)");
			return NULL;
		}

		ASTNode_t* data = malloc(sizeof(ASTNode_t));
		data->line = currentToken.line;
		data->kind = AST_IDENTIFIER;
		data->identifier.name_start = currentToken.start;
		data->identifier.name_lenght = currentToken.length;
		
		ConsumeToken();

		return data;
	}
	if(Match(BraceLeftType)) {
		ASTNode_t* expr = Expression();
		if(!expr) return NULL;
		Token_t out = Expect(BraceRightType, "Expected ')'");
		if(out.type == InvalidTokenType) return NULL;
		ASTNode_t* data = malloc(sizeof(ASTNode_t));
		data->line = currentToken.line;
		data->kind = AST_GROUP;
		data->group.expr = expr;
		return data;
	}/*
	if (Match(StringType)) {
		
	}*/

	LogError(currentToken.line, "Cant parse primary token");
	return NULL;
}

ASTNode_t* Postfix() {
	ASTNode_t* expr = Primary();
	if(!expr) return NULL;

	while(Match(ContainType)) {
		GetText(&currentToken);
		Token_t token = Expect(SymbolType, "Expected member name (Postfix)");
		if(token.type == InvalidTokenType) return NULL;

		if(isVariableType(tokenText, NULL) || isVariableControl(tokenText, NULL)) {
			LogError(token.line, "Expected member name (Postfix)");
			return NULL;
		}
		
		ASTNode_t* tmp = malloc(sizeof(ASTNode_t));
		tmp->line = currentToken.line;
		tmp->kind = AST_MEMBER_ACCS;
		tmp->member_accs.obj = expr;
		tmp->member_accs.member = tokenText;
		expr = tmp;
	}
	if(Check(IncrementType) || Check(DecrementType)) {
		ASTNode_t* tmp = malloc(sizeof(ASTNode_t));
		tmp->line = currentToken.line;
		tmp->kind = AST_UNARY_EXPR;
		tmp->unary_expr.op = currentToken.type;
		tmp->unary_expr.effected = expr;	
		ConsumeToken();
		expr = tmp;
	}

	return expr;
}

ASTNode_t* Unary() {
	if(Check(PtrType)) {
		TokenType_t op = currentToken.type;
		ConsumeToken();
		ASTNode_t* data = malloc(sizeof(ASTNode_t));
		data->line = currentToken.line;
		data->kind = AST_UNARY_EXPR;
		data->unary_expr.op = op;
		data->unary_expr.effected = Unary();
		if(!data->unary_expr.effected) {
			free(data);
			return NULL;
		}
		return data;
	}

	return Postfix();
}

ASTNode_t* Term() {
	ASTNode_t* left = Unary();
	if(!left)return NULL;

	while(Check(MultiplicationType) || Check(DivisionType)) {
		TokenType_t op = currentToken.type;
		ConsumeToken();
		ASTNode_t* right = Unary();
		if(!right) return NULL;
		ASTNode_t* tmp = malloc(sizeof(ASTNode_t));
		tmp->line = currentToken.line;
		tmp->kind = AST_BINARY_EXPR;
		tmp->binary_expr.left = left;
		tmp->binary_expr.op = op;
		tmp->binary_expr.right = right;
		left = tmp;
	}

	return left;
}

ASTNode_t* Expression() {
	ASTNode_t* left = Term();
	if(!left) return NULL;

	while (Check(AdditionType) || Check(SubtractionType)) {
		TokenType_t op = currentToken.type;
		ConsumeToken();
		ASTNode_t* right = Term();
		if(!right) return NULL;
		ASTNode_t* tmp = malloc(sizeof(ASTNode_t));
		tmp->line = currentToken.line;
		tmp->kind = AST_BINARY_EXPR;
		tmp->binary_expr.left = left;
		tmp->binary_expr.op = op;
		tmp->binary_expr.right = right;
		left = tmp;
	}

	return left;
}

ASTNode_t* Block() {
	Token_t out = Expect(CurlyLeftType, "Expected '{' (Block)");
	if(out.type == InvalidTokenType) return NULL;

	ASTNode_t* block = malloc(sizeof(ASTNode_t));
	block->line = currentToken.line;
	block->kind = AST_BLOCK;
	VectorInit(&block->block.content, 1, sizeof(ASTNode_t*));

	while(!Check(CurlyRightType) && !Check(EOFType)){
		ASTNode_t* result = Statement();
		if(!result) {
			VectorFree(&block->block.content);
			free(block);
			return NULL;
		}
		VectorPush(&block->block.content, &result);
	}

	out = Expect(CurlyRightType, "Expected '}' (Block)");
	if(out.type == InvalidTokenType) {
		free(block);
		return NULL;
	}

	return block;
}

ASTNode_t* DeclareOrFunc() {
	TypeInfo_t type = Type();
	if(type.type == invalidVariableType) return NULL;
	Token_t token = Expect(SymbolType, "Expected identifier (DeclareOrFunc)");
	if(token.type == InvalidTokenType) return NULL;
	GetText(&token);
	if(isVariableType(tokenText, NULL) || isVariableControl(tokenText, NULL)){
		LogError(token.line, "Symbol is not identifier (DeclareOrFunc)");
		return NULL;
	}

	if(Match(PtrType)) {
		ASTNode_t* func = malloc(sizeof(ASTNode_t));
		func->line = currentToken.line;
		func->kind = AST_FUNC_DECL;
		func->line = token.line;
		func->func_decl.name_start = token.start;
		func->func_decl.name_length = token.length;
		
		if(!Check(CurlyLeftType)) { // so that it has parameters
			VectorInit(&func->func_decl.params, 1, sizeof(Parameter_t));
			do {
				Parameter_t param;
				param.dataType = Type();
				if(param.dataType.type == invalidVariableType) {
					VectorFree(&func->func_decl.params);
					free(func);
					return NULL;
				}
				if(Check(SymbolType)) {
					param.name_start = token.start;
					param.name_lenght = token.length;
					ConsumeToken();
				}
				VectorPush(&func->func_decl.params, &param);
			} while (Match(SeparatorType));
		}
		func->func_decl.body = Block();
		if(!func->func_decl.body) {
			VectorFree(&func->func_decl.params);
			free(func);
			return NULL;
		}
		return func;
	}

	ASTNode_t* decl = malloc(sizeof(ASTNode_t));
	decl->line = currentToken.line;
	decl->kind = AST_VAR_DECL;
	decl->var_decl.type = type;
	printf("type pointerDepth %d\n", type.pointerDepth);
	decl->var_decl.name_start = token.start;
	decl->var_decl.name_length = token.length;

	if(Match(AssignType)) {
		decl->var_decl.initializer = Expression();
		if(!decl->var_decl.initializer) {
			free(decl);
			return NULL;
		}
	}

	Token_t out = Expect(LineType, "Expected ';' or '=' (DeclareOrFunc)");
	if(out.type == InvalidTokenType) {
		free(decl);
		return NULL;
	}

	return decl;
}

ASTNode_t* Statement() {
	GetText(&currentToken);
	if(isVariableType(tokenText, NULL) || Check(PtrType)) {
		ASTNode_t* declOrFunc = DeclareOrFunc();
		if(!declOrFunc) return NULL;
		return declOrFunc;
	}

	if(Check(CurlyLeftType)) {
		ASTNode_t* block = Block();
		if(!block) return NULL;
		return block;
	}

	ASTNode_t* target = Expression();
	if(!target) return NULL;

	if(Match(AssignType)) {
		if(target->kind != AST_IDENTIFIER && target->kind != AST_MEMBER_ACCS) {
			LogError(currentToken.line, "Invalid assignment target (Statement)");
		}

		
		ASTNode_t* value = Expression();
		if(!value) return NULL;
		Token_t out = Expect(LineType, "Expected ';'");
		if(out.type == InvalidTokenType) return NULL;
		ASTNode_t* data = malloc(sizeof(ASTNode_t));
		data->line = currentToken.line;
		data->kind = AST_ASSIGNMENT;
		data->assign.target = target;
		data->assign.value = value;

		return data;
	}
	if(target->kind == AST_UNARY_EXPR && Match(LineType)){
		return target;
	}

	LogError(currentToken.line, "invalid operator in expression (Statement)");

	return NULL;
}

int ProcessTokens(){
	currentToken = ((Token_t*)tokens.data)[0];

	astTree = Block();
	if(!astTree){
		printf("Parsing stops because of error\n");
		return 1;
	}

	return 0;
}


#pragma once

#include <lexer.h>
#include <TokenType.h>

typedef struct TypeInfo {
	VariableType_t type;
	int pointerDepth;
} TypeInfo_t;

typedef struct Parameter {
	TypeInfo_t dataType;
	const char* name_start;
	int name_lenght;
} Parameter_t;


typedef struct ASTNode ASTNode_t;

typedef struct {
	int op;
	ASTNode_t *effected;
} ASTUnaryExpr_t;

typedef struct {
    ASTNode_t *left;
    int op;
    ASTNode_t *right;
} ASTBinaryExpr_t;


typedef struct {
	const char* name_start;
	int name_lenght;
}ASTIdentifier_t;

typedef struct {
	TypeInfo_t type;

    const char *name_start;
    int name_length;

    ASTNode_t *initializer;
} ASTVarDecl_t;

typedef struct {
	TypeInfo_t returnType;
    const char *name_start;
    int name_length;
    
    Vector params;

    ASTNode_t *body;
} ASTFuncDecl_t;

typedef struct {
	Vector content;
} ASTBlock_t;

typedef struct {
	ASTNode_t* obj;
	const char* member;
} ASTMemberAcs_t;

typedef struct {
	int value;
} ASTInt_t;

typedef struct {
	ASTNode_t* expr;
} ASTGroup_t;

typedef struct {
	ASTNode_t* target;
	ASTNode_t* value;
} ASTAssign_t;

typedef enum {
    // Declarations
    AST_VAR_DECL,    // e.g., u8 a = 2;
    AST_FUNC_DECL,   // e.g., u64 test: u8 a { ... }

    // Expressions
    AST_LITERAL,     // e.g., 2
    AST_IDENTIFIER,  // e.g., a
    AST_UNARY_EXPR,  // e.g., :a (address-of / pointer deref)
    AST_BINARY_EXPR, // e.g., a + b (for future math)
	AST_MEMBER_ACCS,
	AST_INT,
	AST_GROUP,
	AST_ASSIGNMENT,

    // Statements & Blocks
    AST_BLOCK,       // e.g., { ... }
} ASTNodeKind;

typedef struct ASTNode {
    ASTNodeKind kind;
    int line; 

    union {
        ASTBinaryExpr_t binary_expr;
        ASTUnaryExpr_t	unary_expr;
		ASTVarDecl_t    var_decl;
		ASTFuncDecl_t	func_decl;
		ASTIdentifier_t	identifier;
		ASTBlock_t		block;
		ASTMemberAcs_t  member_accs;
		ASTInt_t		intBlock;
		ASTGroup_t		group;
		ASTAssign_t		assign;
    };
} ASTNode_t;


void ProcessTokens();

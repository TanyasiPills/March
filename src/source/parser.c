#include <string.h>
#include <TokenType.h>

typedef struct {
	const char* text;
	ControlType_t type;
} ControlMapping;

typedef struct {
    const char *text;
    VariableType_t type;
} VariableMapping;

static const VariableMapping varTypes[] = {
    {"u8",  u8Type},
    {"fn",  identifierType},
};

static const ControlMapping conTypes[] = {
	{"return", ReturnType},
	{"while", WhileType},
	{"for", ForType}
};


VariableType_t lookup_variable(const char *text) {
    int num_keywords = sizeof(varTypes) / sizeof(varTypes[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(varTypes[i].text, text) == 0) {
            return varTypes[i].type;
        }
    }
    
    return identifierType; 
}


typedef struct DataType {
	VariableType_t type;
	int pointerDepth;

	const char* name_start;
	int name_lenght;
} DataType_t;


typedef enum {
    // Declarations
    AST_VAR_DECL,    // e.g., u8 a = 2;
    AST_FUNC_DECL,   // e.g., u64 test: u8 a { ... }

    // Expressions
    AST_LITERAL,     // e.g., 2
    AST_IDENTIFIER,  // e.g., a
    AST_UNARY_EXPR,  // e.g., :a (address-of / pointer deref)
    AST_BINARY_EXPR, // e.g., a + b (for future math)

    // Statements & Blocks
    AST_BLOCK,       // e.g., { ... }
    AST_RETURN_STMT  // e.g., return ... (for inside functions)
} ASTNodeKind;

typedef struct ASTNode ASTNode_t;

typedef struct {
	int operator_type;
	ASTNode_t *effected;
} ASTUnaryExpr_t;

typedef struct {
    ASTNode_t *left;
    int operator_type;
    ASTNode_t *right;
} ASTBinaryExpr_t;

typedef struct {
	DataType_t type;
	const char* start;
	int value_length;
} ASTLiteral_t;

typedef struct {
	const char* name_start;
	int name_lenght;
}ASTIdentifier_t;

typedef struct {
	DataType_t dataType;
    const char *name_start;
    int name_length;
    ASTNode_t *initializer;
} ASTVarDecl_t;

typedef struct Parameter {
	DataType_t dataType;
	const char* name_start;
	int name_lenght;
} Parameter_t;

typedef struct {
    const char *return_type_start; // e.g., "u64" int return_type_length;
    const char *name_start;        // e.g., "test"
    int name_length;

    
    Parameter_t *params;
    int param_count;

    ASTNode_t *body;
} ASTFuncDecl_t;

typedef struct {
	ASTNode_t* content;
	int nodecount;
} ASTFunctionBody_t;

typedef struct {
	
} ASTReturnStmt_t;



struct ASTNode {
    ASTNodeKind kind;
    int line; 

    union {
        ASTBinaryExpr_t binary_expr;
        ASTUnaryExpr_t	unary_expr;
		ASTVarDecl_t    var_decl;
		ASTFuncDecl_t	func_decl;
        ASTLiteral_t	litreral;
		ASTIdentifier_t	identifier;
    };
};



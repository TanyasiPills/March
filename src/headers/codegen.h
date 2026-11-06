#include <parser.h>


typedef struct {
	Vector key;
	Vector value;
} Dictionary;

extern Dictionary variables;
extern Dictionary functions;


int Analize(ASTNode_t* root);

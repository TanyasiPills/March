// Sematic analyzer

#include "TokenType.h"
#include "logger.h"
#include "parser.h"
#include "vector.h"
#include <codegen.h>
#include <stdio.h>
#include <string.h>

Dictionary variables;
Dictionary functions;

TypeInfo_t currentVarType;
TypeInfo_t goalVarType;

ASTNode_t* FindVariable(const char* namestart, int len) {
	char* key = malloc(sizeof(char)*len+1);
	key[len] = '\0';
	memcpy(key, namestart, len);
	for (uint64_t i = 0; i < variables.key.size; i++) {
		printf("key: %s; current key: %s\n", key, ((char**)variables.key.data)[i]);
		if(strcmp(((char**)variables.key.data)[i], key) == 0){
			free(key);
			return ((ASTNode_t**)variables.value.data)[i];
		}

	}
	free(key);
	return NULL;
}

bool CanStore(ASTNode_t* node, TypeInfo_t type) {
	switch (node->kind) {
		case AST_LITERAL:
			if(type.type > 7) return false;
			return true;
			break;
		case AST_IDENTIFIER:{
			ASTNode_t* variable = FindVariable(node->identifier.name_start, node->identifier.name_lenght); 
			if(!variable){
				LogCDError("Cant find variable referenced by identifier (CanStore)", node->kind);
				return false;
			}
			if(type.pointerDepth > 0) return true;

			bool data = variable->var_decl.type.type <= type.type;
			if(!data){
				printf("type input: %d, type of decl: %d\n", type.type, variable->var_decl.type.type);
				printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, variable->var_decl.type.pointerDepth);
				LogCDError("Variable referenced cant store type (CanStore)", node->kind);
				return false;
			}
			return data;
			} break;
		default:
			LogCDError("cant find node type, so cant check if storage is compatable (CanStore)", node->kind);
			return false;
	}
}

bool MatchType(VariableType_t type1, VariableType_t type2){
	return type1 == type2;
}

bool MatchTypeInfo(TypeInfo_t type1, TypeInfo_t type2) {
	return MatchType(type1.type, type2.type) && (type1.pointerDepth == type2.pointerDepth);
}

void AddToDic(Dictionary* dic, const char* name, int nameLenght, ASTNode_t* data) {
	char* key = malloc(sizeof(char)*nameLenght+1);
	key[nameLenght] = '\0';
	memcpy(key, name, nameLenght);

	printf("adding key: %s\n", key);

	VectorPush(&(dic->key), &key);
	VectorPush(&(dic->value), &data);
}

int RecursiveAnalizing(ASTNode_t* node) {
	if(node == NULL){
		printf("da fak, passed null\n");
		return 1;
	}
	int error;
	switch (node->kind) {
		case AST_BLOCK:
			printf("block\n");
			for (uint64_t i = 0; i < node->block.content.size; i++) {
				error = RecursiveAnalizing(((ASTNode_t**)node->block.content.data)[i]);
				if(error) return error;
			}
			break;
		case AST_VAR_DECL: {
			printf("var declare\n");
			goalVarType = node->var_decl.type;
			error = RecursiveAnalizing(node->var_decl.initializer);
			if(error) return error;

			if(!MatchTypeInfo(goalVarType, currentVarType)) {
				LogCDError("initializer type doesnt align with declare type", node->kind);
				return 1;
			}
			
			AddToDic(&variables, node->var_decl.name_start, node->var_decl.name_length, node);
			goalVarType.type = invalidVariableType;
			} break;
		case AST_BINARY_EXPR: {
			printf("binary expr\n");
			error = RecursiveAnalizing(node->binary_expr.left);
			if(error) return error;
			TypeInfo_t left = currentVarType;

			error = RecursiveAnalizing(node->binary_expr.right);
			if(error) return error;
			TypeInfo_t right = currentVarType;

			if(!MatchTypeInfo(left, right)){
				LogCDError("mismached types in binary expression (RecursiveAnalizing)", node->kind);
				
				return 1;
			}
			} break;
		case AST_UNARY_EXPR: {
			printf("unary\n");
			error = RecursiveAnalizing(node->unary_expr.effected);
			if(error) return error;

			if(node->unary_expr.op == PtrType){
				currentVarType.pointerDepth++;
			}
			} break;
		case AST_LITERAL: {
			printf("literal\n");
			if(!CanStore(node, goalVarType)){
				return 1;
			}
			
			TypeInfo_t varType =  { goalVarType.type, 0 };
			currentVarType = varType;
			} break;
		case AST_IDENTIFIER: {
			printf("identifier\n");
			if(!CanStore(node, goalVarType)) {
				return 1;
			}

			TypeInfo_t varType =  { goalVarType.type, 0 };
			currentVarType = varType;
			} break;
		case AST_GROUP: {
			error = RecursiveAnalizing(node->group.expr);
			if(error) return error;
			} break;
		default:
			LogCDError("Cant find AST node kind (RecursiveAnalizing)", node->kind);
			return 1;
	}

	return 0;
}

void AllocData() {
	VectorInit(&variables.key, 2, sizeof(char*));
	VectorInit(&variables.value, 2, sizeof(ASTNode_t*));
	
	VectorInit(&functions.key, 2, sizeof(char*));
	VectorInit(&functions.value, 2, sizeof(ASTNode_t*));
}

void FreeData() {
	VectorFree(&variables.key);
	VectorFree(&variables.value);

	VectorFree(&functions.key);
	VectorFree(&functions.value);
}


int Analize(ASTNode_t* root) {
	AllocData();

	printf("\n");

	if(root->kind != AST_BLOCK) {
		LogCDError("AST doesnt start as block (Analize)", root->kind);
		FreeData();
		return 1;
	}
	int error = RecursiveAnalizing(root);

	if(error) {
		FreeData();
		return error;
	}

	return 0;
}

// Sematic analyzer

#include "TokenType.h"
#include "logger.h"
#include "parser.h"
#include "vector.h"
#include <codegen.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

Dictionary variables;
Dictionary functions;

TypeInfo_t currentVarType;
TypeInfo_t goalVarType;

ASTNode_t* currVar = NULL;
ASTNode_t* currFunc = NULL;

ASTNode_t* FindIdentifier(Dictionary dic, const char* namestart, int len) {
	char* key = malloc(sizeof(char)*len+1);
	key[len] = '\0';
	memcpy(key, namestart, len);
	for (uint64_t i = 0; i < dic.key.size; i++) {
		if(strcmp(((char**)dic.key.data)[i], key) == 0){
			free(key);
			return ((ASTNode_t**)dic.value.data)[i];
		}

	}
	free(key);
	return NULL;
}

bool CanStore(ASTNode_t* node, TypeInfo_t type) {
	switch (node->kind) {
		case AST_NUMBER:
			if(type.type > 7) return false;
			return true;
			break;
		case AST_STRING:
			bool data = type.pointerDepth == 1;
			if(!data) {
				LogCDError("Variable cant store string, it must be a pointer (CanStore)", node->kind);
				return false;
			}
			return true;
			break;
		case AST_IDENTIFIER:{
			ASTNode_t* variable = FindIdentifier(variables, node->identifier.name_start, node->identifier.name_lenght); 
			ASTNode_t* function;

			if(!variable){
				function = FindIdentifier(functions, node->identifier.name_start, node->identifier.name_lenght);
				if(!function) {
					LogCDError("Cant find variable referenced by identifier (CanStore)", node->kind);
					return false;
				}
				currFunc = function;
			} else {
				currVar = variable;
			}

			if(type.pointerDepth > 0) return true;
			
			bool data;
			if(variable)
				data = variable->var_decl.type.type <= type.type;
			else
				data = function->func_decl.returnType.type <= type.type;

			if(!data){
				if(variable){
					printf("type input: %d, type of decl: %d\n", type.type, variable->var_decl.type.type);
					printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, variable->var_decl.type.pointerDepth);
					LogCDError("Variable referenced cant store type (CanStore)", node->kind);
				} else {
					printf("type input: %d, type of decl: %d\n", type.type, function->func_decl.returnType.type);
					printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, function->func_decl.returnType.pointerDepth);
					LogCDError("Function referenced doesnt return correct type (CanStore)", node->kind);
				}
				return false;
			}
			return data;
			} break;
		case AST_FUNC_DECL: {
			bool data = node->func_decl.returnType.type <= type.type;

			if(!data) {
				printf("type input: %d, type of decl: %d\n", type.type, node->func_decl.returnType.type);
				printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, node->func_decl.returnType.pointerDepth);
				LogCDError("Function referenced doesnt return correct type (CanStore)", node->kind);
				return false;
			}
			return true;
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

bool AlreadyDeclared(const char* name) {
	for (uint64_t i = 0; i < variables.key.size; i++) {
		if(strcmp(name, ((char**)variables.key.data)[i]) == 0)
			return true;
	}

	for (uint64_t i = 0; i < functions.key.size; i++) {
		if(strcmp(name, ((char**)functions.key.data)[i]) == 0)
			return true;
	}

	return false;
}

void AddToDic(Dictionary* dic, const char* name, int nameLenght, ASTNode_t* data) {
	if(AlreadyDeclared(name)) {
		LogCDError("Redeclaretion of literal", data->kind);
		return;
	}

	char* key = malloc(sizeof(char)*nameLenght+1);
	key[nameLenght] = '\0';
	memcpy(key, name, nameLenght);

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
			printf("Block\n");
			for (uint64_t i = 0; i < node->block.content.size; i++) {
				error = RecursiveAnalizing(((ASTNode_t**)node->block.content.data)[i]);
				if(error) return error;
			}
			break;
		case AST_VAR_DECL: {
			printf("Var declare\n");
			goalVarType = node->var_decl.type;
			if(node->var_decl.initializer) {
				error = RecursiveAnalizing(node->var_decl.initializer);
				if(error) return error;
			}

			if(!MatchTypeInfo(goalVarType, currentVarType)) {
				LogCDError("initializer type doesnt align with declare type (RecursiveAnalizing)", node->kind);
				return 1;
			}
			
			AddToDic(&variables, node->var_decl.name_start, node->var_decl.name_length, node);
			goalVarType.type = invalidVariableType;
			} break;
		case AST_BINARY: {
			printf("Binary expr\n");
			error = RecursiveAnalizing(node->binary.left);
			if(error) return error;
			TypeInfo_t left = currentVarType;

			error = RecursiveAnalizing(node->binary.right);
			if(error) return error;
			TypeInfo_t right = currentVarType;

			if(!MatchTypeInfo(left, right)){
				LogCDError("mismached types in binary expression (RecursiveAnalizing)", node->kind);
				
				return 1;
			}
			} break;
		case AST_UNARY: {
			printf("Unary\n");
			error = RecursiveAnalizing(node->unary.effected);
			if(error) return error;

			if(node->unary.op == ReferenceType){
				currentVarType.pointerDepth++;
				break;
			}
			if(node->unary.op == DereferenceType) {
				currentVarType.pointerDepth--;
				break;
			}
			} break;
		case AST_NUMBER: {
			printf("Number\n");
			if(!CanStore(node, goalVarType)){
				return 1;
			}	
			
			TypeInfo_t varType =  { invalidVariableType, 0 };

			if(goalVarType.type != invalidVariableType)
				varType.type = goalVarType.type;
			else{
				LogCDError("Goal variable type is not set, cant compare literals (RecursiveAnalizing)", node->kind);
				return 1;
			}

			varType.pointerDepth = 0;

			currentVarType = varType;
			} break;
		case AST_IDENTIFIER: {
			printf("Identifier\n");
			if(!CanStore(node, goalVarType)) {
				return 1;
			}
			
			if(currFunc && currFunc->func_decl.params.size > 0) {
				LogCDError("Funcition reference doesnt provide needed arguments (RecursiveAnalizing)", node->kind);
				return 1;
			}

			TypeInfo_t varType = {invalidVariableType, -1};
			if(goalVarType.type != invalidVariableType)
				varType.type = goalVarType.type;
			else {
				LogCDError("Goal variable type is not set, cant compare literals (RecursiveAnalizing)", node->kind);
			}
			if(currVar) {
				varType.pointerDepth = currVar->var_decl.type.pointerDepth;
			}
			else if(currFunc) {
				varType.pointerDepth = currFunc->func_decl.returnType.pointerDepth;
			}

			currentVarType = varType;

			currFunc = NULL;
			currVar = NULL;
			} break;
		case AST_GROUP: {
			printf("Group\n");
			error = RecursiveAnalizing(node->group.expr);
			if(error) return error;
			} break;
		case AST_FUNC_DECL: {
			printf("Function decleration\n");
			AddToDic(&functions, node->func_decl.name_start, node->func_decl.name_length, node);
			RecursiveAnalizing(node->func_decl.body);
			} break;
		case AST_FUNC_CALL: {
			printf("Function call\n");
			ASTNode_t* function = FindIdentifier(functions, node->func_call.identifier->identifier.name_start, node->func_call.identifier->identifier.name_lenght);
			if(!function) {
				LogCDError("Function reference points to a non-existant function (RecursiveAnalizing)", node->kind);
				return 1;
			}

			if(!CanStore(function, goalVarType)){
				return 1;
			}

			uint32_t paramCount = node->func_call.params.size;
			if(function->func_decl.params.size != paramCount){
				LogCDError("Function reference doesnt pass the correct amount of parameters (RecursiveAnalizing)", node->kind);
			}

			TypeInfo_t storedType = goalVarType;

			for (size_t i = 0; i < paramCount; i++) {
				ASTNode_t* curNode = ((ASTNode_t**)node->func_call.params.data)[i];
				TypeInfo_t curType = ((Parameter_t*)function->func_decl.params.data)[i].dataType; 
				if(!CanStore(curNode, curType) ) {
					LogCDError("Function call parameter types dont align with the definition of the function (RecursiveAnalizing)", node->kind);
					return 1;
				}
				goalVarType = curType;
				RecursiveAnalizing(curNode);

				if(!MatchTypeInfo(goalVarType, currentVarType)) {
					LogCDError("parameter type doesnt align with function declaretion parameter type (RecursiveAnalizing)", node->kind);
					return 1;
				}
			}	

			goalVarType = storedType;
		
			TypeInfo_t returnType = { goalVarType.type, function->func_decl.returnType.pointerDepth };
			currentVarType = returnType;
			} break;
		case AST_STRING: {
			printf("String\n");
			if(!CanStore(node, goalVarType)){
				return 1;
			}	
			
			TypeInfo_t varType;
			if(goalVarType.type != invalidVariableType)
				varType.type = goalVarType.type;
			else {
				LogCDError("Goal variable type is not set, cant compare literals (RecursiveAnalizing)", node->kind);
				return 1;
			}

			varType.pointerDepth = 1;

			currentVarType = varType;
			} break;
		case AST_ASSIGNMENT: {
			printf("Assignment\n");
			ASTNode_t* curVal = FindIdentifier(variables, node->assign.target->identifier.name_start, node->assign.target->identifier.name_lenght);

			if(!curVal){
				LogCDError("Cant find the variable to assign to (RecursiveAnalizing)", node->kind);
				return 1;
			}

			goalVarType = curVal->var_decl.type;

			error = RecursiveAnalizing(node->assign.value);
			if(error) return error;

			if(!MatchTypeInfo(goalVarType, currentVarType)) {
				LogCDError("Assignment target type doesnt align with assignment value type (RecursiveAnalizing)", node->kind);
			}

			goalVarType.type = invalidVariableType;

			} break;
		case AST_LOGICAL: {
			printf("Logical\n");
				
			} break;
		case AST_CONTROL: {
			printf("Control; type=%d\n", node->control.type);
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

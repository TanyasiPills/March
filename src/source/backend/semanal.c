// Sematic analyzer

#include "TokenType.h"
#include "logger.h"
#include "parser.h"
#include "vector.h"
#include <codegen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Dictionary variables;
Dictionary functions;

TypeInfo_t currentReturnType;
TypeInfo_t currentVarType;
TypeInfo_t goalVarType;

ASTNode_t* currVar = NULL;
ASTNode_t* currFunc = NULL;

int32_t prefixTextCount = 0;
char* prefixText;

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
			if(type.type > 9) return false;
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
		case AST_BOOLEAN: {
			bool data = type.type <= f64Type;

			if(!data) {
				printf("type input: %d, type of decl: %d\n", type.type, node->func_decl.returnType.type);
				printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, node->func_decl.returnType.pointerDepth);
				LogCDError("Function referenced doesnt return correct type (CanStore)", node->kind);
				return false;
			}

			return true;
			} break;
		case AST_GROUP: {
				bool data = currentVarType.type <= type.type;

				if(!data) {
					printf("type input: %d, type of decl: %d\n", type.type, node->func_decl.returnType.type);
					printf("ptrdepth input: %d, ptrdepth of decl: %d\n", type.pointerDepth, node->func_decl.returnType.pointerDepth);
					LogCDError("Function referenced doesnt return correct type (CanStore)", node->kind);
					return false;
				}

				return true;
			} break;
		case AST_UNARY: {
				bool data = currentVarType.type <= type.type;

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

bool CanLogical(TypeInfo_t left, TypeInfo_t right){
	if(left.type <= 3) return right.type <= 3;
	if(left.type > 3 && left.type <= 7) return right.type > 3 && right.type <= 7;
	if(left.type > 7 && left.type <= 9) return right.type > 7 && right.type <= 9;

	return false;
}


bool MatchTypeInfo(TypeInfo_t type1, TypeInfo_t type2) {
	return CanLogical(type1, type2) && (type1.pointerDepth == type2.pointerDepth);
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

void TPrefix(){
	if(prefixText) free(prefixText);
	prefixText = malloc(sizeof(char)* prefixTextCount+1);
	for (size_t i = 0; i < prefixTextCount; i++) {	
		prefixText[i] = '\t';
	}
	prefixText[prefixTextCount] = '\0';
}

int RecursiveAnalizing(ASTNode_t* node) {
	if(node == NULL){
		printf("da fak, passed null\n");
		return 1;
	}
	
	TPrefix();

	int error;
	switch (node->kind) {
		case AST_BLOCK:
			printf("%sBlock\n",prefixText);

			for (uint64_t i = 0; i < node->block.content.size; i++) {
				prefixTextCount++;
				error = RecursiveAnalizing(((ASTNode_t**)node->block.content.data)[i]);
				prefixTextCount--;
				if(error) return error;
			}
			break;
		case AST_VAR_DECL: {
			printf("%sVar declare\n",prefixText);
			goalVarType = node->var_decl.type;
			if(node->var_decl.initializer) {
				prefixTextCount++;
				error = RecursiveAnalizing(node->var_decl.initializer);
				prefixTextCount--;
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
			printf("%sBinary expr\n",prefixText);
			

			// for more complex checking binary also needs goal type
			printf("%sLeft:\n",prefixText);
			prefixTextCount++;
			error = RecursiveAnalizing(node->binary.left);
			prefixTextCount--;
			if(error) return error;
			TypeInfo_t left = currentVarType;

			TPrefix();

			printf("%sRight:\n",prefixText);
			prefixTextCount++;
			error = RecursiveAnalizing(node->binary.right);
			prefixTextCount--;
			if(error) return error;
			TypeInfo_t right = currentVarType;

			if(node->binary.op == CastType) {
				printf("left: %d; right: %d\n", left.pointerDepth, right.pointerDepth);
				if(CanStore(node->binary.right, left) && left.pointerDepth == right.pointerDepth) {
					currentVarType = left;	
					break;
				}

				LogCDError("cant cast expression to the specified type", node->kind);
				return 1;
			}

			if(!MatchTypeInfo(left, right)){

				printf("left kind: %d; right kind: %d\n", left.type, right.type);
				LogCDError("mismached types in binary expression (RecursiveAnalizing)", node->kind);
				
				return 1;
			}

			} break;
		case AST_UNARY: {
			printf("%sUnary\n",prefixText);
			
			prefixTextCount++;
			error = RecursiveAnalizing(node->unary.effected);
			prefixTextCount--;
			if(error) return error;

			if(node->unary.op == ReferenceType){
				currentVarType.pointerDepth++;
				break;
			}
			if(node->unary.op == DereferenceType) {
				currentVarType.pointerDepth--;
				break;
			}

			if(node->unary.op == IncrementType || node->unary.op == DecrementType){
				break;
			}

			LogCDError("Cant find unary type (RecursiveAnalizing)", node->kind);
			return 1;
			} break;
		case AST_NUMBER: {
			printf("%sNumber\n",prefixText);

			if(!CanStore(node, goalVarType)){
				LogCDError("Goal variable type cant store the curret literal (RecursiveAnalizing)", node->kind);
				return 1;
			}	
			
			TypeInfo_t varType =  { invalidVariableType, 0 };

			if(node->number.type == UNSIGNED) varType.type = u64Type;
			else if(node->number.type == SIGNED) varType.type = s64Type;
			else varType.type = f64Type;

			varType.pointerDepth = 0;

			currentVarType = varType;
			} break;
		case AST_IDENTIFIER: {
			printf("%sIdentifier\n",prefixText);

			if(!CanStore(node, goalVarType)) {
				LogCDError("Identifier cant be stored in goal variable type", node->kind);
				return 1;
			}
			
			if(currFunc && currFunc->func_decl.params.size > 0) {
				LogCDError("Funcition reference doesnt provide needed arguments (RecursiveAnalizing)", node->kind);
				return 1;
			}

			TypeInfo_t varType = {invalidVariableType, -1};

			if(currVar) {
				varType = currVar->var_decl.type;
			}
			else if(currFunc) {
				varType = currFunc->func_decl.returnType;
			}

			currentVarType = varType;

			currFunc = NULL;
			currVar = NULL;
			} break;
		case AST_GROUP: {
			printf("%sGroup\n",prefixText);
			prefixTextCount++;
			error = RecursiveAnalizing(node->group.expr);
			prefixTextCount--;
			if(error) return error;
			} break;
		case AST_FUNC_DECL: {
			printf("%sFunction decleration\n", prefixText);

			AddToDic(&functions, node->func_decl.name_start, node->func_decl.name_length, node);

			TypeInfo_t tmp = currentReturnType;
			currentReturnType = node->func_decl.returnType;

			prefixTextCount++;
			RecursiveAnalizing(node->func_decl.body);
			prefixTextCount--;

			currentReturnType = tmp;
			} break;
		case AST_FUNC_CALL: {
			printf("%sFunction call\n", prefixText);

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
				prefixTextCount++;
				RecursiveAnalizing(curNode);
				prefixTextCount--;

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
			printf("%sString\n",prefixText);

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
			printf("%sAssignment\n",prefixText);
			
			if(node->assign.target->kind == identifierType);

			RecursiveAnalizing(node->assign.target);
			goalVarType = currentVarType;

			prefixTextCount++;
			error = RecursiveAnalizing(node->assign.value);
			prefixTextCount--;
			if(error) return error;

			if(!MatchTypeInfo(goalVarType, currentVarType)) {
				LogCDError("Assignment target type doesnt align with assignment value type (RecursiveAnalizing)", node->kind);
			}

			goalVarType.type = invalidVariableType;

			} break;
		case AST_LOGICAL: {
			printf("%sLogical\n",prefixText);
			prefixTextCount++;	
			printf("%sLeft:\n",prefixText);
			error = RecursiveAnalizing(node->logical.left);
			prefixTextCount--;
			if(error) return error;
			TypeInfo_t left = currentVarType;

			TPrefix();

			printf("%sRight:\n", prefixText);
			prefixTextCount++;
			error = RecursiveAnalizing(node->logical.right);
			prefixTextCount--;
			if(error) return error;
			TypeInfo_t right = currentVarType;

			if(!CanLogical(left, right)) {
				printf("left kind: %d; right kind: %d\n", left.type, right.type);
				LogCDError("mismached types in logical expression (RecursiveAnalizing)", node->kind);
				
				return 1;
			}
			} break;
		case AST_CONTROL: {
			printf("%sControl; type=%d\n", prefixText, node->control.type);

			switch (node->control.type) {
				case ElifType:
				case IfType: {
					if(node->control.data[0]->kind != AST_LOGICAL && node->control.data[0]->kind != AST_IDENTIFIER) {
						LogCDError("If statement expression must be a logical comparison or variable reference (RecursiveAnalizing)", node->kind);
						return 1;
					}

					goalVarType.type = f64Type;
					goalVarType.pointerDepth = 0;
					
					prefixTextCount++;
					RecursiveAnalizing(node->control.data[0]);
					prefixTextCount--;

					TPrefix();

					prefixTextCount++;
					RecursiveAnalizing(node->control.data[1]);
					prefixTextCount--;
					} break;
				case WhileType: {
					if(node->control.data[0]->kind != AST_LOGICAL && node->control.data[0]->kind != AST_IDENTIFIER) {
						LogCDError("While statement expression must be a logical comparison or variable reference (RecursiveAnalizing)", node->kind);
						return 1;
					}

					goalVarType.type = f64Type;
					goalVarType.pointerDepth = 0;
					
					prefixTextCount++;
					RecursiveAnalizing(node->control.data[0]);
					prefixTextCount--;
					} break;
				case ForType: {
					prefixTextCount++;

					if(node->control.data[0]->kind != AST_VAR_DECL) {
						LogCDError("For statement's first expression must be a variable decleration (RecursiveAnalizing)", node->kind);
						return 1;
					}

					RecursiveAnalizing(node->control.data[0]);

					if(node->control.data[1]->kind != AST_LOGICAL && node->control.data[1]->kind != AST_IDENTIFIER) {
						LogCDError("For statement's second expression must be a logical comparison or variable reference (RecursiveAnalizing)", node->kind);
						return 1;
					}

					goalVarType.type = f64Type;
					goalVarType.pointerDepth = 0;

					error = RecursiveAnalizing(node->control.data[1]);
					if(error) return error;

					// idk if i should check the last parameter stores
					error = RecursiveAnalizing(node->control.data[2]);
					if(error) return error;

					error = RecursiveAnalizing(node->control.data[3]);
					if(error) return error;
					} break;
				case ElseType: {
					RecursiveAnalizing(node->control.data[0]);
					} break;
				case ReturnType: {
					currentVarType.type = invalidVariableType;
					goalVarType = currentReturnType;
					RecursiveAnalizing(node->control.data[0]);

					if(!MatchTypeInfo(currentVarType, currentReturnType)) {
						LogCDError("Returned type doesnt match the function return type", node->kind);
						return 1;
					}
					break;
				}
				default:
					LogCDError("Cannot find control type (RecursiveAnalizing)", node->kind);
					return 1;
					break;
			}
			} break;
		case AST_BOOLEAN: {
			printf("%sBoolean\n", prefixText);
			if(!CanStore(node, goalVarType)){
				return 1;
			}

			TypeInfo_t varType;
			if(goalVarType.type != invalidVariableType)
				varType.type = goalVarType.type;
			else {
				LogCDError("Goal variable type is not set, cant compare booleans (RecursiveAnalizing)", node->kind);
				return 1;
			}

			varType.pointerDepth = 0;

			currentVarType = varType;

			}break;
		case AST_CAST: {
			printf("%sCast\n",prefixText);
			currentVarType = node->cast.info;
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

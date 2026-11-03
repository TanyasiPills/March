#pragma once

typedef enum VariableType {
	u8Type,
    u16Type,
    u32Type,
    u64Type,

	s8Type,
	s16Type,
	s32Type,
	s64Type,

	f32Type,
	f64Type,

	identifierType

} VariableType_t;

typedef enum ControlType {
	ReturnType,
	ForType,
	WhileType,
	InvalidType
} ControlType_t;

typedef enum TokenType{
    //Variables
	SymbolType,
    PtrType, 
    StringType,
	NumberType,

    //Math
    AssignType, 
    AdditionType, 
    AdditionByType, 
    SubtractionType,
    SubtractionByType,
    MultiplicationType,
    MultiplicationByType,
    DivisionType,
    DivisionByType,
	IncrementType,
	DecrementType,

    //logic
    BraceLeftType,
    BraceRightType,
	EqualType,
	NotEqualType,
	LessOrEqualType,
	MoreOrEqualType,
	LessType,
	MoreType,
	NotType,

	CurlyLeftType,
	CurlyRightType,

    //file
    LineType,
	SeparatorType,
	ContainType,
	EOFType
} TokenType_t;

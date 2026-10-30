# March Grammar
> MARCH: compilation based programing language, written in C
Language for the SPRING project

## Variables
```
bit - 1 bit
biT - 2 bit

set - 8 bit - 1 byte
seT - 16 bit - 2 byte

int - 16 bit - 2 byte
inT - 32 bit - 4 byte

float - 32 bit - 4 byte -> floating point type
floaT - 64 bit - 8 byte -> floating point type

lot - 128 bit - 16 byte
loT - 256 bit - 32 byte

zit - 128 bit - 16 byte -> floating point type
ziT - 256 bit - 32 byte -> floating point type

pointer:
:int - pointer to an int
:float - pointer to a float
and so on...
```
## Data Structures
Larger elements that can hold multiple variables
### List
```
List:int exampleList; //Default memory size: size of the set variable type(int in this case) + pointer to next element

Each element has a pointer to the next one, structured as a linked list
```
### Array
```
Array:int exampleArray[200]; //Default memory size: pointer to the start + size of variable(int in this case) times the size of the array

Arrays cant be initialized without size, will throw error on compilation
```
### Dictionary
```
Needs more sleep
```
### Map
```
Needs more tea
```

## Function behaviour and Types
Return types work like in any other C language:
```
int Function : {}
```
Functions can return every variable type and custom struct + void

But the function does not use "()" for the variables:
```
int Function : int a, int b 
{
    return a + b;
}
```

## Custom Structs
The current ways to create different types, schemas and classes
### Type
They are a more complex way to struct your memory, types cant have functions
```
Type example {
    int a;
    zit b;
}
```
### Schema
Classes but every variable(if not defined elsehow) are public
```
Schema example {
    int a;
    float b;

    example: int aIn, float bIn {
        a = aIn;
        b = bIn;
    }
}
```
### Class
Every variable(if not defined elsehow) are private
```
Class example {
    int a;
    float b;

    example: int aIn, float bIn {
        a = aIn;
        b = bIn;
    }
}
```

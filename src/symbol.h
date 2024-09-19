#ifndef SYMBOL_H
#define SYMBOL_H

extern int next_index; 

#include <stdlib.h>
#include "ast.h"

typedef struct Symbol {
    char *name;
    int value;
    struct Symbol *next;
    int memoryLocation;
} Symbol;

extern Symbol *symbolTable; 

Symbol *createSymbol(char *name, int value);
void insertSymbol(char *name, int value, int memoryLocation, Symbol **symbolTable);
Symbol *findSymbol(char *name, Symbol *symbolTable);
void printSymbolTable(Symbol *symbolTable);
void declareVariable(char *name, Symbol **symbolTable);
int evaluateExpression(AstNode *node, Symbol *symbolTable);

#endif
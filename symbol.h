#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "ast.h"

// Representation of a symbol
typedef struct Symbol {
    char *name;
    int value;
    struct Symbol *next;
} Symbol;

// Declarations
Symbol *createSymbol(char *name, int value);
void insertSymbol(char *name, int value, Symbol **symbolTable);
Symbol *findSymbol(char *name, Symbol *symbolTable);
void printSymbolTable(Symbol *symbolTable);
void declareVariable(char *name, Symbol **symbolTable);
int evaluateExpression(AstNode *node, Symbol *symbolTable);

#endif
#ifndef SYMBOL_H
#define SYMBOL_H

typedef struct Symbol {
    char *name;
    int value;
    struct Symbol *next;
} Symbol;

Symbol *createSymbol(char *name, int value);
void insertSymbol(char *name, int value);
Symbol *findSymbol(char *name);
void printSymbolTable();
void checkUndeclaredVariable(char *name); 
void declareVariable(char *name);
int evaluateExpression(AstNode *node);



#endif 

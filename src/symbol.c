#include "symbol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int next_index = 1; 

Symbol *createSymbol(char *name, int value) {
    Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->next = NULL;
    return symbol;
}

void insertSymbol(char *name, int value, int memoryLocation, Symbol **symbolTable) {
    Symbol *symbol = createSymbol(name, value);
    symbol->memoryLocation = memoryLocation;
    symbol->next = *symbolTable;
    *symbolTable = symbol;
}

Symbol *findSymbol(char *name, Symbol *symbolTable) {
    Symbol *current = symbolTable;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}



void declareVariable(char *name, Symbol **symbolTable) {
    Symbol *existingSymbol = findSymbol(name, *symbolTable);

    if (existingSymbol == NULL) {
        int memoryLocation = next_index++;
        insertSymbol(name, 0, memoryLocation, symbolTable);
    }
}


void printSymbolTable(Symbol *symbolTable) {
    Symbol *current = symbolTable;
    printf("Symbol Table:\n");
    printf("Name\tValue\n");
    printf("----\t-----\n");
    while (current != NULL) {
        printf("%s\t%d\n", current->name, current->value);
        current = current->next;
    }
}

int evaluateExpression(AstNode *node, Symbol *symbolTable) {
    if (node == NULL) return 0;
    switch (node->nodeType) {
        case NODE_NUMBER:
            return atoi(node->value);
        case NODE_ID: {
            Symbol *symbol = findSymbol(node->value, symbolTable);
            return symbol ? symbol->value : 0;
        }
        case NODE_PLUS:
            return evaluateExpression(node->left, symbolTable) + evaluateExpression(node->right, symbolTable);
        case NODE_MINUS:
            return evaluateExpression(node->left, symbolTable) - evaluateExpression(node->right, symbolTable);
        case NODE_MUL:
            return evaluateExpression(node->left, symbolTable) * evaluateExpression(node->right, symbolTable);
        case NODE_DIV:
            return evaluateExpression(node->left, symbolTable) / evaluateExpression(node->right, symbolTable);
        case NODE_LT:
            return evaluateExpression(node->left, symbolTable) < evaluateExpression(node->right, symbolTable);
        case NODE_EQ:
            return evaluateExpression(node->left, symbolTable) == evaluateExpression(node->right, symbolTable);
        default:
            return 0;
    }
}
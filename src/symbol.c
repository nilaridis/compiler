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
    symbol->memoryLocation = memoryLocation;  // Εκχώρηση της σωστής θέσης μνήμης
    symbol->next = *symbolTable;
    *symbolTable = symbol;
    printf("Inserted variable %s with memory location %d\n", name, symbol->memoryLocation);
}

Symbol *findSymbol(char *name, Symbol *symbolTable) {
    Symbol *current = symbolTable;
    while (current != NULL) {
        printf("Checking variable %s with memory location %d\n", current->name, current->memoryLocation);  // Έλεγχος για κάθε σύμβολο
        if (strcmp(current->name, name) == 0) {
            printf("Found variable %s with memory location %d\n", name, current->memoryLocation);
            return current;
        }
        current = current->next;
    }
    printf("Variable %s not found\n", name);
    return NULL;
}



void declareVariable(char *name, Symbol **symbolTable) {
    Symbol *existingSymbol = findSymbol(name, *symbolTable);

    if (existingSymbol == NULL) {
        int memoryLocation = next_index++;  // Αυξάνουμε το next_index και εκχωρούμε τη νέα διεύθυνση μνήμης
        printf("Assigned memory location %d to variable %s\n", memoryLocation, name);
        insertSymbol(name, 0, memoryLocation, symbolTable);  // Εισάγουμε τη μεταβλητή με τη σωστή διεύθυνση
    } else {
        printf("Variable %s already exists with memory location %d\n", name, existingSymbol->memoryLocation);
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
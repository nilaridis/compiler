#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdio.h>

typedef enum {
    NODE_IF,
    NODE_ELSE,
    NODE_ASSIGN,
    NODE_REPEAT,
    NODE_READ,
    NODE_WRITE,
    NODE_NUMBER,
    NODE_ID,
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_LT,
    NODE_EQ,
    NODE_SEQ
} NodeType;

typedef struct AstNode {
    NodeType nodeType;
    struct AstNode *left;
    struct AstNode *right;
    char *value;
} AstNode;

AstNode *createNode(NodeType nodeType, AstNode *left, AstNode *right, char *value);
void printTree(AstNode *node, int level);

#endif
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* nodeTypeToString(NodeType nodeType) {
    switch (nodeType) {
        case NODE_IF: return "IF";
        case NODE_ELSE: return "ELSE";
        case NODE_ASSIGN: return "ASSIGN";
        case NODE_REPEAT: return "REPEAT";
        case NODE_READ: return "READ";
        case NODE_WRITE: return "WRITE";
        case NODE_NUMBER: return "NUMBER";
        case NODE_ID: return "ID";
        case NODE_PLUS: return "PLUS";
        case NODE_MINUS: return "MINUS";
        case NODE_MUL: return "MUL";
        case NODE_DIV: return "DIV";
        case NODE_LT: return "LT";
        case NODE_EQ: return "EQ";
        case NODE_SEQ: return "SEQ";
        default: return "UNKNOWN";
    }
}

AstNode *createNode(NodeType nodeType, AstNode *left, AstNode *right, char *value) {
    AstNode *node = (AstNode *)malloc(sizeof(AstNode));
    node->nodeType = nodeType;
    node->left = left;
    node->right = right;
    node->value = value ? strdup(value) : NULL;
    return node;
}

void printTree(AstNode *node, int level) {
    if (node == NULL) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s", nodeTypeToString(node->nodeType));
    if (node->value) printf(" (%s)", node->value);
    printf("\n");
    printTree(node->left, level + 1);
    printTree(node->right, level + 1);
}
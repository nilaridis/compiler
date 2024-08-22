#ifndef AST_H
#define AST_H

typedef struct AstNode {
    int nodeType;
    struct AstNode *left;
    struct AstNode *right;
    char *value;
} AstNode;

AstNode *createNode(int nodeType, AstNode *left, AstNode *right, char *value);
void printTree(AstNode *node, int level);



#endif /* AST_H */

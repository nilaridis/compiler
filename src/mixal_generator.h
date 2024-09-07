#ifndef MIXAL_GENERATOR_H
#define MIXAL_GENERATOR_H

#include "ast.h"
#include <stdio.h>  // For FILE
extern FILE *fout;  // Output file stream

// Function to generate MIXAL code from the AST
void printMixal(AstNode *root);

// Helper functions to print MIXAL operations
void printCo(const char* text);
void printLa(int label);
void printLaOpNu(const char* label, const char* operation, int num);
void printLaOpOp(const char* label, const char* operation, const char* operand);
void printVaOpNu(int index, const char* operation, int num);
void printOpOp(const char* operation, const char* operand);
void printOpNu(const char* operation, int num);
void printOpOpNu(const char* operation, const char* operand, int num);
void printOpVa(const char* operation, int index);
void printOpAr(const char* operation, int index);
void printOpLa(const char* operation, int index);
void printOpLi(const char* operation, int value);

// Helper functions for processing the AST
void gen_toA(AstNode *node);
void gen_notA(AstNode *node);
void gen_operation(AstNode *node, const char* opMemory, const char* opConst);
void gen_toA_bynotA(AstNode *node);
void gen_to1_bynotA(AstNode *node);
void gen_to1(AstNode *node);
void gen_checkindex(AstNode *node);
void gen_checkoverflow(int where);

// Main generation function
int gen(AstNode *n);

#endif // MIXAL_GENERATOR_H

#ifndef MIXAL_GENERATOR_H
#define MIXAL_GENERATOR_H

#include "ast.h"
#include "symbol.h"
#include <stdio.h>

extern FILE *fout;
extern Symbol *symbolTable; 

void generateProgramCode(AstNode* node);
void generateMixalCode(AstNode* node);
void generateAssignmentCode(AstNode* node);
void generateIfCode(AstNode* node);
void generateWriteCode(AstNode* node);
void generateSeqCode(AstNode* node);
void generateNumber(AstNode* node);
void generateID(AstNode* node);
void generatePlus(AstNode* node);
void generateMinus(AstNode* node);
void generateMul(AstNode* node);
void generateDiv(AstNode* node);
void generateLT(AstNode* node);
void generateEQ(AstNode* node);
void generateReadCode(AstNode* node);
void generateRepeatCode(AstNode* node);

#endif // MIXAL_GENERATOR_H

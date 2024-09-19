#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol.h"
#include "mixal_generator.h"

int variable_count = 0; 
static int temp_count = 1;

void generateProgramCode(AstNode* node) {
    fprintf(fout, " ORIG 2000\n");
    generateMixalCode(node->left);
    fprintf(fout, " END 2000\n");
}


extern Symbol *symbolTable;

void generateAssignmentCode(AstNode* node) {
    Symbol *symbol = findSymbol(node->left->value, symbolTable);
    
    if (symbol == NULL) {
        declareVariable(node->left->value, &symbolTable);
        symbol = findSymbol(node->left->value, symbolTable);
        fprintf(fout, " STZ %d(0:5)\n", symbol->memoryLocation);
    }

    generateMixalCode(node->right);
    fprintf(fout, " STA %d(0:5)\n", symbol->memoryLocation);
}


void generateNumber(AstNode* node) {
    fprintf(fout, " ENTA %s\n", node->value);
}

void generateID(AstNode* node) {
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", symbol->memoryLocation);
    }
}


void generateIfCode(AstNode* node) {
    if ((node->nodeType == NODE_IF) && (node->right->nodeType != NODE_ELSE)) {

        generateMixalCode(node->left);

        if (node->left->nodeType == NODE_LT) {
            fprintf(fout, " JL THEN\n");
        } else if (node->left->nodeType == NODE_EQ) {
            fprintf(fout, " JE THEN\n");
        }

        fprintf(fout," JMP ENDIF\n");
        fprintf(fout, "THEN NOP\n");
        generateMixalCode(node->right->left);
        generateMixalCode(node->right->right);
        
        fprintf(fout, " JMP ENDIF\n");
        fprintf(fout, "ENDIF NOP\n");
    } else if ((node->nodeType == NODE_IF)&& (node->right->nodeType == NODE_ELSE)){

        generateMixalCode(node->left);

        if (node->left->nodeType == NODE_LT) {
            fprintf(fout, " JL THEN\n");
        } else if (node->left->nodeType == NODE_EQ) {
            fprintf(fout, " JE THEN\n");
        }

        fprintf(fout, " JMP ELSE\n");
        fprintf(fout, "THEN NOP\n");
        generateMixalCode(node->right->left);
        fprintf(fout, " JMP ENDIF\n");
        fprintf(fout, "ELSE NOP\n");
        generateMixalCode(node->right->right);
        fprintf(fout, " JMP ENDIF\n");
        fprintf(fout, "ENDIF NOP\n");
    }
}


void generatePlus(AstNode* node) {
    int addTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", addTemp);

    generateMixalCode(node->left);
    fprintf(fout, " STA TEMP%d\n", addTemp);
    generateMixalCode(node->right);
    fprintf(fout, " ADD TEMP%d\n", addTemp);
}



void generateMinus(AstNode* node) {
    int subTemp = temp_count++;
    fprintf(fout, "OPPTEMP EQU 0\n");
    fprintf(fout, "TEMP%d EQU 0\n", subTemp);

    generateMixalCode(node->left);
    fprintf(fout, " STA TEMP%d\n", subTemp);

    generateMixalCode(node->right);
    fprintf(fout, " SUB TEMP%d\n", subTemp);

    fprintf(fout, " STA OPPTEMP\n");
    fprintf(fout, " ENTA 0\n");
    fprintf(fout, " SUB OPPTEMP\n");
}


void generateMul(AstNode* node) {
    int mulTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", mulTemp);

    fprintf(fout, " STZ TEMP%d\n", mulTemp);

    generateMixalCode(node->left);

    fprintf(fout, " STA TEMP%d\n", mulTemp);

    generateMixalCode(node->right);

    fprintf(fout, " MUL TEMP%d\n", mulTemp);
    fprintf(fout, " STX TEMP%d\n", mulTemp);
    fprintf(fout, " LDA TEMP%d\n", mulTemp);
    fprintf(fout, " ENTX 0\n");
}


void generateDiv(AstNode* node) {
    int divTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", divTemp);
    fprintf(fout, "SWAPTEMP EQU 1\n");

    generateMixalCode(node->left);

    fprintf(fout, " STA TEMP%d\n", divTemp);
    temp_count++;

    generateMixalCode(node->right);

    // Εκτέλεση της διαίρεσης, αντιστροφή και φόρτωση τιμών
    fprintf(fout, " STA SWAPTEMP\n");          // Αποθήκευση του A στο SWAPTEMP
    fprintf(fout, " LDX SWAPTEMP\n");          // Φόρτωση του A στον X
    fprintf(fout, " LDA TEMP%d\n", divTemp);   // Φόρτωση του προσωρινού αποτελέσματος στον A
    fprintf(fout, " STX TEMP%d\n", divTemp);   // Αποθήκευση του X στο TEMP
    fprintf(fout, " STA SWAPTEMP\n");          // Αποθήκευση του A στο SWAPTEMP
    fprintf(fout, " LDX SWAPTEMP\n");          // Φόρτωση του SWAPTEMP στον X
    fprintf(fout, " ENTA 0\n");                // Μηδενισμός του καταχωρητή A
    fprintf(fout, " DIV TEMP%d\n", divTemp);   // Διαίρεση με το TEMP
}


void generateLT(AstNode* node) {
    generateMixalCode(node->left);

    Symbol *symbol = findSymbol(node->right->value, symbolTable);
    if (symbol != NULL) {
        fprintf(fout, " CMPA %d(0:5)\n", symbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->right->value);
    }
}


void generateEQ(AstNode* node) {
    generateMixalCode(node->left);

    Symbol *symbol = findSymbol(node->right->value, symbolTable);
    if (symbol != NULL) {
        fprintf(fout, " CMPA %d(0:5)\n", symbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->right->value);
    }
}

void generateReadCode(AstNode* node) {
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        int input_buffer_address = 1000;
        int input_device = 19;

        fprintf(fout, " IN %d(%d)\n", input_buffer_address, input_device);

        fprintf(fout, " JBUS *(%d)\n", input_device);

        fprintf(fout, " LDX %d(0:5)\n", input_buffer_address);

        fprintf(fout, " NUM\n");

        fprintf(fout, " STA %d(0:5)\n", symbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->value);
    }
}

void generateRepeatCode(AstNode* node) {
    static int repeatLabelCounter = 0;
    int currentRepeatLabel = repeatLabelCounter++;

    fprintf(fout, "REPEAT%d NOP\n", currentRepeatLabel);
    generateMixalCode(node->left);

    Symbol *rightSymbol = findSymbol(node->right->left->value, symbolTable);
    if (rightSymbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", rightSymbol->memoryLocation);

        if (node->right->right->nodeType == NODE_NUMBER) {
            fprintf(fout, " ENTA %s\n", node->right->right->value);
        }

        fprintf(fout, " CMPA 1(0:5)\n");
        fprintf(fout, " JE ENDREPEAT%d\n", currentRepeatLabel);
    }

    fprintf(fout, " JMP REPEAT%d\n", currentRepeatLabel);

    fprintf(fout, "ENDREPEAT%d NOP\n", currentRepeatLabel);
}

int write_counter = 0;

void generateWriteCode(AstNode* node) {
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        int current_write = write_counter++;

        fprintf(fout, " LDA %d(0:5)\n", symbol->memoryLocation);

        fprintf(fout, " CHAR\n");

        fprintf(fout, " STA 1987(0:5)\n");

        fprintf(fout, " STX 1988(0:5)\n");

        fprintf(fout, " ENTX 45\n");
        fprintf(fout, " JAN KPO%d\n", current_write);
        fprintf(fout, " ENTX 44\n");
        fprintf(fout, "KPO%d NOP\n", current_write);

        fprintf(fout, " STX 1986(0:5)\n");

        fprintf(fout, " OUT 1986(2:3)\n");
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->value);
    }
}


void generateSeqCode(AstNode* node) {
    if (node->nodeType == NODE_SEQ) {
        generateMixalCode(node->left);
        generateMixalCode(node->right);
    }
}


void generateMixalCode(AstNode* node) {
    if (node == NULL) return;

    switch (node->nodeType) {
        case NODE_PROGRAM:
            generateProgramCode(node);
            break;
        case NODE_ASSIGN:
            generateAssignmentCode(node);
            break;
        case NODE_IF:
            generateIfCode(node);
            break;
        case NODE_WRITE:
            generateWriteCode(node);
            break;
        case NODE_READ:
            generateReadCode(node);
            break;
        case NODE_SEQ:
            generateSeqCode(node);
            break;
        case NODE_NUMBER:
            generateNumber(node);
            break;
        case NODE_ID:
            generateID(node);
            break;
        case NODE_PLUS:
            generatePlus(node);
            break;
        case NODE_MINUS:
            generateMinus(node);
            break;
        case NODE_MUL:
            generateMul(node);
            break;
        case NODE_DIV:
            generateDiv(node);
            break;
        case NODE_LT:
            generateLT(node);
            break;
        case NODE_EQ:
            generateEQ(node);
            break;
        case NODE_REPEAT:
            generateRepeatCode(node);
            break;
        default:
            fprintf(fout, "NOP\n");
    }
}

#include "mixal_generator.h"
#include <stdio.h>
#include <stdlib.h>

FILE *fout; // Output file stream

int next_index = 1;
int next_location = 2000;
int next_label = 1;
int next_buf = 0;

void printGlobalVariables() {
    // Print the initial global variables and constants
    printLaOpNu("BUF1", "CON", 0);
    printLaOpNu("BUF2", "CON", 0);
    printLaOpNu("BUF3", "CON", 0);
    printLaOpNu("TMP", "EQU", 3000);
    printLaOpNu("BUF", "EQU", 3001);
}

void declareMixalVariable(const char* name, int location) {
    fprintf(fout, "%s\tEQU\t%d\n", name, location);
}

void printStart() {
    // Print the start of the program
    printCo("start");
    printLaOpNu("", "ORIG", 100);
    printLaOpOp("START", "NOP", "");
}

void printEnd() {
    // Print the end of the program
    printCo("end");
    printLaOpNu("", "ENTA", 0);  // rA = 0
    printLaOpOp("", "HLT", "");  // Halt
    printLaOpNu("ERR", "ENTA", 1);  // Error: rA = 1
    printLaOpOp("", "HLT", "");  // Halt
    printLaOpOp("", "END", "START");  // End of program
}


// Function to start generating MIXAL code
void printMixal(AstNode *root) {
    next_index = 1;
    next_location = 2000;
    next_label = 1;
    next_buf = 0;

    printGlobalVariables();
    printCo("global variables");

    // Declare initial variables
    declareMixalVariable("V1", 2000);  // V1
    declareMixalVariable("V2", 2001);  // V2

    // Generate code for all the nodes in the syntax tree
    gen(root);

    // Define labels used for jumps (e.g., L1)
    fprintf(fout, "L1\tNOP\t\n");

    // Add start, print statements, and end of program
    printStart();
    printCo("statements");
    gen(root);  // Generate statements
    printEnd();
}

// Print comment
void printCo(const char* text) {
    fprintf(fout, "*\t\t\t\t\t%s\n", text);
}

// Print label
void printLa(int label) {
    fprintf(fout, "L%d\tNOP\t\n", label);
}

// Print label, operation, and number
void printLaOpNu(const char* label, const char* operation, int num) {
    fprintf(fout, "%s\t%s\t%d\n", label, operation, num);
}

// Print label, operation, and operand
void printLaOpOp(const char* label, const char* operation, const char* operand) {
    fprintf(fout, "%s\t%s\t%s\n", label, operation, operand);
}

// Print variable, operation, and number
void printVaOpNu(int index, const char* operation, int num) {
    fprintf(fout, "V%d\t%s\t%d\n", index, operation, num);
}

// Print operation and operand
void printOpOp(const char* operation, const char* operand) {
    fprintf(fout, "\t%s\t%s\n", operation, operand);
}

// Print operation and number
void printOpNu(const char* operation, int num) {
    fprintf(fout, "\t%s\t%d\n", operation, num);
}

// Print operation, operand, and number
void printOpOpNu(const char* operation, const char* operand, int num) {
    fprintf(fout, "\t%s\t%s%d\n", operation, operand, num);
}

// Print operation and variable
void printOpVa(const char* operation, int index) {
    fprintf(fout, "\t%s\tV%d\n", operation, index);
}

// Print operation and array
void printOpAr(const char* operation, int index) {
    fprintf(fout, "\t%s\tV%d,1\n", operation, index);
}

// Print operation and label
void printOpLa(const char* operation, int index) {
    fprintf(fout, "\t%s\tL%d\n", operation, index);
}

// Print operation and literal value
void printOpLi(const char* operation, int value) {
    fprintf(fout, "\t%s\t=%d=\n", operation, value);
}

// Main generation function, processes nodes recursively
int gen(AstNode *n) {
    if (!n) return 0;

    switch(n->nodeType) {
        case NODE_SEQ:
            // Process left and right nodes in sequence
            gen(n->left);
            gen(n->right);
            break;

        case NODE_ASSIGN:
            // Handle assignment node
            gen(n->right);  // Evaluate right-hand side expression
            printOpVa("STA", next_index++);  // Store result in the variable
            next_location++;  // Increment memory location
            break;

        case NODE_IF:
            // Handle if statement
            gen(n->left);  // Evaluate condition (should be comparison)
            printOpVa("CMPA", next_index - 1);  // Compare with variable
            printOpLa("JAZ", next_label);  // Jump if zero
            next_label++;  // Increment label counter
            break;

        case NODE_WRITE:
            // Handle write statement
            gen(n->left);  // Evaluate expression to print
            printCo("print start");
            printOpOp("ENTX", "44");
            printOpOp("JANN", "*+2");
            printOpOp("ENTX", "45");
            printOpOp("JBUS", "*(18)");
            printOpOp("STX", "BUF1");
            printOpOp("CHAR", "");
            printOpOp("STA", "BUF2");
            printOpOp("STX", "BUF3");
            printOpOp("OUT", "BUF1(18)");
            printCo("print end");
            break;

        case NODE_NUMBER:
            // Handle number node
            printOpNu("ENTA", atoi(n->value));  // Load number into register rA
            break;

        case NODE_ID:
            // Handle identifier node (variable)
            printOpVa("LDA", next_index++);  // Load variable into register rA
            break;

        case NODE_LT:  // Handle less than (<) comparison
            gen(n->left);  // Evaluate left-hand side
            gen(n->right);  // Evaluate right-hand side
            printOpOp("CMPA", n->right->value);  // Compare the two values
            printOpOp("JL", "*+2");  // If less, jump
            break;

        case NODE_EQ:  // Handle equality (==) comparison
            gen(n->left);  // Evaluate left-hand side
            gen(n->right);  // Evaluate right-hand side
            printOpOp("CMPA", n->right->value);  // Compare the two values
            printOpOp("JE", "*+2");  // If equal, jump
            break;

        // Add cases for arithmetic and logical operations (PLUS, MINUS, etc.)
        case NODE_PLUS:
            gen(n->left);
            gen(n->right);
            printOpOp("ADD", "");  // Add two operands
            break;

        case NODE_MINUS:
            gen(n->left);
            gen(n->right);
            printOpOp("SUB", "");  // Subtract two operands
            break;

        // Add other cases here...

        default:
            fprintf(fout, "*UNKNOWN NODE TYPE\n");
            break;
    }

    return 0;
}

// Helper Functions

// Symbol in rA.
// Moves a given symbol to rA.
void gen_toA(AstNode *node) {
    if (node->nodeType == NODE_NUMBER) {
        printOpNu("LDA", atoi(node->value));
    } else if (node->nodeType == NODE_ID) {
        printOpVa("LDA", next_index++);
    }
}

// Symbol not in rA.
// Moves elsewhere content related to a given symbol from rA so it can be written over.
void gen_notA(AstNode *node) {
    if (next_buf) {
        printOpOpNu("STA", "BUF+", next_buf);
    } else {
        printOpOp("STA", "BUF");
    }
    next_buf++;

    if (next_buf > 999) {
        fprintf(stderr, "Memory overflow\n");
        exit(1);
    }
}

// Moves a given symbol to rA and restores the changes from notA().
void gen_toA_bynotA(AstNode *node) {
    gen_toA(node);  // Just move the node to rA (restore from BUF)
    next_buf--;
    if (next_buf) {
        printOpOpNu("LDA", "BUF+", next_buf);
    } else {
        printOpOp("LDA", "BUF");
    }
}

// Moves a given symbol to r1 and restores the changes from notA().
void gen_to1_bynotA(AstNode *node) {
    if (node->nodeType == NODE_NUMBER) {
        printOpNu("ENT1", atoi(node->value));
    } else {
        gen_notA(node);  // Move content elsewhere first
        next_buf--;
        if (next_buf) {
            printOpOpNu("LD1", "BUF+", next_buf);
        } else {
            printOpOp("LD1", "BUF");
        }
    }
}

// Symbol in r1.
// Moves a given symbol to r1.
void gen_to1(AstNode *node) {
    if (node->nodeType == NODE_NUMBER) {
        printOpNu("ENT1", atoi(node->value));
    } else if (node->nodeType == NODE_ID) {
        printOpVa("LD1", next_index++);
    }
}

// Checks a given symbol used as an index if it's negative. If so, terminate execution.
void gen_checkindex(AstNode *node) {
    if (node->nodeType == NODE_NUMBER) {
        if (atoi(node->value) < 0) printOpOp("JMP", "ERR");
    } else {
        printOpOp("J1N", "ERR");
    }
}

// Checks if overflow occurred. If so, terminate execution.
void gen_checkoverflow(int where) {
    switch (where) {
        case 0:
            printOpOp("JOV", "ERR");
            break;
        case 1:
            printOpOp("JANZ", "ERR");
            break;
    }
}

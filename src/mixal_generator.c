#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol.h"
#include "mixal_generator.h"


int variable_count = 0; 

void generateProgramCode(AstNode* node) {
    fprintf(fout, " ORIG 2000\n");  // Ορισμός αρχικής διεύθυνσης του MIXAL προγράμματος
    generateMixalCode(node->left);  // Εκτέλεση του υπόλοιπου προγράμματος
    fprintf(fout, " END 2000\n");  // Τέλος προγράμματος
}

// Παράδειγμα συνάρτησης για παραγωγή MIXAL για κόμβο ανάθεσης
extern Symbol *symbolTable;  // Δήλωση του πίνακα συμβόλων

void generateAssignmentCode(AstNode* node) {
    // Δηλώνουμε τη μεταβλητή αν δεν υπάρχει ήδη
    Symbol *symbol = findSymbol(node->left->value, symbolTable);
    
    if (symbol == NULL) {
        // Αν η μεταβλητή δεν υπάρχει, τη δηλώνουμε και κάνουμε μηδενισμό της τιμής
        declareVariable(node->left->value, &symbolTable);
        symbol = findSymbol(node->left->value, symbolTable);
        fprintf(fout, " STZ %d(0:5)\n", symbol->memoryLocation);  // Μηδενισμός μόνο την πρώτη φορά
    }

    // Τώρα εκχωρούμε τη νέα τιμή
    generateMixalCode(node->right);  // Υπολογισμός του δεξιού μέρους της ανάθεσης
    fprintf(fout, " STA %d(0:5)\n", symbol->memoryLocation);  // Αποθήκευση της τιμής στη μεταβλητή
}





void generateNumber(AstNode* node) {
    fprintf(fout, " ENTA %s\n", node->value);  // Φορτώνει τον αριθμό στην A
}

void generateID(AstNode* node) {
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", symbol->memoryLocation);  // Χρήση της σωστής διεύθυνσης μνήμης
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->value);
    }
}

// Παράδειγμα συνάρτησης για παραγωγή MIXAL για κόμβο IF
void generateIfCode(AstNode* node) {
    if (node->nodeType == NODE_IF) {
        generateMixalCode(node->left);  // Συνθήκη (π.χ., 30 < x)
        fprintf(fout, " CMPA %d(0:5)\n", findSymbol(node->left->right->value, symbolTable)->memoryLocation);  // Σύγκριση με το x
        fprintf(fout, " JL THEN\n");  // Άλμα στο 'then' block αν η συνθήκη είναι αληθής
        fprintf(fout, " JMP ELSE\n");  // Άλμα στο 'else' block αν η συνθήκη είναι ψευδής

        // THEN block
        fprintf(fout, "THEN NOP\n");
        generateMixalCode(node->right->left);  // Εντολές μέσα στο THEN block (π.χ., fact := 6)

        // Άλμα έξω από το 'else' block για να παρακαμφθεί
        fprintf(fout, " JMP ENDIF\n");

        // ELSE block
        fprintf(fout, "ELSE NOP\n");
        generateMixalCode(node->right->right);  // Εντολές μέσα στο ELSE block (π.χ., fact := 10, x := x - 1)

        // Τέλος IF-ELSE block
        fprintf(fout, "ENDIF NOP\n");
    }
}


void generatePlus(AstNode* node) {
    Symbol *leftSymbol = findSymbol(node->left->value, symbolTable);
    if (leftSymbol != NULL) {
        // Φόρτωση της τιμής του αριστερού ορίσματος (fact) στον A
        fprintf(fout, " LDA %d(0:5)\n", leftSymbol->memoryLocation);

        // Έλεγχος αν το δεξί όρισμα είναι αριθμός ή μεταβλητή (ID)
        if (node->right->nodeType == NODE_NUMBER) {
            // Αποθήκευση του αριθμού στο X
            fprintf(fout, " ENTX %s\n", node->right->value);
        } else if (node->right->nodeType == NODE_ID) {
            // Φόρτωση της τιμής της μεταβλητής στο X
            Symbol *rightSymbol = findSymbol(node->right->value, symbolTable);
            if (rightSymbol != NULL) {
                fprintf(fout, " LDX %d(0:5)\n", rightSymbol->memoryLocation);
            }
        }

        // Αποθήκευση της τιμής του X προσωρινά και πρόσθεση
        fprintf(fout, " STX 0(0:5)\n");  // Αποθήκευση της τιμής του X στη θέση 0
        fprintf(fout, " ADD 0(0:5)\n");  // Πρόσθεση της τιμής του X με την τιμή του A

        // Αποθήκευση του νέου αποτελέσματος ξανά στη μεταβλητή fact
        fprintf(fout, " STA %d(0:5)\n", leftSymbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->left->value);
    }
}



void generateMinus(AstNode* node) {
    Symbol *leftSymbol = findSymbol(node->left->value, symbolTable);
    if (leftSymbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", leftSymbol->memoryLocation);  // Φόρτωση της τιμής του fact στον accumulator

        // Έλεγχος αν το δεξί όρισμα είναι αριθμός ή μεταβλητή (ID)
        if (node->right->nodeType == NODE_NUMBER) {
            fprintf(fout, " ENTX %s\n", node->right->value);   // Φόρτωση αριθμού στο X
        } else if (node->right->nodeType == NODE_ID) {
            Symbol *rightSymbol = findSymbol(node->right->value, symbolTable);
            if (rightSymbol != NULL) {
                fprintf(fout, " LDX %d(0:5)\n", rightSymbol->memoryLocation);  // Φόρτωση της τιμής της μεταβλητής στο X
            }
        }

        // Αποθήκευση του X προσωρινά και αφαίρεση
        fprintf(fout, " STX 0(0:5)\n");  // Αποθήκευση της τιμής του X προσωρινά
        fprintf(fout, " SUB 0(0:5)\n");  // Αφαίρεση της τιμής του X από τον A

        // Αποθήκευση του νέου αποτελέσματος στη μεταβλητή fact
        fprintf(fout, " STA %d(0:5)\n", leftSymbol->memoryLocation);
    }
}


void generateMul(AstNode* node) {
    Symbol *leftSymbol = findSymbol(node->left->value, symbolTable);
    if (leftSymbol != NULL) {
        // Φόρτωση της τιμής του αριστερού ορίσματος (fact) στον A
        fprintf(fout, " LDA %d(0:5)\n", leftSymbol->memoryLocation);

        // Έλεγχος αν το δεξί όρισμα είναι αριθμός ή μεταβλητή (ID)
        if (node->right->nodeType == NODE_NUMBER) {
            // Αποθήκευση του αριθμού στο X
            fprintf(fout, " ENTX %s\n", node->right->value);
        } else if (node->right->nodeType == NODE_ID) {
            // Φόρτωση της τιμής της μεταβλητής στο X
            Symbol *rightSymbol = findSymbol(node->right->value, symbolTable);
            if (rightSymbol != NULL) {
                fprintf(fout, " LDX %d(0:5)\n", rightSymbol->memoryLocation);
            }
        }

        // Αποθήκευση της τιμής του X προσωρινά και πολλαπλασιασμός
        fprintf(fout, " STX 0(0:5)\n");  // Αποθήκευση της τιμής του X στη θέση 0
        fprintf(fout, " MUL 0(0:5)\n");  // Πολλαπλασιασμός της τιμής του X με την τιμή του A

        // Αποθήκευση των αποτελεσμάτων του A και X
        fprintf(fout, " STA 0(0:0)\n");  // Αποθήκευση του A
        fprintf(fout, " STX 0(1:5)\n");  // Αποθήκευση του X

        // Αποθήκευση του νέου αποτελέσματος στη μεταβλητή fact
        fprintf(fout, " LDA 0(0:5)\n");  // Φόρτωση του αποτελέσματος
        fprintf(fout, " STA %d(0:5)\n", leftSymbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->left->value);
    }
}


void generateDiv(AstNode* node) {
    // Φόρτωση του αριστερού ορίσματος (fact) στον accumulator (A)
    Symbol *leftSymbol = findSymbol(node->left->value, symbolTable);
    if (leftSymbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", leftSymbol->memoryLocation);  // Φόρτωση του αριστερού ορίσματος (fact)

        // Έλεγχος για το δεξί όρισμα (αριθμός ή ID)
        if (node->right->nodeType == NODE_NUMBER) {
            // Έλεγχος για διαίρεση με το μηδέν
            if (strcmp(node->right->value, "0") == 0) {
                fprintf(fout, " JMP DIVZERO\n");  // Άλμα σε ετικέτα DIVZERO αν το δεξί μέρος είναι 0
                return;
            }
            fprintf(fout, " ENTX %s\n", node->right->value);  // Φόρτωση του αριθμού στο X
        } else if (node->right->nodeType == NODE_ID) {
            Symbol *rightSymbol = findSymbol(node->right->value, symbolTable);
            if (rightSymbol != NULL) {
                fprintf(fout, " LDX %d(0:5)\n", rightSymbol->memoryLocation);  // Φόρτωση του δεξιού ορίσματος στο X

                // Έλεγχος αν η τιμή είναι μηδέν
                fprintf(fout, " ENT1 0\n");
                fprintf(fout, " CMP1 %d(0:5)\n", rightSymbol->memoryLocation);  // Σύγκριση με το 0
                fprintf(fout, " JE DIVZERO\n");  // Αν είναι μηδέν, πήγαινε σε DIVZERO
            }
        }

        // Μηδενισμός του καταχωρητή X πριν τη διαίρεση
        fprintf(fout, " STX 0(0:5)\n");
        fprintf(fout, " ENTX 0\n");

        // Εκτέλεση της διαίρεσης
        fprintf(fout, " DIV 0(0:5)\n");

        // Αποθήκευση του αποτελέσματος της διαίρεσης στον accumulator (A)
        fprintf(fout, " STA %d(0:5)\n", leftSymbol->memoryLocation);

        // Ολοκλήρωση
        fprintf(fout, " JMP ENDDIV\n");
        fprintf(fout, "DIVZERO NOP\n");
        fprintf(fout, " HLT\n");
        fprintf(fout, "ENDDIV NOP\n");
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->left->value);
    }
}








void generateLT(AstNode* node) {
    // Αριστερή τιμή (30)
    generateMixalCode(node->left);

    // Δεξιά τιμή (x, το οποίο είναι σε μια διεύθυνση μνήμης)
    Symbol *symbol = findSymbol(node->right->value, symbolTable);
    if (symbol != NULL) {
        fprintf(fout, " CMPA %d(0:5)\n", symbol->memoryLocation);  // Σύγκριση με τη διεύθυνση του x
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->right->value);
    }
}


void generateEQ(AstNode* node) {
    Symbol *leftSymbol = findSymbol(node->left->value, symbolTable);
    if (leftSymbol != NULL) {
        // Φόρτωση της αριστερής τιμής (π.χ., x) στον καταχωρητή A
        fprintf(fout, " LDA %d(0:5)\n", leftSymbol->memoryLocation);

        // Έλεγχος αν η δεξιά τιμή είναι αριθμός ή μεταβλητή
        if (node->right->nodeType == NODE_NUMBER) {
            // Αν είναι αριθμός, συγκρίνουμε με αυτόν τον αριθμό
            fprintf(fout, " ENTA %s\n", node->right->value);  // Φόρτωση του αριθμού στον A
        } else if (node->right->nodeType == NODE_ID) {
            // Αν είναι μεταβλητή, συγκρίνουμε με τη διεύθυνση της μεταβλητής στη μνήμη
            Symbol *rightSymbol = findSymbol(node->right->value, symbolTable);
            if (rightSymbol != NULL) {
                // Φόρτωση της δεξιάς τιμής στον X
                fprintf(fout, " LDX %d(0:5)\n", rightSymbol->memoryLocation);
            } else {
                fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->right->value);
                return;
            }
        }

        // Τώρα κάνουμε σύγκριση της τιμής στον A με τη διεύθυνση της δεξιάς τιμής
        fprintf(fout, " CMPA %d(0:5)\n", leftSymbol->memoryLocation);

        // Αν είναι ίσες, κάνουμε άλμα σε συγκεκριμένη ετικέτα
        fprintf(fout, " JE EQUAL\n");
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->left->value);
    }
}



void generateReadCode(AstNode* node) {
    fprintf(fout, " IN 1986(0:5)\n");  // Είσοδος τιμής
    fprintf(fout, " STA %s(0:5)\n", node->value);  // Αποθήκευση τιμής
}

void generateRepeatCode(AstNode* node) {
    static int repeatLabelCounter = 0;
    int currentLabel = repeatLabelCounter++;

    // Αρχή του repeat
    fprintf(fout, "REPEAT%d NOP\n", currentLabel);

    // Γεννάμε τον κώδικα για το σώμα της επανάληψης (π.χ., x := x - 1)
    generateMixalCode(node->left);

    // Φορτώνουμε την τιμή της μεταβλητής x για σύγκριση
    Symbol *rightSymbol = findSymbol(node->right->left->value, symbolTable);  // Αν το x είναι το αριστερό παιδί
    if (rightSymbol != NULL) {
        fprintf(fout, " LDA %d(0:5)\n", rightSymbol->memoryLocation);  // Φορτώνουμε το x

        // Γεννάμε τον κώδικα για την τιμή σύγκρισης (π.χ., until x = 2)
        if (node->right->right->nodeType == NODE_NUMBER) {
            fprintf(fout, " ENTA %s\n", node->right->right->value);  // Φορτώνουμε το 2 για σύγκριση
        }

        // Σύγκριση με 2 και έξοδος από τη βρόχο αν η συνθήκη ικανοποιείται
        fprintf(fout, " CMPA 1(0:5)\n");  // Σύγκριση του A με 0 (το οποίο θα έχει φορτωθεί με το 2)
        fprintf(fout, " JE ENDREPEAT%d\n", currentLabel);  // Έξοδος από τον βρόχο αν x = 2
    }

    // Αν η συνθήκη δεν ικανοποιείται, επιστροφή στην αρχή της βρόχου
    fprintf(fout, " JMP REPEAT%d\n", currentLabel);

    // Τέλος της βρόχου
    fprintf(fout, "ENDREPEAT%d NOP\n", currentLabel);
}




int write_counter = 0;  // Μετρητής για μοναδικές ετικέτες

void generateWriteCode(AstNode* node) {
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        int current_write = write_counter++;  // Χρησιμοποιούμε διαφορετικό αριθμό για κάθε ετικέτα

        // Φόρτωση της τιμής της μεταβλητής στον καταχωρητή A
        fprintf(fout, " LDA %d(0:5)\n", symbol->memoryLocation);

        // Μετατροπή της τιμής σε χαρακτήρα
        fprintf(fout, " CHAR\n");

        // Αποθήκευση της τιμής στη μνήμη (1987)
        fprintf(fout, " STA 1987(0:5)\n");

        // Αποθήκευση του καταχωρητή X στη μνήμη (1988)
        fprintf(fout, " STX 1988(0:5)\n");

        // Εντολές για μορφοποίηση της εκτύπωσης
        fprintf(fout, " ENTX 45\n");
        fprintf(fout, " JAN KPO%d\n", current_write);  // Μοναδική ετικέτα για κάθε `write`
        fprintf(fout, " ENTX 44\n");
        fprintf(fout, "KPO%d NOP\n", current_write);  // Μοναδική ετικέτα για το σημείο άλματος

        // Αποθήκευση του καταχωρητή X στη διεύθυνση 1986
        fprintf(fout, " STX 1986(0:5)\n");

        // Εκτύπωση της τιμής από τη μνήμη
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


// Κεντρική συνάρτηση που καλεί τη σωστή συνάρτηση για κάθε τύπο κόμβου
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
            fprintf(fout, "NOP\n");  // Προεπιλογή για άγνωστους κόμβους
    }
}

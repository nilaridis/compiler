#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol.h"
#include "mixal_generator.h"

int variable_count = 0; 
static int temp_count = 1;

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
    if ((node->nodeType == NODE_IF) && (node->right->nodeType != NODE_ELSE)) {
        generateMixalCode(node->left);  // Συνθήκη (π.χ., 30 < x)
        fprintf(fout, " JL THEN\n");  // Άλμα στο 'then' block αν η συνθήκη είναι αληθής
        // fprintf(fout, " JMP ELSE\n");  // Άλμα στο 'else' block αν η συνθήκη είναι ψευδής

        // THEN block
        fprintf(fout, "THEN NOP\n");
        generateMixalCode(node->right->left);  // Εντολές μέσα στο THEN block (π.χ., fact := 6)

        // Άλμα έξω από το 'else' block για να παρακαμφθεί
        generateMixalCode(node->right->right);  // Εντολές μέσα στο ELSE block (π.χ., fact := 10, x := x - 1)
        fprintf(fout, " JMP ENDIF\n");

        // ELSE block
        // fprintf(fout, "ELSE NOP\n");

        // Τέλος IF-ELSE block
        fprintf(fout, "ENDIF NOP\n");
    } else if ((node->nodeType == NODE_IF)&& (node->right->nodeType == NODE_ELSE)){
        generateMixalCode(node->left);
        fprintf(fout, " JL THEN\n");
        fprintf(fout, " JMP ELSE\n");
        fprintf(fout, "THEN NOP\n");
        generateMixalCode(node->right->left);  // Εντολές μέσα στο THEN block (π.χ., fact := 6)
        fprintf(fout, " JMP ENDIF\n");
        fprintf(fout, "ELSE NOP\n");
        generateMixalCode(node->right->right);  // Εντολές μέσα στο ELSE block (π.χ., fact := 10, x := x - 1)
        fprintf(fout, " JMP ENDIF\n");
        fprintf(fout, "ENDIF NOP\n");
    }
}


void generatePlus(AstNode* node) {
    int addTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", addTemp);

    // Γεννάμε τον κώδικα για το αριστερό όρισμα
    generateMixalCode(node->left);
    
    // Αποθήκευση του προσωρινού αποτελέσματος
    fprintf(fout, " STA TEMP%d\n", addTemp);

    // Γεννάμε τον κώδικα για το δεξί όρισμα
    generateMixalCode(node->right);

    // Πρόσθεση του προσωρινού αποτελέσματος με το νέο
    fprintf(fout, " ADD TEMP%d\n", addTemp);
}



void generateMinus(AstNode* node) {
    int subTemp = temp_count++;
    fprintf(fout, "OPPTEMP EQU 0\n");
    fprintf(fout, "TEMP%d EQU 0\n", subTemp);

    // Γεννάμε τον κώδικα για το αριστερό όρισμα
    generateMixalCode(node->left);

    // Αποθήκευση του προσωρινού αποτελέσματος
    fprintf(fout, " STA TEMP%d\n", subTemp);

    // Γεννάμε τον κώδικα για το δεξί όρισμα
    generateMixalCode(node->right);

    // Αφαίρεση του προσωρινού αποτελέσματος από το τρέχον αποτέλεσμα
    fprintf(fout, " SUB TEMP%d\n", subTemp);

    // Προαιρετικά: Μετατροπή A σε 0-A (αν απαιτείται για την έκφραση)
    fprintf(fout, " STA OPPTEMP\n");
    fprintf(fout, " ENTA 0\n");
    fprintf(fout, " SUB OPPTEMP\n");
}



void generateMul(AstNode* node) {
    int mulTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", mulTemp);

    // Μηδενισμός της προσωρινής μεταβλητής πριν τη χρήση
    fprintf(fout, " STZ TEMP%d\n", mulTemp);

    // Γεννάμε τον κώδικα για το αριστερό όρισμα
    generateMixalCode(node->left);

    // Αποθήκευση του προσωρινού αποτελέσματος
    fprintf(fout, " STA TEMP%d\n", mulTemp);

    // Γεννάμε τον κώδικα για το δεξί όρισμα
    generateMixalCode(node->right);

    // Φόρτωση του προσωρινού αποτελέσματος και πολλαπλασιασμός
    fprintf(fout, " MUL TEMP%d\n", mulTemp);  // Πολλαπλασιασμός με το προηγούμενο προσωρινό αποτέλεσμα
    fprintf(fout, " STX TEMP%d\n", mulTemp);  // Αποθήκευση του X στο προσωρινό
    fprintf(fout, " LDA TEMP%d\n", mulTemp);  // Φόρτωση του αποτελέσματος στον A
    fprintf(fout, " ENTX 0\n");               // Μηδενισμός του X μετά την πράξη
}


void generateDiv(AstNode* node) {
    int divTemp = temp_count++;
    fprintf(fout, "TEMP%d EQU 0\n", divTemp);
    fprintf(fout, "SWAPTEMP EQU 1\n");

    // Μηδενισμός της προσωρινής μεταβλητής πριν τη χρήση
    // fprintf(fout, " STZ TEMP%d\n", divTemp);

    // Γεννάμε τον κώδικα για το αριστερό όρισμα (διαιρούμενο)
    generateMixalCode(node->left);

    // Αποθήκευση του προσωρινού αποτελέσματος
    fprintf(fout, " STA TEMP%d\n", divTemp);
    temp_count++;

    // Γεννάμε τον κώδικα για το δεξί όρισμα (διαιρέτης)
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
    Symbol *symbol = findSymbol(node->value, symbolTable);
    if (symbol != NULL) {
        int input_buffer_address = 1000;  // Buffer εισόδου
        int input_device = 19;            // Συσκευή εισόδου (πληκτρολόγιο)

        // Εντολή για ανάγνωση από τη μονάδα 19
        fprintf(fout, " IN %d(%d)\n", input_buffer_address, input_device);

        // Έλεγχος της κατάστασης της συσκευής εισόδου
        fprintf(fout, " JBUS *(%d)\n", input_device);

        // Φόρτωση των δεδομένων που διαβάστηκαν στον καταχωρητή X
        fprintf(fout, " LDX %d(0:5)\n", input_buffer_address);

        // Μετατροπή της τιμής σε αριθμητική μορφή
        fprintf(fout, " NUM\n");

        // Αποθήκευση της τιμής στη μεταβλητή (μόνο τα πρώτα 5 bytes)
        fprintf(fout, " STA %d(0:5)\n", symbol->memoryLocation);
    } else {
        fprintf(stderr, "Error: Variable %s not found in symbol table.\n", node->value);
    }
}

void generateRepeatCode(AstNode* node) {
    static int repeatLabelCounter = 0;
    int currentRepeatLabel = repeatLabelCounter++;

    // Αρχή του repeat
    fprintf(fout, "REPEAT%d NOP\n", currentRepeatLabel);

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
        fprintf(fout, " JE ENDREPEAT%d\n", currentRepeatLabel);  // Έξοδος από τον βρόχο αν x = 2
    }

    // Αν η συνθήκη δεν ικανοποιείται, επιστροφή στην αρχή της βρόχου
    fprintf(fout, " JMP REPEAT%d\n", currentRepeatLabel);

    // Τέλος της βρόχου
    fprintf(fout, "ENDREPEAT%d NOP\n", currentRepeatLabel);
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

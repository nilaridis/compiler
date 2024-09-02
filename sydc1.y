%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"  
    #include "symbol.h"

    Symbol *symbolTable = NULL;
    struct AstNode *root = NULL;

    void yyerror(const char *s);
    int yylex(void);
    void executeNode(struct AstNode *node);
%}

%union {
  int yint;
  char ystr[100];
  struct AstNode *node;
}

%token <yint> DEC_CONST
%token <ystr> ID
%token IF THEN ELSE WRITE READ REPEAT UNTIL END
%token EQ
%token ASSIGN LT
%token <ystr> '(' ')' ';'
%token <ystr> '+' '-' '*' '/'

%type <node> program stmt_seq stmt assign_stmt if_stmt repeat_stmt read_stmt write_stmt exp simple_exp term factor rel_exp

%%

program:
    stmt_seq { root = $1; }  // Αποθηκεύουμε τη ρίζα του δέντρου στην παγκόσμια μεταβλητή root
    ;

stmt_seq:
    stmt_seq ';' stmt { $$ = createNode(';', $1, $3, NULL); }
    | stmt { $$ = $1; }
    ;

stmt:
    assign_stmt
    | if_stmt
    | repeat_stmt
    | read_stmt
    | write_stmt
    ;

assign_stmt:
    ID ASSIGN exp {
        declareVariable($1);  // Δήλωση της μεταβλητής κατά τη δημιουργία του δέντρου
        $$ = createNode('=', createNode('I', NULL, NULL, $1), $3, NULL); 
    }
    ;

if_stmt:
    IF exp THEN stmt_seq END { 
        $$ = createNode('I', $2, $4, NULL); // Δημιουργία κόμβου if
    }
    | IF exp THEN stmt_seq ELSE stmt_seq END { 
        $$ = createNode('I', $2, createNode('E', $4, $6, NULL), NULL); // Δημιουργία κόμβου if-else
    }
    ;

repeat_stmt:
    REPEAT stmt_seq UNTIL exp { $$ = createNode('R', $2, $4, NULL); }
    ;

read_stmt:
    READ ID {
        declareVariable($2);
        Symbol *symbol = findSymbol($2);
        if (symbol == NULL) {
            insertSymbol($2, 0);
        }
        $$ = createNode('R', NULL, NULL, strdup($2)); 
    }
    ;

write_stmt:
    WRITE ID {
        checkUndeclaredVariable($2);
        Symbol *symbol = findSymbol($2);
        if (symbol != NULL) {
            printf("Value of %s: %d\n", $2, symbol->value);
        } else {
            printf("Undefined variable %s\n", $2);
        }
        $$ = createNode('W', NULL, NULL, strdup($2)); 
    }
    ;

exp:
    rel_exp { $$ = $1; }
    ;


rel_exp:
    simple_exp
    | simple_exp LT simple_exp { $$ = createNode('<', $1, $3, NULL); }
    | simple_exp EQ simple_exp { $$ = createNode('=', $1, $3, NULL); }
    ;

simple_exp:
    term
    | simple_exp '+' term { $$ = createNode('+', $1, $3, NULL); }
    | simple_exp '-' term { $$ = createNode('-', $1, $3, NULL); }
    ;

term:
    factor
    | term '*' factor { $$ = createNode('*', $1, $3, NULL); }
    | term '/' factor { $$ = createNode('/', $1, $3, NULL); }
    ;

factor:
    DEC_CONST { 
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d", $1);
        $$ = createNode('N', NULL, NULL, strdup(buffer)); 
    }
    | ID {
        checkUndeclaredVariable($1);
        $$ = createNode('I', NULL, NULL, strdup($1));
    }
    | '(' exp ')' { $$ = $2; }
    ;

%%

AstNode *createNode(int nodeType, AstNode *left, AstNode *right, char *value) {
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
    printf("%c", node->nodeType);
    if (node->value) printf(" (%s)", node->value);
    printf("\n");
    printTree(node->left, level + 1);
    printTree(node->right, level + 1);
}

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

Symbol *createSymbol(char *name, int value) {
    Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->next = NULL;
    return symbol;
}

void insertSymbol(char *name, int value) {
    Symbol *symbol = createSymbol(name, value);
    symbol->next = symbolTable;
    symbolTable = symbol;
}

Symbol *findSymbol(char *name) {
    Symbol *current = symbolTable;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void checkUndeclaredVariable(char *name) {
    if (findSymbol(name) == NULL) {
        fprintf(stderr, "Semantic Error: Undeclared variable %s\n", name);
        exit(1);
    }
}

void declareVariable(char *name) {
    if (findSymbol(name) == NULL) {
        insertSymbol(name, 0); // Προσθήκη της μεταβλητής με αρχική τιμή 0
    }
}


void printSymbolTable() {
    Symbol *current = symbolTable;
    printf("Symbol Table:\n");
    printf("Name\tValue\n");
    printf("----\t-----\n");
    while (current != NULL) {
        printf("%s\t%d\n", current->name, current->value);
        current = current->next;
    }
}

int evaluateExpression(AstNode *node) {
    if (node == NULL) return 0;
    switch (node->nodeType) {
        case 'N': // Number
            return atoi(node->value);
        case 'I': { // Identifier
            Symbol *symbol = findSymbol(node->value);
            return symbol ? symbol->value : 0;
        }
        case '+':
            return evaluateExpression(node->left) + evaluateExpression(node->right);
        case '-':
            return evaluateExpression(node->left) - evaluateExpression(node->right);
        case '*':
            return evaluateExpression(node->left) * evaluateExpression(node->right);
        case '/':
            return evaluateExpression(node->left) / evaluateExpression(node->right);
        case '<':
            return evaluateExpression(node->left) < evaluateExpression(node->right);
        case '=':
            return evaluateExpression(node->left) == evaluateExpression(node->right);
        default:
            return 0;
    }
}

void executeNode(AstNode *node) {
    if (node == NULL) return;

    switch (node->nodeType) {
        case 'I': { // If statement
            int cond = evaluateExpression(node->left); // Αξιολόγηση της συνθήκης
            if (cond) {
                executeNode(node->right); // Εκτέλεση του THEN μπλοκ
            } else if (node->right && node->right->nodeType == 'E') {
                executeNode(node->right->right); // Εκτέλεση του ELSE μπλοκ αν υπάρχει
            }
            break;
        }
        case '=': { // Assignment
            Symbol *symbol = findSymbol(node->left->value);
            if (symbol != NULL) {
                symbol->value = evaluateExpression(node->right);
                printf("Assigned %d to %s\n", symbol->value, node->left->value);
            } else {
                // Αν η μεταβλητή δεν βρεθεί, την εισάγουμε στον πίνακα συμβόλων
                insertSymbol(node->left->value, evaluateExpression(node->right));
                printf("Assigned %d to %s\n", evaluateExpression(node->right), node->left->value);
            }
            break;
        }
        case 'R': { // Read statement
            // Δεδομένου ότι το διάβασμα είναι προσομοιωμένο, δεν εκτελούμε τίποτα εδώ
            break;
        }
        case 'W': { // Write statement
            Symbol *symbol = findSymbol(node->value);
            if (symbol != NULL) {
                printf("Value of %s: %d\n", node->value, symbol->value);
            }
            break;
        }
        case ';': { // Sequence of statements
            executeNode(node->left);
            executeNode(node->right);
            break;
        }
        case '*':
        case '/':
        case '+':
        case '-': {
            // Εκτέλεση των μαθηματικών πράξεων αν χρειαστεί
            break;
        }
        default:
            printf("Unknown node type: %c\n", node->nodeType);
            break;
    }
}


int main() {
    yyparse();
    executeNode(root); // Χρήση της παγκόσμιας μεταβλητής root
    printSymbolTable();
    return 0;
}
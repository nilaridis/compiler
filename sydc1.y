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
        $$ = createNode('L', NULL, NULL, strdup($2)); // Δημιουργία κόμβου Load
    }
    ;

write_stmt:
    WRITE ID {
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
            // Έλεγχος αν η μεταβλητή έχει δηλωθεί
            if (findSymbol(node->left->value) == NULL) {
                declareVariable(node->left->value); // Δήλωση της μεταβλητής αν δεν έχει δηλωθεί
            }
            Symbol *symbol = findSymbol(node->left->value);
            if (symbol != NULL) {
                symbol->value = evaluateExpression(node->right);
                printf("Assigned %d to %s\n", symbol->value, node->left->value);
            }
            break;
        }
        case 'R': { // Repeat statement
            do {
                executeNode(node->left); // Εκτέλεση του μπλοκ εντολών
            } while (!evaluateExpression(node->right)); // Επανάληψη έως ότου η συνθήκη γίνει true
            break;
        }
        case 'L': { // Read statement (χρήση του 'L' για τη read)
            // Δήλωση της μεταβλητής αν δεν υπάρχει
            if (findSymbol(node->value) == NULL) {
                declareVariable(node->value);
            }
            Symbol *symbol = findSymbol(node->value);
            // Μπορείτε να προσθέσετε εδώ λογική για ανάγνωση τιμής από τον χρήστη αν χρειάζεται.
            printf("Reading value for %s\n", node->value);
            break;
        }
        case 'W': { // Write statement
            // Έλεγχος αν η μεταβλητή έχει δηλωθεί πριν την εκτύπωση
            if (findSymbol(node->value) == NULL) {
                fprintf(stderr, "Semantic Error: Undeclared variable %s\n", node->value);
                exit(1);
            }
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
        default:
            printf("Unknown node type: %c\n", node->nodeType);
            break;
    }
}


int main() {
    yyparse();  // Παρσάρισμα του εισερχόμενου κώδικα για να δημιουργηθεί το AST
    printf("Syntax Tree:\n");
    printTree(root, 0);  // Εκτύπωση του συντακτικού δέντρου ξεκινώντας από τη ρίζα
    printf("\nExecuting program:\n");
    executeNode(root);  // Εκτέλεση του AST χρησιμοποιώντας τη ρίζα
    printSymbolTable();  // Εκτύπωση του πίνακα συμβόλων μετά την εκτέλεση
    return 0;
}
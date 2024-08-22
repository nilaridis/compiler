%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"  
    #include "symbol.h"

    Symbol *symbolTable = NULL;

    void yyerror(const char *s);
    int yylex(void);
%}

%union{
  int yint;
  char ystr[100];
  struct AstNode *node;
}

%token <yint> DEC_CONST
%token <ystr> ID
%token ELSE THEN IF WRITE READ REPEAT UNTIL END
%token EQ
%token ASSIGN LT
%token <ystr> '(' ')' ';'
%token <ystr> '+' '-' '*' '/'

%type <node> program stmt_seq stmt assign_stmt if_stmt repeat_stmt read_stmt write_stmt exp simple_exp term factor rel_exp

%%

program:
    stmt_seq { printTree($1, 0); }
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
        checkUndeclaredVariable($1);
         Symbol *symbol = findSymbol($1);
        if (symbol == NULL) {
            insertSymbol($1, $3->value ? atoi($3->value) : 0);
        } else {
            symbol->value = $3->value ? atoi($3->value) : 0;
        }
         
         $$ = createNode('=', createNode('I', NULL, NULL, $1), $3, NULL); 
    }
    ;

if_stmt:
    IF exp THEN stmt_seq END { $$ = createNode('I', $2, $4, NULL); }
    | IF exp THEN stmt_seq ELSE stmt_seq END { $$ = createNode('I', $2, createNode('E', $4, $6, NULL), NULL); }
    ;

repeat_stmt:
    REPEAT stmt_seq UNTIL exp { $$ = createNode('R', $2, $4, NULL); }
    ;

read_stmt:
    READ ID {
        Symbol *symbol = findSymbol($2);
        if (symbol == NULL) {
            insertSymbol($2, 0);
        }

         $$ = createNode('R', NULL, NULL, strdup($2)); 
    }
    ;

write_stmt:
    WRITE ID {
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
    rel_exp{$$ = $1;}
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
        Symbol *symbol = findSymbol($1);
        if (symbol != NULL) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "%d", symbol->value);
            $$ = createNode('I', NULL, NULL, strdup(buffer));
        } else {
            $$ = createNode('I', NULL, NULL, strdup($1));
        }
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


int main() {
    yyparse();
    printSymbolTable();
    return 0;
}
%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "ast.h"  
    #include "symbol.h"
    #include "mixal_generator.h" 

    Symbol *symbolTable = NULL;
    struct AstNode *root = NULL;

    FILE *fout;  // Ορισμός της μεταβλητής fout


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
%token IF THEN ELSE WRITE REPEAT UNTIL END READ
%token EQ
%token ASSIGN LT
%token <ystr> '(' ')' ';'
%token <ystr> '+' '-' '*' '/'

%type <node> program stmt_seq stmt assign_stmt if_stmt repeat_stmt read_stmt write_stmt exp simple_exp term factor rel_exp

%%

program:
    stmt_seq { 
        $$ = createNode(NODE_PROGRAM, $1, NULL, NULL); 
        root = $$;
    }
    ;

stmt_seq:
    stmt_seq ';' stmt { $$ = createNode(NODE_SEQ, $1, $3, NULL); }
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
        $$ = createNode(NODE_ASSIGN, createNode(NODE_ID, NULL, NULL, $1), $3, NULL); 
    }
    ;

if_stmt:
    IF exp THEN stmt_seq END { 
        $$ = createNode(NODE_IF, $2, $4, NULL);
    }
    | IF exp THEN stmt_seq ELSE stmt_seq END { 
        $$ = createNode(NODE_IF, $2, createNode(NODE_ELSE, $4, $6, NULL), NULL);
    }
    ;

repeat_stmt:
    REPEAT stmt_seq UNTIL exp { $$ = createNode(NODE_REPEAT, $2, $4, NULL); }
    ;

read_stmt:
    READ ID {
        $$ = createNode(NODE_READ, NULL, NULL, strdup($2));
    }
    ;

write_stmt:
    WRITE ID {
        $$ = createNode(NODE_WRITE, NULL, NULL, strdup($2)); 
    }
    ;

exp:
    rel_exp { $$ = $1; }
    ;

rel_exp:
    simple_exp
    | simple_exp LT simple_exp { $$ = createNode(NODE_LT, $1, $3, NULL); }
    | simple_exp EQ simple_exp { $$ = createNode(NODE_EQ, $1, $3, NULL); }
    ;

simple_exp:
    term
    | simple_exp '+' term { $$ = createNode(NODE_PLUS, $1, $3, NULL); }
    | simple_exp '-' term { $$ = createNode(NODE_MINUS, $1, $3, NULL); }
    ;

term:
    factor
    | term '*' factor { $$ = createNode(NODE_MUL, $1, $3, NULL); }
    | term '/' factor { $$ = createNode(NODE_DIV, $1, $3, NULL); }
    ;

factor:
    DEC_CONST { 
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d", $1);
        $$ = createNode(NODE_NUMBER, NULL, NULL, strdup(buffer)); 
    }
    | ID {
        $$ = createNode(NODE_ID, NULL, NULL, strdup($1));
    }
    | '(' exp ')' { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

void executeNode(AstNode *node) {
    if (node == NULL) return;

    switch (node->nodeType) {
        case NODE_PROGRAM: {
            executeNode(node->left);  // Typically, the left child will hold the statements for a program
            break;
        }
        case NODE_IF: {
            int cond = evaluateExpression(node->left, symbolTable);
            if (cond) {
                executeNode(node->right);
            } else if (node->right && node->right->nodeType == NODE_ELSE) {
                executeNode(node->right->right);
            }
            break;
        }
        case NODE_ASSIGN: {
            if (findSymbol(node->left->value, symbolTable) == NULL) {
                declareVariable(node->left->value, &symbolTable);
            }
            Symbol *symbol = findSymbol(node->left->value, symbolTable);
            if (symbol != NULL) {
                symbol->value = evaluateExpression(node->right, symbolTable);
                printf("Assigned %d to %s\n", symbol->value, node->left->value);
            }
            break;
        }
        case NODE_REPEAT: {
            do {
                executeNode(node->left);
            } while (!evaluateExpression(node->right, symbolTable));
            break;
        }
        case NODE_READ: {
            if (findSymbol(node->value, symbolTable) == NULL) {
                declareVariable(node->value, &symbolTable);
            }
            Symbol *symbol = findSymbol(node->value, symbolTable);
            printf("Reading value for %s\n", node->value);
            break;
        }
        case NODE_WRITE: {
            if (findSymbol(node->value, symbolTable) == NULL) {
                fprintf(stderr, "Semantic Error: Undeclared variable %s\n", node->value);
                exit(1);
            }
            Symbol *symbol = findSymbol(node->value, symbolTable);
            if (symbol != NULL) {
                printf("Value of %s: %d\n", node->value, symbol->value);
            }
            break;
        }
        case NODE_SEQ: {
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
    yyparse();
    printf("Syntax Tree:\n");
    printTree(root, 0);
    printf("\nExecuting program:\n");
    executeNode(root);
    printSymbolTable(symbolTable);

    //Open the output file for MIXAL code
    fout = fopen("asm.mixal", "w");
    if (!fout) {
        fprintf(stderr, "Error: Could not open output file asm.mixal.\n");
        return 1;
    }

    // Generate MIXAL code and write it to the file
    printf("\nGenerating MIXAL code:\n");
    generateMixalCode(root);

    // Close the output file
    fclose(fout);

     
    return 0;
}
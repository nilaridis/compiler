%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "sydc1.tab.h"
    #include "ast.h"

%}

%%

"read"              {
                        printf("%s \n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return READ;
                    }
"write"             {
                        printf("%s \n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return WRITE;
                    }
"if"                {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return IF;
                    }
"then"              {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return THEN;
                    }
"else"              {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return ELSE;
                    }
"repeat"            {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return REPEAT;
                    }
"until"             {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return UNTIL;
                    }
"end"               {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return END;
                    }

[0-9]+              {
                        printf("%s\n", yytext);
                        yylval.yint = atoi(yytext);
                        return DEC_CONST;
                    }
"*"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return '*';
                    }
"/"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return '/';
                    }
"+"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return '+';
                    }
"-"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return '-';
                    }
":="                {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return ASSIGN;
                    }
"<"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return LT;
                    }
"="                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return EQ;
                    }
"("                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return '(';
                    }
")"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return ')';
                    }
";"                 {
                        printf("%s\n", yytext);
                        strcpy(yylval.ystr, yytext);
                        return ';';
                    }
[a-zA-Z][a-zA-Z0-9]* {
                        printf("id : %s\n", yytext);
                        strcpy(yylval.ystr,yytext);
                        return ID;
                    }
[ \t\n]+            { /* αγνόησε λευκούς χαρακτήρες */ }
.                   { printf("Unknown Character %s\n", yytext); }

%%


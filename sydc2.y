void yyerror(char *s)
{
   fputs(s,stderr); putc('\n',stderr);
}
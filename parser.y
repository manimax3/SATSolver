%{
int yylex();
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
void yyerror(char *s);
%}

%union {int num; char id;}
%start line
%token print
%token SEMI
%token <num> number
%type <num> line

%%
line : print number ';' 		{printf("Line rule: %i\n", $2); $$ = $2;}
	 | ';' {$$ = 0;}
	 | line ';' {$$ = 0;}
	 | line print number ';' 		{printf("Line rule: %i\n", $3); $$ = $3;}
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}


%{
int yylex();
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "ast.h"
void yyerror(char *s);

extern int result;
%}

%union {int num; char id;}
%start expr
%token print
%token SEMI
%token <num> number
%type <num> line
%type <num> expr

%%
expr : line {result = $1;}

line : print number ';' 		{ $$ = subtract_one($2) * 2;printf("Line rule: %i\n", $2);}
	 | ';' {$$ = 0;}
	 | line ';' {$$ = 0;}
	 | line print number ';' 		{ $$ = subtract_one($3) * 2;printf("Line rule: %i\n", $3);}
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}


%{
#include <cstdio>
#include <cstdlib>
#include "ast.h"
extern StatementList finalstmtlist;
%}

%require "3.2"
%language "c++"
%define api.value.type variant
%define api.token.constructor

%code requires {
#include <string>
#include "ast.h"
}

%code provides {
yy::parser::symbol_type yylex();
}

%token IMPLICATION
%token BIIMPLICATION
%token <std::string> PREDICATE
%token AND 
%token OR
%token NEGATION

%right IMPLICATION BIIMPLICATION
%left OR
%left AND
%precedence NEGATION


%token TRUE
%token FALSE
%token SET
%token PRINT
%token ATOMS
%token TABLE
%token NNF
%token KNF

%token <std::string> EXPRNAME

%token EndOfFile 0

%type <std::shared_ptr<Expression>> expression
%type <Statement*> setstmt printstmt stmt
%type <StatementList> stmtlist file

%start file

%%

file : stmtlist { finalstmtlist = std::move($1); $$ = std::move($1); }

stmtlist : stmt { $$ = StatementList($1); }
	 | stmtlist stmt { $$ = std::move($1); $$.add($2); }

stmt : setstmt ';'
	 | printstmt ';'
	 /* | expression ';' { $$ = static_cast<Statement*>($1);} */

setstmt : SET ':' PREDICATE TRUE { $$ = new Statement($3, std::make_shared<ConstantExpression>(true)); }
		| SET ':' PREDICATE FALSE { $$ = new Statement($3, std::make_shared<ConstantExpression>(false)); }
		| SET PREDICATE TRUE { $$ = new Statement($2, std::make_shared<ConstantExpression>(false)); }
		| SET PREDICATE FALSE { $$ = new Statement($2, std::make_shared<ConstantExpression>(false)); }
		| SET PREDICATE ':' expression { $$ = new Statement($2, $4); }
		| SET EXPRNAME ':' expression { $$ = new Statement($2, $4, true); }

printstmt : PRINT ':' expression { $$ = new Statement($3); }
		  | PRINT expression { $$ = new Statement($2); }
		  | PRINT ATOMS expression { $$ = new Statement($3, Statement::PrintAtoms); }
		  | PRINT TABLE expression { $$ = new Statement($3, Statement::PrintTable); }
		  | PRINT NNF expression { $$ = new Statement($3, Statement::PrintNNF); }
		  | PRINT KNF expression { $$ = new Statement($3, Statement::PrintKNF); }

expression : PREDICATE {$$ = std::make_shared<PredExpression>($1); }
		   | EXPRNAME { $$ = std::make_shared<NamedExpression>($1); }
		   | TRUE { $$ = std::make_shared<ConstantExpression>(true); }
		   | FALSE { $$ = std::make_shared<ConstantExpression>(false); }
		   | expression IMPLICATION expression {$$ = std::make_shared<BinaryExpression>($1, $3, BinaryExpression::Impl);}
		   | expression BIIMPLICATION expression {$$ = std::make_shared<BinaryExpression>($1, $3, BinaryExpression::BiImpl);}
		   | expression AND expression {$$ = std::make_shared<BinaryExpression>($1, $3, BinaryExpression::And);}
		   | expression OR expression {$$ = std::make_shared<BinaryExpression>($1, $3, BinaryExpression::Or);}
		   | NEGATION expression {$$ = std::make_shared<NegExpression>($2);}
		   | '(' expression ')' { $$ = $2; }
%%

void yyerror(char *s) {
printf("Error: %s\n", s);
}

void yy::parser::error(const std::string& error)
{
	std::cerr << error << '\n';
}



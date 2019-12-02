#include <cstdio>
#include "parser.hpp"

int result;

int main(int argc, char** argv)
{
	yyparse();
	return 0;
}

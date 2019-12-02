#include <cstdio>
#include "parser.hpp"

int main(int argc, char** argv)
{
	printf("Hello world!\n");
	printf("%i Argument(s) have been submitted!\n", argc - 1);
	yyparse();
	return 0;
}

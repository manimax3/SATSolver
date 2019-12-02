#include <cstdio>
#include "parser.hpp"

int result;

int main(int argc, char** argv)
{
	printf("Hello world!\n");
	printf("%i Argument(s) have been submitted!\n", argc - 1);
	yyparse();
	printf("Final value was: %i\n", result);
	return 0;
}

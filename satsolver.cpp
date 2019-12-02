#include "parser.hpp"
#include <cstdio>

int result;

int main(int argc, char **argv)
{
    yyparse();
    return 0;
}

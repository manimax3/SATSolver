#include "parser.hpp"
#include <cstdio>

extern FILE* yyin;

int main(int argc, char **argv)
{
    if (argc == 2) {
        yyin = fopen(argv[1], "r");
    }

    yyparse();

    if (argc == 2) {
        fclose(yyin);
    }
    return 0;
}

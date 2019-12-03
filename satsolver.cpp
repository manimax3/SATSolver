#include "ast.h"
#include "parser.hpp"
#include <cstdio>

extern FILE * yyin;
StatementList finalstmtlist;

int main(int argc, char **argv)
{
    if (argc == 2) {
        yyin = fopen(argv[1], "r");
    }

    yy::parser parser;
    parser.parse();
    finalstmtlist.run();

    if (argc == 2) {
        fclose(yyin);
    }
    return 0;
}

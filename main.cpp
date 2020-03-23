#include "parser.h"

int main(){
    Parser p;
    while(p.hasMoreCommands())
        p.advance();
    return 0;
}
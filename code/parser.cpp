#ifndef PARSER_H
#define PARSER_H
#include "Parser.h"
#include "Error.h"
#endif

Base *Parser::parse()
{
    Base *Res = parseS();
    return Res;
}

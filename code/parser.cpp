#ifndef PARSER_H
#define PARSER_H
#include "Parser.h"
#include "Error.h"
#endif

Base *Parser::parse()
{
    llvm::SmallVector<Statement*> statements;
    while (!Tok.is(Token::eof)) {
        switch (Tok.getKind()) {
            case Token::identifier:
            {
                AssignStatement* state = parseAssign();
			    statements.push_back(state);
			    break;   
            }
        }
    }
    return new Base(statements);
}

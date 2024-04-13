#ifndef _PARSER_H
#define _PARSER_H
#include "AST.h"
#include "lexer.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
using namespace std;

class Parser
{
    Lexer &Lex;
    Token Tok;
    bool HasError;

    void error()
    {
        llvm::errs() << "Unexpected: " << Tok.getText() << "\n";
        HasError = true;
    }

    void advance() { Lex.next(Tok); }

    bool expect(Token::TokenKind Kind)
    {
        if (Tok.getKind() != Kind)
        {
            error();
            return true;
        }
        return false;
    }

    bool consume(Token::TokenKind Kind)
    {
        if (expect(Kind))
            return true;
        advance();
        return false;
    }

    // parsing functions according to the regex
    // pattern specified. each one produces its own node
    // one node can have multiple subnodes inside it
public:
    Base *parse();
    Base *parseStatement();
    IfStatement *parseIf();
    ElseIfStatement *parseElseIf();
    AssignStatement *parseUnaryExpression(Token &token);
    Expression *parseExpression();
    Expression *parseLogicalExpression();
    Expression *parseLogicalComparison();
    Expression *parseLogicalTerm();
    Expression *parseIntExpression();
    Expression *parseTerm();
    Expression *parsePower();
    Expression *parseFactor();
    AssignStatement *parseAssign(llvm::StringRef name);
    llvm::SmallVector<DecStatement *> parseDefine();
    void check_for_semicolon();

public:
    // initializes all members and retrieves the first token
    Parser(Lexer &Lex) : Lex(Lex), HasError(false)
    {
        advance();
    }

    // get the value of error flag
    bool hasError() { return HasError; }

};

#endif
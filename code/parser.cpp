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

Base *Parser::parseStatement()
{
    llvm::SmallVector<Statement *> statements;
    while (!Tok.is(Token::r_brace) && !Tok.is(Token::eof))
    {
        switch (Tok.getKind())
        {
        case Token::KW_if:
        {
            IfStatement *statement = parseIf();
            statements.push_back(statement);
            break;
        }
        default:
        {
            Error::UnexpectedToken(Tok.getKind());
        }
        }
    }
    return new Base(statements);
}

IfStatement *Parser::parseIf()
{
    advance();
    if (!Tok.is(Token::l_paren))
    {
        Error::LeftParenthesisExpected();
    }

    advance();
    Expression *condition = parseLogicalValue();

    if (!Tok.is(Token::r_paren))
    {
        Error::RightParenthesisExpected();
    }

    advance();
    if (!Tok.is(Token::l_brace))
    {
        Error::LeftBraceExpected();
    }
    Base *allIfStatements = parseStatement();
    if (!Tok.is(Token::r_brace))
    {
        Error::RightBraceExpected();
    }
    advance();

    // parse else if and else statements
    llvm::SmallVector<ElseIfStatement *> elseIfStatements;
    ElseStatement *elseStatement = nullptr;
    bool hasElseIf = false;
    bool hasElse = false;

    while (Tok.is(Token::KW_else))
    {
        advance();
        if (Tok.is(Token::KW_if))
        {
            IfStatement *ifStatement = parseIf();
            elseIfStatements.push_back(new ElseIfStatement(ifStatement));
            hasElseIf = true;
        }
        else if (Tok.is(Token::l_brace))
        {
            advance();
            Base *allElseStatements = parseStatement();
            if (!Tok.is(Token::r_brace))
            {
                Error::RightBraceExpected();
            }
            advance();
            elseStatement = new ElseStatement(allElseStatements, Statement::StatementType::Else);
            hasElse = true;
            break;
        }
        else
        {
            Error::RightBraceExpected();
        }
    }

    return new IfStatement(condition, allIfStatements->getStatements(), elseIfStatements, elseStatement, hasElseIf, hasElse, Statement::StatementType::If);
}

#ifndef PARSER_H
#define PARSER_H
#include "parser.h"
#include "error.h"
#include <string>
using namespace std;
#endif

Base *Parser::parse()
{
    llvm::SmallVector<Statement *> statements;
    bool isComment = false;
    while (!Tok.is(Token::eof))
    {
        if (isComment)
        {
            if (Tok.is(Token::uncomment))
            {
                isComment = false;
            }
            advance();
            continue;
        }

        switch (Tok.getKind())
        {
        case Token::KW_int:
        {
            llvm::SmallVector<DecStatement *> states = Parser::parseDefine();
            if (states.size() == 0)
            {
                return nullptr;
            }
            while (states.size() > 0)
            {
                statements.push_back(states.back());
                states.pop_back();
            }
            break;
        }
        case Token::KW_bool:
        {
            llvm::SmallVector<DecStatement *> states = Parser::parseDefine();
            if (states.size() == 0)
            {
                return nullptr;
            }
            while (states.size() > 0)
            {
                statements.push_back(states.back());
                states.pop_back();
            }
            break;
        }
        case Token::identifier:
        {
            llvm::StringRef name = Tok.getText();
            Token current = Tok;
            advance();
            if (!Tok.isOneOf(Token::plus_plus, Token::minus_minus))
            {
                AssignStatement *assign = parseAssign(name);
                statements.push_back(assign);
            }
            else
            {
                AssignStatement *assign = parseUnaryExpression(current);
                statements.push_back(assign);
            }
            Parser::check_for_semicolon();
            break;
        }
        case Token::KW_print:
        {
            advance();
            if (!Tok.is(Token::l_paren))
            {
                Error::LeftParenthesisExpected();
            }
            advance();
            // token should be identifier
            if (!Tok.is(Token::identifier))
            {
                Error::VariableExpected();
            }
            Expression *variable_to_be_printed = new Expression(Tok.getText());
            advance();
            if (!Tok.is(Token::r_paren))
            {
                Error::RightParenthesisExpected();
            }
            advance();
            Parser::check_for_semicolon();
            PrintStatement *print_statement = new PrintStatement(variable_to_be_printed);
            statements.push_back(print_statement);
            break;
        }
        case Token::comment:
        {
            isComment = true;
            advance();
            break;
        }
        case Token::KW_if:
        {
            IfStatement *statement = parseIf();
            statements.push_back(statement);
            break;
        }
        }
        return new Base(statements);
    }
}

void Parser::check_for_semicolon()
{
    if (!Tok.is(Token::semi_colon))
    {
        Error::SemiColonExpected();
    }
    advance();
}

AssignStatement *Parser::parseUnaryExpression(Token &token)
{
    if (Tok.is(Token::plus_plus))
    {
        advance();
        if (token.is(Token::identifier))
        {
            Expression *tok = new Expression(token.getText());
            Expression *one = new Expression(1);
            return new AssignStatement(tok, new BinaryOp(BinaryOp::Plus, tok, one));
        }
        else
        {
            Error::VariableExpected();
        }
    }
    else if (Tok.is(Token::minus_minus))
    {
        advance();
        if (token.is(Token::identifier))
        {
            Expression *tok = new Expression(token.getText());
            Expression *one = new Expression(1);
            return new AssignStatement(tok, new BinaryOp(BinaryOp::Minus, tok, one));
        }
        else
        {
            Error::VariableExpected();
        }
    }
}
llvm::SmallVector<DecStatement *> Parser::parseDefine()
{
    advance();
    llvm::SmallVector<DecStatement *> states;
    while (!Tok.is(Token::semi_colon))
    {
        llvm::StringRef name;
        Expression *value = nullptr;
        if (Tok.is(Token::identifier))
        {
            name = Tok.getText();
            advance();
        }
        else
        {
            Error::VariableExpected();
        }
        if (Tok.is(Token::equal))
        {
            advance();
            value = parseExpression();
            advance();
        }
        else if (Tok.is(Token::comma))
        {
            advance();
        }
        else if (Tok.is(Token::semi_colon))
        {
            break;
        }
        else
        {
            Error::VariableExpected();
        }
        DecStatement *state = new DecStatement(new Expression(name), value);
        states.push_back(state);
    }
    return states;
}

Expression *Parser::parseExpression()
{
    Expression *expr;
    bool isFound = false;
    try
    {
        expr = parseIntExpression();
        isFound = true;
    }
    catch (...)
    {
    }
    try
    {
        expr = parseLogicalExpression();
        isFound = true;
    }
    catch (...)
    {
    }
    if (!isFound)
    {
        Error::ExpressionExpected();
    }
    return expr;
}

Expression *Parser::parseLogicalExpression()
{
    Expression *left = parseLogicalComparison();
    while (Tok.isOneOf(Token::KW_and, Token::KW_or))
    {
        BooleanOp::Operator Op;
        switch (Tok.getKind())
        {
        case Token::KW_and:
            Op = BooleanOp::And;
            break;
        case Token::KW_or:
            Op = BooleanOp::Or;
            break;
        default:
            break;
        }
        advance();
        Expression *Right = parseLogicalComparison();
        left = new BooleanOp(Op, left, Right);
    }
    return left;
}

Expression *Parser::parseLogicalComparison()
{
    Expression *left = parseLogicalTerm();
    while (Tok.isOneOf(Token::equal_equal, Token::not_equal, Token::less, Token::less_equal, Token::greater, Token::greater_equal))
    {
        BooleanOp::Operator Op;
        switch (Tok.getKind())
        {
        case Token::equal_equal:
            Op = BooleanOp::Equal;
            break;
        case Token::not_equal:
            Op = BooleanOp::NotEqual;
            break;
        case Token::less:
            Op = BooleanOp::Less;
            break;
        case Token::less_equal:
            Op = BooleanOp::LessEqual;
            break;
        case Token::greater:
            Op = BooleanOp::Greater;
            break;
        case Token::greater_equal:
            Op = BooleanOp::GreaterEqual;
            break;
        default:
            break;
        }
        advance();
        Expression *Right = parseLogicalTerm();
        left = new BooleanOp(Op, left, Right);
    }
    return left;
}

Expression *Parser::parseLogicalTerm()
{
    Expression *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::l_paren:
    {
        advance();
        Res = parseLogicalExpression();
        if (!consume(Token::r_paren))
            break;
    }
    case Token::KW_true:
    {
        Res = new Expression(true);
        advance();
        break;
    }
    case Token::KW_false:
    {
        Res = new Expression(false);
        advance();
        break;
    }
    default: // error handling
    {
        Res = parseIntExpression();
        if (Res == nullptr)
        {
            Error::ExpressionExpected();
        }
    }
    }
    return Res;
}

Expression *Parser::parseIntExpression()
{
    Expression *Left = parseTerm();
    while (Tok.isOneOf(Token::plus, Token::minus))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::plus) ? BinaryOp::Plus : BinaryOp::Minus;
        advance();
        Expression *Right = parseTerm();
        Left = new BinaryOp(Op, Left, Right);
    }
    return left;
}

Expression *Parser::parseTerm()
{
    Expression *Left = parsePower();
    while (Tok.isOneOf(Token::star, Token::slash, Token::mod))
    {
        BinaryOp::Operator Op =
            Tok.is(Token::star) ? BinaryOp::Mul : Tok.is(Token::slash) ? BinaryOp::Div
                                                                       : BinaryOp::Mod;
        advance();
        Expression *Right = parsePower();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expression *Parser::parsePower()
{
    Expression *Left = parseFactor();
    while (Tok.is(Token::power))
    {
        BinaryOp::Operator Op =
            BinaryOp::Pow;
        advance();
        Expression *Right = parseFactor();
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;
}

Expression *Parser::parseFactor()
{
    Expression *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::number:
    {
        int number;
        Tok.getText().getAsInteger(10, number);
        Res = new Expression(number);
        advance();
        break;
    }
    case Token::ident:
    {
        Res = new Expression(Tok.getText());
        advance();
        break;
    }
    case Token::l_paren:
    {
        advance();
        Res = parseExpr();
        if (!consume(Token::r_paren))
            break;
    }
    default: // error handling
    {
        Error::NumberVariableExpected();
    }
    }
    return Res;
}

AssignStatement *Parser::parseAssign(llvm::StringRef name)
{
    llvm::StringRef name;
    Expression *value = nullptr;
    if (Tok.is(Token::equal))
    {
        advance();
        value = parseExpression();
        advance();
    }
    else
    {
        Error::EqualExpected();
    }
    return new AssignStatement(name, value);
}

Base *Parser::parseStatement()
{
    llvm::SmallVector<Statement *> statements;
    bool isComment = false;
    while (!Tok.is(Token::r_brace) && !Tok.is(Token::eof))
    {
        if (isComment)
        {
            if (Tok.is(Token::uncomment))
            {
                isComment = false;
            }
            advance();
            continue;
        }

        switch (Tok.getKind())
        {
        case Token::identifier:
        {
            llvm::StringRef name = Tok.getText();
            Token current = Tok;
            advance();
            if (!Tok.isOneOf(Token::plus_plus, Token::minus_minus))
            {
                AssignStatement *assign = parseAssign(name);
                statements.push_back(assign);
            }
            else
            {
                AssignStatement *assign = parseUnaryExpression(current);
                statements.push_back(assign);
            }
            Parser::check_for_semicolon();
            break;
        }
        case Token::KW_print:
        {
            advance();
            if (!Tok.is(Token::l_paren))
            {
                Error::LeftParenthesisExpected();
            }
            advance();
            // token should be identifier
            if (!Tok.is(Token::identifier))
            {
                Error::VariableExpected();
            }
            Expression *tok = new Expression(Tok.getText());
            advance();
            if (!Tok.is(Token::r_paren))
            {
                Error::RightParenthesisExpected();
            }
            advance();
            Parser::check_for_semicolon();
            PrintStatement *print_statement = new PrintStatement(tok);
            statements.push_back(print_statement);
            break;
        }
        case Token::comment:
        {
            isComment = true;
            advance();
            break;
        }
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
            elseIfStatements.push_back(parseElseIf());
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

ElseIfStatement *Parser::parseElseIf()
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

    return new ElseIfStatement(condition, allIfStatements->getStatements(), Statement::StatementType::ElseIf);
}

#include "Error.h"
#include "Lexer.h"
#include <string>


void Error::VariableExpected()
{
    cout << "Variable expected" << endl;
    exit(3);
}

void Error::InvalidDataForExpectedDataType()
{
    cout << "Invalid data for expected data type" << endl;
    exit(3);
}

void Error::ExpressionExpected()
{
    cout << "Expression expected" << endl;
    exit(3);
}

void Error::EqualExpected()
{
    cout << "Equal expected" << endl;
    exit(3);
}

void Error::LeftParenthesisExpected()
{
    cout << "Left parenthesis expected: '('\n";
    exit(3);
}

void Error::RightParenthesisExpected()
{
    cout << "Right parenthesis expected: ')'\n";
    exit(3);
}

void Error::LeftBraceExpected()
{
    cout << "Left brace expected: '{'\n";
    exit(3);
}

void Error::RightBraceExpected()
{
    cout << "Right brace expected: '}'\n";
    exit(3);
}

void Error::UnexpectedToken(Token::TokenKind &kind)
{
    cout << "Unexpected token: " << kind.getText() << "\n";
    exit(3);
}

void Error::SemiColonExpected()
{
    cout << "Semi colon expected: ';'\n";
    exit(3);
}

void Error::NumberVariableExpected()
{
    cout << "Number or variable expected\n";
    exit(3);
}
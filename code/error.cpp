#include "error.h"

void Error::VariableExpected()
{
    cout << "Variable expected" << endl;
}

void Error::InvalidDataForExpectedDataType()
{
    cout << "Invalid data for expected data type" << endl;
}

void Error::ExpressionExpected()
{
    cout << "Expression expected" << endl;
}

void Error::EqualExpected()
{
    cout << "Equal expected" << endl;
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

void Error::UnexpectedToken(Token::TokenKind kind)
{
    cout << "Unexpected token: " << kind.getText() << "\n";
    exit(3);
}

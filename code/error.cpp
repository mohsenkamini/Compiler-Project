#include "Error.h"

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

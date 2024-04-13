#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include <string>
#include "lexer.h"
using namespace std;

class Error
{

public:
	static void LeftParenthesisExpected();
	static void RightParenthesisExpected();
	static void LeftBraceExpected();
	static void RightBraceExpected();
	static void UnexpectedToken(Token::TokenKind);
	static void VariableExpected();
	static void InvalidDataForExpectedDataType();
	static void ExpressionExpected();
	static void EqualExpected();
	static void SemiColonExpected();
};

#endif

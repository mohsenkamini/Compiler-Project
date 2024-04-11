#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "Lexer.h"
using namespace std;

class Error
{

public:
	static void LeftParenthesisExpected();
	static void RightParenthesisExpected();
	static void LeftBraceExpected();
	static void RightBraceExpected();
};

#endif

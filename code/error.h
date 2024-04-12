#ifndef ERROR_H
#define ERROR_H

#include <iostream>
#include "lexer.h"
using namespace std;

class Error
{

public:
	// Add error functions here like: static void SemiColonNotFound();
	static void VariableExpected();
	static void InvalidDataForExpectedDataType();	
	static void ExpressionExpected();
};

#endif

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

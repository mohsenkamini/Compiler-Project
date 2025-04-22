#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "AST.h"

class CodeGen {
public:
    void compile(ProgramNode *root, bool optimize, int unroll);
};

#endif 

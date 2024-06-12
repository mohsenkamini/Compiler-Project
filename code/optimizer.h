#ifndef OPTIMIZER
#define OPTIMIZER

#include "AST.h"

Expression *updateExpression(Expression *expression, llvm::StringRef iterator, int increase);
llvm::SmallVector<Statement*> completeUnroll(ForStatement* forStatement);

#endif
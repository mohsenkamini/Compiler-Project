#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "llvm/Support/CommandLine.h"
#include "AST.h"

Expression *updateExpression(Expression *expression, llvm::StringRef iterator, int increase);
Statement *updateStatement(Statement *statement, llvm::StringRef iterator, int increase);
llvm::SmallVector<Statement*> completeUnroll(ForStatement *forStatement);
    

#endif
#ifndef OPTIMIZER
#define OPTIMIZER

#include "AST.h"

llvm::SmallVector<Statement*> completeUnroll(ForStatement* forStatement);

#endif
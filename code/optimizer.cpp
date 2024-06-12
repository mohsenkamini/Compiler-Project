#include "optimizer.h"

llvm::SmallVector<Statement *> completeUnroll(ForStatement *forStatement)
{
    llvm::SmallVector<Statement *> unrolledStatements;
    llvm::SmallVector<Statement *> body = forStatement->getStatements();

    // Get for variables and constants
    int initialIterator = forStatement->getInitialAssign().getRValue().getNumber();
    int conditionValue = forStatement->getCondition().getBooleanOp().getRight().getNumber();
    int updateValue = forStatement->getUpdateAssign().getRValue().getNumber();

    // Simulate the for in the program. Iterate on body and push back new statements with updated indices
    for (int i = initialIterator; i < conditionValue; i += updateValue)
    {
        for (Statement *statement : body)
        {
            Statement *newStatement = statement;
            if (statement.getKind() == Statement::StatementType::Assignment)
            {
                // TODO: Nested expressions
                Expression *right = statement.getRValue();
                if (right.isVariable() && right.getValue() == forStatement.getInitialAssign().getLValue().getValue())
                {
                    Expression *index = new Expression(i);
                    newStatement = new BinaryOp(BinaryOp::Plus, right, index)
                }
            }
            unrolledStatements.push_back(newStatement);
        }
    }
}
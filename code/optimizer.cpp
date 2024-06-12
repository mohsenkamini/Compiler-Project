#include "optimizer.h"


Expression *updateExpression(Expression *expression, llvm::StringRef iterator, int increase)
{
    if (expression->isVariable() && expression->getValue() == iterator)
    {
        return new BinaryOp(BinaryOp::Plus, iterator, new Expression(increase));
    }
    if (expression->isBinaryOp())
    {
        BinaryOp *binaryOp = (BinaryOp *)expression;
        Expression *left = updateExpression(binaryOp->getLeft(), iterator, increase);
        Expression *right = updateExpression(binaryOp->getRight(), iterator, increase);
        return new BinaryOp(binaryOp->getOperator(), left, right);
    }
    if (expression->isBooleanOp())
    {
        BooleanOp *booleanOp = (BooleanOp *)expression;
        Expression *left = updateExpression(booleanOp->getLeft(), iterator, increase);
        Expression *right = updateExpression(booleanOp->getRight(), iterator, increase);
        return new BooleanOp(booleanOp->getOperator(), left, right);
    }
    return expression;
}

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
                Assignment *assignment = (Assignment *)statement;
                Expression *right = assignment->getRValue();
                Expression *newRight = updateExpression(right, forStatement->getInitialAssign().getLValue().getValue(), i);
                newStatement = new Assignment(assignment->getLValue(), newRight);
            }
            unrolledStatements.push_back(newStatement);
        }
    }
}
#include "optimizer.h"

Expression *updateExpression(Expression *expression, llvm::StringRef iterator, int increase)
{
    if (expression->isVariable() && expression->getValue() == iterator)
    {
        return new BinaryOp(BinaryOp::Plus, new Expression(iterator), new Expression(increase));
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

Statement *updateStatement(Statement *statement, llvm::StringRef iterator, int increase)
{
    AssignStatement *assignment = (AssignStatement *)statement;
    Expression *right = assignment->getRValue();
    Expression *newRight = updateExpression(right, iterator, increase);
    return new AssignStatement(assignment->getLValue(), newRight);
}

llvm::SmallVector<Statement *> completeUnroll(ForStatement *forStatement)
{
    llvm::SmallVector<Statement *> unrolledStatements;
    llvm::SmallVector<Statement *> body = forStatement->getStatements();

    // Get for variables and constants
    int initialIterator = forStatement->getInitialAssign()->getRValue()->getNumber();
    BooleanOp *condition_boolean_op = (BooleanOp *)forStatement->getCondition();
    int conditionValue = condition_boolean_op->getRight()->getNumber();
    int updateValue = ((BinaryOp *)forStatement->getUpdateAssign()->getRValue())->getRight()->getNumber();

    // Simulate the for in the program. Iterate on body and push back new statements with updated indices
    if (condition_boolean_op->getOperator() == BooleanOp::LessEqual) {
        for (int i = initialIterator; i <= conditionValue; i += updateValue)
        {
            for (Statement *statement : body)
            {
                Statement *newStatement = updateStatement(statement, forStatement->getInitialAssign()->getLValue()->getValue(), i);
                unrolledStatements.push_back(newStatement);
            }
        }
    }
    else if (condition_boolean_op->getOperator() == BooleanOp::Less) {
        for (int i = initialIterator; i < conditionValue; i += updateValue)
        {
            for (Statement *statement : body)
            {
                Statement *newStatement = updateStatement(statement, forStatement->getInitialAssign()->getLValue()->getValue(), i);
                unrolledStatements.push_back(newStatement);
            }
        }
    }

    return unrolledStatements;
}


llvm::SmallVector<Statement *> completeUnroll(WhileStatement *whileStatement)
{
    llvm::SmallVector<Statement *> unrolledStatements;
    llvm::SmallVector<Statement *> body = whileStatement->getStatements();
    llvm::SmallVector<Statement *> newBody;

    BooleanOp *condition_boolean_op = (BooleanOp *)whileStatement->getCondition();
    int conditionValue = condition_boolean_op->getRight()->getNumber();
    if(condition_boolean_op->getOperator() == BooleanOp::LessEqual){
        conditionValue++;
    }
    llvm::StringRef iteratorVar = condition_boolean_op->getLeft()->getValue();
    int initialIterator = 0;
    int updateValue = 0;
    // find initial value of iterator from the body
    for(Statement *statement : body){
        AssignStatement* assignStatement = (AssignStatement *)statement;
        if(assignStatement->getLValue()->getValue() == iteratorVar){
            updateValue = ((BinaryOp *)assignStatement->getRValue())->getRight()->getNumber();
            continue;
        }
        newBody.push_back(statement);
    }    
    for (int i = initialIterator; i < conditionValue; i += updateValue){
        for (Statement *statement : newBody)
        {
            Statement *newStatement = updateStatement(statement, iteratorVar, i);
            unrolledStatements.push_back(newStatement);
        }
    }
    return unrolledStatements;
}

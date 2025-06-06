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

llvm::SmallVector<Statement *> completeUnroll(ForStatement *forStatement, int k){
    llvm::SmallVector<Statement *> unrolledStatements;
    llvm::SmallVector<Statement *> body = forStatement->getStatements();

    // Get for variables and constants
    int initialIterator = forStatement->getInitialAssign()->getRValue()->getNumber();
    BooleanOp *condition_boolean_op = (BooleanOp *)forStatement->getCondition();
    int conditionValue = condition_boolean_op->getRight()->getNumber();
    int conditionValue_arash = conditionValue;
    if(condition_boolean_op->getOperator() == BooleanOp::LessEqual){
        conditionValue++;
    }
    int updateValue = ((BinaryOp *)forStatement->getUpdateAssign()->getRValue())->getRight()->getNumber();
    if(k>0){
        llvm::SmallVector<Statement *> newForBody;
        for(Statement *statement : body){
            
            for(int i = 0; i < k; i++){
                AssignStatement* assignStatement = (AssignStatement *)statement;
                Statement *newStatement = updateStatement(statement, forStatement->getInitialAssign()->getLValue()->getValue(), i * updateValue);
                newForBody.push_back(newStatement);
            }
        }
        AssignStatement * newForUpdate = new AssignStatement(forStatement->getUpdateAssign()->getLValue(), new BinaryOp(BinaryOp::Plus, forStatement->getUpdateAssign()->getLValue(), new Expression(k * updateValue)));
        Expression* newCondition = new BooleanOp(condition_boolean_op->getOperator(), forStatement->getInitialAssign()->getLValue(), new Expression(conditionValue_arash - updateValue));
        ForStatement* newForStatement = new ForStatement(newCondition, newForBody, forStatement->getInitialAssign(), newForUpdate, Statement::StatementType::For, true);
        unrolledStatements.push_back(newForStatement);
        if(conditionValue % (k * updateValue) != 0){
             body.push_back(forStatement->getUpdateAssign());
             WhileStatement* afterForStatement = new WhileStatement(forStatement->getCondition(), body, Statement::StatementType::While, true);  
             unrolledStatements.push_back(afterForStatement);
         }
        return unrolledStatements;
    }
    for (int i = initialIterator; i < conditionValue; i += updateValue){
        for (Statement *statement : body)
        {
            Statement *newStatement = updateStatement(statement, forStatement->getInitialAssign()->getLValue()->getValue(), i);
            unrolledStatements.push_back(newStatement);
        }
    }

    return unrolledStatements;
}


llvm::SmallVector<Statement *> completeUnroll(WhileStatement *whileStatement, int k)
{
    llvm::SmallVector<Statement *> unrolledStatements;
    llvm::SmallVector<Statement *> body = whileStatement->getStatements();
    llvm::SmallVector<Statement *> newBody;

    BooleanOp *condition_boolean_op = (BooleanOp *)whileStatement->getCondition();
    int conditionValue = condition_boolean_op->getRight()->getNumber();
    int conditionValue_arash = conditionValue;
    if(condition_boolean_op->getOperator() == BooleanOp::LessEqual){
        conditionValue++;
    }
    llvm::StringRef iteratorVar = condition_boolean_op->getLeft()->getValue();
    int initialIterator = 0;
    int updateValue = 0;
    // find initial value of iterator from the body
    AssignStatement* updateValueStatement;
    for(Statement *statement : body){
        AssignStatement* assignStatement = (AssignStatement *)statement;
        if(assignStatement->getLValue()->getValue() == iteratorVar){
            updateValueStatement = assignStatement;
            updateValue = ((BinaryOp *)assignStatement->getRValue())->getRight()->getNumber();
            continue;
        }
        newBody.push_back(statement);
    }


    if(k > 0){
        llvm::SmallVector<Statement *> newForBody;
        for(Statement *statement : newBody){
            for(int i = 0; i < k; i++){
                AssignStatement* assignStatement = (AssignStatement *)statement;
                Statement *newStatement = updateStatement(statement, iteratorVar, i * updateValue);
                newForBody.push_back(newStatement);
            }
        }
        AssignStatement * newForUpdate = new AssignStatement(condition_boolean_op->getLeft(), new BinaryOp(BinaryOp::Plus, condition_boolean_op->getLeft(), new Expression(k * updateValue)));
        Expression* newCondition = new BooleanOp(condition_boolean_op->getOperator(), condition_boolean_op->getLeft(), new Expression(conditionValue_arash - updateValue));
        AssignStatement* newInitialAssign = new AssignStatement(condition_boolean_op->getLeft(), new Expression(0));
        ForStatement* newForStatement = new ForStatement(newCondition, newForBody, newInitialAssign, newForUpdate, Statement::StatementType::For, true);
        unrolledStatements.push_back(newForStatement);
        if(conditionValue % (k * updateValue) != 0){
             newBody.push_back(updateValueStatement);
             WhileStatement* afterForStatement = new WhileStatement(whileStatement->getCondition(), newBody, Statement::StatementType::While, true);  
             unrolledStatements.push_back(afterForStatement);
         }
        return unrolledStatements;
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

// for(i 0 ta 10){
// x = x + 1;

// }

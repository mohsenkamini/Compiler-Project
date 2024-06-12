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

Statement *updateStatement(Statement *statement, llvm::StringRef iterator, int increase)
{
    if (statement.getKind() == Statement::StatementType::Assignment)
    {
        Assignment *assignment = (Assignment *)statement;
        Expression *right = assignment->getRValue();
        Expression *newRight = updateExpression(right, iterator, increase);
        return new Assignment(assignment->getLValue(), newRight);
    }
    if (statement->getKind() == Statement::StatementType::Declaration)
    {
        Declaration *declaration = (Declaration *)statement;
        Expression *right = declaration->getRValue();
        Expression *newRight = updateExpression(right, iterator, increase);
        return new Declaration(declaration->getLValue(), newRight);
    }
    if (statement->getKind() == Statement::StatementType::If)
    {
        IfStatement *ifStatement = (IfStatement *)statement;

        // Update condition
        Expression *condition = ifStatement->getCondition();
        Expression *newCondition = updateExpression(condition, iterator, increase);

        // Update body
        llvm::SmallVector<Statement *> ifBody = ifStatement->getStatements();
        llvm::SmallVector<Statement *> newIfBody;
        for (Statement *s : ifBody)
        {
            Statement *newIfStatement = updateStatement(s, iterator, increase);
            newIfBody.push_back(newIfStatement);
        }

        // Update else if
        llvm::SmallVector<ElseIfStatement *> newElseIfStatements;
        if (ifStatement->HasElseIf())
        {
            llvm::SmallVector<ElseIfStatement *> elseIfStatements = ifStatement->getElseIfStatements();
            for (ElseIfStatement *elseIfStatement : elseIfStatements)
            {
                Expression *elseIfCondition = elseIfStatement->getCondition();
                Expression *newElseIfCondition = updateExpression(elseIfCondition, iterator, increase);
                llvm::SmallVector<Statement *> elseIfBody = elseIfStatement->getStatements();
                llvm::SmallVector<Statement *> newElseIfBody;
                for (Statement *s : elseIfBody)
                {
                    Statement *newElseIfStatement = updateStatement(s, iterator, increase);
                    newElseIfBody.push_back(newElseIfStatement);
                }
                newElseIfStatements.push_back(new ElseIfStatement(newElseIfCondition, newElseIfBody, Statement::StatementType::ElseIf));
            }
        }

        // Update else
        ElseStatement *newElseStatement;
        if (ifStatement->HasElse())
        {
            ElseStatement *elseStatement = ifStatement->getElseStatement();
            llvm::SmallVector<Statement *> newElseBody;
            llvm::SmallVector<Statement *> elseBody = elseStatement->getStatements();
            for (Statement *s : elseBody)
            {
                Statement *newElseBodyStatement = updateStatement(s, iterator, increase);
                newElseBody.push_back(newElseBodyStatement);
            }
            newElseStatement = new ElseStatement(newElseBody, Statement::StatementType::Else);
        }

        return new IfStatement(newCondition, newIfBody, newElseIfStatements, newElseStatement, ifStatement->HasElseIf(), ifStatement->HasElse(), Statement::StatementType::If);
    }
    return statement;
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
            Statement *newStatement = updateStatement(statement, forStatement->getInitialAssign().getLValue().getValue(), i);
            unrolledStatements.push_back(newStatement);
        }
    }
    return unrolledStatements;
}

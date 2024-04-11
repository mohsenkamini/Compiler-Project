#ifndef AST_H
#define AST_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

class AST;                      // Abstract Syntax Tree
class Base;                     // top level program
class Unary;
class Print;
class BinaryOp;                 // binary operation of numbers and identifiers
class Statement;                // top level statement
class BooleanOp;                // boolean operation like 3 > 6*2;
class Expression;               // top level expression that is evaluated to boolean, int or variable name at last
class IfStatement;
class DecStatement;             // declaration statement like int a;
class LoopStatement;
class ElseIfStatement;
class ElseStatement;
class AssignStatement;          // assignment statement like a = 3;
class CommentStatement;
class afterCheckStatement;

class ElseIfStatement : public Statement
{
private:
    Expression *Condition;
    llvm::SmallVector<Statement *> Statements;

public:
    ElifStatement(Expression *condition, llvm::SmallVector<Statement *> statements, StateMentType type) : Condition(condition), Statements(statements), Statement(type) {}

    Expression *getCondition()
    {
        return Condition;
    }

    llvm::SmallVector<Statement *> getStatements()
    {
        return Statements;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

#endif

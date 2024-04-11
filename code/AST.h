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

// stores information of a statement. For example x=56; is a statement
class Statement : public TopLevelEntity
{
public:
    enum StatementType
    {
        If,
        ElseIf,
        Else
    };

private:
    StatementType Type;

public:
    StatementType getKind()
    {
        return Type;
    }

    Statement(StatementType type) : Type(type) {}
    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class IfStatement : public Statement
{

private:
    Expression *condition;
    llvm::SmallVector<Statement *> statements;
    llvm::SmallVector<ElseIfStatement *> elseIfStatements;
    ElseStatement *elseStatement;
    bool hasElseIf;
    bool hasElse;

public:
    IfStatement(Expression *condition,
                llvm::SmallVector<Statement *> statements,
                llvm::SmallVector<ElseIfStatement *> elseIfStatements,
                ElseStatement *elseStatement,
                bool hasElseIf,
                bool hasElse,
                StatementType type) : condition(condition),
                                      statements(statements),
                                      elseIfStatements(elseIfStatements),
                                      elseStatement(elseStatement),
                                      hasElseIf(hasElseIf),
                                      hasElse(hasElse),
                                      Statement(type) {}

    Expression *getCondition()
    {
        return condition;
    }

    bool HasElseIf()
    {
        return hasElseIf;
    }

    bool HasElse()
    {
        return hasElse;
    }

    llvm::SmallVector<ElseIfStatement *> getElseIfStatements()
    {
        return elseIfStatements;
    }

    llvm::SmallVector<Statement *> getStatements()
    {
        return statements;
    }

    ElseStatement *getElseStatement()
    {
        return elseStatement;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

class ElseIfStatement
{
private:
    IfStatement *ifStatement;

public:
    ElseIfStatement(IfStatement *ifStatement) : ifStatement(ifStatement) {}

    IfStatement *getIfStatement()
    {
        return ifStatement;
    }
};

class ElseStatement : public Statement
{

private:
    llvm::SmallVector<Statement *> statements;

public:
    ElseStatement(llvm::SmallVector<Statement *> statements, StatementType type) : statements(statements), Statement(type) {}

    llvm::SmallVector<Statement *> getStatements()
    {
        return statements;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

#endif

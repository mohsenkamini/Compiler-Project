#ifndef AST_H
#define AST_H

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"

class AST;                      // Abstract Syntax Tree
class Base;                     // top level program
class PrintStatement;
class BinaryOp;                 // binary operation of numbers and identifiers
class Statement;                // top level statement
class BooleanOp;                // boolean operation like 3 > 6*2;
class Expression;               // top level expression that is evaluated to boolean, int or variable name at last
class IfStatement;
class DecStatement;             // declaration statement like int a;
class ElseIfStatement;
class ElseStatement;
class AssignStatement;          // assignment statement like a = 3;
// class afterCheckStatement;


class ASTVisitor
{
public:
	// Virtual visit functions for each AST node type
	virtual void visit(AST&) {}
	virtual void visit(Expression&) {}
	virtual void visit(Base&) = 0;
	virtual void visit(Statement&) = 0;
	virtual void visit(BinaryOp&) = 0;
	virtual void visit(DecStatement&) = 0;
	virtual void visit(AssignStatement&) = 0;
	virtual void visit(BooleanOp&) = 0;
	virtual void visit(IfStatement&) = 0;
	virtual void visit(ElseIfStatement&) = 0;
	virtual void visit(ElseStatement&) = 0;
    virtual void visit(PrintStatement&) = 0;
	// virtual void visit(LoopStatement&) = 0;
};

class AST {
public:
	virtual ~AST() {}
	virtual void accept(ASTVisitor& V) = 0;
};

class TopLevelEntity : AST {
public:
	TopLevelEntity() {}
};


class Expression : public TopLevelEntity {
public:
	enum ExpressionType {
		Number,
		Identifier,
		Boolean,
		BinaryOpType,
		BooleanOpType
	};
private:
	ExpressionType Type;
	llvm::StringRef Value;
	int NumberVal;
	bool BoolVal;
	BooleanOp* BOVal;

public:
	Expression() {}
	Expression(llvm::StringRef value) : Type(ExpressionType::Identifier), Value(value) {} // store string
	Expression(int value) : Type(ExpressionType::Number), NumberVal(value) {} // store number
	Expression(bool value) : Type(ExpressionType::Boolean), BoolVal(value) {} // store boolean
	Expression(BooleanOp* value) : Type(ExpressionType::BooleanOpType), BOVal(value) {} // store boolean
	Expression(ExpressionType type) : Type(type) {}

	bool isNumber() {
		if (Type == ExpressionType::Number)
			return true;
		return false;
	}

	bool isBoolean() {
		if (Type == ExpressionType::Boolean)
			return true;
		return false;
	}

	bool isVariable() {
		if (Type == ExpressionType::Identifier)
			return true;
		return false;
	}

	llvm::StringRef getValue() {
		return Value;
	}

	int getNumber() {
		return NumberVal;
	}

	BooleanOp* getBooleanOp() {
		return BOVal;
	}

	bool getBoolean() {
		return BoolVal;
	}
	ExpressionType getKind()
	{
		return Type;
	}

	virtual void accept(ASTVisitor& V) override
	{
		V.visit(*this);
	}
};

class BooleanOp : public Expression
{
public:
	enum Operator
	{
		LessEqual,
		Less,
		Greater,
		GreaterEqual,
		Equal,
		NotEqual,
		And,
		Or
	};

private:
	Expression* Left;                               
	Expression* Right;                              
	Operator Op;

public:
	BooleanOp(Operator Op, Expression* L, Expression* R) : Op(Op), Left(L), Right(R), Expression(ExpressionType::BooleanOpType) { }

	Expression* getLeft() { return Left; }

	Expression* getRight() { return Right; }

	Operator getOperator() { return Op; }

	virtual void accept(ASTVisitor& V) override
	{
		V.visit(*this);
	}
};


class BinaryOp : public Expression
{
public:
	enum Operator
	{
		Plus,
		Minus,
		Mul,
		Div,
		Mod,
		Pow
	};

private:
	Expression* Left;                               // Left-hand side expression
	Expression* Right;                              // Right-hand side expression
	Operator Op;                              // Operator of the binary operation

public:
	BinaryOp(Operator Op, Expression* L, Expression* R) : Op(Op), Left(L), Right(R), Expression(ExpressionType::BinaryOpType) {}

	Expression* getLeft() { return Left; }

	Expression* getRight() { return Right; }

	Operator getOperator() { return Op; }

	virtual void accept(ASTVisitor& V) override
	{
		V.visit(*this);
	}
};


class Base : public AST
{
private:
    llvm::SmallVector<Statement *> statements;

public:
    Base(llvm::SmallVector<Statement *> Statements) : statements(Statements) {}
    llvm::SmallVector<Statement *> getStatements() { return statements; }

    llvm::SmallVector<Statement *>::const_iterator begin() { return statements.begin(); }

    llvm::SmallVector<Statement *>::const_iterator end() { return statements.end(); }
    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};

// stores information of a statement. For example x=56; is a statement
class Statement : public TopLevelEntity
{
public:
    enum StatementType
    {
        If,
        ElseIf,
        Else,
        Print
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

class PrintStatement : public Statement
{
    private:
    Expression * identifier;
    public:
    PrintStatement(Expression * identifier, StatementType type) : identifier(identifier), Statement(type) {}
    Expression * getIdentifier()
    {
        return identifier;
    }
    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
    }
};


class DecStatement : public Statement {
private:

	Expression* lvalue;
	Expression* rvalue;
	Statement::StatementType type;

public:
	DecStatement(Expression* lvalue, Expression* rvalue) : lvalue(lvalue), rvalue(rvalue), type(Statement::StatementType::Declaration), Statement(type) { }
	DecStatement(Expression* lvalue) : lvalue(lvalue), rvalue(rvalue), type(Statement::StatementType::Declaration), Statement(type) { rvalue = new Expression(0); }

	Expression* getLValue() {
		return lvalue;
	}

	Expression* getRValue() {
		return rvalue;
	}

	virtual void accept(ASTVisitor& V) override
	{
		V.visit(*this);
	}
};


class AssignStatement : public Statement {
private:

	Expression* lvalue;
	Expression* rvalue;
	Statement::StatementType type;

public:
	AssignStatement(Expression* lvalue, Expression* rvalue) : lvalue(lvalue), rvalue(rvalue), type(Statement::StatementType::Assignment), Statement(type) { }
	Expression* getLValue() {
		return lvalue;
	}

	Expression* getRValue() {
		return rvalue;
	}

	virtual void accept(ASTVisitor& V) override
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

class ElseIfStatement : public Statement
{

private:
    Expression *condition;
    llvm::SmallVector<Statement *> statements;

public:
    ElseIfStatement(Expression *condition, llvm::SmallVector<Statement *> statements, StatementType type) : condition(condition), statements(statements), Statement(type) {}

    Expression *getCondition()
    {
        return condition;
    }

    llvm::SmallVector<Statement *> getStatements()
    {
        return statements;
    }

    virtual void accept(ASTVisitor &V) override
    {
        V.visit(*this);
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

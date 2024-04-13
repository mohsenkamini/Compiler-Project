#include "semantic.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace
{
    class DeclCheck : public ASTVisitor
    {
        llvm::StringSet<> Scope;
        bool HasError;

        enum ErrorType
        {
            Twice,
            Not,
            DivByZero
        };

        void error(ErrorType ET, llvm::StringRef V)
        {
            if (ET == ErrorType::DivByZero)
            {
                llvm::errs() << "Division by zero is not allowed."
                             << "\n";
            }
            else
            {
                llvm::errs() << "Variable " << V << " is " << (ET == Twice ? "already" : "not") << " declared!\n";
            }
            HasError = true;
            exit(3);
        }

    public:
        DeclCheck() : HasError(false) {}

        bool hasError() { return HasError; }

        virtual void visit(Base &Node) override
        {
            for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        };

        virtual void visit(Unary &Node) override{
            // TODO: Implement
        };

        virtual void visit(PrintStatement &Node) override
        {
            Expression *declaration = (Expression *)Node.getIdentifier();
            declaration->accept(*this);
        };

        virtual void visit(BinaryOp &Node) override
        {
            if (Node.getLeft())
            {
                Node.getLeft()->accept(*this);
            }
            else
            {
                HasError = true;
            }
            if (Node.getRight())
            {
                Node.getRight()->accept(*this);
            }
            else
            {
                HasError = true;
            }

            // Divide by zero check
            if (Node.getOperator() == BinaryOp::Operator::Div)
            {
                Expression *right = (Expression *)Node.getRight();
                if (right->isNumber() && right->getNumber() == 0)
                {
                    error(DivByZero, ((Expression *)Node.getLeft())->getValue());
                }
            }
        };

        virtual void visit(Statement &Node) override
        {
            if (Node.getKind() == Statement::StatementType::Declaration)
            {
                DecStatement *declaration = (DecStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::Assignment)
            {
                AssignStatement *declaration = (AssignStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::If)
            {
                IfStatement *declaration = (IfStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::ElseIf)
            {
                ElseIfStatement *declaration = (ElseIfStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::Else)
            {
                ElseStatement *declaration = (ElseStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::Print)
            {
                Print *declaration = (Print *)&Node;
                declaration->accept(*this);
            }
            // TODO: While and For
        };

        virtual void visit(BooleanOp &Node) override
        {
            if (Node.getLeft())
            {
                Node.getLeft()->accept(*this);
            }
            else
            {
                HasError = true;
            }
            if (Node.getRight())
            {
                Node.getRight()->accept(*this);
            }
            else
            {
                HasError = true;
            }
        };

        virtual void visit(Expression &Node) override{
            // TODO: Implement
        };

        virtual void visit(DecStatement &Node) override{
            // TODO: Implement
        };

        virtual void visit(IfStatement &Node) override
        {
            Expression *declaration = (Expression *)Node.getCondition();
            declaration->accept(*this);
            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        };

        virtual void visit(ElseIfStatement &Node) override
        {
            Expression *declaration = (Expression *)Node.getCondition();
            declaration->accept(*this);
            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        };

        virtual void visit(ElseStatement &Node) override
        {
            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        };

        virtual void visit(LoopStatement &Node) override{
            // TODO: Implement

        };

        virtual void visit(AssignStatement &Node) override{
            // TODO: Implement

        };

        virtual void visit(CommentStatement &Node) override{
            // TODO: Implement

        };
    };
}

bool Semantic::semantic(AST *Tree)
{
    if (!Tree)
        return false;
    DeclCheck Check;
    Tree->accept(Check);
    return Check.hasError();
}

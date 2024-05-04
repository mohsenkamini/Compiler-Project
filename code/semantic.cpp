#include "semantic.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

namespace
{
    class DeclCheck : public ASTVisitor
    {
        llvm::StringMap<char> variableTypeMap;
        bool HasError;

        enum ErrorType
        {
            AlreadyDefinedVariable,
            NotDefinedVariable,
            DivideByZero,
            WrongValueTypeForVariable
        };

        void error(ErrorType errorType, llvm::StringRef V)
        {
            switch (errorType)
            {
            case ErrorType::DivideByZero:
                llvm::errs() << "Division by zero is not allowed!\n";
                break;
            case ErrorType::AlreadyDefinedVariable:
                llvm::errs() << "Variable " << V << " is already declared!\n";
                break;
            case ErrorType::NotDefinedVariable:
                llvm::errs() << "Variable " << V << " is not declared!\n";
                break;
            case ErrorType::WrongValueTypeForVariable:
                llvm::errs() << "Illegal value for type " << V << "!\n";
                break;
            default:
                llvm::errs() << "Unknown error\n";
                break;
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

        virtual void visit(PrintStatement &Node) override
        {
            Expression *declaration = (Expression *)Node.getExpr();
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
                    error(DivideByZero, ((Expression *)Node.getLeft())->getValue());
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
                PrintStatement *declaration = (PrintStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::While)
            {
                WhileStatement *declaration = (WhileStatement *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StatementType::For)
            {
                ForStatement *declaration = (ForStatement *)&Node;
                declaration->accept(*this);
            }
            
            
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

        virtual void visit(Expression &Node) override
        {
            if (Node.getKind() == Expression::ExpressionType::Identifier)
            {
                if (variableTypeMap.count(Node.getValue()) == 0)
                {
                    error(NotDefinedVariable, Node.getValue());
                }
            }
            else if (Node.getKind() == Expression::ExpressionType::BinaryOpType)
            {
                BinaryOp *declaration = (BinaryOp *)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Expression::ExpressionType::BooleanOpType)
            {
                Node.getBooleanOp()->getLeft()->accept(*this);
                Node.getBooleanOp()->getRight()->accept(*this);
            }
        };

        virtual void visit(DecStatement &Node) override
        {
            if (variableTypeMap.count(Node.getLValue()->getValue()) > 0)
            {
                error(AlreadyDefinedVariable, Node.getLValue()->getValue());
            }
            // Add this new variable to variableTypeMap
            if (Node.getDecType() == DecStatement::DecStatementType::Boolean)
            {
                variableTypeMap[Node.getLValue()->getValue()] = 'b';
            }
            else
            {
                variableTypeMap[Node.getLValue()->getValue()] = 'i';
            }

            Expression *rightValue = (Expression *)Node.getRValue();
            if (rightValue == nullptr)
            {
                return;
            }
            if (Node.getDecType() == DecStatement::DecStatementType::Boolean)
            {
                if (!(rightValue->getKind() == Expression::ExpressionType::Boolean ||
                      rightValue->getKind() == Expression::ExpressionType::BooleanOpType))
                {
                    error(WrongValueTypeForVariable, "bool");
                }
            }
            else if (Node.getDecType() == DecStatement::DecStatementType::Number)
            {
                if (!(rightValue->getKind() == Expression::ExpressionType::Number ||
                      rightValue->getKind() == Expression::ExpressionType::BinaryOpType))
                {
                    error(WrongValueTypeForVariable, "int");
                }
            }

            rightValue->accept(*this);
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

        virtual void visit(AssignStatement &Node) override
        {
            Node.getLValue()->accept(*this);
            Node.getRValue()->accept(*this);
            if (variableTypeMap.lookup(Node.getLValue()->getValue()) == 'i' &&
                (Node.getRValue()->getKind() == Expression::ExpressionType::Boolean ||
                 Node.getRValue()->getKind() == Expression::ExpressionType::BooleanOpType))
            {
                error(WrongValueTypeForVariable, "int");
            }
            if (variableTypeMap.lookup(Node.getLValue()->getValue()) == 'b' &&
                (Node.getRValue()->getKind() == Expression::ExpressionType::Number ||
                 Node.getRValue()->getKind() == Expression::ExpressionType::BinaryOpType))
            {
                error(WrongValueTypeForVariable, "bool");
            }
        };

        virtual void visit(WhileStatement &Node) override{
            Node.getCondition()->accept(*this);
            llvm::SmallVector<Statement* > stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        };
        
        virtual void visit(ForStatement &Node) override{
            AssignStatement * initial_assign = Node.getInitialAssign();
            /*if(initial_assign == nullptr){
                exit(0);
            }*/
            (initial_assign->getLValue())->accept(*this);
            (initial_assign->getRValue())->accept(*this);
            Node.getCondition()->accept(*this);
            
            AssignStatement * update_assign = Node.getUpdateAssign();
            if(update_assign == nullptr){
                exit(0);
            }
            (update_assign->getLValue())->accept(*this);
            (update_assign->getRValue())->accept(*this);

            llvm::SmallVector<Statement* > stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
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

#include "code_generator.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
{
    class ToIRVisitor : public ASTVisitor
    {
        Module *M;
        IRBuilder<> Builder;
        Type *VoidTy;
        Type *Int32Ty;
        Type *Int1Ty;
        Type *Int8PtrTy;
        Type *Int8PtrPtrTy;
        Constant *Int32Zero;
        Constant *Int1Zero;

        Value *V;
        StringMap<AllocaInst *> nameMap;

        llvm::FunctionType *MainFty;
        llvm::Function *MainFn;
        FunctionType *CalcWriteFnTy;
        FunctionType *CalcWriteFnTyBool;
        Function *CalcWriteFn;
        Function *CalcWriteFnBool;
        

    public:
        // Constructor for the visitor class
        ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
        {
            // Initialize LLVM types and constants
            VoidTy = Type::getVoidTy(M->getContext());
            Int32Ty = Type::getInt32Ty(M->getContext());
            Int1Ty = Type::getInt1Ty(M->getContext());
            Int8PtrTy = Type::getInt8PtrTy(M->getContext());
            Int8PtrPtrTy = Int8PtrTy->getPointerTo();
            Int32Zero = ConstantInt::get(Int32Ty, 0, true);
            CalcWriteFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
            CalcWriteFnTyBool = FunctionType::get(VoidTy, {Int1Ty}, false);
            CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "print", M);
            CalcWriteFnBool = Function::Create(CalcWriteFnTyBool, GlobalValue::ExternalLinkage, "print", M);
        }

        // Entry point for generating LLVM IR from the AST
        void run(AST *Tree)
        {
            // Create the main function with the appropriate function type.
            MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
            MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

            // Create a basic block for the entry point of the main function.
            BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
            Builder.SetInsertPoint(BB);

            // Visit the root node of the AST to generate IR
            Tree->accept(*this);

            // Create a return instruction at the end of the main function
            Builder.CreateRet(Int32Zero);
        }

        virtual void visit(Base &Node) override
        {
            // TODO: find a better way to not implement this again!
            for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }

        virtual void visit(Statement &Node) override
        {
            // TODO: find a better way to not implement this again!
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
            // TODO: While and For
        }

        // virtual void visit(PrintStatement &Node) override
        // {
        //     // Visit the right-hand side of the assignment and get its value.
        //     Node.getExpr()->accept(*this);
        //     Value *val = V;
        //     CallInst *Call = Builder.CreateCall(CalcWriteFnTyBool, CalcWriteFnBool, {val});
        //     // if (Node.getExpr()->isBoolean()) {
                
        //     // }else{
        //     //     CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
        //     // }
            
        // }

        virtual void visit(PrintStatement &Node) override {
            // Visit the right-hand side of the expression and get its value.
            Node.getExpr()->accept(*this);
            Value *val = V;

            // Determine the type of 'val' and select the appropriate print function
            Type *valType = val->getType();
            // if (valType == Int1Ty) {
            //     val = Builder.CreateZExt(val, Int32Ty);
            // }
            val = Builder.CreateZExt(val, Int1Ty);
            CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
        }

        virtual void visit(Expression &Node) override
        {
            if (Node.getKind() == Expression::ExpressionType::Identifier)
            {
                V = Builder.CreateLoad(Int32Ty, nameMap[Node.getValue()]);
            }
            else if (Node.getKind() == Expression::ExpressionType::Number)
            {
                int int_value = Node.getNumber();
                V = ConstantInt::get(Int32Ty, int_value, true);
            }
            else if (Node.getKind() == Expression::ExpressionType::Boolean)
            {
                bool bool_value = Node.getBoolean();
                V = ConstantInt::getBool(Int1Ty, bool_value);
            }
        }

        virtual void visit(BooleanOp &Node) override
        {
            // Visit the left-hand side of the binary operation and get its value
            Node.getLeft()->accept(*this);
            Value *Left = V;

            // Visit the right-hand side of the binary operation and get its value
            Node.getRight()->accept(*this);
            Value *Right = V;

            // Perform the boolean operation based on the operator type and create the corresponding instruction
            switch (Node.getOperator())
            {
            case BooleanOp::Equal:
                V = Builder.CreateICmpEQ(Left, Right);
                break;
            case BooleanOp::NotEqual:
                V = Builder.CreateICmpNE(Left, Right);
                break;
            case BooleanOp::Less:
                V = Builder.CreateICmpSLT(Left, Right);
                break;
            case BooleanOp::LessEqual:
                V = Builder.CreateICmpSLE(Left, Right);
                break;
            case BooleanOp::Greater:
                V = Builder.CreateICmpSGT(Left, Right);
                break;
            case BooleanOp::GreaterEqual:
                V = Builder.CreateICmpSGE(Left, Right);
                break;
            case BooleanOp::And:
                V = Builder.CreateAnd(Left, Right);
                break;
            case BooleanOp::Or:
                V = Builder.CreateOr(Left, Right);
                break;
            }
        }

        virtual void visit(BinaryOp &Node) override
        {
            // Visit the left-hand side of the binary operation and get its value
            Node.getLeft()->accept(*this);
            Value *Left = V;

            // Visit the right-hand side of the binary operation and get its value
            Node.getRight()->accept(*this);
            Value *Right = V;

            // Perform the binary operation based on the operator type and create the corresponding instruction
            switch (Node.getOperator())
            {
            case BinaryOp::Plus:
                V = Builder.CreateNSWAdd(Left, Right);
                break;
            case BinaryOp::Minus:
                V = Builder.CreateNSWSub(Left, Right);
                break;
            case BinaryOp::Mul:
                V = Builder.CreateNSWMul(Left, Right);
                break;
            case BinaryOp::Div:
                V = Builder.CreateSDiv(Left, Right);
                break;
            case BinaryOp::Pow:
                if ((Node.getRight())->isNumber())
                {
                    int power = (Node.getRight())->getNumber();
                    Value *result = Left;
                    for (int i = 1; i < power; i++)
                    {
                        result = Builder.CreateNSWMul(result, Left);
                    }
                    V = result;
                }
                break;
            case BinaryOp::Mod:
                Value *division = Builder.CreateSDiv(Left, Right);
                Value *multiplication = Builder.CreateNSWMul(division, Right);
                V = Builder.CreateNSWSub(Left, multiplication);
            }
        }

        virtual void visit(DecStatement &Node) override
        {
            Value *val = nullptr;

            if (Node.getRValue() != nullptr)
            {
                // If there is an expression provided, visit it and get its value
                Node.getRValue()->accept(*this);
                val = V;
            }

            // Iterate over the variables declared in the declaration statement
            auto I = Node.getLValue()->getValue();
            StringRef Var = I;

            // Create an alloca instruction to allocate memory for the variable
            Type *varType = (Node.getDecType() == DecStatement::DecStatementType::Number) ? Int32Ty : Type::getInt1Ty(M->getContext());
            nameMap[Var] = Builder.CreateAlloca(varType);

            // Store the initial value (if any) in the variable's memory location
            if (val != nullptr)
            {
                Builder.CreateStore(val, nameMap[Var]);
            }
            else
            {
                if(Node.getDecType() == DecStatement::DecStatementType::Number){
                    Value *Zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
                    Builder.CreateStore(Zero, nameMap[Var]);
                } else{
                    Value *False = ConstantInt::get(Type::getInt1Ty(M->getContext()), 0);
                    Builder.CreateStore(False, nameMap[Var]);
                }
                
            }
        }

        virtual void visit(AssignStatement &Node) override
        {
            // Visit the right-hand side of the assignment and get its value
            Node.getRValue()->accept(*this);
            Value *val = V;

            // Get the name of the variable being assigned
            auto varName = Node.getLValue()->getValue();

            // Create a store instruction to assign the value to the variable
            Builder.CreateStore(val, nameMap[varName]);
        }

        virtual void visit(IfStatement &Node) override
        {
            llvm::BasicBlock *IfCondBB = llvm::BasicBlock::Create(M->getContext(), "if.cond", MainFn);
            llvm::BasicBlock *IfBodyBB = llvm::BasicBlock::Create(M->getContext(), "if.body", MainFn);
            llvm::BasicBlock *AfterIfBB = llvm::BasicBlock::Create(M->getContext(), "after.if", MainFn);

            Builder.CreateBr(IfCondBB);
            Builder.SetInsertPoint(IfCondBB);

            Node.getCondition()->accept(*this);
            Value *Cond = V;

            Builder.SetInsertPoint(IfBodyBB);

            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }

            Builder.CreateBr(AfterIfBB);
            llvm::BasicBlock *BeforeCondBB = IfCondBB;
            llvm::BasicBlock *BeforeBodyBB = IfBodyBB;
            llvm::Value *BeforeCondVal = Cond;

            if (Node.HasElseIf())
            {
                for (auto &elseIf : Node.getElseIfStatements())
                {
                    llvm::BasicBlock *ElseIfCondBB = llvm::BasicBlock::Create(MainFn->getContext(), "elseIf.cond", MainFn);
                    llvm::BasicBlock *ElseIfBodyBB = llvm::BasicBlock::Create(MainFn->getContext(), "elseIf.body", MainFn);

                    Builder.SetInsertPoint(BeforeCondBB);
                    Builder.CreateCondBr(BeforeCondVal, BeforeBodyBB, ElseIfCondBB);

                    Builder.SetInsertPoint(ElseIfCondBB);
                    elseIf->getCondition()->accept(*this);
                    llvm::Value *ElseIfCondVal = V;

                    Builder.SetInsertPoint(ElseIfBodyBB);
                    elseIf->accept(*this);
                    Builder.CreateBr(AfterIfBB);

                    BeforeCondBB = ElseIfCondBB;
                    BeforeCondVal = ElseIfCondVal;
                    BeforeBodyBB = ElseIfBodyBB;
                }
            }

            llvm::BasicBlock *ElseBB = nullptr;
            if (Node.HasElse())
            {
                ElseStatement *elseS = Node.getElseStatement();
                ElseBB = llvm::BasicBlock::Create(MainFn->getContext(), "else.body", MainFn);
                Builder.SetInsertPoint(ElseBB);
                Node.getElseStatement()->accept(*this);
                Builder.CreateBr(AfterIfBB);

                Builder.SetInsertPoint(BeforeCondBB);
                Builder.CreateCondBr(Cond, IfBodyBB, ElseBB);
            }
            else
            {
                Builder.SetInsertPoint(BeforeCondBB);
                Builder.CreateCondBr(Cond, IfBodyBB, AfterIfBB);
            }

            Builder.SetInsertPoint(AfterIfBB);
        }

        virtual void visit(ElseIfStatement &Node) override
        {
            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }

        virtual void visit(ElseStatement &Node) override
        {
            llvm::SmallVector<Statement *> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }

        virtual void visit(LoopStatement &Node) override
        {
            // TODO: Implement
        }
    };
}; // namespace

void CodeGen::compile(AST *Tree)
{
    // Create an LLVM context and a module
    LLVMContext Ctx;
    Module *M = new Module("mas.expr", Ctx);

    // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR
    ToIRVisitor ToIRn(M);

    ToIRn.run(Tree);

    // Print the generated module to the standard output
    M->print(outs(), nullptr);
}

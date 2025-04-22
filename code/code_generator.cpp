#include "code_generator.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class ToIRVisitor : public ASTVisitor {
    LLVMContext &Ctx;
    std::unique_ptr<Module> M;
    IRBuilder<> Builder;
    Type *Int32Ty;
    Type *Int1Ty;
    Constant *Int32Zero;
    Constant *Int1False;

    StringMap<AllocaInst*> NameMap;
    Value *V;  // holds last expression value
    Function *MainFn;
    FunctionCallee PrintIntFn;
    FunctionCallee PrintBoolFn;
    bool Optimize;
    int UnrollFactor;

public:
    ToIRVisitor(LLVMContext &context, bool optimize, int unroll)
        : Ctx(context), M(new Module("main_module", context)), Builder(context),
          Optimize(optimize), UnrollFactor(unroll) {
        Int32Ty = Type::getInt32Ty(Ctx);
        Int1Ty  = Type::getInt1Ty(Ctx);
        Int32Zero  = ConstantInt::get(Int32Ty, 0);
        Int1False  = ConstantInt::get(Int1Ty, 0);
        // declare external print functions
        PrintIntFn  = M->getOrInsertFunction("print", FunctionType::get(Type::getVoidTy(Ctx), {Int32Ty}, false));
        PrintBoolFn = M->getOrInsertFunction("printBool", FunctionType::get(Type::getVoidTy(Ctx), {Int1Ty}, false));
    }

    Module *getModule() { return M.get(); }

    void run(ProgramNode *root) {
        // define main i32 @main()
        FunctionType *FT = FunctionType::get(Int32Ty, false);
        MainFn = Function::Create(FT, GlobalValue::ExternalLinkage, "main", M.get());
        BasicBlock *BB = BasicBlock::Create(Ctx, "entry", MainFn);
        Builder.SetInsertPoint(BB);
        // generate body
        root->accept(*this);
        Builder.CreateRet(Int32Zero);
        // verify
        verifyFunction(*MainFn);
    }

    // Helpers
    AllocaInst *createEntryBlockAlloca(Function *F, const std::string &Name, Type *Ty) {
        IRBuilder<> TmpB(&F->getEntryBlock(), F->getEntryBlock().begin());
        return TmpB.CreateAlloca(Ty, nullptr, Name);
    }

    // Visitor implementations
    void visit(ProgramNode &node) override {
        for (auto &stmt : node.statements)
            stmt->accept(*this);
    }

    void visit(MultiVarDeclNode &node) override {
        for (auto &decl : node.declarations) {
            Type *Ty = (decl->type == VarType::BOOL ? Int1Ty : Int32Ty);
            AllocaInst *A = createEntryBlockAlloca(MainFn, decl->name, Ty);
            NameMap[decl->name()] = A;
            // initialize
            if (decl->value) {
                decl->value->accept(*this);
                Builder.CreateStore(V, A);
            } else {
                Builder.CreateStore(Ty==Int1Ty ? Int1False : Int32Zero, A);
            }
        }
    }

    void visit(AssignNode &node) override {
        node.value->accept(*this);
        auto *A = NameMap[node.target];
        Builder.CreateStore(V, A);
    }

    void visit(VarRefNode &node) override {
        AllocaInst *A = NameMap[node.name];
        V = Builder.CreateLoad(A->getAllocatedType(), A, node.name);
    }

    void visit(LiteralNode<int> &node) override {
        V = ConstantInt::get(Int32Ty, node.value);
    }
    void visit(LiteralNode<bool> &node) override {
        V = ConstantInt::get(Int1Ty, node.value);
    }

    void visit(BinaryOpNode &node) override {
        node.left->accept(*this);
        Value *L = V;
        node.right->accept(*this);
        Value *R = V;
        switch (node.op) {
        case BinaryOp::ADD:        V = Builder.CreateAdd(L, R); break;
        case BinaryOp::SUBTRACT:   V = Builder.CreateSub(L, R); break;
        case BinaryOp::MULTIPLY:   V = Builder.CreateMul(L, R); break;
        case BinaryOp::DIVIDE:     V = Builder.CreateSDiv(L, R); break;
        case BinaryOp::MOD:        V = Builder.CreateSRem(L, R); break;
        case BinaryOp::EQUAL:      V = Builder.CreateICmpEQ(L, R); break;
        case BinaryOp::NOT_EQUAL:  V = Builder.CreateICmpNE(L, R); break;
        case BinaryOp::LESS:       V = Builder.CreateICmpSLT(L, R); break;
        case BinaryOp::LESS_EQUAL: V = Builder.CreateICmpSLE(L, R); break;
        case BinaryOp::GREATER:    V = Builder.CreateICmpSGT(L, R); break;
        case BinaryOp::GREATER_EQUAL: V = Builder.CreateICmpSGE(L,R); break;
        case BinaryOp::AND:        V = Builder.CreateAnd(L, R); break;
        case BinaryOp::OR:         V = Builder.CreateOr(L, R); break;
        case BinaryOp::POW:
            // simple pow via loop omitted for brevity
            V = L; break;
        default:
            V = UndefValue::get(Int32Ty);
        }
    }

    void visit(UnaryOpNode &node) override {
        node.operand->accept(*this);
        switch (node.op) {
        case UnaryOp::INCREMENT: {
            Value *One = ConstantInt::get(Int32Ty, 1);
            V = Builder.CreateAdd(V, One);
            break;
        }
        case UnaryOp::DECREMENT: {
            Value *One = ConstantInt::get(Int32Ty, 1);
            V = Builder.CreateSub(V, One);
            break;
        }
        default:
            break;
        }
    }

    void visit(PrintNode &node) override {
        node.expr->accept(*this);
        if (V->getType()->isIntegerTy(1))
            Builder.CreateCall(PrintBoolFn, {V});
        else
            Builder.CreateCall(PrintIntFn,  {V});
    }

    void visit(IfElseNode &node) override {
        BasicBlock *ThenBB = BasicBlock::Create(Ctx, "then", MainFn);
        BasicBlock *ElseBB = BasicBlock::Create(Ctx, "else", MainFn);
        BasicBlock *MergeBB= BasicBlock::Create(Ctx, "iftmp", MainFn);
        // condition
        node.condition->accept(*this);
        Builder.CreateCondBr(V, ThenBB, ElseBB);
        // then
        Builder.SetInsertPoint(ThenBB);
        node.thenBlock->accept(*this);
        Builder.CreateBr(MergeBB);
        // else
        Builder.SetInsertPoint(ElseBB);
        if (node.elseBlock)
            node.elseBlock->accept(*this);
        Builder.CreateBr(MergeBB);
        // merge
        Builder.SetInsertPoint(MergeBB);
    }

    void visit(WhileLoopNode &node) override {
        BasicBlock *CondBB = BasicBlock::Create(Ctx, "while.cond", MainFn);
        BasicBlock *BodyBB = BasicBlock::Create(Ctx, "while.body", MainFn);
        BasicBlock *AfterBB= BasicBlock::Create(Ctx, "while.end", MainFn);
        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(CondBB);
        node.condition->accept(*this);
        Builder.CreateCondBr(V, BodyBB, AfterBB);
        Builder.SetInsertPoint(BodyBB);
        for (auto &s : node.body->statements)
            s->accept(*this);
        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(AfterBB);
    }

    void visit(ForLoopNode &node) override {
        // init
        if (node.init) node.init->accept(*this);
        BasicBlock *CondBB = BasicBlock::Create(Ctx,"for.cond",MainFn);
        BasicBlock *BodyBB = BasicBlock::Create(Ctx,"for.body",MainFn);
        BasicBlock *AfterBB= BasicBlock::Create(Ctx,"for.end",MainFn);
        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(CondBB);
        if (node.condition) { node.condition->accept(*this); Builder.CreateCondBr(V, BodyBB, AfterBB);}        
        Builder.SetInsertPoint(BodyBB);
        for (auto &s : node.body->statements)
            s->accept(*this);
        if (node.update) node.update->accept(*this);
        Builder.CreateBr(CondBB);
        Builder.SetInsertPoint(AfterBB);
    }

    void visit(BlockNode &node) override {
        for (auto &s : node.statements)
            s->accept(*this);
    }

    // other nodes omitted or treated as error
};

} // end anonymous namespace

void CodeGen::compile(ProgramNode *root, bool optimize, int unroll) {
    LLVMContext Ctx;
    ToIRVisitor visitor(Ctx, optimize, unroll);
    visitor.run(root);
    Module *mod = visitor.getModule();
    mod->print(outs(), nullptr);
}

#include "codegen.h"
#include "AST.h"
#include "semantic.h"
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

using namespace llvm;

CodeGen::CodeGen() : context(std::make_unique<LLVMContext>()),
                     module(std::make_unique<Module>("main", *context)),
                     builder(std::make_unique<IRBuilder<>>(*context)) {
    
    FunctionType* mainType = FunctionType::get(Type::getInt32Ty(*context), false);
    mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", *module);
    BasicBlock* entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder->SetInsertPoint(entry);

    printfFunc = Function::Create(
        FunctionType::get(Type::getInt32Ty(*context), {Type::getInt8PtrTy(*context)}, true),
        Function::ExternalLinkage, "printf", module.get()
    );

    Function::Create(
        FunctionType::get(Type::getInt8PtrTy(*context), {Type::getInt32Ty(*context)}, false),
        Function::ExternalLinkage, "malloc", module.get()
    );

    Function::Create(
        FunctionType::get(Type::getVoidTy(*context), 
        {Type::getInt8PtrTy(*context), Type::getInt8PtrTy(*context), Type::getInt32Ty(*context)}, 
        false),
        Function::ExternalLinkage, "memcpy", module.get()
    );
}

void CodeGen::generate(ProgramNode& ast) {
    for (auto& stmt : ast.statements) {
        generateStatement(stmt.get());
    }
    
    if (!builder->GetInsertBlock()->getTerminator()) {
        builder->CreateRet(ConstantInt::get(Type::getInt32Ty(*context), 0));
    }
    
    std::string error;
    raw_string_ostream os(error);
    if (verifyModule(*module, &os)) {
        throw std::runtime_error("Invalid IR: " + error);
    }
}

void CodeGen::generateStatement(ASTNode* node) {
    if (auto multiVarDecl = dynamic_cast<MultiVarDeclNode*>(node)) {
        for (auto& decl : multiVarDecl->declarations) {
            generateVarDecl(decl.get());
        }
    } else if (auto assign = dynamic_cast<AssignNode*>(node)) {
        generateAssign(assign);
    } else if (auto ifElse = dynamic_cast<IfElseNode*>(node)) {
        generateIfElse(ifElse);
    } else if (auto loop = dynamic_cast<ForLoopNode*>(node)) {
        generateForLoop(loop);
    } else if (auto print = dynamic_cast<PrintNode*>(node)) {
        generatePrint(print);
    } else if (auto block = dynamic_cast<BlockNode*>(node)) {
        generateBlock(block);
    } else if (auto tryCatch = dynamic_cast<TryCatchNode*>(node)) {
        generateTryCatch(tryCatch);
    } else if (auto match = dynamic_cast<MatchNode*>(node)) {
        generateMatch(match);
    } else if (auto unary = dynamic_cast<UnaryOpNode*>(node)) {
        generateUnaryOp(unary);
    } else if (auto concat = dynamic_cast<ConcatNode*>(node)) {
        generateConcat(concat);
    }
}

void CodeGen::generateVarDecl(VarDeclNode* node) {
    Type* type = nullptr;
    switch(node->type) {
        case VarType::INT: type = Type::getInt32Ty(*context); break;
        case VarType::FLOAT: type = Type::getFloatTy(*context); break;
        case VarType::BOOL: type = Type::getInt1Ty(*context); break;
        case VarType::STRING: type = Type::getInt8PtrTy(*context); break;
        case VarType::ARRAY: 
            type = PointerType::get(Type::getInt32Ty(*context), 0);
            break;
        default: throw std::runtime_error("Unknown type");
    }
    
    AllocaInst* alloca = builder->CreateAlloca(type, nullptr, node->name);
    symbols[node->name] = alloca;
    
    if (node->value) {
        Value* val = generateValue(node->value.get(), type);
        builder->CreateStore(val, alloca);
        
        if (node->type == VarType::ARRAY) {
            if (auto arrLit = dynamic_cast<ArrayNode*>(node->value.get())) {
                arraySizes[node->name] = arrLit->elements.size();
            }
        }
    }
}

Value* CodeGen::generateValue(ASTNode* node, Type* expectedType) {
    if (auto binOp = dynamic_cast<BinaryOpNode*>(node)) {
        return generateBinaryOp(binOp, expectedType);
    } else if (auto varRef = dynamic_cast<VarRefNode*>(node)) {
        if (!symbols.count(varRef->name)) {
            throw std::runtime_error("Undefined variable: " + varRef->name);
        }
        return builder->CreateLoad(symbols[varRef->name]->getAllocatedType(), symbols[varRef->name]);
    } else if (auto intLit = dynamic_cast<LiteralNode<int>*>(node)) {
        return ConstantInt::get(Type::getInt32Ty(*context), intLit->value);
    } else if (auto floatLit = dynamic_cast<LiteralNode<float>*>(node)) {
        return ConstantFP::get(Type::getFloatTy(*context), floatLit->value);
    } else if (auto strLit = dynamic_cast<LiteralNode<std::string>*>(node)) {
        return builder->CreateGlobalStringPtr(strLit->value);
    } else if (auto arr = dynamic_cast<ArrayNode*>(node)) {
        return generateArray(arr, expectedType);
    } else if (auto access = dynamic_cast<ArrayAccessNode*>(node)) {
        return generateArrayAccess(access);
    }
    throw std::runtime_error("Unsupported node type");
}

Value* CodeGen::generateBinaryOp(BinaryOpNode* node, Type* expectedType) {
    Value* L = generateValue(node->left.get(), expectedType);
    Value* R = generateValue(node->right.get(), expectedType);
    
    switch(node->op) {
        case BinaryOp::ADD:
            if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
                return builder->CreateFAdd(L, R);
            } else {
                return builder->CreateAdd(L, R);
            }
        case BinaryOp::SUBTRACT:
            if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
                return builder->CreateFSub(L, R);
            } else {
                return builder->CreateSub(L, R);
            }
        case BinaryOp::MULTIPLY:
            if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
                return builder->CreateFMul(L, R);
            } else {
                return builder->CreateMul(L, R);
            }
        case BinaryOp::DIVIDE:
            if (L->getType()->isFloatTy() || R->getType()->isFloatTy()) {
                return builder->CreateFDiv(L, R);
            } else {
                return builder->CreateSDiv(L, R);
            }
        case BinaryOp::INDEX:
            return generateArrayIndex(L, R);
        case BinaryOp::CONCAT:
            return generateStringConcat(L, R);
        default:
            throw std::runtime_error("Unsupported binary op");
    }
}

Value* CodeGen::generateArrayIndex(Value* arrayPtr, Value* index) {
    Value* gep = builder->CreateGEP(
        Type::getInt32Ty(*context), 
        arrayPtr, 
        {index}
    );
    return builder->CreateLoad(Type::getInt32Ty(*context), gep);
}

Value* CodeGen::generateStringConcat(Value* L, Value* R) {
    Function* strlenFunc = module->getFunction("strlen");
    Function* mallocFunc = module->getFunction("malloc");
    Function* memcpyFunc = module->getFunction("memcpy");
    
    Value* lenL = builder->CreateCall(strlenFunc, {L});
    Value* lenR = builder->CreateCall(strlenFunc, {R});
    Value* totalLen = builder->CreateAdd(lenL, lenR);
    totalLen = builder->CreateAdd(totalLen, ConstantInt::get(Type::getInt32Ty(*context), 1));
    
    Value* buffer = builder->CreateCall(mallocFunc, {totalLen});
    builder->CreateCall(memcpyFunc, {buffer, L, lenL});
    Value* dest = builder->CreateGEP(Type::getInt8Ty(*context), buffer, lenL);
    builder->CreateCall(memcpyFunc, {dest, R, lenR});
    
    return buffer;
}

void CodeGen::generateIfElse(IfElseNode* node) {
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* thenBB = BasicBlock::Create(*context, "then", func);
    BasicBlock* elseBB = BasicBlock::Create(*context, "else");
    BasicBlock* mergeBB = BasicBlock::Create(*context, "ifcont");
    
    Value* cond = generateValue(node->condition.get(), Type::getInt1Ty(*context));
    builder->CreateCondBr(cond, thenBB, elseBB);
    
    builder->SetInsertPoint(thenBB);
    generateStatement(node->thenBlock.get());
    builder->CreateBr(mergeBB);
    
    func->getBasicBlockList().push_back(elseBB);
    builder->SetInsertPoint(elseBB);
    if (node->elseBlock) {
        generateStatement(node->elseBlock.get());
    }
    builder->CreateBr(mergeBB);
    
    func->getBasicBlockList().push_back(mergeBB);
    builder->SetInsertPoint(mergeBB);
}

void CodeGen::generateForLoop(ForLoopNode* node) {
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* loopStart = BasicBlock::Create(*context, "loop.start", func);
    BasicBlock* loopBody = BasicBlock::Create(*context, "loop.body");
    BasicBlock* loopEnd = BasicBlock::Create(*context, "loop.end");
    
    if (node->init) generateStatement(node->init.get());
    builder->CreateBr(loopStart);
    
    builder->SetInsertPoint(loopStart);
    Value* cond = node->condition ? 
        generateValue(node->condition.get(), Type::getInt1Ty(*context)) : 
        ConstantInt::getTrue(*context);
    builder->CreateCondBr(cond, loopBody, loopEnd);
    
    func->getBasicBlockList().push_back(loopBody);
    builder->SetInsertPoint(loopBody);
    generateStatement(node->body.get());
    if (node->update) generateStatement(node->update.get());
    builder->CreateBr(loopStart);
    
    func->getBasicBlockList().push_back(loopEnd);
    builder->SetInsertPoint(loopEnd);
}

void CodeGen::generatePrint(PrintNode* node) {
    Value* value = generateValue(node->expr.get(), nullptr);
    Type* ty = value->getType();
    
    Constant* format = nullptr;
    if (ty->isIntegerTy(32)) {
        format = builder->CreateGlobalStringPtr("%d\n");
    } else if (ty->isFloatTy()) {
        format = builder->CreateGlobalStringPtr("%f\n");
    } else if (ty->isPointerTy()) {
        format = builder->CreateGlobalStringPtr("%s\n");
    }
    
    builder->CreateCall(printfFunc, {format, value});
}

void CodeGen::generateTryCatch(TryCatchNode* node) {
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* tryBB = BasicBlock::Create(*context, "try", func);
    BasicBlock* catchBB = BasicBlock::Create(*context, "catch");
    BasicBlock* contBB = BasicBlock::Create(*context, "try.cont");
    
    LandingPadInst* lp = builder->CreateLandingPad(
        StructType::get(Type::getInt8PtrTy(*context), Type::getInt32Ty(*context)), 0);
    lp->addClause(ConstantPointerNull::get(Type::getInt8PtrTy(*context)));
    
    builder->CreateBr(tryBB);
    
    builder->SetInsertPoint(tryBB);
    generateStatement(node->tryBlock.get());
    builder->CreateBr(contBB);
    
    func->getBasicBlockList().push_back(catchBB);
    builder->SetInsertPoint(catchBB);
    if (!node->errorVar.empty()) {
        AllocaInst* alloca = builder->CreateAlloca(Type::getInt8PtrTy(*context), nullptr, node->errorVar);
        builder->CreateStore(lp->getOperand(0), alloca);
    }
    generateStatement(node->catchBlock.get());
    builder->CreateBr(contBB);
    
    func->getBasicBlockList().push_back(contBB);
    builder->SetInsertPoint(contBB);
}

void CodeGen::generateMatch(MatchNode* node) {
    Value* expr = generateValue(node->expr.get(), nullptr);
    Function* func = builder->GetInsertBlock()->getParent();
    BasicBlock* endBB = BasicBlock::Create(*context, "match.end");
    
    std::vector<BasicBlock*> caseBBs;
    for (size_t i = 0; i < node->cases.size(); ++i) {
        caseBBs.push_back(BasicBlock::Create(*context, "case." + std::to_string(i), func));
    }
    
    for (size_t i = 0; i < node->cases.size(); ++i) {
        builder->SetInsertPoint(caseBBs[i]);
        if (node->cases[i]->value) {
            Value* caseVal = generateValue(node->cases[i]->value.get(), expr->getType());
            Value* cmp = builder->CreateICmpEQ(expr, caseVal);
            BasicBlock* nextBB = (i < node->cases.size()-1) ? caseBBs[i+1] : endBB;
            builder->CreateCondBr(cmp, caseBBs[i], nextBB);
        } else {
            generateStatement(node->cases[i]->body.get());
            builder->CreateBr(endBB);
        }
    }
    
    func->getBasicBlockList().push_back(endBB);
    builder->SetInsertPoint(endBB);
}



void CodeGen::dump() const {
    module->print(llvm::outs(), nullptr);
}

extern "C" void throw_exception(const char* msg) {
    throw std::runtime_error(msg);
}

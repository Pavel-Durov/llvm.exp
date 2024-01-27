#ifndef EvaLLVM_h
#define EvaLLVM_h

#include "iostream"
#include <regex>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "./Environment.h"
#include "parser/EvaParser.h"

using syntax::EvaParser;

using Env = std::shared_ptr<Environment>;
class EvaLLVM {

public:
  EvaLLVM() : parser(std::make_unique<EvaParser>()) {
    moduleInit();
    setupExternFuncs();
    setupGlobalEnvironment();
  }

  void exec(const std::string &program) {
    auto ast = parser->parse("(begin " + program + ")");
    compile(ast);
    // print module to the output stream
    module->print(llvm::outs(), nullptr);
    // save to file
    saveModuleToFile("./out.ll");
  }

private:
  void compile(const Exp &ast) {
    // create main function
    createFunction("main",
                   llvm::FunctionType::get(builder->getInt32Ty(), false),
                   GlobalEnv);
    gen(ast, GlobalEnv);
    builder->CreateRet(builder->getInt32(0));
  }

  // main compile loop
  llvm::Value *gen(const Exp &exp, Env env) {
    switch (exp.type) {
    case ExpType::NUMBER:
      return builder->getInt32(exp.number);
    case ExpType::STRING: {
      auto re = std::regex("\\\\n");
      auto str = std::regex_replace(exp.string, re, "\n");
      return builder->CreateGlobalStringPtr(str);
    }
    case ExpType::SYMBOL:
      // Boolean
      if (exp.string == "true" || exp.string == "false") {
        return builder->getInt1(exp.string == "true" ? true : false);
      } else {
        auto varName = exp.string;
        llvm::Value *value = env->lookup(varName);
        // Variables

        // Local variables
        if (auto localVar = llvm::dyn_cast<llvm::AllocaInst>(value)) {
          return builder->CreateLoad(localVar->getAllocatedType(),
                                     localVar, varName.c_str());
        }

        // Global variables
        else if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
          return builder->CreateLoad(globalVar->getInitializer()->getType(),
                                     globalVar, varName.c_str());
        }
      }

      return builder->getInt32(0);

    case ExpType::LIST:
      auto tag = exp.list[0];
      if (tag.type == ExpType::SYMBOL) {
        auto op = tag.string;

        // Variables declaration: (var x (+ y 10))
        // Typed: (var (x number) 42)
        // Note: locals are allocated on the stack
        if (op == "var") {
          auto varNameDecl = exp.list[1];
          auto varName = extractVarName(varNameDecl);
          // initializer
          auto init = gen(exp.list[2], env);
          // type
          auto varTy = extractVarType(varNameDecl);
          // variable
          auto varBinding = allocVar(varName, varTy, env);
          // set value
          return builder->CreateStore(init, varBinding);
        } else if (op == "begin") {

          llvm::Value *blockRes;
          // Compile each expression in the block, last expression is the
          // result.
          for (auto i = 1; i < exp.list.size(); i++) {
            blockRes = gen(exp.list[i], env);
          }
          return blockRes;
        } else if (op == "printf") {
          auto printfFn = module->getFunction("printf");
          std::vector<llvm::Value *> args{};
          for (auto i = 1; i < exp.list.size(); i++) {
            args.push_back(
                gen(exp.list[i], env)); // TODO: add local block env support
          }
          return builder->CreateCall(printfFn, args);
        }
      }
    }
    // Unreachable
    return builder->getInt32(0);
  }
  // Extracts var or parameter name considering the type annotation
  // x -> x
  // (x number) -> x
  std::string extractVarName(const Exp &exp) {
    return exp.type == ExpType::LIST ? exp.list[0].string : exp.string;
  }

  // Extract var or paramter type with i32 as default
  // x -> i32
  // (x number) -> number
  llvm::Type *extractVarType(const Exp &exp) {
    return exp.type == ExpType::LIST ? getTypeFromString(exp.list[1].string)
                                     : builder->getInt32Ty();
  }

  llvm::Type *getTypeFromString(const std::string &type_) {
    if (type_ == "number") {
      return builder->getInt32Ty();
    }
    if (type_ == "string") {
      return builder->getInt8Ty()->getPointerTo();
    }

    return builder->getInt32Ty();
  }
  llvm::Value *allocVar(const std::string &name, llvm::Type *type_, Env env) {
    varBuilder->SetInsertPoint(&fn->getEntryBlock());

    auto var = varBuilder->CreateAlloca(type_, 0, name.c_str());
    env->define(name, var);
    return var;
  }

  llvm::GlobalVariable *createGlobalVar(const std::string &name,
                                        llvm::Constant *init) {
    module->getOrInsertGlobal(name, init->getType());
    auto variable = module->getNamedGlobal(name);
    variable->setAlignment(llvm::MaybeAlign(4));
    variable->setConstant(false);
    variable->setInitializer(init);
    return variable;
  }
  void setupExternFuncs() {
    auto bytePrtType = builder->getInt8Ty()->getPointerTo();
    // standard library printf function
    module->getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(builder->getInt32Ty(), bytePrtType, true));
  }

  llvm::Function *createFunction(const std::string &fName,
                                 llvm::FunctionType *fType, Env env) {
    auto fn = module->getFunction(fName);
    if (fn == nullptr) {
      fn = createFucntionProto(fName, fType, env);
    }
    createFunctionBlock(fn);
    return fn;
  }

  void createFunctionBlock(llvm::Function *fn) {
    auto entry = createBB("entry", fn);
    builder->SetInsertPoint(entry);
  }

  llvm::BasicBlock *createBB(const std::string &name,
                             llvm::Function *fn = nullptr) {
    return llvm::BasicBlock::Create(*ctx, name, fn);
  }

  llvm::Function *createFucntionProto(const std::string &fName,
                                      llvm::FunctionType *fType, Env env) {
    auto fn = llvm::Function::Create(
        fType, llvm::Function::ExternalLinkage, /* make function visible */
        fName, *module);
    verifyFunction(*fn);
    env->define(fName, fn);
    return fn;
  }

  void saveModuleToFile(const std::string &filename) {
    std::error_code errorCode;
    llvm::raw_fd_ostream outLL(filename, errorCode);
    module->print(outLL, nullptr);
  }

  void moduleInit() {
    ctx = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    varBuilder = std::make_unique<llvm::IRBuilder<>>(*ctx);
  }
  void setupGlobalEnvironment() {
    std::map<std::string, llvm::Value *> globalObj{
        {"VERSION", builder->getInt32(42)}};
    std::map<std::string, llvm::Value *> globalRec{};

    for (auto &obj : globalObj) {
      globalRec[obj.first] =
          createGlobalVar(obj.first, (llvm::Constant *)obj.second);
    }
    GlobalEnv = std::make_shared<Environment>(globalRec, nullptr);
  }
  // Parser
  std::unique_ptr<EvaParser> parser;

  // Global environment
  std::shared_ptr<Environment> GlobalEnv;

  // Currently compiling function
  llvm::Function *fn;

  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> module;

  // Variables declaration builder
  //
  std::unique_ptr<llvm::IRBuilder<>> varBuilder;
  //
  /**
   * IR builder
   * Used for creating instruction and inserting them into the basic block:
   * either at the end, or at a specific location.
   */
  std::unique_ptr<llvm::IRBuilder<>> builder;
};
#endif // EvaLLVM_h

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
        auto values = env->lookup(varName);
        // Variables
        // return module->getNamedGlobal(exp.string)->getInitializer();
        if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(values)) {
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
        if (op == "var") {
          auto varName = exp.list[1].string;
          auto init = gen(exp.list[2], env);
          return createGlobalVar(varName, (llvm::Constant *)init);
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
  std::unique_ptr<llvm::IRBuilder<>> builder;
};
#endif // EvaLLVM_h

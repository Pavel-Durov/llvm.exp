#ifndef Eva_h
#define Eva_h

#include "parser/EvaGrammar.h"
#include "iostream"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <string>

using syntax::EvaGrammar;

class EvaLLVM {

public:
  EvaLLVM() : parser(std::make_unique<EvaGrammar>()) {
    moduleInit();
    setupExternFuncs();
  }

  void exec(const std::string &program) {
    auto ast = parser->parse(program);
    compile(ast);
    // print module to the output stream
    module->print(llvm::outs(), nullptr);
    // save to file
    saveModuleToFile("./out.ll");
  }

private:
  void compile(const Exp &ast) {
    // create main function
    fn = createFunction("main",
                        llvm::FunctionType::get(builder->getInt32Ty(), false));

    auto result = gen(ast);
    builder->CreateRet(builder->getInt32(0));
  }

  // main compile loop
  llvm::Value *gen(const Exp &exp) {
    switch (exp.type) {
    case ExpType::NUMBER:
      return builder->getInt32(exp.number);
    case ExpType::STRING:
      return builder->CreateGlobalStringPtr(exp.string);
    case ExpType::SYMBOL:
      // TODO: handle symbols
      return builder->getInt32(0);
    case ExpType::LIST:
      auto tag = exp.list[0];
      if (tag.type == ExpType::SYMBOL) {
        auto op = tag.string;
        if (op == "printf") {
          auto printfFn = module->getFunction("printf");
          std::vector<llvm::Value *> args{};
          for (auto i = 1; i < exp.list.size(); i++) {
            args.push_back(gen(exp.list[i]));
          }
          return builder->CreateCall(printfFn, args);
        }
      }
    }
    // Unreachable
    return builder->getInt32(0);
  }

  void setupExternFuncs() {
    auto bytePrtType = builder->getInt8Ty()->getPointerTo();
    // standard library printf function
    module->getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(builder->getInt32Ty(), bytePrtType, true));
  }

  llvm::Function *createFunction(const std::string &fName,
                                 llvm::FunctionType *fType) {
    auto fn = module->getFunction(fName);
    if (fn == nullptr) {
      fn = createFucntionProto(fName, fType);
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
                                      llvm::FunctionType *fType) {
    auto fn = llvm::Function::Create(
        fType, llvm::Function::ExternalLinkage, /* make function visible */
        fName, *module);
    verifyFunction(*fn);
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
  // Parser
  std::unique_ptr<EvaGrammar> parser;

  // Currently compiling function
  llvm::Function *fn;

  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
};
#endif // Eva_h

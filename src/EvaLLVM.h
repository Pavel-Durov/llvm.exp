#ifndef Eva_h
#define Eva_h

#include "iostream"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <string>

class EvaLLVM {

public:
  EvaLLVM() {
    moduleInit();
    setupExternFuncs();
  }

  void exec(const std::string &program) {
    compile();
    // print module to the output stream
    module->print(llvm::outs(), nullptr);
    // save to file
    saveModuleToFile("./out.ll");
  }

private:
  void compile() {
    fn = createFunction("main",
                        llvm::FunctionType::get(builder->getInt32Ty(), false));

    auto result = gen();
    builder->CreateRet(builder->getInt32(0));
  }

  // main compile loop
  llvm::Value *gen() {
    auto str = builder->CreateGlobalStringPtr("Hello, World!");
    auto printfFn = module->getFunction("printf");
    // args:
    std::vector<llvm::Value *> args{str};
    auto call = builder->CreateCall(printfFn, args);
    return call;
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
  // Currently compiling function
  llvm::Function *fn;

  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
};
#endif // Eva_h

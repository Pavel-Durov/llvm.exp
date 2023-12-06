#ifndef Eva_h
#define Eva_h

#include <string>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class EvaLLVM
{

public:
    EvaLLVM()
    {
        moduleInit();
    }

    void exec(const std::string &program)
    {
        // 1. TODO: Parse the program
        // 2. TODO: Compile to LLVM IR

        // print module to the output stream
        module->print(llvm::outs(), nullptr);
        // save to file
        saveModuleToFile("./out.ll");
    }

private:
    void moduleInit()
    {
        ctx = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }

    void saveModuleToFile(const std::string &filename)
    {
        std::error_code errorCode;
        llvm::raw_fd_ostream outLL(filename, errorCode);
        module->print(outLL, nullptr);
    }

    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
};
#endif // Eva_h
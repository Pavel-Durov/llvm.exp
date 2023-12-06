# include "./EvaLLVM.h"

int main (int argc, char const *argv[]) { 
    std::string program = "42";
    EvaLLVM vm;

    vm.exec(program);

    return 42; 
}

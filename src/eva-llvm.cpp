#include "./EvaLLVM.h"

int main(int argc, char const *argv[]) {
  std::string program = R"(
    (printf "Value %d\n" 42)
  )";
  EvaLLVM vm;

  vm.exec(program);

  return 0;
}

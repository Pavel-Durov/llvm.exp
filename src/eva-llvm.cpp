#include "./EvaLLVM.h"

int main(int argc, char const *argv[]) {
  std::string program = R"(
    (printf "x %d\n" 42)
    (begin 
      (var x "hello")
      (printf "X: %s\n" x))

    (printf "X %d\n" x)
  )";
  EvaLLVM vm;

  vm.exec(program);

  return 0;
}

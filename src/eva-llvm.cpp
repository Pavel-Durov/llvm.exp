#include "./EvaLLVM.h"

int main(int argc, char const *argv[]) {
  std::string program = R"(
    (printf "VERSION %d\n" VERSION)
  )";
  EvaLLVM vm;

  vm.exec(program);

  return 0;
}

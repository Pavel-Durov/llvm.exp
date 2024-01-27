#include "./EvaLLVM.h"

int main(int argc, char const *argv[]) {
  std::string program = R"(
    // (printf "VERSION %d\n" VERSION)
    // (begin 
    //   (var VERSION "hello")
    //   (printf "VERSION %s\n" VERSION))

    (printf "VERSION %d\n" VERSION)
  )";
  EvaLLVM vm;

  vm.exec(program);

  return 0;
}


clang-test-ll:
	clang++ -S -emit-llvm ./src/min.cpp
	clang++ ./min.ll -o ./min
	./min
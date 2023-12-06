
LLVM_CONFIG_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core`
LINK_FLAGS=-L/opt/homebrew/lib

OUT=eva-llvm
EVA_SRC=src/eva-llvm.cpp

build:
	clang++ ${LINK_FLAGS} ${LLVM_CONFIG_FLAGS} ${EVA_SRC} -o ${OUT}

llvm-flags:
	@echo $(LLVM_CONFIG_FLAGS)

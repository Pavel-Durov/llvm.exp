SHELL := /bin/bash

LLVM_CONFIG_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core`
LINK_FLAGS=-L/opt/homebrew/lib

OUT=./eva-llvm
OUT_LL=./out.ll
EVA_SRC=src/eva-llvm.cpp

clean:
	rm -f ${OUT} ${OUT_LL}
	
compile: clean
	clang++ ${LINK_FLAGS} ${LLVM_CONFIG_FLAGS} ${EVA_SRC} -o ${OUT} -fexceptions

compile-run: compile
	${OUT}

parser-gen:
	syntax-cli -g ./src/parser/EvaGrammar.bnf -m LALR1 -o ./src/parser/EvaParser.h

run: parser-gen compile-run
	lli ${OUT_LL}

llvm-flags:
	@echo $(LLVM_CONFIG_FLAGS)

lint:
	clang-format -i ./src/*.cpp ./src/*.h

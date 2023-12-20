SHELL := /bin/bash

LLVM_CONFIG_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core`
LINK_FLAGS=-L/opt/homebrew/lib

OUT=./eva-llvm
OUT_LL=./out.ll
EVA_SRC=src/eva-llvm.cpp

compile:
	clang++ ${LINK_FLAGS} ${LLVM_CONFIG_FLAGS} ${EVA_SRC} -o ${OUT}

compile-run: compile
	${OUT}

run: compile-run
	lli ${OUT_LL}

llvm-flags:
	@echo $(LLVM_CONFIG_FLAGS)

lint:
	clang-format -i ./src/*.cpp ./src/*.h

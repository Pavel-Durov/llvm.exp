# LLVM exploration tools and code examples

[LLVM tools](./docs/TOOLS.md)

## LLVM installation

```shell
sudo port install llvm-17
port select --list llvm # see list
sudo port select --set llvm mp-llvm-17 # set active llvm
npm install -g syntax-cli # syntax-cli utility
```

## LLVM Components

`Module` - General container for functions, global variables, constants etc

`Context` - Container of modules and metadata

`IR Builder` - Library for generating IR instructions.

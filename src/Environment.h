#ifndef Environment_h
#define Environment_h

#include <map>
#include <memory>
#include <string>

#include "./Logger.h"
#include "llvm/IR/Value.h"

class Environment : public std::enable_shared_from_this<Environment> {
public:
  Environment(std::map<std::string, llvm::Value *> record,
              std::shared_ptr<Environment> parent)
      : record_(record), parent_(parent) {}

  llvm::Value *define(std::string name, llvm::Value *value) {
    record_[name] = value;
    return value;
  }

  llvm::Value *lookup(std::string name) { return resolve(name)->record_[name]; }

private:
  std::shared_ptr<Environment> resolve(std::string name) {
    if (record_.find(name) != record_.end()) {
      return shared_from_this();
    }
    if (parent_ == nullptr) {
      DIE << "Variable \"" << name << "\" is not defined.";
    }
    return parent_->resolve(name);
  }
  // bindings
  std::map<std::string, llvm::Value *> record_;
  // link to parent environment
  std::shared_ptr<Environment> parent_;
};
#endif // Environment_h

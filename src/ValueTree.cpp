//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

namespace llvm {

namespace anderson {

ValueTree::ValueTree(const llvm::Module &module) noexcept
  : _module(module),
    _roots(),
    _allocaRoots(),
    _globalRoots()
{
  for (const auto &globalVariable : module.globals()) {
    _roots[&globalVariable] = std::make_unique<ValueTreeNode>(&globalVariable);
  }

  for (const auto &func : module) {
    _roots[&func] = std::make_unique<ValueTreeNode>(&func);
    for (const auto &arg : func.args()) {
      _roots[&arg] = std::make_unique<ValueTreeNode>(&arg);
    }
    for (const auto &bb : func) {
      for (const auto &inst : bb) {
        _roots[&inst] = std::make_unique<ValueTreeNode>(&inst);
      }
    }
  }
}

} // namespace anderson

} // namespace llvm

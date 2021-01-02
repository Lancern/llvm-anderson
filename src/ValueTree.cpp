//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

namespace llvm {

namespace anderson {

template <typename ...Args>
static std::unique_ptr<ValueTreeNode> CreateNode(size_t &numPointees, size_t &numPointers, Args&&... args) noexcept {
  auto node = std::make_unique<ValueTreeNode>(std::forward<Args>(args)...);
  numPointees += node->GetNumPointees();
  numPointers += node->GetNumPointers();
  return node;
}

ValueTree::ValueTree(const llvm::Module &module) noexcept
  : _module(module),
    _roots(),
    _allocaMemoryRoots(),
    _globalMemoryRoots(),
    _argumentMemoryRoots(),
    _returnValueRoots(),
    _numPointees(0),
    _numPointers(0)
{
  for (const auto &globalVariable : module.globals()) {
    _roots[&globalVariable] = CreateNode(_numPointees, _numPointers, &globalVariable);
    _globalMemoryRoots[&globalVariable] = CreateNode(_numPointees, _numPointers, GlobalMemoryValueTag { }, &globalVariable);
  }

  for (const auto &func : module) {
    _roots[&func] = CreateNode(_numPointees, _numPointers, &func);
    _returnValueRoots[&func] = CreateNode(_numPointees, _numPointers, FunctionReturnValueTag { }, &func);
    for (const auto &arg : func.args()) {
      _roots[&arg] = CreateNode(_numPointees, _numPointers, ArgumentMemoryValueTag { }, &arg);
    }
    for (const auto &bb : func) {
      for (const auto &inst : bb) {
        _roots[&inst] = CreateNode(_numPointees, _numPointers, &inst);
        auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(&inst);
        if (allocaInst) {
          _allocaMemoryRoots[allocaInst] = CreateNode(_numPointees, _numPointers, StackMemoryValueTag { }, allocaInst);
        }
      }
    }
  }
}

} // namespace anderson

} // namespace llvm

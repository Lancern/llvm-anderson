//
// Created by Sirui Mu on 2021/1/1.
//

#include "PointsToSolver.h"

namespace llvm {

namespace anderson {

void PointsToSolver::Solve() noexcept {
  AddTrivialPointerAssignments();

  // TODO: Implement PointsToSolver::Solve
}

void PointsToSolver::AddTrivialPointerAssignments() const noexcept {
  // Add points-to constraints from global variables to corresponding global memory values.
  for (const auto &globalVariable : _module.globals()) {
    auto globalVariableNode = _valueTree->GetValueNode(&globalVariable);
    auto globalVariableMemoryNode = _valueTree->GetGlobalMemoryNode(&globalVariable);
    assert(globalVariableNode->isPointer());
    globalVariableNode->pointer()->AssignedAddressOf(globalVariableMemoryNode->pointee());
  }

  // Add points-to constraints from exported function arguments to corresponding argument memory values.
  for (const auto &function : _module) {
    if (!llvm::GlobalValue::isExternalLinkage(function.getLinkage())) {
      continue;
    }
    for (const auto &arg : function.args()) {
      if (!arg.getType()->isPointerTy()) {
        continue;
      }
      auto argNode = _valueTree->GetValueNode(&arg);
      auto argMemoryNode = _valueTree->GetArgumentMemoryNode(&arg);
      assert(argNode->isPointer());
      argNode->pointer()->AssignedAddressOf(argMemoryNode->pointee());
    }
  }

  // Add points-to constraints from `alloca` pointers to corresponding stack memory values.
  for (const auto &function : _module) {
    for (const auto &bb : function) {
      for (const auto &inst : bb) {
        auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(&inst);
        if (!allocaInst) {
          continue;
        }

        auto ptrValue = static_cast<const llvm::Value *>(&inst);
        auto ptrNode = _valueTree->GetValueNode(ptrValue);
        auto stackMemoryNode = _valueTree->GetAllocaMemoryNode(allocaInst);
        assert(ptrNode->isPointer());

        ptrNode->pointer()->AssignedAddressOf(stackMemoryNode->pointee());
      }
    }
  }
}

} // namespace anderson

} // namespace llvm

//
// Created by Sirui Mu on 2021/1/1.
//

#ifndef LLVM_ANDERSON_SRC_POINTS_TO_SOLVER_H
#define LLVM_ANDERSON_SRC_POINTS_TO_SOLVER_H

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <memory>
#include <utility>

#include <llvm/IR/Module.h>

namespace llvm {

namespace anderson {

class PointsToSolver {
public:
  explicit PointsToSolver(const llvm::Module &module) noexcept
    : _module(module),
      _valueTree(std::make_unique<ValueTree>(module))
  { }

  ValueTree* GetValueTree() const noexcept {
    return _valueTree.get();
  }

  std::unique_ptr<ValueTree> TakeValueTree() noexcept {
    return std::move(_valueTree);
  }

  void Solve() noexcept;

private:
  static bool RelaxNode(ValueTreeNode &node) noexcept;

  static bool RelaxAssignedPointee(Pointer *pointer, const PointerAssignedPointee &edge) noexcept;

  static bool RelaxAssignedElementPtr(Pointer *pointer, const PointerAssignedElementPtr &edge) noexcept;

  static bool RelaxPointeeAssigned(Pointer *pointer, const PointeeAssignedPointer &edge) noexcept;

  const llvm::Module &_module;
  std::unique_ptr<ValueTree> _valueTree;

  void AddTrivialPointerAssignments() const noexcept;

  void RelaxPointsToConstraints() const noexcept;
};

} // namespace anderson

} // namespace llvm

#endif // LLVM_ANDERSON_SRC_POINTS_TO_SOLVER_H

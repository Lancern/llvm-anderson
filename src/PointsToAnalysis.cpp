//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

#include <cassert>

namespace p2a {

PointeeSet* PointsToAnalysis::GetPointsToSet(const llvm::Value *pointer) noexcept {
  return const_cast<PointeeSet *>(const_cast<const PointsToAnalysis *>(this)->GetPointsToSet(pointer));
}

const PointeeSet* PointsToAnalysis::GetPointsToSet(const llvm::Value *pointer) const noexcept {
  assert(pointer->getType()->isPointerTy() && "The given value is not a pointer");
  auto it = _pointeeSets.find(pointer);
  if (it != _pointeeSets.end()) {
    return &it->second;
  } else {
    return nullptr;
  }
}

PointsToAnalysis::PointsToAnalysis(char &id) noexcept
  : llvm::ModulePass { id }
{ }

} // namespace p2a

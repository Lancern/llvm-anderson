//
// Created by msr on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <cassert>

namespace llvm {

namespace anderson {

bool Pointer::classof(const Pointee *obj) noexcept {
  return obj->isPointer();
}

Pointer::Pointer(ValueTreeNode &node) noexcept
  : Pointee { node },
    _pointsTo(),
    _pointsToPointeesOf()
{ }

void Pointer::AddPointsTo(Pointee *pointee) noexcept {
  assert(pointee && "pointee cannot be null");
  _pointsTo.emplace(pointee);
}

void Pointer::AddPointsToPointeesOf(Pointer *anotherPointer) noexcept {
  assert(anotherPointer && "anotherPointer cannot be null");
  _pointsToPointeesOf.emplace(anotherPointer);
}

} // namespace anderson

} // namespace llvm

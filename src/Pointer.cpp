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
    _pointsToPointeesOf(),
    _pointsToPointeesOfPointeesOf(),
    _pointeePointsToPointeesOf()
{ }

void Pointer::AddPointsTo(Pointee *pointee) noexcept {
  assert(pointee && "pointee cannot be null");
  _pointsTo.emplace(pointee);
}

void Pointer::AddPointsToPointeesOf(Pointer *pointer) noexcept {
  assert(pointer && "pointer cannot be null");
  _pointsToPointeesOf.emplace(pointer);
}

void Pointer::AddPointsToPointeesOfPointeesOf(Pointer *pointer) noexcept {
  assert(pointer && "pointer cannot be null");
  _pointsToPointeesOfPointeesOf.emplace(pointer);
}

void Pointer::AddPointeePointsToPointeesOf(Pointer *pointer) noexcept {
  assert(pointer && "pointer cannot be null");
  _pointeePointsToPointeesOf.emplace(pointer);
}

PointeeSet& Pointer::GetPointeeSet() noexcept {
  return _pointees;
}

const PointeeSet& Pointer::GetPointeeSet() const noexcept {
  return _pointees;
}

} // namespace anderson

} // namespace llvm

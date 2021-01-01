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
    _assignedElementPtr(),
    _assignedPointee(),
    _pointeeAssigned(),
    _pointees()
{ }

void Pointer::AssignedElementPtr(Pointer *pointer, std::vector<PointerIndex> indexSequence) noexcept {
  assert(pointer && "pointer cannot be null");
  _assignedElementPtr.emplace(pointer, std::move(indexSequence));
}

void Pointer::AssignedPointee(Pointer *pointer) noexcept {
  assert(pointer && "pointer cannot be null");
  _assignedPointee.emplace(pointer);
}

void Pointer::PointeeAssigned(Pointer *pointer) noexcept {
  assert(pointer && "pointer cannot be null");
  _pointeeAssigned.emplace(pointer);
}

PointeeSet& Pointer::GetPointeeSet() noexcept {
  return _pointees;
}

const PointeeSet& Pointer::GetPointeeSet() const noexcept {
  return _pointees;
}

} // namespace anderson

} // namespace llvm

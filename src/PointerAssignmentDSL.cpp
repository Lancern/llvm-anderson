//
// Created by Sirui Mu on 2020/12/27.
//

#include "p2a/PointerAssignmentDSL.h"

#include <cassert>
#include <utility>

#include <llvm/IR/Type.h>

namespace p2a {

PointerAssignment::~PointerAssignment() noexcept = default;

PointerAssignmentKind PointerAssignment::kind() const noexcept {
  return _kind;
}

PointerAssignment::PointerAssignment(
    PointerAssignmentKind kind, const llvm::Value *lhs, const llvm::Value *rhs) noexcept
    : _kind(kind), _lhs(lhs), _rhs(rhs)
{ }

const llvm::Value* PointerAssignment::lhs() const noexcept {
  return _lhs;
}

const llvm::Value* PointerAssignment::rhs() const noexcept {
  return _rhs;
}

PointerAssignedAddress::PointerAssignedAddress(const llvm::Value *targetPointer, const llvm::Value *value) noexcept
  : PointerAssignment { PointerAssignmentKind::PointerAssignedAddress, targetPointer, value }
{
  assert(targetPointer->getType()->isPointerTy() && "targetPointer is not a pointer");
  assert(targetPointer->getType()->getPointerElementType() == value->getType() && "Incompatible pointer assignment");
}

const llvm::Value* PointerAssignedAddress::targetPointer() const noexcept {
  return lhs();
}

const llvm::Value* PointerAssignedAddress::value() const noexcept {
  return rhs();
}

bool PointerAssignedAddress::classof(const PointerAssignment *dsl) noexcept {
  return dsl->kind() == PointerAssignmentKind::PointerAssignedAddress;
}

constexpr static const size_t UnknownPointerDerefIndex = static_cast<size_t>(-1);

PointerDerefIndex::PointerDerefIndex() noexcept
  : PointerDerefIndex { UnknownPointerDerefIndex }
{ }

PointerDerefIndex::PointerDerefIndex(size_t index) noexcept
  : _index(index)
{ }

bool PointerDerefIndex::isUnknown() const noexcept {
  return _index == UnknownPointerDerefIndex;
}

bool PointerDerefIndex::isConstant() const noexcept {
  return _index != UnknownPointerDerefIndex;
}

size_t PointerDerefIndex::index() const noexcept {
  assert(isConstant() && "This deref index is not a constant index");
  return _index;
}

PointerAssignedPointerDeref::PointerAssignedPointerDeref(
    const llvm::Value *targetPointer,
    const llvm::Value *sourcePointer,
    std::vector<PointerDerefIndex> indexes) noexcept
    : PointerAssignment {PointerAssignmentKind::PointerAssignedPointerDeref, targetPointer, sourcePointer },
      _indexes(std::move(indexes))
{
  assert(targetPointer->getType()->isPointerTy() && "targetPointer is not a pointer");
  assert(sourcePointer->getType()->isPointerTy() && "sourcePointer is not a pointer");
  // TODO: Maybe more assertion checks here to ensure every index is valid?
}

const llvm::Value* PointerAssignedPointerDeref::targetPointer() const noexcept {
  return lhs();
}

const llvm::Value* PointerAssignedPointerDeref::sourcePointer() const noexcept {
  return rhs();
}

const std::vector<PointerDerefIndex>& PointerAssignedPointerDeref::indexes() const noexcept {
  return _indexes;
}

bool PointerAssignedPointerDeref::classof(const PointerAssignment *dsl) noexcept {
  return dsl->kind() == PointerAssignmentKind::PointerAssignedPointerDeref;
}

PointerDerefAssignedPointer::PointerDerefAssignedPointer(
    const llvm::Value *targetPointerPointer, const llvm::Value *sourcePointer) noexcept
    : PointerAssignment { PointerAssignmentKind::PointerDerefAssignedPointer, targetPointerPointer, sourcePointer }
{
  assert(targetPointerPointer->getType()->isPointerTy() && "targetPointerPointer is not a pointer");
  assert(targetPointerPointer->getType()->getPointerElementType()->isPointerTy() && "targetPointerPointer is not a pointer to a pointer");
  assert(sourcePointer->getType()->isPointerTy() && "sourcePointer is not a pointer");
  assert(targetPointerPointer->getType()->getPointerElementType() == sourcePointer->getType() && "Incompatible pointer assignment");
}

const llvm::Value* PointerDerefAssignedPointer::targetPointerPointer() const noexcept {
  return lhs();
}

const llvm::Value* PointerDerefAssignedPointer::sourcePointer() const noexcept {
  return rhs();
}

bool PointerDerefAssignedPointer::classof(const PointerAssignment *dsl) noexcept {
  return dsl->kind() == PointerAssignmentKind::PointerDerefAssignedPointer;
}

} // namespace p2a

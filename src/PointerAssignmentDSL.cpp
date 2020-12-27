//
// Created by Sirui Mu on 2020/12/27.
//

#include "p2a/PointerAssignmentDSL.h"

#include <cassert>

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

PointerAssignedPointer::PointerAssignedPointer(
    const llvm::Value *targetPointer, const llvm::Value *sourcePointer) noexcept
    : PointerAssignment { PointerAssignmentKind::PointerAssignedPointer, targetPointer, sourcePointer }
{
  assert(targetPointer->getType()->isPointerTy() && "targetPointer is not a pointer");
  assert(sourcePointer->getType()->isPointerTy() && "sourcePointer is not a pointer");
  assert(sourcePointer->getType() == sourcePointer->getType() && "Incompatible pointer assignment");
}

const llvm::Value* PointerAssignedPointer::targetPointer() const noexcept {
  return lhs();
}

const llvm::Value* PointerAssignedPointer::sourcePointer() const noexcept {
  return rhs();
}

bool PointerAssignedPointer::classof(const PointerAssignedPointer *dsl) noexcept {
  return dsl->kind() == PointerAssignmentKind::PointerAssignedPointer;
}

PointerAssignedPointerDeref::PointerAssignedPointerDeref(
    const llvm::Value *targetPointer, const llvm::Value *sourcePointerPointer) noexcept
    : PointerAssignment { PointerAssignmentKind::PointerAssignedPointerDeref, targetPointer, sourcePointerPointer }
{
  assert(targetPointer->getType()->isPointerTy() && "targetPointer is not a pointer");
  assert(sourcePointerPointer->getType()->isPointerTy() && "sourcePointer is not a pointer");
  assert(sourcePointerPointer->getType()->getPointerElementType()->isPointerTy() && "sourcePointer is not a pointer to a pointer");
  assert(targetPointer->getType() == sourcePointerPointer->getType()->getPointerElementType() && "Incompatible pointer assignment");
}

const llvm::Value* PointerAssignedPointerDeref::targetPointer() const noexcept {
  return lhs();
}

const llvm::Value* PointerAssignedPointerDeref::sourcePointerPointer() const noexcept {
  return rhs();
}

bool PointerAssignedPointerDeref::classof(const PointerAssignedPointer *dsl) noexcept {
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

bool PointerDerefAssignedPointer::classof(const PointerAssignedPointer *dsl) noexcept {
  return dsl->kind() == PointerAssignmentKind::PointerDerefAssignedPointer;
}

} // namespace p2a

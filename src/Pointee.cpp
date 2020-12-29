//
// Created by Sirui Mu on 2020/12/29.
//

#include "p2a/PointsToAnalysis.h"

#include <cassert>

#include <llvm/Support/Casting.h>

namespace p2a {

bool Pointee::IsRootValue(const llvm::Value *value) noexcept {
  assert(value && "value cannot be null");

  return llvm::isa<llvm::GlobalObject>(value) ||
      llvm::isa<llvm::AllocaInst>(value);
}

Pointee::Pointee(const llvm::Type* type) noexcept
  : _type(type),
    _value(nullptr),
    _parent(nullptr),
    _offset(InvalidOffset),
    _children()
{
  assert(type && "type cannot be null");

  InitializeChildren();
}

Pointee::Pointee(const llvm::Value *value) noexcept
  : _type(nullptr),
    _value(value),
    _parent(nullptr),
    _offset(InvalidOffset),
    _children()
{
  assert(IsRootValue(value) && "value cannot serve as a valid root pointee object");

  if (llvm::isa<llvm::GlobalObject>(value)) {
    auto globalObject = llvm::cast<llvm::GlobalObject>(value);
    _type = globalObject->getValueType();
    return;
  }

  if (llvm::isa<llvm::AllocaInst>(value)) {
    auto allocaInst = llvm::cast<llvm::AllocaInst>(value);
    _type = allocaInst->getAllocatedType();
    return;
  }

  __builtin_unreachable();
}

Pointee::Pointee(const llvm::GlobalObject *globalObject) noexcept
  : _type(globalObject->getValueType()),
    _value(globalObject),
    _parent(nullptr),
    _offset(InvalidOffset),
    _children()
{
  assert(globalObject && "globalObject cannot be null");

  InitializeChildren();
}

Pointee::Pointee(const llvm::AllocaInst *allocaInst) noexcept
  : _type(allocaInst->getAllocatedType()),
    _value(allocaInst),
    _parent(nullptr),
    _offset(InvalidOffset),
    _children()
{
  assert(allocaInst && "allocaInst cannot be null");

  InitializeChildren();
}

Pointee::Pointee(const llvm::Type *type, const Pointee *parent, size_t offset) noexcept
  : _type(type),
    _value(nullptr),
    _parent(parent),
    _offset(offset),
    _children()
{
  assert(type && "type cannot be null");
  assert(parent && "parent cannot be null");
  assert(offset != InvalidOffset && "Invalid offset");

  InitializeChildren();
}

Pointee::Pointee(Pointee &&) noexcept { // NOLINT(cppcoreguidelines-pro-type-member-init)
  assert(false && "Move constructor of Pointee should not be used");
}

Pointee& Pointee::operator=(Pointee &&) noexcept {
  assert(false && "Move assignment operator of Pointee should not be used");
}

Pointee::~Pointee() noexcept = default;

bool Pointee::isRoot() const noexcept {
  return _parent == nullptr;
}

bool Pointee::isExternal() const noexcept {
  if (!isRoot()) {
    return _parent->isExternal();
  }

  if (!_value) {
    return true;
  }

  auto globalVariable = llvm::dyn_cast<llvm::GlobalVariable>(_value);
  if (!globalVariable) {
    return false;
  }

  return llvm::GlobalValue::isAvailableExternallyLinkage(globalVariable->getLinkage());
}

bool Pointee::isGlobal() const noexcept {
  if (!isRoot()) {
    return _parent->isGlobal();
  }

  if (!_value) {
    return false;
  }

  return llvm::isa<llvm::GlobalVariable>(_value);
}

bool Pointee::isAlloca() const noexcept {
  if (!isRoot()) {
    return _parent->isAlloca();
  }

  if (!_value) {
    return false;
  }

  return llvm::isa<llvm::AllocaInst>(_value);
}

const llvm::Type* Pointee::type() const noexcept {
  return _type;
}

const llvm::GlobalObject* Pointee::globalObject() const noexcept {
  if (!isRoot()) {
    return _parent->globalObject();
  }

  return llvm::cast<llvm::GlobalVariable>(_value);
}

const llvm::AllocaInst* Pointee::allocaInst() const noexcept {
  if (!isAlloca()) {
    return _parent->allocaInst();
  }

  return llvm::cast<llvm::AllocaInst>(_value);
}

const Pointee* Pointee::parent() const noexcept {
  return _parent;
}

size_t Pointee::offset() const noexcept {
  return _offset;
}

const Pointee* Pointee::root() const noexcept {
  if (isRoot()) {
    return this;
  }
  return _parent->root();
}

bool Pointee::isPointer() const noexcept {
  return _type->isPointerTy();
}

bool Pointee::isArray() const noexcept {
  return _type->isArrayTy();
}

bool Pointee::isStruct() const noexcept {
  return _type->isStructTy();
}

bool Pointee::hasChildren() const noexcept {
  return isPointer() || isArray() || isStruct();
}

size_t Pointee::GetNumChildren() const noexcept {
  return _children.size();
}

const Pointee& Pointee::GetChild(size_t i) const noexcept {
  assert(i >= 0 && i < _children.size() && "i is out of range");
  return _children[i];
}

void Pointee::InitializeChildren() noexcept {
  if (isPointer()) {
    _children.emplace_back(_type->getPointerElementType(), this, 0);
    return;
  }

  if (isArray()) {
    _children.reserve(_type->getArrayNumElements());
    for (uint64_t i = 0; i < _type->getArrayNumElements(); ++i) {
      _children.emplace_back(_type->getArrayElementType(), this, static_cast<size_t>(i));
    }
    return;
  }

  if (isStruct()) {
    _children.reserve(_type->getStructNumElements());
    for (uint64_t i = 0; i < _type->getStructNumElements(); ++i) {
      _children.emplace_back(_type->getStructElementType(i), this, static_cast<size_t>(i));
    }
    return;
  }
}

} // namespace p2a

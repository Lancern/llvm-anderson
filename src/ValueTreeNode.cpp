//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <functional>

#include <llvm/Support/Casting.h>

namespace llvm {

namespace anderson {

ValueTreeNode::ValueTreeNode(const llvm::Value *value) noexcept
    : _type(value->getType()),
      _value(value),
      _parent(nullptr),
      _offset(0),
      _children()
{
  assert(value && "value cannot be null");

  Initialize();
}

ValueTreeNode::ValueTreeNode(const llvm::Type *type, ValueTreeNode *parent, size_t offset) noexcept
    : _type(type),
      _value(nullptr),
      _parent(parent),
      _offset(offset),
      _children()
{
  assert(type && "type cannot be null");
  assert(parent && "parent cannot be null");

  Initialize();
}

const llvm::Type* ValueTreeNode::type() const noexcept {
  return _type;
}

const llvm::Value* ValueTreeNode::value() const noexcept {
  return _value;
}

ValueTreeNode* ValueTreeNode::parent() const noexcept {
  return _parent;
}

size_t ValueTreeNode::offset() const noexcept {
  return _offset;
}

Pointee* ValueTreeNode::pointee() noexcept {
  return _pointee.get();
}

const Pointee* ValueTreeNode::pointee() const noexcept {
  return _pointee.get();
}

bool ValueTreeNode::isRoot() const noexcept {
  return _parent == nullptr;
}

bool ValueTreeNode::isGlobal() const noexcept {
  if (_parent) {
    return _parent->isGlobal();
  }
  return llvm::isa<llvm::GlobalObject>(_value);
}

bool ValueTreeNode::isExternal() const noexcept {
  if (_parent) {
    return _parent->isExternal();
  }

  if (!isGlobal()) {
    return false;
  }

  auto globalObject = llvm::cast<llvm::GlobalObject>(_value);
  return llvm::GlobalValue::isAvailableExternallyLinkage(globalObject->getLinkage());
}

bool ValueTreeNode::isPointer() const noexcept {
  return _type->isPointerTy();
}

Pointer* ValueTreeNode::pointer() noexcept {
  return llvm::cast<Pointer>(pointee());
}

const Pointer* ValueTreeNode::pointer() const noexcept {
  return llvm::cast<Pointer>(pointee());
}

bool ValueTreeNode::hasChildren() const noexcept {
  return !_children.empty();
}

size_t ValueTreeNode::GetNumChildren() const noexcept {
  return _children.size();
}

ValueTreeNode* ValueTreeNode::GetChild(size_t index) noexcept {
  assert(index >= 0 && index < _children.size() && "index is out of range");
  return _children[index].get();
}

const ValueTreeNode* ValueTreeNode::GetChild(size_t index) const noexcept {
  assert(index >= 0 && index < _children.size() && "index is out of range");
  return _children[index].get();
}

void ValueTreeNode::Initialize() noexcept {
  InitializePointee();
  InitializeChildren();
}

void ValueTreeNode::InitializeChildren() noexcept {
  size_t numChildren = 0;
  std::function<const llvm::Type *(size_t)> childTypeGetter;

  if (_type->isArrayTy()) {
    numChildren = _type->getArrayNumElements();
    childTypeGetter = [this](size_t) {
      return _type->getArrayElementType();
    };
  } else if (_type->isStructTy()) {
    numChildren = _type->getStructNumElements();
    childTypeGetter = [this](size_t index) {
      return _type->getStructElementType(index);
    };
  }

  if (numChildren) {
    _children.reserve(numChildren);
    for (size_t i = 0; i < numChildren; ++i) {
      _children.push_back(std::make_unique<ValueTreeNode>(childTypeGetter(i), this, i));
    }
  }
}

void ValueTreeNode::InitializePointee() noexcept {
  if (_type->isPointerTy()) {
    _pointee = std::make_unique<Pointer>(*this);
  } else {
    _pointee = std::make_unique<Pointee>(*this);
  }
}

} // namespace anderson

} // namespace llvm

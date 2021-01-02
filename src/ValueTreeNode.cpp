//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <functional>

namespace llvm {

namespace anderson {

ValueTreeNode::ValueTreeNode(const llvm::Value *value) noexcept
  : _type(value->getType()),
    _value(value),
    _kind(ValueKind::Normal),
    _parent(nullptr),
    _offset(0),
    _children(),
    _numPointees(0),
    _numPointers(0)
{
  assert(value && "value cannot be null");
  Initialize();
}

ValueTreeNode::ValueTreeNode(StackMemoryValueTag, const llvm::AllocaInst *stackMemoryAllocator) noexcept
  : _type(stackMemoryAllocator->getAllocatedType()),
    _value(stackMemoryAllocator),
    _kind(ValueKind::StackMemory),
    _parent(nullptr),
    _offset(0),
    _children(),
    _numPointees(0),
    _numPointers(0)
{
  assert(stackMemoryAllocator && "stackMemoryAllocator cannot be null");
  Initialize();
}

ValueTreeNode::ValueTreeNode(GlobalMemoryValueTag, const llvm::GlobalVariable *globalVariable) noexcept
  : _type(globalVariable->getValueType()),
    _value(globalVariable),
    _kind(ValueKind::GlobalMemory),
    _parent(nullptr),
    _offset(0),
    _children(),
    _numPointees(0),
    _numPointers(0)
{
  assert(globalVariable && "globalVariable cannot be null");
  Initialize();
}

ValueTreeNode::ValueTreeNode(ArgumentMemoryValueTag, const llvm::Argument *argument) noexcept
  : _type(argument->getType()->getPointerElementType()),
    _value(argument),
    _kind(ValueKind::ArgumentMemory),
    _parent(nullptr),
    _offset(0),
    _children(0),
    _numPointees(0),
    _numPointers(0)
{
  assert(argument && "argument cannot be null");
  assert(argument->getType()->isPointerTy() && "argument should be a pointer");
  Initialize();
}

ValueTreeNode::ValueTreeNode(FunctionReturnValueTag, const llvm::Function *function) noexcept
  : _type(function->getReturnType()),
    _value(function),
    _kind(ValueKind::FunctionReturnValue),
    _parent(nullptr),
    _offset(0),
    _children(0),
    _numPointees(0),
    _numPointers(0)
{
  assert(function && "function cannot be null");
  Initialize();
}

ValueTreeNode::ValueTreeNode(const llvm::Type *type, ValueTreeNode *parent, size_t offset) noexcept
  : _type(type),
    _value(nullptr),
    _kind(parent->_kind),
    _parent(parent),
    _offset(offset),
    _children(),
    _numPointees(0),
    _numPointers(0)
{
  assert(type && "type cannot be null");
  assert(parent && "parent cannot be null");
  Initialize();
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

  _numPointees = 1;
  _numPointers = static_cast<size_t>(_type->isPointerTy());
  for (const auto &child : _children) {
    _numPointees += child->_numPointees;
    _numPointers += child->_numPointers;
  }
}

} // namespace anderson

} // namespace llvm

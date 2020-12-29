//
// Created by Sirui Mu on 2020/12/29.
//

#include "p2a/PointsToAnalysis.h"

#include <cassert>

namespace p2a {

Pointer::Pointer(const llvm::Value *pointerValue) noexcept
  : _pointerType(pointerValue->getType()),
    _value(pointerValue),
    _parent(nullptr),
    _offset(0),
    _children()
{
  assert(pointerValue && "pointerValue cannot be null");
  assert(_pointerType->isPointerTy() && "The specified value is not a pointer value");

  InitializeChildren();
}

Pointer::Pointer(const llvm::Type *type, Pointer *parent, size_t offset) noexcept
  : _pointerType(type),
    _value(nullptr),
    _parent(parent),
    _offset(0),
    _children()
{
  assert(type && "type cannot be null");
  assert(parent && "parent cannot be null");
  assert(type->isPointerTy() && "The specified type is not a pointer type");

  InitializeChildren();
}

bool Pointer::isRoot() const noexcept {
  return _parent == nullptr;
}

const llvm::Type* Pointer::pointerType() const noexcept {
  return _pointerType;
}

const llvm::Type* Pointer::pointeeType() const noexcept {
  return _pointerType->getPointerElementType();
}

const llvm::Value* Pointer::pointerValue() const noexcept {
  return _value;
}

const Pointer* Pointer::parent() const noexcept {
  return _parent;
}

size_t Pointer::offset() const noexcept {
  return _offset;
}

bool Pointer::isPointeePointer() const noexcept {
  return pointeeType()->isPointerTy();
}

bool Pointer::isPointeePointerArray() const noexcept {
  return pointeeType()->isArrayTy() && pointeeType()->getArrayElementType()->isPointerTy();
}

bool Pointer::isPointeeStruct() const noexcept {
  return pointeeType()->isStructTy();
}

Pointer* Pointer::GetRootPointer() noexcept {
  if (isRoot()) {
    return this;
  }
  return _parent->GetRootPointer();
}

const Pointer* Pointer::GetRootPointer() const noexcept {
  return const_cast<Pointer *>(this)->GetRootPointer();
}

bool Pointer::hasChildren() const noexcept {
  return isPointeePointer() || isPointeePointerArray() || isPointeeStruct();
}

Pointer& Pointer::GetChild(size_t index) noexcept {
  return _children.at(index);
}

const Pointer& Pointer::GetChild(size_t index) const noexcept {
  return _children.at(index);
}

auto Pointer::children() noexcept -> llvm::iterator_range<Iterator> {
  return llvm::iterator_range<Iterator> { _children.begin(), _children.end() };
}

auto Pointer::children() const noexcept -> llvm::iterator_range<ConstIterator> {
  return llvm::iterator_range<ConstIterator> { _children.cbegin(), _children.cend() };
}

void Pointer::InitializeChildren() noexcept {
  if (isPointeePointer()) {
    _children.emplace(
        std::piecewise_construct,
        std::make_tuple(0),
        std::make_tuple(pointeeType(), this, 0));
    return;
  }

  if (isPointeePointerArray()) {
    size_t arraySize = pointeeType()->getArrayNumElements();
    for (size_t i = 0; i < arraySize; ++i) {
      _children.emplace(
          std::piecewise_construct,
          std::make_tuple(i),
          std::make_tuple(pointeeType()->getArrayElementType(), this, i));
    }
    return;
  }

  if (isPointeeStruct()) {
    size_t structSize = pointeeType()->getStructNumElements();
    for (size_t i = 0; i < structSize; ++i) {
      auto fieldType = pointeeType()->getStructElementType(i);
      if (!fieldType->isPointerTy()) {
        continue;
      }
      _children.emplace(
          std::piecewise_construct,
          std::make_tuple(i),
          std::make_tuple(fieldType, this, i));
    }
    return;
  }
}

} // namespace p2a

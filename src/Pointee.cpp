//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

namespace p2a {

Pointee::Pointee() noexcept
  : _value(nullptr)
{ }

Pointee::Pointee(const llvm::Value *value) noexcept
  : _value(value)
{ }

bool Pointee::isExternalObject() const noexcept {
  return _value != nullptr;
}

const llvm::Value* Pointee::value() const noexcept {
  return _value;
}

} // namespace p2a

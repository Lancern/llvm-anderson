//
// Created by Sirui Mu on 2020/12/29.
//

#include "llvm-anderson/PointsToAnalysis.h"

#include <cassert>

namespace llvm {

namespace anderson {

PointeeSet::PointeeSet(const llvm::Type *type) noexcept
    : _type(type),
      _pointees() {}

const llvm::Type *PointeeSet::type() const noexcept {
  return _type;
}

size_t PointeeSet::size() const noexcept {
  return _pointees.size();
}

auto PointeeSet::pointees() const noexcept -> llvm::iterator_range<Iterator> {
  return llvm::iterator_range<Iterator>{_pointees.cbegin(), _pointees.cend()};
}

void PointeeSet::Add(const Pointee *pointee) noexcept {
  assert(pointee->type() == _type && "Incompatible pointee types");
  _pointees.insert(pointee);
}

void PointeeSet::MergeTo(PointeeSet &target) const noexcept {
  assert(_type == target._type && "Incompatible pointee types");
  for (auto pointee : _pointees) {
    target._pointees.insert(pointee);
  }
}

void PointeeSet::MergeFrom(const PointeeSet &source) noexcept {
  source.MergeTo(*this);
}

} // namespace anderson

} // namespace llvm

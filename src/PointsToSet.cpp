//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

#include <cassert>

namespace p2a {

PointsToSet::PointsToSet(const llvm::Type *pointeeType) noexcept
  : _pointees(),
    _pointeeType(pointeeType)
{ }

size_t PointsToSet::size() const noexcept {
  return _pointees.size();
}

const llvm::Type* PointsToSet::pointeeType() const noexcept {
  return _pointeeType;
}

bool PointsToSet::hasExternalPointees() const noexcept {
  return _pointees.find(nullptr) != _pointees.end();
}

void PointsToSet::Add(Pointee pointee) noexcept {
  if (!pointee.isExternalObject()) {
    assert(pointee.value()->getType() == _pointeeType && "Wrong pointee type");
    _pointees.insert(pointee.value());
  } else {
    _pointees.insert(nullptr);
  }
}

void PointsToSet::MergeTo(PointsToSet &to) const noexcept {
  for (auto pointee : _pointees) {
    to.Add(Pointee { pointee });
  }
}

void PointsToSet::MergeFrom(const PointsToSet &from) noexcept {
  from.MergeTo(*this);
}

} // namespace p2a

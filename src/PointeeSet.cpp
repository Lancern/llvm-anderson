//
// Created by Sirui Mu on 2021/1/1.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <algorithm>

namespace llvm {

namespace anderson {

PointeeSet::iterator::iterator(inner_iterator inner) noexcept
  : _inner(inner)
{ }

Pointee* PointeeSet::iterator::operator*() const noexcept {
  return *_inner;
}

PointeeSet::iterator& PointeeSet::iterator::operator++() noexcept {
  ++_inner;
  return *this;
}

PointeeSet::iterator PointeeSet::iterator::operator++(int) & noexcept { // NOLINT(cert-dcl21-cpp)
  auto old = *this;
  ++_inner;
  return old;
}

bool PointeeSet::iterator::operator==(const iterator &rhs) const noexcept {
  return _inner == rhs._inner;
}

bool PointeeSet::iterator::operator!=(const iterator &rhs) const noexcept {
  return _inner != rhs._inner;
}

PointeeSet::const_iterator::const_iterator(inner_iterator inner) noexcept
  : _inner(inner)
{ }

PointeeSet::const_iterator::const_iterator(iterator iter) noexcept
  : _inner(iter._inner)
{ }

const Pointee* PointeeSet::const_iterator::operator*() const noexcept {
  return *_inner;
}

PointeeSet::const_iterator& PointeeSet::const_iterator::operator++() noexcept {
  ++_inner;
  return *this;
}

PointeeSet::const_iterator PointeeSet::const_iterator::operator++(int) & noexcept { // NOLINT(cert-dcl21-cpp)
  auto old = *this;
  ++_inner;
  return old;
}

bool PointeeSet::const_iterator::operator==(const const_iterator &rhs) const noexcept {
  return _inner == rhs._inner;
}

bool PointeeSet::const_iterator::operator!=(const const_iterator &rhs) const noexcept {
  return _inner != rhs._inner;
}

size_t PointeeSet::size() const noexcept {
  return _pointees.size();
}

PointeeSet::iterator PointeeSet::begin() noexcept {
  return iterator { _pointees.begin() };
}

PointeeSet::const_iterator PointeeSet::begin() const noexcept {
  return cbegin();
}

PointeeSet::iterator PointeeSet::end() noexcept {
  return iterator { _pointees.end() };
}

PointeeSet::const_iterator PointeeSet::end() const noexcept {
  return cend();
}

PointeeSet::const_iterator PointeeSet::cbegin() noexcept {
  return const_iterator { _pointees.cbegin() };
}

PointeeSet::const_iterator PointeeSet::cbegin() const noexcept {
  return const_iterator { _pointees.cbegin() };
}

PointeeSet::const_iterator PointeeSet::cend() noexcept {
  return const_iterator { _pointees.cend() };
}

PointeeSet::const_iterator PointeeSet::cend() const noexcept {
  return const_iterator { _pointees.cend() };
}

bool PointeeSet::insert(Pointee *pointee) noexcept {
  return _pointees.insert(pointee).second;
}

PointeeSet::iterator PointeeSet::find(Pointee *pointee) noexcept {
  auto inner = _pointees.find(pointee);
  return iterator { inner };
}

PointeeSet::const_iterator PointeeSet::find(const Pointee *pointee) const noexcept {
  auto inner = _pointees.find(const_cast<Pointee *>(pointee));
  return const_iterator { inner };
}

size_t PointeeSet::count(const Pointee *pointee) const noexcept {
  auto it = find(pointee);
  if (it == end()) {
    return 0;
  }
  return 1;
}

bool PointeeSet::isSubset(const PointeeSet &another) const noexcept {
  return std::all_of(another.begin(), another.end(), [this](const Pointee *anotherElement) {
    return count(anotherElement) == 1;
  });
}

bool PointeeSet::isSubsetOf(const PointeeSet &another) const noexcept {
  return another.isSubset(*this);
}

bool PointeeSet::MergeFrom(const PointeeSet &source) noexcept {
  auto newElement = false;
  for (auto sourceElement : source) {
    if (insert(const_cast<Pointee *>(sourceElement))) {
      newElement = true;
    }
  }
  return newElement;
}

bool PointeeSet::MergeTo(PointeeSet &target) const noexcept {
  return target.MergeFrom(*this);
}

bool PointeeSet::operator==(const PointeeSet &rhs) const noexcept {
  return _pointees == rhs._pointees;
}

bool PointeeSet::operator!=(const PointeeSet &rhs) const noexcept {
  return _pointees != rhs._pointees;
}

PointeeSet& PointeeSet::operator+=(const PointeeSet &rhs) noexcept {
  MergeFrom(rhs);
  return *this;
}

} // namespace anderson

} // namespace anderson

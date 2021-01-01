//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <functional>

namespace llvm {

namespace anderson {

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

} // namespace anderson

} // namespace llvm

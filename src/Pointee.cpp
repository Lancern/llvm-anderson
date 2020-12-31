//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

namespace llvm {

namespace anderson {

Pointee::Pointee(ValueTreeNode &node) noexcept
  : _node(node)
{ }

ValueTreeNode* Pointee::node() noexcept {
  return &_node;
}

const ValueTreeNode* Pointee::node() const noexcept {
  return &_node;
}

bool Pointee::isPointer() const noexcept {
  return _node.isPointer();
}

bool Pointee::isExternal() const noexcept {
  return _node.isExternal();
}

} // namespace anderson

} // namespace llvm

//
// Created by Sirui Mu on 2020/12/31.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

namespace llvm {

namespace anderson {

bool Pointee::isPointer() const noexcept {
  return _node.isPointer();
}

bool Pointee::isExternal() const noexcept {
  return _node.isExternal();
}

} // namespace anderson

} // namespace llvm

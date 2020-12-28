//
// Created by Sirui Mu on 2020/12/27.
//

#include "PointerAssignmentDSLConverter.h"

#include <cassert>

#include <llvm/IR/Instructions.h>
#include <llvm/Support/Casting.h>

#define LLVM_POINTER_INST_LIST(H) \
  H(LoadInst)                     \
  H(StoreInst)                    \
  H(GetElementPtrInst)

namespace p2a {

namespace {

template <typename Instruction>
struct PointerAssignmentDSLConverter { };

template <>
struct PointerAssignmentDSLConverter<llvm::GetElementPtrInst> {
  static std::vector<std::unique_ptr<PointerAssignment>> GetDSL(const llvm::GetElementPtrInst &instr) noexcept {
    std::vector<PointerDerefIndex> derefSequence;
    derefSequence.reserve(instr.getNumIndices());

    for (const auto& index : instr.indices()) {
      auto indexConstant = llvm::dyn_cast_or_null<llvm::ConstantInt>(index.get());
      if (indexConstant) {
        derefSequence.emplace_back(static_cast<size_t>(indexConstant->getSExtValue()));
      } else {
        derefSequence.emplace_back();
      }
    }

    return {
      std::make_unique<PointerAssignedPointerDeref>(&instr, instr.getPointerOperand(), std::move(derefSequence))
    };
  }
};

template <>
struct PointerAssignmentDSLConverter<llvm::LoadInst> {
  static std::vector<std::unique_ptr<PointerAssignment>> GetDSL(const llvm::LoadInst &instr) noexcept {
    if (!instr.getType()->isPointerTy()) {
      return { };
    }

    auto sourcePointer = instr.getOperand(0);
    assert(sourcePointer->getType()->getPointerElementType()->isPointerTy() && "Load operand is not a pointer to a pointer");

    return {
      std::make_unique<PointerAssignedPointerDeref>(
          &instr, sourcePointer, std::vector<PointerDerefIndex> { PointerDerefIndex { 0 } }),
    };
  }
};

template <>
struct PointerAssignmentDSLConverter<llvm::StoreInst> {
  static std::vector<std::unique_ptr<PointerAssignment>> GetDSL(const llvm::StoreInst &instr) noexcept {
    auto targetPointer = instr.getPointerOperand();
    auto sourceValue = instr.getValueOperand();
    if (!sourceValue->getType()->isPointerTy()) {
      return { };
    }

    return {
      std::make_unique<PointerDerefAssignedPointer>(targetPointer, sourceValue),
    };
  }
};

} // namespace <anonymous>

std::vector<std::unique_ptr<PointerAssignment>> GetPointerAssignmentDSL(const llvm::Instruction &instr) noexcept {
#define DISPATCH_CONVERTER(inst) \
  if (llvm::isa<llvm::inst>(instr)) { \
    return PointerAssignmentDSLConverter<llvm::inst>::GetDSL(llvm::dyn_cast<llvm::inst>(instr)); \
  }
LLVM_POINTER_INST_LIST(DISPATCH_CONVERTER)
#undef DISPATCH_CONVERTER
  return { };
}

} // namespace p2a

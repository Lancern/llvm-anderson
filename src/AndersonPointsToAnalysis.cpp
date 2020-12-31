//
// Created by Sirui Mu on 2020/12/30.
//

#include "llvm-anderson/AndersonPointsToAnalysis.h"

#include <llvm/IR/Instructions.h>

namespace llvm {

namespace anderson {

class AndersonPointsToAnalysis::PointsToSolver {
  // TODO: Implement AndersonPointsToAnalysis::PointsToSolver

public:
  ValueTree* GetValueTree() const noexcept {
    return _valueTree.get();
  }

private:
  std::unique_ptr<ValueTree> _valueTree;
};

namespace {

using PointsToBuilder = AndersonPointsToAnalysis::PointsToSolver;

template <typename Instruction>
struct PointerInstructionHandler { };

template <>
struct PointerInstructionHandler<llvm::ExtractValueInst> {
  static void Handle(PointsToBuilder &builder, const llvm::ExtractValueInst &inst) noexcept {
    // TODO: Implement PointerInstructionHandler<llvm::ExtractValueInst>::Handle
  }
};

template <>
struct PointerInstructionHandler<llvm::GetElementPtrInst> {
  static void Handle(PointsToBuilder &builder, const llvm::GetElementPtrInst &inst) noexcept {
    // TODO: Implement PointerInstructionHandler<llvm::GetElementPtrInst>::Handle
  }
};

template <>
struct PointerInstructionHandler<llvm::LoadInst> {
  static void Handle(PointsToBuilder &builder, const llvm::LoadInst &inst) noexcept {
    // TODO: Implement PointerInstructionHandler<llvm::LoadInst>::Handle
  }
};

template <>
struct PointerInstructionHandler<llvm::SelectInst> {
  static void Handle(PointsToBuilder &builder, const llvm::SelectInst &inst) noexcept {
    // TODO: Implement PointerInstructionHandler<llvm::SelectInst>::Handle
  }
};

template <>
struct PointerInstructionHandler<llvm::StoreInst> {
  static void Handle(PointsToBuilder &builder, const llvm::StoreInst &inst) noexcept {
    // TODO: Implement PointerInstructionHandler<llvm::StoreInst>::Handle
  }
};

#define LLVM_POINTER_INST_LIST(H) \
  H(ExtractValueInst)             \
  H(GetElementPtrInst)            \
  H(LoadInst)                     \
  H(SelectInst)                   \
  H(StoreInst)

void DispatchInstruction(PointsToBuilder &builder, const llvm::Instruction &inst) noexcept {
#define INST_DISPATCHER(instType)                                                                 \
  if (llvm::isa<llvm::instType>(inst)) {                                                          \
    PointerInstructionHandler<llvm::instType>::Handle(builder, llvm::cast<llvm::instType>(inst)); \
  }
LLVM_POINTER_INST_LIST(INST_DISPATCHER)
#undef INST_DISPATCHER
}

#undef LLVM_POINTER_INST_LIST

} // namespace <anonymous>

char AndersonPointsToAnalysis::ID = 0;

AndersonPointsToAnalysis::AndersonPointsToAnalysis() noexcept
  : llvm::ModulePass { ID },
    _graphBuilder(nullptr)
{ }

AndersonPointsToAnalysis::~AndersonPointsToAnalysis() noexcept = default;

bool AndersonPointsToAnalysis::runOnModule(llvm::Module &module) {
  // TODO: Implement AndersonPointsToAnalysis::runOnModule
  return false;
}

ValueTree* AndersonPointsToAnalysis::GetValueTree() noexcept {
  return _graphBuilder->GetValueTree();
}

const ValueTree* AndersonPointsToAnalysis::GetValueTree() const noexcept {
  return _graphBuilder->GetValueTree();
}

static llvm::RegisterPass<AndersonPointsToAnalysis> RegisterAnderson { // NOLINT(cert-err58-cpp)
  "anderson",
  "Anderson points-to analysis",
  true,
  true
};

} // namespace anderson

} // namespace llvm

//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

#include <memory>

namespace p2a {

namespace {

std::unique_ptr<PointerAssignment> GetPointerAssignmentDSL(const llvm::Instruction &instruction) noexcept {
  // TODO: Implement GetPointerAssignmentDSL.
  return nullptr;
}

} // namespace <anonymous>

bool FlowInsensitivePointsToAnalysis::runOnModule(llvm::Module &module) {
  for (const auto& func : module) {
    for (const auto& bb : func) {
      for (const auto& instr : bb) {
        auto dsl = GetPointerAssignmentDSL(instr);
        if (dsl) {
          UpdateWithPointerAssignment(*dsl);
        }
      }
    }
  }

  FinishUpdate();

  return false; // The module is not modified.
}

} // namespace p2a

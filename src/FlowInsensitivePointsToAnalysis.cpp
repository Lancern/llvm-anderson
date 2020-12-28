//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

#include "PointerAssignmentDSLConverter.h"

namespace p2a {

FlowInsensitivePointsToAnalysis::FlowInsensitivePointsToAnalysis(char &id) noexcept
  : PointsToAnalysis { id }
{ }

bool FlowInsensitivePointsToAnalysis::runOnModule(llvm::Module &module) {
  for (const auto& func : module) {
    for (const auto& bb : func) {
      for (const auto& instr : bb) {
        auto dslList = GetPointerAssignmentDSL(instr);
        for (const auto& dsl : dslList) {
          if (dsl) {
            UpdateWithPointerAssignment(*dsl);
          }
        }
      }
    }
  }

  FinishUpdate();

  return false; // The module is not modified.
}

} // namespace p2a

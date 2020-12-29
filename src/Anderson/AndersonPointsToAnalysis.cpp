//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

namespace p2a {

char AndersonPointsToAnalysis::ID;

AndersonPointsToAnalysis::AndersonPointsToAnalysis() noexcept
  : FlowInsensitivePointsToAnalysis { ID }
{ }

void AndersonPointsToAnalysis::UpdateWithPointerAssignment(const PointerAssignment &assignment) {
  // TODO: Implement AndersonPointsToAnalysis::UpdateWithPointerAssignment
}

void AndersonPointsToAnalysis::FinishUpdate() {
  // TODO: Implement AndersonPointsToAnalysis::FinishUpdate
}

static RegisterPointsToAnalysisPass<AndersonPointsToAnalysis> X {
  "anderson-points-to",
  "Anderson points-to analysis"
};

} // namespace p2a

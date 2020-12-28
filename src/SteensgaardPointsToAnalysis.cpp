//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

namespace p2a {

SteensgaardPointsToAnalysis::SteensgaardPointsToAnalysis() noexcept
  : FlowInsensitivePointsToAnalysis { PassId }
{ }

void SteensgaardPointsToAnalysis::UpdateWithPointerAssignment(const PointerAssignment &assignment) {
  // TODO: Implement SteensgaardPointsToAnalysis::UpdateWithPointerAssignment
}

void SteensgaardPointsToAnalysis::FinishUpdate() {
  // TODO: Implement SteensgaardPointsToAnalysis::FinishUpdate
}

char SteensgaardPointsToAnalysis::PassId;

static RegisterPointsToAnalysisPass<SteensgaardPointsToAnalysis> X {
  "steensgaard-points-to",
  "Steensgaard points-to analysis"
};

} // namespace p2a

//
// Created by Sirui Mu on 2020/12/26.
//

#include "p2a/PointsToAnalysis.h"

namespace p2a {

char SteensgaardPointsToAnalysis::ID;

SteensgaardPointsToAnalysis::SteensgaardPointsToAnalysis() noexcept
  : FlowInsensitivePointsToAnalysis { ID }
{ }

void SteensgaardPointsToAnalysis::UpdateWithPointerAssignment(const PointerAssignment &assignment) {
  // TODO: Implement SteensgaardPointsToAnalysis::UpdateWithPointerAssignment
}

void SteensgaardPointsToAnalysis::FinishUpdate() {
  // TODO: Implement SteensgaardPointsToAnalysis::FinishUpdate
}

static RegisterPointsToAnalysisPass<SteensgaardPointsToAnalysis> X {
  "steensgaard-points-to",
  "Steensgaard points-to analysis"
};

} // namespace p2a

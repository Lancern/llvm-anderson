//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef P2A_POINTS_TO_ANALYSIS_H
#define P2A_POINTS_TO_ANALYSIS_H

#include <cstddef>
#include <type_traits>
#include <unordered_map>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

#include "p2a/PointerAssignmentDSL.h"

namespace p2a {

/**
 * The pointee object of a pointer.
 */
class Pointee {
  // TODO: Implement class Pointee
}; // class Pointee

/**
 * The set of all possible pointees of a pointer.
 */
class PointeeSet {
  // TODO: Implement class PointeeSet
}; // class PointeeSet

/**
 * Provide abstract base class for points-to analysis.
 */
class PointsToAnalysis : public llvm::ModulePass {
public:
  /**
   * Gets the points-to set of the given pointer.
   *
   * @param pointer the pointer value.
   * @return the points-to set of the given pointer.
   */
  PointeeSet* GetPointsToSet(const llvm::Value *pointer) noexcept;

  /**
   * Gets the points-to set of the given pointer.
   *
   * @param pointer the pointer value.
   * @return the points-to set of the given pointer.
   */
  const PointeeSet* GetPointsToSet(const llvm::Value *pointer) const noexcept;

protected:
  /**
   * Construct a new PointsToAnalysis object.
   *
   * @param id the identifier of the pass.
   */
  explicit PointsToAnalysis(char &id) noexcept;

private:
  std::unordered_map<const llvm::Value *, PointeeSet> _pointeeSets;
}; // class PointsToAnalysis

class FlowInsensitivePointsToAnalysis : public PointsToAnalysis {
public:
  bool runOnModule(llvm::Module &module) final;

protected:
  /**
   * Construct a new FlowInsensitivePointsToAnalysis object.
   *
   * @param id the pass identifier.
   */
  explicit FlowInsensitivePointsToAnalysis(char &id) noexcept;

  /**
   * When overridden in derived classes, update the analysis state with the given pointer assignment DSL.
   *
   * @param assignment the pointer assignment DSL.
   */
  virtual void UpdateWithPointerAssignment(const PointerAssignment &assignment) = 0;

  /**
   * When overridden in derived classes, finish all analysis and cache analysis result.
   */
  virtual void FinishUpdate() = 0;
};

/**
 * Implement Anderson points-to analysis.
 */
class AndersonPointsToAnalysis : public FlowInsensitivePointsToAnalysis {
public:
  /**
   * Construct a new AndersonPointsToAnalysis object.
   */
  explicit AndersonPointsToAnalysis() noexcept;

protected:
  void UpdateWithPointerAssignment(const PointerAssignment &assignment) final;

  void FinishUpdate() final;

private:
  static char PassId;
};

/**
 * Implement Steensgaard points-to analysis.
 */
class SteensgaardPointsToAnalysis : public FlowInsensitivePointsToAnalysis {
public:
  /**
   * Construct a new SteensgaardPointsToAnalysis object.
   */
  explicit SteensgaardPointsToAnalysis() noexcept;

protected:
  void UpdateWithPointerAssignment(const PointerAssignment &assignment) final;

  void FinishUpdate() final;

private:
  static char PassId;
};

template <typename AnalysisPass>
class RegisterPointsToAnalysisPass : public llvm::RegisterPass<AnalysisPass> {
public:
  static_assert(
      std::is_base_of<PointsToAnalysis, AnalysisPass>::value,
      "AnalysisPass should derive from PointsToAnalysis");

  explicit RegisterPointsToAnalysisPass(const char* name, const char* description) noexcept
    : llvm::RegisterPass<AnalysisPass> { name, description, true, true }
  { }
};

} // namespace p2a

#endif // P2A_POINTS_TO_ANALYSIS_H

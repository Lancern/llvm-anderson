//
// Created by msr on 2020/12/26.
//

#ifndef P2A_POINTS_TO_ANALYSIS_H
#define P2A_POINTS_TO_ANALYSIS_H

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/Pass.h>

#include "p2a/PointerAssignmentDSL.h"

namespace p2a {

/**
 * A wrapper around llvm::Value that represents the pointee object of a pointer.
 */
class Pointee {
public:
  /**
   * Construct a new Pointee object that represents a pointee defined outside of the current module.
   */
  explicit Pointee() noexcept;

  /**
   * Construct a new Pointee object.
   *
   * @param value the pointee value.
   */
  explicit Pointee(const llvm::Value *value) noexcept;

  /**
   * Determine whether this pointee object is defined outside of the current module.
   *
   * @return a value indicating whether this pointee object is defined externally.
   */
  bool isExternalObject() const noexcept;

  /**
   * Get the pointee value.
   *
   * @return the pointee value. If this pointee object is defined outside of the current module, this function returns
   * nullptr.
   */
  const llvm::Value* value() const noexcept;

private:
  const llvm::Value* _value;
}; // class Pointee

/**
 * The set of all possible pointees of a pointer.
 */
class PointsToSet {
public:
  /**
   * Iterator of PointsToSet.
   */
  class Iterator {
  public:
    /**
     * Construct a new Iterator object.
     *
     * @param inner the inner iterator object.
     */
    explicit Iterator(typename std::unordered_set<const llvm::Value *>::const_iterator inner) noexcept
      : _inner(inner)
    { }

    Iterator& operator++() noexcept {
      ++_inner;
      return *this;
    }

    Iterator operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
      Iterator ret { _inner };
      ++_inner;
      return ret;
    }

    Pointee operator*() const noexcept {
      return Pointee { *_inner };
    }

  private:
    typename std::unordered_set<const llvm::Value *>::const_iterator _inner;
  }; // class Iterator

  /**
   * Construct a new PointsToSet object.
   *
   * @param pointeeType type of the pointee object.
   */
  explicit PointsToSet(const llvm::Type *pointeeType) noexcept;

  /**
   * Get the number of objects contained in this set.
   *
   * @return the number of objects contained in this set.
   */
  size_t size() const noexcept;

  /**
   * Get the type of the pointee objects.
   *
   * @return the type of the pointee objects.
   */
  const llvm::Type* pointeeType() const noexcept;

  Iterator begin() const noexcept {
    return Iterator { _pointees.begin() };
  }

  Iterator end() const noexcept {
    return Iterator { _pointees.end() };
  }

  Iterator cbegin() const noexcept {
    return begin();
  }

  Iterator cend() const noexcept {
    return end();
  }

  /**
   * Get a value indicating whether the pointer can points to objects given by external code.
   *
   * @return a value indicating whether the pointer can points to objects given by external code.
   */
  bool hasExternalPointees() const noexcept;

  /**
   * Add the given object to this set.
   *
   * @param pointee the pointee object.
   */
  void Add(Pointee pointee) noexcept;

  /**
   * Merge this set into the given set.
   *
   * @param to the target set.
   */
  void MergeTo(PointsToSet &to) const noexcept;

  /**
   * Merge all objects in the given set into this set.
   *
   * @param from the set.
   */
  void MergeFrom(const PointsToSet &from) noexcept;

private:
  std::unordered_set<const llvm::Value *> _pointees;
  const llvm::Type *_pointeeType;
}; // class PointsToSet

/**
 * Provide abstract base class for points-to analysis.
 */
class PointsToAnalysis : public virtual llvm::ModulePass {
public:
  /**
   * Gets the points-to set of the given pointer.
   *
   * @param pointer the pointer value.
   * @return the points-to set of the given pointer.
   */
  PointsToSet* GetPointsToSet(const llvm::Value *pointer) noexcept;

  /**
   * Gets the points-to set of the given pointer.
   *
   * @param pointer the pointer value.
   * @return the points-to set of the given pointer.
   */
  const PointsToSet* GetPointsToSet(const llvm::Value *pointer) const noexcept;

protected:
  /**
   * Construct a new PointsToAnalysis object.
   *
   * @param id the identifier of the pass.
   */
  explicit PointsToAnalysis(char &id) noexcept;

private:
  std::unordered_map<const llvm::Value *, PointsToSet> _pointeeSets;
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

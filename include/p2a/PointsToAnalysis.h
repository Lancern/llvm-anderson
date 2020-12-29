//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef P2A_POINTS_TO_ANALYSIS_H
#define P2A_POINTS_TO_ANALYSIS_H

#include <cstddef>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

#include "p2a/PointerAssignmentDSL.h"

namespace p2a {

/**
 * The pointee object of a pointer.
 */
class Pointee {
public:
  constexpr static const size_t InvalidOffset = static_cast<size_t>(-1);

  /**
   * Determine whether the given value can be served as a root pointee object.
   *
   * This function effectively determines whether the given value is either `llvm::GlobalVariable` or
   * `llvm::AllocaInst`.
   *
   * This function triggers an assertion failure if `value` is null.
   *
   * @param value the value.
   * @return whether the given value can be served as a root pointee object.
   */
  static bool IsRootValue(const llvm::Value *value) noexcept;

  /**
   * Construct a new Pointee object that represents an externally defined pointee object of the specified type.
   *
   * @param type type of the pointee object.
   */
  explicit Pointee(const llvm::Type *type) noexcept;

  /**
   * Construct a new Pointee object that represents the specified value as the pointee object.
   *
   * If `IsRootValue(value)` returns false, this function triggers an assertion failure.
   *
   * @param value the value as the pointee object.
   */
  explicit Pointee(const llvm::Value *value) noexcept;

  /**
   * Construct a new Pointee object that represents the specified global object as the pointee object.
   *
   * @param globalObject the global object as the pointee object.
   */
  explicit Pointee(const llvm::GlobalObject *globalObject) noexcept;

  /**
   * Construct a new Pointee object that represents the specified stack allocated variable as the pointee object.
   *
   * @param allocaInst the stack allocated object.
   */
  explicit Pointee(const llvm::AllocaInst *allocaInst) noexcept;

  /**
   * Construct a new Pointee object that represents the subobject at the specified offset in the specified parent
   * pointee object.
   *
   * @param type type of the subobject.
   * @param parent the parent pointee object.
   * @param offset the offset of this new pointee object within the parent pointee object.
   */
  explicit Pointee(const llvm::Type *type, const Pointee *parent, size_t offset) noexcept;

  Pointee(const Pointee &) = delete;

  /**
   * The move constructor of Pointee should not be used during runtime.
   */
  Pointee(Pointee &&) noexcept;

  Pointee& operator=(const Pointee &) = delete;

  /**
   * The move assignment operator of Pointee should not be used during runtime.
   */
  Pointee& operator=(Pointee &&) noexcept;

  ~Pointee() noexcept;

  /**
   * Determine whether this pointee object is a root object, i.e. whether it's either a global variable or a stack
   * allocated variable.
   *
   * @return whether this pointee object is a root object.
   */
  bool isRoot() const noexcept;

  /**
   * Determine whether this pointee object is defined outside of the current module.
   *
   * @return whether this pointee object is defined outside of the current module.
   */
  bool isExternal() const noexcept;

  /**
   * Determine whether this pointee object is a global variable or a subobject of a global variable.
   *
   * @return whether this pointee object is a global variable or a subobject of a global variable.
   */
  bool isGlobal() const noexcept;

  /**
   * Determine whether this pointee object is a stack allocated variable or a subobject of a stack allocated variable.
   *
   * @return whether this pointee object is a stack allocated variable or a subobject of a stack allocated variable.
   */
  bool isAlloca() const noexcept;

  /**
   * Get the type of this pointee object.
   *
   * @return the type of this pointee object.
   */
  const llvm::Type* type() const noexcept;

  /**
   * Get the global variable definition of this pointee object.
   *
   * If this pointee object is neither a global variable nor a subobject of a global variable, this function triggers
   * an assertion failure.
   *
   * @return the global variable definition of this pointee object.
   */
  const llvm::GlobalObject* globalObject() const noexcept;

  /**
   * Get the `alloca` instruction that allocates this pointee object on stack.
   *
   * If this pointee object is neither a stack allocated variable nor a subobject of a stack allocated variable, this
   * function triggers an assertion failure.
   *
   * @return the `alloca` instruction that allocates this pointee object on stack.
   */
  const llvm::AllocaInst* allocaInst() const noexcept;

  /**
   * Get the parent pointee object that this pointee object is a subobject of it.
   *
   * @return the parent pointee object. If this pointee object is not a subobject of any pointee objects, returns
   * nullptr.
   */
  const Pointee* parent() const noexcept;

  /**
   * Get the offset of this subobject within the parent pointee object.
   *
   * @return the offset of this subobject within the parent pointee object. If this pointee object is not a subobject
   * of any pointee objects, returns `InvalidOffset`.
   */
  size_t offset() const noexcept;

  /**
   * Get the root pointee object.
   *
   * @return the root pointee object.
   */
  const Pointee* root() const noexcept;

  /**
   * Determine whether this pointee object is a pointer.
   *
   * @return whether this pointee object is a pointer.
   */
  bool isPointer() const noexcept;

  /**
   * Determine whether this pointee object is an array.
   *
   * @return whether this pointee object is an array.
   */
  bool isArray() const noexcept;

  /**
   * Determine whether this pointee object is a struct.
   *
   * @return whether this pointee object is a struct.
   */
  bool isStruct() const noexcept;

  /**
   * Determine whether this pointee object may have children, i.e. pointee objects deeper in the hierarchy.
   *
   * This function effectively checks that whether this pointee object is a pointer, an array or a struct.
   *
   * @return whether this pointee object may have children.
   */
  bool hasChildren() const noexcept;

  /**
   * Get the number of children of this pointee object.
   *
   * @return the number of children of this pointee object.
   */
  size_t GetNumChildren() const noexcept;

  /**
   * Get the i-th child of this pointee object.
   *
   * This function triggers an assertion failure if i is out of range.
   *
   * @param i the index of the child.
   * @return the i-th child of this pointee object.
   */
  const Pointee& GetChild(size_t i) const noexcept;

private:
  const llvm::Type *_type;
  const llvm::Value *_value;
  const Pointee *_parent;
  size_t _offset;
  std::vector<Pointee> _children;

  void InitializeChildren() noexcept;
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
   * Identifier of this analysis pass.
   */
  static char ID;

  /**
   * Construct a new AndersonPointsToAnalysis object.
   */
  explicit AndersonPointsToAnalysis() noexcept;

protected:
  void UpdateWithPointerAssignment(const PointerAssignment &assignment) final;

  void FinishUpdate() final;
};

/**
 * Implement Steensgaard points-to analysis.
 */
class SteensgaardPointsToAnalysis : public FlowInsensitivePointsToAnalysis {
public:
  /**
   * Identifier of this analysis pass.
   */
  static char ID;

  /**
   * Construct a new SteensgaardPointsToAnalysis object.
   */
  explicit SteensgaardPointsToAnalysis() noexcept;

protected:
  void UpdateWithPointerAssignment(const PointerAssignment &assignment) final;

  void FinishUpdate() final;
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

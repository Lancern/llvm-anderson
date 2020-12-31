//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef LLVM_ANDERSON_POINTS_TO_ANALYSIS_H
#define LLVM_ANDERSON_POINTS_TO_ANALYSIS_H

#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

#define NON_COPIABLE_NON_MOVABLE(className)             \
  className(const className &) = delete;                \
  className(className &&) noexcept = delete;            \
  className& operator=(const className &) = delete;     \
  className& operator=(className &&) noexcept = delete;

namespace llvm {

namespace anderson {

class Pointee;
class Pointer;
class ValueTreeNode;

class PointsTo {
public:
  explicit PointsTo(Pointee *pointee) noexcept
    : _pointee(pointee)
  { }

  Pointee* pointee() noexcept { return _pointee; }

  const Pointee* pointee() const noexcept { return _pointee; }

  bool operator==(const PointsTo &rhs) const noexcept {
    return _pointee == rhs._pointee;
  }

  bool operator!=(const PointsTo &rhs) const noexcept {
    return !operator==(rhs);
  }

  friend class PointsToHasher;

private:
  Pointee *_pointee;
};

struct PointsToHasher {
  size_t operator()(const PointsTo &e) const noexcept {
    return std::hash<const Pointee *> { }(e._pointee);
  }
};

class PointsToPointeesOf {
public:
  explicit PointsToPointeesOf(Pointer *anotherPointer) noexcept
    : _anotherPointer(anotherPointer)
  { }

  Pointer* pointer() noexcept { return _anotherPointer; }

  const Pointer* pointer() const noexcept { return _anotherPointer; }

  bool operator==(const PointsToPointeesOf &rhs) const noexcept {
    return _anotherPointer == rhs._anotherPointer;
  }

  bool operator!=(const PointsToPointeesOf &rhs) const noexcept {
    return !operator==(rhs);
  }

  friend struct PointsToPointeesOfHasher;

private:
  Pointer *_anotherPointer;
};

struct PointsToPointeesOfHasher {
  size_t operator()(const PointsToPointeesOf &e) const noexcept {
    return std::hash<const Pointer *> { }(e._anotherPointer);
  }
};

class Pointee {
public:
  explicit Pointee(ValueTreeNode &node) noexcept;

  NON_COPIABLE_NON_MOVABLE(Pointee)

  ValueTreeNode* node() noexcept;

  const ValueTreeNode* node() const noexcept;

  bool isPointer() const noexcept;

  bool isExternal() const noexcept;

private:
  ValueTreeNode &_node;
};

class Pointer : public Pointee {
public:
  static bool classof(const Pointee *obj) noexcept;

  explicit Pointer(ValueTreeNode &node) noexcept;

  NON_COPIABLE_NON_MOVABLE(Pointer)

  void AddPointsTo(Pointee *pointee) noexcept;

  void AddPointsToPointeesOf(Pointer *anotherPointer) noexcept;

private:
  std::unordered_set<PointsTo, PointsToHasher> _pointsTo;
  std::unordered_set<PointsToPointeesOf, PointsToPointeesOfHasher> _pointsToPointeesOf;
};

class ValueTreeNode {
public:
  explicit ValueTreeNode(const llvm::Value *value) noexcept;

  explicit ValueTreeNode(const llvm::Type *type, ValueTreeNode *parent, size_t offset) noexcept;

  NON_COPIABLE_NON_MOVABLE(ValueTreeNode)

  const llvm::Type *type() const noexcept;

  const llvm::Value *value() const noexcept;

  ValueTreeNode* parent() const noexcept;

  size_t offset() const noexcept;

  Pointee* pointee() noexcept;

  const Pointee* pointee() const noexcept;

  bool isRoot() const noexcept;

  bool isGlobal() const noexcept;

  bool isExternal() const noexcept;

  bool isPointer() const noexcept;

  Pointer* pointer() noexcept;

  const Pointer* pointer() const noexcept;

  bool hasChildren() const noexcept;

  size_t GetNumChildren() const noexcept;

  ValueTreeNode* GetChild(size_t index) noexcept;

  const ValueTreeNode* GetChild(size_t index) const noexcept;

private:
  const llvm::Type *_type;
  const llvm::Value *_value;
  ValueTreeNode *_parent;
  size_t _offset;
  std::vector<std::unique_ptr<ValueTreeNode>> _children;
  std::unique_ptr<Pointee> _pointee;

  void Initialize() noexcept;

  void InitializeChildren() noexcept;

  void InitializePointee() noexcept;
};

class ValueTree {
public:
  explicit ValueTree(const llvm::Module &module) noexcept;

  ValueTreeNode* GetNode(const llvm::Value *value) noexcept;

  const ValueTreeNode* GetNode(const llvm::Value *value) const noexcept;

private:
  const llvm::Module &_module;
  std::unordered_map<const llvm::Value *, std::unique_ptr<ValueTreeNode>> _roots;
};

/**
 * Implementation of Anderson points-to analysis algorithm as a LLVM module pass.
 */
class AndersonPointsToAnalysis : public llvm::ModulePass {
public:
  class PointsToSolver;

  static char ID;

  /**
   * Construct a new AndersonPointsToAnalysis object.
   */
  explicit AndersonPointsToAnalysis() noexcept;

  NON_COPIABLE_NON_MOVABLE(AndersonPointsToAnalysis)

  ~AndersonPointsToAnalysis() noexcept override;

  bool runOnModule(llvm::Module &module) final;

  /**
   * Get the value tree which contains analysis result.
   *
   * @return the value tree which contains analysis result.
   */
  ValueTree* GetValueTree() noexcept;

  /**
   * Get the value tree which contains analysis result.
   *
   * @return the value tree which contains analysis result.
   */
  const ValueTree* GetValueTree() const noexcept;

private:
  std::unique_ptr<PointsToSolver> _graphBuilder;
};

} // namespace anderson

} // namespace llvm

#endif // LLVM_ANDERSON_POINTS_TO_ANALYSIS_H

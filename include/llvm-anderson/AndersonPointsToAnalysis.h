//
// Created by Sirui Mu on 2020/12/26.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

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
  struct Hash {
    size_t operator()(const PointsTo &e) const noexcept {
      return std::hash<Pointee *> { }(e._pointee);
    }
  };

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

private:
  Pointee *_pointee;
};

class PointsToPointeesOf {
public:
  struct Hash {
    size_t operator()(const PointsToPointeesOf &e) const noexcept {
      return std::hash<Pointer *> { }(e._pointer);
    }
  };

  explicit PointsToPointeesOf(Pointer *pointer) noexcept
    : _pointer(pointer)
  { }

  Pointer* pointer() noexcept { return _pointer; }

  const Pointer* pointer() const noexcept { return _pointer; }

  bool operator==(const PointsToPointeesOf &rhs) const noexcept {
    return _pointer == rhs._pointer;
  }

  bool operator!=(const PointsToPointeesOf &rhs) const noexcept {
    return !operator==(rhs);
  }

private:
  Pointer *_pointer;
};

class PointsToPointeesOfPointeesOf {
public:
  struct Hash {
    size_t operator()(const PointsToPointeesOfPointeesOf &pt) const noexcept {
      return std::hash<Pointer *> { }(pt._pointer);
    }
  };

  explicit PointsToPointeesOfPointeesOf(Pointer *pointer) noexcept
    : _pointer(pointer)
  { }

  Pointer *pointer() noexcept { return _pointer; }

  const Pointer* pointer() const noexcept { return _pointer; }

  bool operator==(const PointsToPointeesOfPointeesOf &rhs) const noexcept {
    return _pointer == rhs._pointer;
  }

  bool operator!=(const PointsToPointeesOfPointeesOf &rhs) const noexcept {
    return !operator==(rhs);
  }

private:
  Pointer *_pointer;
};

class PointeePointsToPointeesOf {
public:
  struct Hash {
    size_t operator()(const PointeePointsToPointeesOf &pt) const noexcept {
      return std::hash<Pointer *> { }(pt._pointer);
    }
  };

  explicit PointeePointsToPointeesOf(Pointer *pointer) noexcept
    : _pointer(pointer)
  { }

  Pointer* pointer() noexcept { return _pointer; }

  const Pointer* pointer() const noexcept { return _pointer; }

  bool operator==(const PointeePointsToPointeesOf &rhs) const noexcept {
    return _pointer == rhs._pointer;
  }

  bool operator!=(const PointeePointsToPointeesOf &rhs) const noexcept {
    return !operator==(rhs);
  }

private:
  Pointer *_pointer;
};

/**
 * A set of GetPointeeSet.
 */
class PointeeSet {
public:
  class const_iterator;

  /**
   * Iterator of PointeeSet.
   */
  class iterator {
  public:
    /**
     * Type of the inner iterator.
     */
    using inner_iterator = typename std::unordered_set<Pointee *>::iterator;

    /**
     * Construct a new iterator object from the given inner iterator.
     *
     * @param inner the inner iterator.
     */
    explicit iterator(inner_iterator inner) noexcept;

    Pointee* operator*() const noexcept;

    iterator& operator++() noexcept;

    iterator operator++(int) & noexcept; // NOLINT(cert-dcl21-cpp)

    bool operator==(const iterator &rhs) const noexcept;

    bool operator!=(const iterator &rhs) const noexcept;

    friend class PointeeSet::const_iterator;

  private:
    inner_iterator _inner;
  };

  /**
   * Constant iterator of PointeeSet.
   */
  class const_iterator {
  public:
    /**
     * Type of the inner iterator.
     */
    using inner_iterator = typename std::unordered_set<Pointee *>::const_iterator;

    /**
     * Construct a new const_iterator object from the given inner iterator.
     *
     * @param inner the inner iterator.
     */
    explicit const_iterator(inner_iterator inner) noexcept;

    const_iterator(iterator iter) noexcept; // NOLINT(google-explicit-constructor)

    const Pointee* operator*() const noexcept;

    const_iterator& operator++() noexcept;

    const_iterator operator++(int) & noexcept; // NOLINT(cert-dcl21-cpp)

    bool operator==(const const_iterator &rhs) const noexcept;

    bool operator!=(const const_iterator &rhs) const noexcept;

  private:
    inner_iterator _inner;
  };

  /**
   * Get the number of elements contained in the PointeeSet.
   *
   * @return the number of elements contained in the PointeeSet.
   */
  size_t size() const noexcept;

  iterator begin() noexcept;

  const_iterator begin() const noexcept;

  iterator end() noexcept;

  const_iterator end() const noexcept;

  const_iterator cbegin() noexcept;

  const_iterator cbegin() const noexcept;

  const_iterator cend() noexcept;

  const_iterator cend() const noexcept;

  /**
   * Insert the given pointee into this set.
   *
   * @param pointee the pointee.
   * @return whether the insertion takes place.
   */
  bool insert(Pointee *pointee) noexcept;

  /**
   * Get the iterator to the specified element.
   *
   * @param pointee the element to find.
   * @return the iterator to the specified element. If no such element are contained in this set, returns `end()`.
   */
  iterator find(Pointee *pointee) noexcept;

  /**
   * Get the iterator to the specified element.
   *
   * @param pointee the element to find.
   * @return the iterator to the specified element. If no such element are contained in this set, returns `end()`.
   */
  const_iterator find(const Pointee *pointee) const noexcept;

  /**
   * Return 1 if `pointee` is in this set, otherwise return 0.
   *
   * @return 1 if `pointee` is in this set, otherwise return 0.
   */
  size_t count(const Pointee *pointee) const noexcept;

  /**
   * Determine whether the specified set is a subset of this set.
   *
   * @param another another pointee set.
   * @return whether the specified set is a subset of this set.
   */
  bool isSubset(const PointeeSet &another) const noexcept;

  /**
   * Determine whether this set is a subset of the specified set.
   *
   * @param another another pointee set.
   * @return whether this set is a subset of the specified set.
   */
  bool isSubsetOf(const PointeeSet &another) const noexcept;

  /**
   * Merge all elements from the specified set into this set.
   *
   * @param source the source pointee set.
   * @return whether at least one new element is added into this set.
   */
  bool MergeFrom(const PointeeSet &source) noexcept;

  /**
   * Merge all elements from this set into the specified set.
   *
   * @param target the target set.
   * @return whether at least one new element is added to the specified set.
   */
  bool MergeTo(PointeeSet &target) const noexcept;

  bool operator==(const PointeeSet &rhs) const noexcept;

  bool operator!=(const PointeeSet &rhs) const noexcept;

  PointeeSet& operator+=(const PointeeSet &rhs) noexcept;

private:
  std::unordered_set<Pointee *> _pointees;
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

  void AddPointsToPointeesOf(Pointer *pointer) noexcept;

  void AddPointsToPointeesOfPointeesOf(Pointer *pointer) noexcept;

  void AddPointeePointsToPointeesOf(Pointer *pointer) noexcept;

  /**
   * Get the pointee set of this pointer.
   *
   * @return the pointee set of this pointer.
   */
  PointeeSet& GetPointeeSet() noexcept;

  /**
   * Get the pointee set of this pointer.
   *
   * @return the pointee set of this pointer.
   */
  const PointeeSet& GetPointeeSet() const noexcept;

private:
  std::unordered_set<PointsTo, PointsTo::Hash> _pointsTo;
  std::unordered_set<PointsToPointeesOf, PointsToPointeesOf::Hash> _pointsToPointeesOf;
  std::unordered_set<PointsToPointeesOfPointeesOf, PointsToPointeesOfPointeesOf::Hash> _pointsToPointeesOfPointeesOf;
  std::unordered_set<PointeePointsToPointeesOf, PointeePointsToPointeesOf::Hash> _pointeePointsToPointeesOf;
  PointeeSet _pointees;
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
  std::unique_ptr<PointsToSolver> _solver;
};

} // namespace anderson

} // namespace llvm

#endif // LLVM_ANDERSON_POINTS_TO_ANALYSIS_H

#pragma clang diagnostic pop
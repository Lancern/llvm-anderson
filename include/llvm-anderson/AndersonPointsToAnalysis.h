//
// Created by Sirui Mu on 2020/12/26.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#ifndef LLVM_ANDERSON_POINTS_TO_ANALYSIS_H
#define LLVM_ANDERSON_POINTS_TO_ANALYSIS_H

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <llvm/ADT/iterator_range.h>
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

namespace details {

template <typename T>
struct PolymorphicHasher {
  size_t operator()(const T &obj) const noexcept {
    return obj.GetHashCode();
  }
};

} // namespace details

class Pointee;
class Pointer;
class ValueTreeNode;

enum class PointerAssignmentKind {
  AssignedElementPtr,
  AssignedPointee,
  PointeeAssigned,
};

class PointerAssignment {
public:
  PointerAssignmentKind kind() const noexcept { return _kind; }

  Pointer* pointer() noexcept { return _pointer; }

  const Pointer* pointer() const noexcept { return _pointer; }

  virtual size_t GetHashCode() const noexcept;

  virtual bool operator==(const PointerAssignment &rhs) const noexcept;

  virtual bool operator!=(const PointerAssignment &rhs) const noexcept {
    return !operator==(rhs);
  }

protected:
  explicit PointerAssignment(PointerAssignmentKind kind, Pointer *pointer) noexcept
    : _kind(kind),
      _pointer(pointer)
  {
    assert(pointer && "pointer cannot be null");
  }

  virtual ~PointerAssignment() noexcept = default;

private:
  PointerAssignmentKind _kind;
  Pointer *_pointer;
};

class PointerIndex {
public:
  constexpr static const size_t DynamicIndex = static_cast<size_t>(-1);

  explicit PointerIndex() noexcept
    : _index(DynamicIndex)
  { }

  explicit PointerIndex(size_t index) noexcept
    : _index(index)
  { }

  size_t index() const noexcept { return _index; }

  bool isConstant() const noexcept { return _index != DynamicIndex; }

  bool isDynamic() const noexcept { return _index == DynamicIndex; }

  bool operator==(const PointerIndex &rhs) const noexcept {
    return _index == rhs._index;
  }

  bool operator!=(const PointerIndex &rhs) const noexcept {
    return _index != rhs._index;
  }

private:
  size_t _index;
};

class PointerAssignedElementPtr : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::AssignedElementPtr;
  }

  explicit PointerAssignedElementPtr(Pointer *pointer, std::vector<PointerIndex> indexSequence) noexcept
    : PointerAssignment {PointerAssignmentKind::AssignedElementPtr, pointer },
      _indexSequence(std::move(indexSequence))
  { }

  llvm::iterator_range<typename std::vector<PointerIndex>::const_iterator> index_sequence() const noexcept {
    return llvm::iterator_range<typename std::vector<PointerIndex>::const_iterator> {
      _indexSequence.cbegin(),
      _indexSequence.cend()
    };
  }

  size_t GetHashCode() const noexcept final;

  bool operator==(const PointerAssignment &rhs) const noexcept final;

private:
  std::vector<PointerIndex> _indexSequence;
};

class PointerAssignedPointee : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::AssignedPointee;
  }

  explicit PointerAssignedPointee(Pointer *pointer) noexcept
    : PointerAssignment {PointerAssignmentKind::AssignedPointee, pointer }
  { }
};

class PointeeAssignedPointer : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::PointeeAssigned;
  }

  explicit PointeeAssignedPointer(Pointer *pointer) noexcept
    : PointerAssignment { PointerAssignmentKind::PointeeAssigned, pointer }
  { }
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

  void AssignedPointer(Pointer *pointer) noexcept {
    AssignedElementPtr(pointer, { PointerIndex { 0 } });
  }

  void AssignedElementPtr(Pointer *pointer, std::vector<PointerIndex> indexSequence) noexcept;

  void AssignedPointee(Pointer *pointer) noexcept;

  void PointeeAssigned(Pointer *pointer) noexcept;

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
  std::unordered_set<PointerAssignedElementPtr, details::PolymorphicHasher<PointerAssignedElementPtr>> _assignedElementPtr;
  std::unordered_set<PointerAssignedPointee, details::PolymorphicHasher<PointerAssignedPointee>> _assignedPointee;
  std::unordered_set<PointeeAssignedPointer, details::PolymorphicHasher<PointeeAssignedPointer>> _pointeeAssigned;
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
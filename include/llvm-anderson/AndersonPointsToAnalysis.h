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

/**
 * Different kinds of pointer assignment statements.
 */
enum class PointerAssignmentKind {
  /**
   * Pointer assignment statement of the form `p = &q[...]`.
   *
   * Note that `p = q` can be regarded as a special case of this pointer assignment form since it's equivalent to
   * `p = &q[0]`.
   */
  AssignedElementPtr,

  /**
   * Pointer assignment statement of the form `p = *q`.
   */
  AssignedPointee,

  /**
   * Pointer assignment statement of the form `*p = q`.
   */
  PointeeAssigned,
};

/**
 * Base class of pointer assignment statements.
 */
class PointerAssignment {
public:
  /**
   * Get the kind of this pointer assignment statement.
   *
   * @return the kind of this pointer assignment statement.
   */
  PointerAssignmentKind kind() const noexcept { return _kind; }

  /**
   * Get the pointer operand on the right hand side of this pointer assignment statement.
   *
   * @return the pointer operand on the right hand side of this pointer assignment statement.
   */
  Pointer* pointer() noexcept { return _pointer; }

  /**
   * Get the pointer operand on the right hand side of this pointer assignment statement.
   *
   * @return the pointer operand on the right hand side of this pointer assignment statement.
   */
  const Pointer* pointer() const noexcept { return _pointer; }

  /**
   * Get the hash code of this PointerAssignment object.
   *
   * @return the hash code of this PointerAssignment object.
   */
  virtual size_t GetHashCode() const noexcept;

  virtual bool operator==(const PointerAssignment &rhs) const noexcept;

  virtual bool operator!=(const PointerAssignment &rhs) const noexcept {
    return !operator==(rhs);
  }

protected:
  /**
   * Construct a new PointerAssignment object.
   *
   * @param kind kind of the pointer assignment statement.
   * @param pointer the pointer operand on the right hand side of this assignment.
   */
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

/**
 * Represent a pointer index operand.
 */
class PointerIndex {
public:
  /**
   * A reserved pointer index value that represents the index is calculated dynamically.
   */
  constexpr static const size_t DynamicIndex = static_cast<size_t>(-1);

  /**
   * Construct a new PointerIndex object that represents a dynamically calculated pointer index operand.
   */
  explicit PointerIndex() noexcept
    : _index(DynamicIndex)
  { }

  /**
   * Construct a new PointerIndex object that represents a compile-time constant pointer index operand.
   *
   * @param index the constant index.
   */
  explicit PointerIndex(size_t index) noexcept
    : _index(index)
  { }

  /**
   * Get the compile-time constant pointer index.
   *
   * @return the compile-time constant pointer index. If this PointerIndex object does not represent a compile-time
   * constant pointer index, returns `DynamicIndex`.
   */
  size_t index() const noexcept { return _index; }

  /**
   * Determine whether this PointerIndex object represents a compile-time constant pointer index.
   *
   * @return whether this PointerIndex object represents a compile-time constant pointer index.
   */
  bool isConstant() const noexcept { return _index != DynamicIndex; }

  /**
   * Determine whether this PointerIndex object represents a dynamically computed pointer index.
   *
   * @return whether this PointerIndex object represents a dynamically computed pointer index.
   */
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

/**
 * Represents a pointer assignment statement of the form `p = &q[...]`.
 */
class PointerAssignedElementPtr : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::AssignedElementPtr;
  }

  /**
   * Construct a new PointerAssignedElementPtr object.
   *
   * @param pointer the pointer operand on the right hand side of the pointer assignment statement.
   * @param indexSequence the sequence of pointer index.
   */
  explicit PointerAssignedElementPtr(Pointer *pointer, std::vector<PointerIndex> indexSequence) noexcept
    : PointerAssignment {PointerAssignmentKind::AssignedElementPtr, pointer },
      _indexSequence(std::move(indexSequence))
  { }

  /**
   * Get an iterator range that contains the sequence of pointer indexes in this pointer assignment statement.
   *
   * @return an iterator range that contains the sequence of pointer indexes in this pointer assignment statement.
   */
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

/**
 * Represent a pointer assignment statement of the form `p = *q`.
 */
class PointerAssignedPointee : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::AssignedPointee;
  }

  /**
   * Construct a new PointerAssignedPointee object.
   *
   * @param pointer the pointer operand on the right hand side of the pointer assignment statement.
   */
  explicit PointerAssignedPointee(Pointer *pointer) noexcept
    : PointerAssignment {PointerAssignmentKind::AssignedPointee, pointer }
  { }
};

/**
 * Represent a pointer assignment statement of the form `*p = q`.
 */
class PointeeAssignedPointer : public PointerAssignment {
public:
  static bool classof(const PointerAssignment *obj) noexcept {
    return obj->kind() == PointerAssignmentKind::PointeeAssigned;
  }

  /**
   * Construct a new PointeeAssignedPointer object.
   *
   * @param pointer the pointer operand on the right hand side of the pointer assignment statement.
   */
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

/**
 * Represent a possible pointee of some pointer.
 */
class Pointee {
public:
  /**
   * Construct a new Pointee object.
   *
   * @param node the location of the pointee in the value tree.
   */
  explicit Pointee(ValueTreeNode &node) noexcept;

  NON_COPIABLE_NON_MOVABLE(Pointee)

  /**
   * Get the location of this pointee in the value tree.
   *
   * @return the location of this pointee in the value tree.
   */
  ValueTreeNode* node() noexcept;

  /**
   * Get the location of this pointee in the value tree.
   *
   * @return the location of this pointee in the value tree.
   */
  const ValueTreeNode* node() const noexcept;

  /**
   * Determine whether this pointee is a pointer.
   *
   * @return whether this pointee is a pointer.
   */
  bool isPointer() const noexcept;

  /**
   * Determine whether this pointee is defined outside of the current module.
   *
   * @return whether this pointee is defined outside of the current module.
   */
  bool isExternal() const noexcept;

private:
  ValueTreeNode &_node;
};

/**
 * Represent a pointer.
 */
class Pointer : public Pointee {
public:
  static bool classof(const Pointee *obj) noexcept;

  /**
   * Construct a new Pointer object.
   *
   * @param node the location of the pointer in the value tree.
   */
  explicit Pointer(ValueTreeNode &node) noexcept;

  NON_COPIABLE_NON_MOVABLE(Pointer)

  /**
   * Specify that this pointer is assigned to the specified pointer somewhere in the program.
   *
   * @param pointer the pointer on the right hand side of the pointer assignment.
   */
  void AssignedPointer(Pointer *pointer) noexcept {
    AssignedElementPtr(pointer, { PointerIndex { 0 } });
  }

  /**
   * Specify that this pointer is assigned to the address of some element in the pointee of the specified pointer, with
   * the specified pointer index sequence.
   *
   * @param pointer the pointer on the right hand side of the pointer assignment.
   * @param indexSequence the pointer index sequence.
   */
  void AssignedElementPtr(Pointer *pointer, std::vector<PointerIndex> indexSequence) noexcept;

  /**
   * Specify that this pointer is assigned to the pointee of the specified pointer.
   *
   * @param pointer the pointer on the right hand side of the pointer assignment.
   */
  void AssignedPointee(Pointer *pointer) noexcept;

  /**
   * Specify that the pointee of this pointer is assigned to the specified pointer.
   *
   * @param pointer the pointer on the right hand side of the pointer assignment.
   */
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

/**
 * A node in the value tree.
 */
class ValueTreeNode {
public:
  /**
   * Construct a new ValueTreeNode object that represents the specified value.
   *
   * @param value the `llvm::Value` of the new node.
   */
  explicit ValueTreeNode(const llvm::Value *value) noexcept;

  /**
   * Construct a new ValueTreeNode object that represents the sub-object of the specified parent value.
   *
   * @param type type of this sub-object.
   * @param parent ValueTreeNode that represents the parent value.
   * @param offset the offset of this sub-object within the parent value.
   */
  explicit ValueTreeNode(const llvm::Type *type, ValueTreeNode *parent, size_t offset) noexcept;

  NON_COPIABLE_NON_MOVABLE(ValueTreeNode)

  /**
   * Get the type of this value.
   *
   * @return the type of this value.
   */
  const llvm::Type *type() const noexcept;

  /**
   * Get the value represented by this node as `llvm::Value` object.
   *
   * @return the value represented by this node as `llvm::Value` object. If this node represents a sub-object of some
   * parent value, return nullptr.
   */
  const llvm::Value *value() const noexcept;

  /**
   * Get the parent node of this node.
   *
   * @return the parent node of this node. If this node does not represent a sub-object of some parent value, return
   * nullptr.
   */
  ValueTreeNode* parent() const noexcept;

  /**
   * Get the offset of the sub-object within the parent object.
   *
   * @return the offset of the sub-object within the parent object. The return value is always 0 if this node does not
   * represent a sub-object.
   */
  size_t offset() const noexcept;

  /**
   * Get the Pointee object connected to this node.
   *
   * @return the Pointee object connected to this node.
   */
  Pointee* pointee() noexcept;

  /**
   * Get the Pointee object connected to this node.
   *
   * @return the Pointee object connected to this node.
   */
  const Pointee* pointee() const noexcept;

  /**
   * Determine whether this node represent a root value, i.e. this value is not a sub-object.
   *
   * @return whether this node represent a root value.
   */
  bool isRoot() const noexcept;

  /**
   * Determine whether the value represented by this node is in global scope.
   *
   * @return whether the value represented by this node is in global scope.
   */
  bool isGlobal() const noexcept;

  /**
   * Determine whether the value represented by this node is defined outside of the current module.
   *
   * @return whether the value represented by this node is defined outside of the current module.
   */
  bool isExternal() const noexcept;

  /**
   * Determine whether the value represented by this node is a pointer.
   *
   * @return whether the value represented by this node is a pointer.
   */
  bool isPointer() const noexcept;

  /**
   * Get the Pointer object connected to this node.
   *
   * If this node does not represent a pointer value, this function triggers an assertion failure.
   *
   * @return the pointer object connected to this node.
   */
  Pointer* pointer() noexcept;

  /**
   * Get the Pointer object connected to this node.
   *
   * If this node does not represent a pointer value, this function triggers an assertion failure.
   *
   * @return the pointer object connected to this node.
   */
  const Pointer* pointer() const noexcept;

  /**
   * Determine whether this node has any child nodes.
   *
   * @return whether this node has any child nodes.
   */
  bool hasChildren() const noexcept;

  /**
   * Get the number of child nodes under this node.
   *
   * @return the number of child nodes under this node.
   */
  size_t GetNumChildren() const noexcept;

  /**
   * Get the child node at the specified index.
   *
   * If the index is out of range, this function triggers an assertion failure.
   *
   * @param index the index.
   * @return the child node at the specified index.
   */
  ValueTreeNode* GetChild(size_t index) noexcept;

  /**
   * Get the child node at the specified index.
   *
   * If the index is out of range, this function triggers an assertion failure.
   *
   * @param index the index.
   * @return the child node at the specified index.
   */
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

/**
 * The value tree that represents the value hierarchy of a program.
 */
class ValueTree {
public:
  /**
   * Construct a new ValueTree object.
   *
   * This constructor builds all possible value trees for each rooted value in the specified module.
   *
   * @param module the LLVM module.
   */
  explicit ValueTree(const llvm::Module &module) noexcept;

  /**
   * Get the value tree node corresponding to the specified rooted value.
   *
   * @param value the rooted value.
   * @return the value tree node corresponding to the specified rooted value. If the specified value is not a valid root
   * of a value tree, return nullptr.
   */
  ValueTreeNode* GetNode(const llvm::Value *value) noexcept;

  /**
   * Get the value tree node corresponding to the specified rooted value.
   *
   * @param value the rooted value.
   * @return the value tree node corresponding to the specified rooted value. If the specified value is not a valid root
   * of a value tree, return nullptr.
   */
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
//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef P2A_POINTER_ASSIGNMENT_DSL_H
#define P2A_POINTER_ASSIGNMENT_DSL_H

#include <vector>

#include <llvm/IR/Value.h>

namespace p2a {

/**
 * Kinds of pointer assignments.
 */
enum class PointerAssignmentKind {
  /**
   * Pointer assignments with the form of `a = &b`.
   */
  PointerAssignedAddress,

  /**
   * Pointer assignment with the form of `a = *b`.
   */
  PointerAssignedPointerDeref,

  /**
   * Pointer assignment with the form of `*a = b`.
   */
  PointerDerefAssignedPointer,
};

/**
 * Base class of pointer assignment DSL.
 */
class PointerAssignment {
public:
  virtual ~PointerAssignment() noexcept;

  /**
   * Get the kind of the current pointer assignment.
   *
   * @return the kind of the current pointer assignment.
   */
  PointerAssignmentKind kind() const noexcept;

protected:
  /**
   * Construct a new PointerAssignment object.
   *
   * @param kind kind of the pointer assignment.
   * @param lhs the lhs operand of the assignment.
   * @param rhs the rhs operand of the assignment.
   */
  explicit PointerAssignment(PointerAssignmentKind kind, const llvm::Value *lhs, const llvm::Value *rhs) noexcept;

  /**
   * Get the lhs operand of the assignment.
   *
   * @return the lhs operand of the assignment.
   */
  const llvm::Value* lhs() const noexcept;

  /**
   * Get the rhs operand of the assignment.
   *
   * @return the rhs operand of the assignment.
   */
  const llvm::Value* rhs() const noexcept;

private:
  PointerAssignmentKind _kind;
  const llvm::Value *_lhs;
  const llvm::Value *_rhs;
};

/**
 * Pointer assignment of the form `a = &b`.
 */
class PointerAssignedAddress : public PointerAssignment {
public:
  /**
   * Construct a new PointerAssignedAddress object.
   *
   * @param targetPointer the pointer being assigned to.
   * @param value the value whose address is being taken.
   */
  explicit PointerAssignedAddress(const llvm::Value *targetPointer, const llvm::Value *value) noexcept;

  /**
   * Get the pointer being assigned to.
   *
   * @return the pointer being assigned to.
   */
  const llvm::Value* targetPointer() const noexcept;

  /**
   * Get the value whose address is being taken.
   *
   * @return the value whose address is being taken.
   */
  const llvm::Value* value() const noexcept;

  static bool classof(const PointerAssignment *dsl) noexcept;
};

/**
 * An index in a pointer dereference sequence.
 */
class PointerDerefIndex {
public:
  /**
   * Construct a new PointerDerefIndex object that represents a runtime-computed index.
   */
  explicit PointerDerefIndex() noexcept;

  /**
   * Construct a new PointerDerefIndex object that represents a compile-time constant index.
   *
   * @param index the index.
   */
  explicit PointerDerefIndex(size_t index) noexcept;

  /**
   * Determine whether this PointerDerefIndex object represents a runtime-computed index.
   *
   * @return whether this PointerDerefIndex object represents a runtime-computed index.
   */
  bool isUnknown() const noexcept;

  /**
   * Determine whether this PointerDerefIndex object represents a compile-time constant index.
   *
   * @return whether this PointerDerefIndex object represents a compile-time constant index.
   */
  bool isConstant() const noexcept;

  /**
   * Get the compile-time constant dereference index.
   *
   * This function will trigger an assertion failure if this PointerDerefIndex does not represent a compile-time
   * constant index.
   *
   * @return the compile-time constant dereference index.
   */
  size_t index() const noexcept;

private:
  size_t _index;
};

/**
 * Pointer assignment of the form `a = &b[indexes...]`.
 */
class PointerAssignedPointerDeref : public PointerAssignment {
public:
  /**
   * Construct a new PointerAssignedPointerDeref object.
   *
   * @param targetPointer the pointer being assigned to.
   * @param sourcePointer the pointer that points to the pointer being assigned from.
   * @param indexes the indexes of the deref sequence.
   */
  explicit PointerAssignedPointerDeref(
      const llvm::Value *targetPointer,
      const llvm::Value *sourcePointer,
      std::vector<PointerDerefIndex> indexes) noexcept;

  /**
   * Get the pointer being assigned to.
   *
   * @return the pointer being assigned from.
   */
  const llvm::Value* targetPointer() const noexcept;

  /**
   * Get the pointer that points to the pointer being assigned from.
   *
   * @return the pointer that points to the pointer being assigned from.
   */
  const llvm::Value* sourcePointer() const noexcept;

  /**
   * Get the indexing sequence.
   *
   * @return the indexing sequence.
   */
  const std::vector<PointerDerefIndex>& indexes() const noexcept;

  static bool classof(const PointerAssignment *dsl) noexcept;

private:
  std::vector<PointerDerefIndex> _indexes;
};

/**
 * Pointer assignment of the form `*a = b`.
 */
class PointerDerefAssignedPointer : public PointerAssignment {
public:
  /**
   * Construct a new PointerDerefAssignedPointer object.
   *
   * @param targetPointerPointer the pointer to the pointer being assigned to.
   * @param sourcePointer the pointer being assigned from.
   */
  explicit PointerDerefAssignedPointer(const llvm::Value* targetPointerPointer, const llvm::Value* sourcePointer) noexcept;

  /**
   * Get the pointer to the pointer being assigned to.
   *
   * @return the pointer to the pointer being assigned to.
   */
  const llvm::Value* targetPointerPointer() const noexcept;

  /**
   * Get the pointer being assigned from.
   *
   * @return the pointer being assigned from.
   */
  const llvm::Value* sourcePointer() const noexcept;

  static bool classof(const PointerAssignment *dsl) noexcept;
};

} // namespace p2a

#endif // P2A_POINTER_ASSIGNMENT_DSL_H

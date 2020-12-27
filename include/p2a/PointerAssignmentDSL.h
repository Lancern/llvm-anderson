//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef P2A_POINTER_ASSIGNMENT_DSL_H
#define P2A_POINTER_ASSIGNMENT_DSL_H

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
   * Pointer assignments with the form of `a = b`.
   */
  PointerAssignedPointer,

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
 * Pointer assignment of the form `a = b`.
 */
class PointerAssignedPointer : public PointerAssignment {
public:
  /**
   * Construct a new `PointerAssignedPointer` object.
   *
   * @param targetPointer the pointer being assigned to.
   * @param sourcePointer the pointer being assigned from.
   */
  explicit PointerAssignedPointer(const llvm::Value* targetPointer, const llvm::Value* sourcePointer) noexcept;

  /**
   * Get the pointer being assigned to.
   *
   * @return the pointer being assigned to.
   */
  const llvm::Value* targetPointer() const noexcept;

  /**
   * Get the pointer being assigned from.
   *
   * @return the pointer being assigned from.
   */
  const llvm::Value* sourcePointer() const noexcept;

  static bool classof(const PointerAssignedPointer *dsl) noexcept;
};

/**
 * Pointer assignment of the form `a = *b`.
 */
class PointerAssignedPointerDeref : public PointerAssignment {
public:
  /**
   * Construct a new PointerAssignedPointerDeref object.
   *
   * @param targetPointer the pointer being assigned to.
   * @param sourcePointerPointer the pointer that points to the pointer being assigned from.
   */
  explicit PointerAssignedPointerDeref(const llvm::Value* targetPointer, const llvm::Value* sourcePointerPointer) noexcept;

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
  const llvm::Value* sourcePointerPointer() const noexcept;

  static bool classof(const PointerAssignedPointer *dsl) noexcept;
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

  static bool classof(const PointerAssignedPointer *dsl) noexcept;
};

} // namespace p2a

#endif // P2A_POINTER_ASSIGNMENT_DSL_H

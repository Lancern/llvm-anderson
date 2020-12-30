//
// Created by Sirui Mu on 2020/12/26.
//

#ifndef P2A_POINTS_TO_ANALYSIS_H
#define P2A_POINTS_TO_ANALYSIS_H

#include <cstddef>
#include <map>
#include <unordered_set>
#include <vector>

#include <llvm/ADT/iterator_range.h>
#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Pass.h>

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
public:
  using Iterator = typename std::unordered_set<const Pointee *>::const_iterator;

  /**
   * Construct a new PointeeSet object.
   *
   * @param type type of the pointee object.
   */
  explicit PointeeSet(const llvm::Type *type) noexcept;

  /**
   * Get the type of the pointees contained in this pointee set.
   *
   * @return the type of the pointees.
   */
  const llvm::Type* type() const noexcept;

  /**
   * Get the number of pointees contained in this PointeeSet object.
   *
   * @return the number of pointees contained in this PointeeSet object.
   */
  size_t size() const noexcept;

  /**
   * Get an iterator range into all contained pointees in this set.
   */
  llvm::iterator_range<Iterator> pointees() const noexcept;

  /**
   * Add the specified pointee object into this pointee set.
   *
   * @param pointee the pointee object.
   */
  void Add(const Pointee *pointee) noexcept;

  /**
   * Merge all pointee objects contained in this pointee set into the specified set.
   *
   * @param target the target pointee set.
   */
  void MergeTo(PointeeSet &target) const noexcept;

  /**
   * Merge all pointee objects from the specified pointee set.
   *
   * @param source the source pointee set.
   */
  void MergeFrom(const PointeeSet &source) noexcept;

private:
  const llvm::Type *_type;
  std::unordered_set<const Pointee *> _pointees;
}; // class PointeeSet

/**
 * Represent a pointer value somewhere in the module.
 */
class Pointer {
public:
  using Iterator = typename std::map<size_t, Pointer>::iterator;

  using ConstIterator = typename std::map<size_t, Pointer>::const_iterator;

  /**
   * Construct a new Pointer object that represents the specified pointer value.
   *
   * @param pointerValue the pointer value.
   */
  explicit Pointer(const llvm::Value *pointerValue) noexcept;

  /**
   * Construct a new Pointer object that represents a pointer value that is pointed to by the specified pointer value
   * with the specified offset.
   *
   * @param type the type of the pointer value.
   * @param parent the parent pointer value.
   * @param offset the offset relative to the parent pointer's pointee.
   */
  explicit Pointer(const llvm::Type *type, Pointer *parent, size_t offset) noexcept;

  /**
   * Determine whether this pointer value is a root pointer, i.e. a pointer that have been materialized into a
   * `llvm::Value`.
   *
   * @return whether this pointer value is a root pointer.
   */
  bool isRoot() const noexcept;

  /**
   * Get the type of the pointer itself.
   *
   * @return the type of the pointer itself.
   */
  const llvm::Type* pointerType() const noexcept;

  /**
   * Get the type of the pointee object.
   *
   * @return the type of the pointee object.
   */
  const llvm::Type* pointeeType() const noexcept;

  /**
   * Get the `llvm::Value` object that represents the materialized pointer.
   *
   * @return the `llvm::Value` object that represents the materialized pointer. If this pointer value is not a root
   * pointer, returns nullptr.
   */
  const llvm::Value* pointerValue() const noexcept;

  /**
   * Get the parent pointer.
   *
   * @return the parent pointer.
   */
  const Pointer* parent() const noexcept;

  /**
   * Get the offset of this pointer within the pointee object of the parent pointer.
   *
   * @return the offset of this pointer within the pointee object of the parent pointer.
   */
  size_t offset() const noexcept;

  /**
   * Determine whether the pointee is a pointer.
   *
   * @return whether the pointee is a pointer.
   */
  bool isPointeePointer() const noexcept;

  /**
   * Determine whether the pointee is a pointer array.
   *
   * @return whether the pointee is a pointer array.
   */
  bool isPointeePointerArray() const noexcept;

  /**
   * Determine whether the pointee is a struct.
   *
   * @return whether the pointee is a struct.
   */
  bool isPointeeStruct() const noexcept;

  /**
   * Get the root pointer from which this pointer can be accessed by a sequence of dereference operations.
   *
   * @return the root pointer.
   */
  Pointer* GetRootPointer() noexcept;

  /**
   * Get the root pointer from which this pointer can be accessed by a sequence of dereference operations.
   *
   * @return the root pointer.
   */
  const Pointer* GetRootPointer() const noexcept;

  /**
   * Determine whether this pointer might refers to any pointers.
   *
   * This function effectively determines whether the pointee is either:
   * - a pointer;
   * - a pointer array;
   * - a struct.
   *
   * @return whether this pointer might refers to any pointers.
   */
  bool hasChildren() const noexcept;

  /**
   * Get the child pointer that can be acquired by a single dereference operation with the specified index.
   *
   * @param index the dereference index.
   * @return the pointer at the specified dereference index.
   */
  Pointer& GetChild(size_t index) noexcept;

  /**
   * Get the child pointer that can be acquired by a single dereference operation with the specified index.
   *
   * @param index the dereference index.
   * @return the pointer at the specified dereference index.
   */
  const Pointer& GetChild(size_t index) const noexcept;

  /**
   * Get all child pointers.
   *
   * @return an iterator range that contains all child pointers.
   */
  llvm::iterator_range<Iterator> children() noexcept;

  /**
   * Get all child pointers.
   *
   * @return an iterator range that contains all child pointers.
   */
  llvm::iterator_range<ConstIterator> children() const noexcept;

private:
  const llvm::Type *_pointerType;
  const llvm::Value *_value;
  Pointer *_parent;
  size_t _offset;
  std::map<size_t, Pointer> _children;

  void InitializeChildren() noexcept;
};

/**
 * Implementation of Anderson points-to analysis algorithm as a LLVM module pass.
 */
class AndersonPointsToAnalysis : public llvm::ModulePass {
  // TODO: Implement class AndersonPointsToAnalysis.
};

} // namespace p2a

#endif // P2A_POINTS_TO_ANALYSIS_H

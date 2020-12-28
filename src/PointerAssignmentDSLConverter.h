//
// Created by Sirui Mu on 2020/12/27.
//

#ifndef P2A_SRC_POINTER_ASSIGNMENT_DSL_CONVERTER_H
#define P2A_SRC_POINTER_ASSIGNMENT_DSL_CONVERTER_H

#include <memory>
#include <vector>

#include <llvm/IR/Instruction.h>

#include "p2a/PointerAssignmentDSL.h"

namespace p2a {

/**
 * Get the list of pointer assignment DSLs from the given LLVM instruction.
 *
 * @param instr the LLVM instruction to parse.
 * @return the list of pointer assignment DSLs extracted from the given LLVM instruction.
 */
std::vector<std::unique_ptr<PointerAssignment>> GetPointerAssignmentDSL(const llvm::Instruction &instr) noexcept;

} // namespace p2a

#endif // P2A_SRC_POINTER_ASSIGNMENT_DSL_CONVERTER_H

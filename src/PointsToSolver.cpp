//
// Created by Sirui Mu on 2021/1/1.
//

#include "PointsToSolver.h"

#include <list>

namespace llvm {

namespace anderson {

void PointsToSolver::Solve() noexcept {
  AddTrivialPointerAssignments();
  RelaxPointsToConstraints();

  auto converged = false;
  auto visitor = [&converged](ValueTreeNode &node) noexcept -> bool {
    converged = true;
    if (!RelaxNode(node)) {
      converged = false;
    }
    return true;
  };

  while (!converged) {
    _valueTree->Visit(visitor);
  }
}

void PointsToSolver::AddTrivialPointerAssignments() const noexcept {
  // Add points-to constraints from global variables to corresponding global memory values.
  for (const auto &globalVariable : _module.globals()) {
    auto globalVariableNode = _valueTree->GetValueNode(&globalVariable);
    auto globalVariableMemoryNode = _valueTree->GetGlobalMemoryNode(&globalVariable);
    assert(globalVariableNode->isPointer());
    globalVariableNode->pointer()->AssignedAddressOf(globalVariableMemoryNode->pointee());
  }

  // Add points-to constraints from exported function arguments to corresponding argument memory values.
  for (const auto &function : _module) {
    if (!llvm::GlobalValue::isExternalLinkage(function.getLinkage())) {
      continue;
    }
    for (const auto &arg : function.args()) {
      if (!arg.getType()->isPointerTy()) {
        continue;
      }
      auto argNode = _valueTree->GetValueNode(&arg);
      auto argMemoryNode = _valueTree->GetArgumentMemoryNode(&arg);
      assert(argNode->isPointer());
      argNode->pointer()->AssignedAddressOf(argMemoryNode->pointee());
    }
  }

  // Add points-to constraints from `alloca` pointers to corresponding stack memory values.
  for (const auto &function : _module) {
    for (const auto &bb : function) {
      for (const auto &inst : bb) {
        auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(&inst);
        if (!allocaInst) {
          continue;
        }

        auto ptrValue = static_cast<const llvm::Value *>(&inst);
        auto ptrNode = _valueTree->GetValueNode(ptrValue);
        auto stackMemoryNode = _valueTree->GetAllocaMemoryNode(allocaInst);
        assert(ptrNode->isPointer());

        ptrNode->pointer()->AssignedAddressOf(stackMemoryNode->pointee());
      }
    }
  }
}

void PointsToSolver::RelaxPointsToConstraints() const noexcept {
  auto visitor = [](ValueTreeNode &node) noexcept -> bool {
    if (!node.isPointer()) {
      return true;
    }

    auto pointer = node.pointer();
    for (auto &e : pointer->assigned_address_of()) {
      pointer->GetPointeeSet().insert(e.pointee());
    }

    return true;
  };

  _valueTree->Visit(visitor);
}

bool PointsToSolver::RelaxNode(ValueTreeNode &node) noexcept {
  if (!node.isPointer()) {
    return true;
  }

  auto nodeConverged = true;
  auto pointer = node.pointer();

  for (auto &e : pointer->assigned_pointee()) {
    if (!RelaxAssignedPointee(pointer, e)) {
      nodeConverged = false;
    }
  }

  for (auto &e : pointer->assigned_element_ptr()) {
    if (!RelaxAssignedElementPtr(pointer, e)) {
      nodeConverged = false;
    }
  }

  for (auto &e : pointer->pointee_assigned()) {
    if (!RelaxPointeeAssigned(pointer, e)) {
      nodeConverged = false;
    }
  }

  return nodeConverged;
}

bool PointsToSolver::RelaxAssignedPointee(Pointer *pointer, const PointerAssignedPointee &edge) noexcept {
  auto converged = true;

  auto rhsPointer = edge.pointer();
  for (auto pointee : rhsPointer->GetPointeeSet()) {
    assert(pointee->isPointer());
    if (pointer->AssignedPointer(pointee->pointer())) {
      converged = false;
    }
  }

  return converged;
}

bool PointsToSolver::RelaxAssignedElementPtr(Pointer *pointer, const PointerAssignedElementPtr &edge) noexcept {
  auto rhsPointer = edge.pointer();

  std::list<ValueTreeNode *> elementNodes;
  for (auto pointee : rhsPointer->GetPointeeSet()) {
    elementNodes.push_back(pointee->node());
  }

  for (const auto &index : edge.index_sequence()) {
    if (index.isConstant()) {
      auto indexValue = index.index();
      for (auto it = elementNodes.begin(); it != elementNodes.end(); ) {
        auto &node = *it;
        if (node->type()->isArrayTy() || node->type()->isStructTy()) {
          node = node->GetChild(indexValue);
        } else {
          it = elementNodes.erase(it);
          continue;
        }
        ++it;
      }
    } else {
      std::list<ValueTreeNode *> nextElementNodes;
      for (auto node : elementNodes) {
        if (!node->type()->isArrayTy() && !node->type()->isStructTy()) {
          continue;
        }
        for (size_t i = 0; i < node->GetNumChildren(); ++i) {
          nextElementNodes.push_back(node->GetChild(i));
        }
      }
      elementNodes.swap(nextElementNodes);
    }
  }

  auto converged = true;
  for (auto node : elementNodes) {
    if (!node->isPointer()) {
      continue;
    }
    if (pointer->GetPointeeSet().MergeFrom(node->pointer()->GetPointeeSet())) {
      converged = false;
    }
  }

  return converged;
}

bool PointsToSolver::RelaxPointeeAssigned(Pointer *pointer, const PointeeAssignedPointer &edge) noexcept {
  auto converged = true;

  auto rhsPointer = edge.pointer();
  for (auto pointee : pointer->GetPointeeSet()) {
    assert(pointee->isPointer());
    if (pointee->pointer()->AssignedPointer(rhsPointer)) {
      converged = false;
    }
  }

  return converged;
}

} // namespace anderson

} // namespace llvm

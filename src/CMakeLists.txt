add_library(LLVMAnderson MODULE
        "${LLVM_ANDERSON_INCLUDE_DIR}/llvm-anderson/AndersonPointsToAnalysis.h"
        AndersonPointsToAnalysis.cpp
        PointerAssignment.cpp
        PointsToSolver.cpp
        PointsToSolver.h
        ValueTree.cpp
        ValueTreeNode.cpp)

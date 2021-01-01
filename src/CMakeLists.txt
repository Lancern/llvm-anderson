add_library(LLVMAnderson MODULE
        "${LLVM_ANDERSON_INCLUDE_DIR}/llvm-anderson/AndersonPointsToAnalysis.h"
        AndersonPointsToAnalysis.cpp
        Pointee.cpp
        PointerAssignment.cpp
        ValueTree.cpp
        ValueTreeNode.cpp)

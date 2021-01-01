add_library(LLVMAnderson MODULE
        "${LLVM_ANDERSON_INCLUDE_DIR}/llvm-anderson/AndersonPointsToAnalysis.h"
        AndersonPointsToAnalysis.cpp
        Pointee.cpp
        PointeeSet.cpp
        Pointer.cpp
        ValueTree.cpp
        ValueTreeNode.cpp)

add_library(LLVMAnderson MODULE
        "${LLVM_ANDERSON_INCLUDE_DIR}/llvm-anderson/PointsToAnalysis.h"
        Pointee.cpp
        PointeeSet.cpp
        Pointer.cpp)

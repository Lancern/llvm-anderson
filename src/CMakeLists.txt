add_library(p2a MODULE
        "${P2A_INCLUDE_DIR}/p2a/PointsToAnalysis.h"
        AndersonPointsToAnalysis.cpp
        FlowInsensitivePointsToAnalysis.cpp
        Pointee.cpp
        PointerAssignmentDSL.cpp
        PointsToAnalysis.cpp
        PointsToSet.cpp
        SteensgaardPointsToAnalysis.cpp)

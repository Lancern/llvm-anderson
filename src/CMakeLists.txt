add_library(p2a MODULE
        "${P2A_INCLUDE_DIR}/p2a/PointsToAnalysis.h"
        AndersonPointsToAnalysis.cpp
        FlowInsensitivePointsToAnalysis.cpp
        PointerAssignmentDSL.cpp
        PointerAssignmentDSLConverter.cpp
        PointerAssignmentDSLConverter.h
        PointsToAnalysis.cpp
        SteensgaardPointsToAnalysis.cpp)

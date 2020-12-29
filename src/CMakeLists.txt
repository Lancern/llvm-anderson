add_library(p2a MODULE
        "${P2A_INCLUDE_DIR/p2a/PointerAssignmentDSL.h}"
        "${P2A_INCLUDE_DIR}/p2a/PointsToAnalysis.h"
        Anderson/AndersonPointsToAnalysis.cpp
        Anderson/AndersonPointsToGraph.h
        FlowInsensitivePointsToAnalysis.cpp
        Pointee.cpp
        PointeeSet.cpp
        PointerAssignmentDSL.cpp
        PointerAssignmentDSLConverter.cpp
        PointerAssignmentDSLConverter.h
        PointsToAnalysis.cpp
        Steensgaard/SteensgaardPointsToAnalysis.cpp)

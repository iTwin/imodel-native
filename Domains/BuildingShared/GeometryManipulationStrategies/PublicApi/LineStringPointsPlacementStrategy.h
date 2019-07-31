/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct LineStringPointsPlacementStrategy : public LineStringPlacementStrategy
    {
    DEFINE_T_SUPER(LineStringPlacementStrategy)

    private:
        LineStringPointsPlacementStrategy() : T_Super() {}

    public:
        static LineStringPointsPlacementStrategyPtr Create() { return new LineStringPointsPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE
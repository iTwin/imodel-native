/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchPhysCreater/ArchPhysCreater/Geometry.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "stdafx.h"      


struct GeometricTools
    {
    public:
        static  BentleyStatus CreateDoorGeometry(ArchitecturalPhysical::DoorPtr door, BuildingPhysical::BuildingPhysicalModelR model );
        static  BentleyStatus CreateWindowGeometry(ArchitecturalPhysical::WindowPtr window, BuildingPhysical::BuildingPhysicalModelR model);
        static  BentleyStatus CreateFrameGeometry(Dgn::GeometryBuilderPtr builder, BuildingPhysical::BuildingPhysicalModelR model, double frameDepth, double frameWidth, double height, double width, bool fullFrame);


    };


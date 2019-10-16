/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ScalableMeshWrapper.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>

struct ScalableMeshWrapper
{
public:
    static void RegisterDomain();
    static void AddTerrainClassifiers(BentleyApi::Dgn::DgnDbR dgnDb, BentleyApi::Dgn::DgnModelId const& clippingsModelId);
}; // ScalableMeshWrapper
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>

struct ScalableMeshWrapper
{
public:
    static void RegisterDomain();
    static void AddTerrainClassifiers(BentleyApi::Dgn::DgnDbR dgnDb, BentleyApi::Dgn::DgnModelId const& clippingsModelId);
}; // ScalableMeshWrapper
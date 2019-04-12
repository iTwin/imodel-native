/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/SpatialCompositionDomain.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>
#include "SpatialCompositionMacros.h"


BEGIN_SPATIALCOMPOSITION_NAMESPACE

//=======================================================================================
//! The DgnDomain for the building schema.
//! @private
//=======================================================================================
struct SpatialCompositionDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(SpatialCompositionDomain, EXPORT_ATTRIBUTE)

private:
    WCharCP _GetSchemaRelativePath () const override { return L"ECSchemas/Domain/SpatialComposition.ecschema.xml"; }

public:
    SpatialCompositionDomain ();
    ~SpatialCompositionDomain () {};
};

END_SPATIALCOMPOSITION_NAMESPACE

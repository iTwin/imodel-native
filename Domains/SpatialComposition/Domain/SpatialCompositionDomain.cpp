/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/SpatialCompositionDomain.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <SpatialComposition/Domain/SpatialCompositionDomain.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_SPATIALCOMPOSITION_NAMESPACE

//=======================================================================================
//  Handler definitions
//=======================================================================================
DOMAIN_DEFINE_MEMBERS(SpatialCompositionDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                 02/2019
//---------------------------------------------------------------------------------------
SpatialCompositionDomain::SpatialCompositionDomain () : DgnDomain(SPATIALCOMPOSITION_SCHEMA_NAME, "SpatialComposition Domain", 1)
    {
    }
    
END_SPATIALCOMPOSITION_NAMESPACE

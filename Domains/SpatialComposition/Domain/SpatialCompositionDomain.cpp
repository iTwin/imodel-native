/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

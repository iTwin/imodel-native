/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ScalableMeshSchema/ScalableMeshDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>
#include <ScalableMeshSchema\ExportMacros.h>
#include <ScalableMeshSchema\ScalableMeshSchemaCommon.h>

BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Mathieu.St-Pierre   02/16
//=======================================================================================
struct ScalableMeshDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ScalableMeshDomain, SCALABLEMESH_SCHEMA_EXPORT)

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    ScalableMeshDomain();
};

END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE

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
#include <ScalableMeshSchema/ScalableMeshSchemaCommon.h>

BEGIN_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct ScalableMeshDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(ScalableMeshDomain, SCALABLEMESHSCHEMA_EXPORT)

    protected:
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

    public:
        ScalableMeshDomain();
    };

END_BENTLEY_SCALABLEMESHSCHEMA_NAMESPACE

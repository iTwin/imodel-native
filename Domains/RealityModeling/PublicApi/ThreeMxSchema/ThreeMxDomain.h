/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ThreeMxSchema/ThreeMxDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnCore/DgnDomain.h>
#include <ThreeMxSchema/ThreeMxSchemaCommon.h>

BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE

//=======================================================================================
//! The DgnDomain for the point cloud schema.
// @bsiclass                                                    Eric.Paquet     05/15
//=======================================================================================
struct ThreeMxDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ThreeMxDomain, THREEMXSCHEMA_EXPORT)

    static  void        InitializeApi();

protected:
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

public:
    ThreeMxDomain();
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: Domain/PublicApi/BuildingSpatialDomain.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>
#include "BuildingSpatialMacros.h"


BEGIN_BUILDINGSPATIAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the building schema.
//! @private
//=======================================================================================
struct BuildingSpatialDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(BuildingSpatialDomain, EXPORT_ATTRIBUTE)

private:
    WCharCP _GetSchemaRelativePath () const override { return L"ECSchemas/Domain/BuildingSpatial.ecschema.xml"; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;

    void InsertDomainAuthorities (Dgn::DgnDbR) const;
    static void InsertCodeSpec (Dgn::DgnDbR, Utf8CP);

public:
    BuildingSpatialDomain ();
    ~BuildingSpatialDomain () {};
};

END_BUILDINGSPATIAL_NAMESPACE

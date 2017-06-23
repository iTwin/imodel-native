/*--------------------------------------------------------------------------------------+
|
|     $Source: ConstraintModel/PublicApi/ConstraintModelDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDomain.h>
#include "ConstraintModelMacros.h"

BEGIN_CONSTRAINTMODEL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the building schema.
//! @private
//=======================================================================================
struct ConstraintModelDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ConstraintModelDomain, CONSTRAINTMODEL_EXPORT)

private:
    WCharCP _GetSchemaRelativePath () const override { return CONSTRAINTMODEL_SCHEMA_PATH; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

//__PUBLISH_SECTION_END__

//__PUBLISH_SECTION_START__
public:
    ConstraintModelDomain ();
    ~ConstraintModelDomain ();

    CONSTRAINTMODEL_EXPORT static bool EnsureConstraintModelECSchemaIsLoaded(Dgn::DgnDbR db);
};

END_CONSTRAINTMODEL_NAMESPACE

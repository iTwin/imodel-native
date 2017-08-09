/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralCommon/PublicAPI/StructuralCommon/StructuralCommonDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralCommonDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Structural Common schema.
//! @ingroup GROUP_StructuralCommon
//=======================================================================================
struct StructuralCommonDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralCommonDomain, STRUCTURAL_DOMAIN_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_COMMON_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

    public:
        StructuralCommonDomain();
        STRUCTURAL_DOMAIN_EXPORT static Dgn::CodeSpecId QueryStructuralCommonCodeSpecId(Dgn::DgnDbCR dgndb);
        STRUCTURAL_DOMAIN_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };


END_BENTLEY_STRUCTURAL_NAMESPACE


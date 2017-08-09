/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralPhysicalDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "StructuralPhysicalDefinitions.h"

BEGIN_BENTLEY_STRUCTURAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Structural Physical schema.
//! @ingroup GROUP_StructuralPhyscal
//=======================================================================================
struct StructuralPhysicalDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(StructuralPhysicalDomain, STRUCTURAL_EXPORT)

    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

    public:
        StructuralPhysicalDomain();
        STRUCTURAL_EXPORT static Dgn::CodeSpecId QueryStructuralPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
        STRUCTURAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };


END_BENTLEY_STRUCTURAL_NAMESPACE


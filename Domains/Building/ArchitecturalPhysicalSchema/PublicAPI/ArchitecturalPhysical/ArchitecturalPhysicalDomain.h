/*--------------------------------------------------------------------------------------+
|
|     $Source: ArchitecturalPhysicalSchema/PublicAPI/ArchitecturalPhysical/ArchitecturalPhysicalDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "ArchitecturalPhysicalDefinitions.h"

BEGIN_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Architectural Physical schema.
//! @ingroup GROUP_ArchitecturalPhyscal
//=======================================================================================
struct ArchitecturalPhysicalDomain : Dgn::DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(ArchitecturalPhysicalDomain, ARCHITECTURAL_PHYSICAL_EXPORT)
protected:
    WCharCP _GetSchemaRelativePath() const override { return BENTLEY_ARCHITECTURAL_PHYSICAL_SCHEMA_PATH; }
    virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
    virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

public:
    ArchitecturalPhysicalDomain();
    ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::CodeSpecId QueryArchitecturalPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
    ARCHITECTURAL_PHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
};

END_BENTLEY_ARCHITECTURAL_PHYSICAL_NAMESPACE



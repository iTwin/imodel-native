/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingPhysicalSchema/PublicAPI/BuildingPhysical/BuildingPhysicalDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "BuildingPhysicalDefinitions.h"

BEGIN_BENTLEY_BUILDING_PHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Architectural Physical schema.
//! @ingroup GROUP_ArchitecturalPhyscal
//=======================================================================================
struct BuildingPhysicalDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(BuildingPhysicalDomain, BUILDING_PHYSICAL_EXPORT)
    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_BUILDING_PHYSICAL_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

    public:
        BuildingPhysicalDomain();
        BUILDING_PHYSICAL_EXPORT static Dgn::CodeSpecId QueryBuildingPhysicalCodeSpecId(Dgn::DgnDbCR dgndb);
        BUILDING_PHYSICAL_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

END_BENTLEY_BUILDING_PHYSICAL_NAMESPACE


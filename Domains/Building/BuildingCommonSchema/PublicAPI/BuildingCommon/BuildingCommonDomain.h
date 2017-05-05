/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingCommonSchema/PublicAPI/BuildingCommon/BuildingCommonDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "BuildingCommonDefinitions.h"

BEGIN_BENTLEY_BUILDING_COMMON_NAMESPACE

//=======================================================================================
//! The DgnDomain for the Architectural Physical schema.
//! @ingroup GROUP_ArchitecturalPhyscal
//=======================================================================================
struct BuildingCommonDomain : Dgn::DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(BuildingCommonDomain, BUILDING_COMMON_EXPORT)
    protected:
        WCharCP _GetSchemaRelativePath() const override { return BENTLEY_BUILDING_COMMON_SCHEMA_PATH; }
        virtual void _OnSchemaImported(Dgn::DgnDbR) const override;
        virtual void _OnDgnDbOpened(Dgn::DgnDbR) const override;

    public:
        BuildingCommonDomain();
        BUILDING_COMMON_EXPORT static Dgn::CodeSpecId QueryBuildingCommonCodeSpecId(Dgn::DgnDbCR dgndb);
        BUILDING_COMMON_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR nameSpace, Utf8StringCR value);
    };

END_BENTLEY_BUILDING_COMMON_NAMESPACE


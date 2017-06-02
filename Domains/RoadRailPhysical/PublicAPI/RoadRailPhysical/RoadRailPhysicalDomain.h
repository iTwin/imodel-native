/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailPhysicalDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadRailPhysical schema.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailPhysicalDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(RoadRailPhysicalDomain, ROADRAILPHYSICAL_EXPORT)

protected:
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    RoadRailPhysicalDomain();

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR, Utf8CP physicalPartitionName);

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/RoadRailAlignmentDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadRailAlignment schema.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct RoadRailAlignmentDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(RoadRailAlignmentDomain, ROADRAILALIGNMENT_EXPORT)

protected:
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    RoadRailAlignmentDomain();

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId QueryAlignmentCategoryId(Dgn::DgnDbCR);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnAuthorityId QueryAlignmentAuthorityId(Dgn::DgnDbCR dgndb);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value);
}; // RoadRailAlignmentDomain

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

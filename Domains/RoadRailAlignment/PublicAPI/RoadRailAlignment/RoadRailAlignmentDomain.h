/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/RoadRailAlignmentDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "AlignmentCategory.h"
#include "AlignmentModel.h"
#include "Alignment.h"
#include "AlignmentProfileViewDefinition.h"
#include "AlignmentXSViewDefinition.h"
#include "AlignmentReferent.h"

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

    ROADRAILALIGNMENT_EXPORT static Dgn::CodeSpecId QueryAlignmentCodeSpecId(Dgn::DgnDbCR dgndb);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnModelCR scopeModel, Utf8StringCR value);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR subject, Utf8CP partitionName);
    static Utf8CP GetDefaultPartitionName() { return "Alignments"; }

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRA_SCHEMA_PATH; }
}; // RoadRailAlignmentDomain

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

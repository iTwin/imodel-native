/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadRailAlignment schema.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct RoadRailAlignmentDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(RoadRailAlignmentDomain, ROADRAILALIGNMENT_EXPORT)

/*private:
    static Dgn::DgnDbStatus InsertViewDefinitions(ConfigurationModelR model);*/

protected:
    //! @private
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    //! @private
    RoadRailAlignmentDomain();

    //! Query for the Alignment CodeSpecId
    //! @param[in] dgndb The DgnDb to query
    //! @return The CodeSpecId of the Alignment partition
    ROADRAILALIGNMENT_EXPORT static Dgn::CodeSpecId QueryAlignmentCodeSpecId(Dgn::DgnDbCR dgndb);

    //! @private
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCode CreateCode(Dgn::DgnModelCR scopeModel, Utf8StringCR value);

    //! @private
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetUpDefinitionPartitions(Dgn::SubjectCR subject);

    //! Returns the partition name used for the general configuration model for Road/Rail elements
    static Utf8CP GetConfigurationPartitionName() { return "Road/Rail Configuration"; }

    //! Returns the partition name used for the DefinitionModel containing Domain Categories
    static Utf8CP GetDomainCategoriesPartitionName() { return "Road/Rail Domain Categories"; }

    //! Returns the code value used for the HorizontalAlignments element in Alignment models
    static Utf8CP GetHorizontalAlignmentsCodeName() { return "Horizontal Alignments"; }

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnModelId QueryConfigurationModelId(Dgn::SubjectCR subject);
    ROADRAILALIGNMENT_EXPORT static Dgn::DefinitionModelPtr QueryConfigurationModel(Dgn::SubjectCR subject);

    ROADRAILALIGNMENT_EXPORT static Dgn::DgnModelId QueryCategoryModelId(Dgn::DgnDbR db);
    ROADRAILALIGNMENT_EXPORT static Dgn::DefinitionModelPtr QueryCategoryModel(Dgn::DgnDbR db);

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRA_SCHEMA_PATH; }
}; // RoadRailAlignmentDomain

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

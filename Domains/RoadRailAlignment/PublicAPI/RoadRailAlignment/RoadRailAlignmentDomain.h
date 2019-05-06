/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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

private:
    static Dgn::DgnDbStatus InsertViewDefinitions(ConfigurationModelR model);

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
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR subject);    

    //! Returns the partition name used for the AlignmentModel containing Design Alignments
    static Utf8CP GetDesignPartitionName() { return "Road/Rail Design Alignments"; }

    //! Returns the partition name used for the AlignmentModel containing 3D Linears
    static Utf8CP Get3DLinearsPartitionName() { return "Road/Rail 3D Linears"; }

    //! Returns the partition name used for the DefinitionModel containing Domain Categories
    static Utf8CP GetDomainCategoriesPartitionName() { return "Road/Rail Domain Categories"; }

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRA_SCHEMA_PATH; }
}; // RoadRailAlignmentDomain

//=======================================================================================
//! Model containing configuration elements such as view-definitions, 
//! for a particular Subject.
//=======================================================================================
struct ConfigurationModel : Dgn::DefinitionModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_ConfigurationModel, Dgn::DefinitionModel);
    friend struct ConfigurationModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(ConfigurationModel::T_Super::CreateParams);

    //! Parameters to create a new instance of a ConfigurationModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, ConfigurationModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit ConfigurationModel(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(ConfigurationModel)
    ROADRAILALIGNMENT_EXPORT Dgn::SubjectCPtr GetParentSubject() const;

    static ConfigurationModelPtr Create(CreateParams const& params) { return new ConfigurationModel(params); }

    static void SetUp(Dgn::SubjectCR);
    static Dgn::DgnModelId QueryModelId(Dgn::SubjectCR);
    ROADRAILALIGNMENT_EXPORT static ConfigurationModelPtr Query(Dgn::SubjectCR);
    static Utf8CP GetDomainPartitionName() { return "Road/Rail Configuration"; }
}; // ConfigurationModel


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ModelHandler for ConfigurationModel
//=======================================================================================
struct ConfigurationModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_ConfigurationModel, ConfigurationModel, ConfigurationModelHandler, Dgn::dgn_ModelHandler::Definition, )
}; // ConfigurationModelHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

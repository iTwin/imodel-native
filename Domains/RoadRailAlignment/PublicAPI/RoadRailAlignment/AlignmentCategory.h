/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentCategory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Helper class to query/manage Bridge-related categories.
//! @ingroup GROUP_BridgePhysical
//=======================================================================================
struct AlignmentCategory : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static void InsertCategory(Dgn::DefinitionModelR model, Utf8CP codeValue, Dgn::ColorDef const& color);
    static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbR, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for alignment elements
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId Get(Dgn::DgnDbR);
}; // BridgePhysicalDomain

//=======================================================================================
//! Model containing categories for Alignments
//=======================================================================================
struct AlignmentCategoryModel : Dgn::DefinitionModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_AlignmentCategoryModel, Dgn::DefinitionModel);
    friend struct AlignmentCategoryModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(AlignmentCategoryModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an AlignmentModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, AlignmentCategoryModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit AlignmentCategoryModel(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentCategoryModel)
    static AlignmentCategoryModelPtr Create(CreateParams const& params) { return new AlignmentCategoryModel(params); }

    static void SetUp(Dgn::DgnDbR);
    static Dgn::DgnModelId GetModelId(Dgn::DgnDbR);
    static AlignmentCategoryModelPtr GetModel(Dgn::DgnDbR);
    static Utf8CP GetPartitionName() { return "AlignmentCategories"; }
}; // AlignmentCategoryModel

//=======================================================================================
//! The ModelHandler for AlignmentCategoryModel
//=======================================================================================
struct AlignmentCategoryModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentCategoryModel, AlignmentCategoryModel, AlignmentCategoryModelHandler, Dgn::dgn_ModelHandler::Definition, )
}; // AlignmentCategoryModelHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

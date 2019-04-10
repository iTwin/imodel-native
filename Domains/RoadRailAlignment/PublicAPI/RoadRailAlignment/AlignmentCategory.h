/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentCategory.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailAlignment.h"
#include "RoadRailAlignmentDomain.h"

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
    static Dgn::DgnCategoryId QueryDomainCategoryId(Dgn::DgnDbR, Utf8CP, bool isSpatial);

public:
    static void InsertDomainCategories(Dgn::DgnDbR db);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for the Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetAlignment(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Horizontal Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetHorizontal(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Vertical Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetVertical(Dgn::DgnDbR);

}; // AlignmentCategory

//=======================================================================================
//! Model containing categories for RoadRails
//=======================================================================================
struct RoadRailCategoryModel : Dgn::DefinitionModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRA_CLASS_RoadRailCategoryModel, Dgn::DefinitionModel);
    friend struct RoadRailCategoryModelHandler;

public:
    struct CreateParams : T_Super::CreateParams
    {
    DEFINE_T_SUPER(RoadRailCategoryModel::T_Super::CreateParams);

    //! Parameters to create a new instance of an RoadRailModel.
    //! @param[in] dgndb The DgnDb for the new DgnModel
    //! @param[in] modeledElementId The DgnElementId of the element this this DgnModel is describing/modeling
    CreateParams(Dgn::DgnDbR dgndb, Dgn::DgnElementId modeledElementId)
        : T_Super(dgndb, RoadRailCategoryModel::QueryClassId(dgndb), modeledElementId)
        {}

    //! @private
    //! This constructor is used only by the model handler to create a new instance, prior to calling ReadProperties on the model object
    CreateParams(DgnModel::CreateParams const& params) : T_Super(params) {}
    }; // CreateParams

protected:
    explicit RoadRailCategoryModel(CreateParams const& params) : T_Super(params) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(RoadRailCategoryModel)

    //! @private
    static RoadRailCategoryModelPtr Create(CreateParams const& params) { return new RoadRailCategoryModel(params); }

    //! @private
    static Dgn::DgnModelId GetModelId(Dgn::DgnDbR);

    //! @private
    ROADRAILALIGNMENT_EXPORT static RoadRailCategoryModelPtr GetModel(Dgn::DgnDbR);
}; // RoadRailCategoryModel


//__PUBLISH_SECTION_END__
//=======================================================================================
//! The ModelHandler for RoadRailCategoryModel
//=======================================================================================
struct RoadRailCategoryModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRA_CLASS_RoadRailCategoryModel, RoadRailCategoryModel, RoadRailCategoryModelHandler, Dgn::dgn_ModelHandler::Definition, )
}; // RoadRailCategoryModelHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

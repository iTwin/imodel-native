/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailCategory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Helper class to query/manage RoadRail-related categories.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailCategory : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static void InsertSpatialCategory(Dgn::DefinitionModelR, Utf8CP, Dgn::ColorDef const&);
    static void InsertDrawingCategory(Dgn::DefinitionModelR, Utf8CP, Dgn::ColorDef const&);
    static Dgn::DgnCategoryId QuerySpatialCategoryId(Dgn::DgnDbR, Utf8CP);
    static Dgn::DgnCategoryId QueryDrawingCategoryId(Dgn::DgnDbR, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRoad(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTrack(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTypicalSectionPoint(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTravelwayDefComponent(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTravelwayStructureDefComponent(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTravelwaySideDefComponent(Dgn::DgnDbR);
}; // RoadRailCategory

//=======================================================================================
//! Model containing categories for RoadRails
//=======================================================================================
struct RoadRailCategoryModel : Dgn::DefinitionModel
{
    DGNMODEL_DECLARE_MEMBERS(BRRP_CLASS_RoadRailCategoryModel, Dgn::DefinitionModel);
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
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadRailCategoryModel)
    static RoadRailCategoryModelPtr Create(CreateParams const& params) { return new RoadRailCategoryModel(params); }

    static void SetUp(Dgn::DgnDbR);
    static Dgn::DgnModelId GetModelId(Dgn::DgnDbR);
    static RoadRailCategoryModelPtr GetModel(Dgn::DgnDbR);
    static Utf8CP GetPartitionName() { return "RoadRailCategories"; }
}; // RoadRailCategoryModel

//=======================================================================================
//! The ModelHandler for RoadRailCategoryModel
//=======================================================================================
struct RoadRailCategoryModelHandler : Dgn::dgn_ModelHandler::Definition
{
MODELHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadRailCategoryModel, RoadRailCategoryModel, RoadRailCategoryModelHandler, Dgn::dgn_ModelHandler::Definition, )
}; // RoadRailCategoryModelHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

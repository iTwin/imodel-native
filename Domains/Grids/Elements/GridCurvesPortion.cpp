/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridCurvesPortion.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/gridsApi.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(GridCurvesPortion)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurvesPortion::GridCurvesPortion
(
CreateParams const& params
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid ()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);
        if (catId.IsValid ())
            DoSetCategoryId (catId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           GridCurvesPortion::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridCurvesPortionPtr                 GridCurvesPortion::Create 
(
Dgn::DgnModelCR model
)
    {
    return new GridCurvesPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurvesPortion::_OnDelete() const
    {
    Dgn::DgnModelPtr subModel = GetSubModel();
    
    IBriefcaseManager::Request request;
    if (Dgn::RepositoryStatus::Success != subModel->PopulateRequest(request, BeSQLite::DbOpcode::Delete))
        return Dgn::DgnDbStatus::LockNotHeld;

    request.SetOptions(IBriefcaseManager::ResponseOptions::All);
    Dgn::IBriefcaseManager::Response response = GetDgnDb().BriefcaseManager().Acquire(request);
    if (Dgn::RepositoryStatus::Success != response.Result())
        return Dgn::DgnDbStatus::LockNotHeld;

    Dgn::DgnDbStatus subModelDeleteStatus = subModel->Delete();
    if (Dgn::DgnDbStatus::Success != subModelDeleteStatus)
        return subModelDeleteStatus;

    for(Dgn::ElementIteratorEntry bundleEntry : GridCurveBundle::MakeGridCurveBundleIterator(*this))
        {
        GridCurveBundleCPtr bundle = GridCurveBundle::Get(GetDgnDb(), bundleEntry.GetElementId());
        if (bundle.IsNull())
            continue;

        Dgn::DgnDbStatus bundleDeleteStatus = bundle->Delete();
        if (Dgn::DgnDbStatus::Success != bundleDeleteStatus)
            return bundleDeleteStatus;
        }

    return T_Super::_OnDelete();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     02/18
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridCurvesPortion::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    Dgn::SpatialLocationModelPtr model = SpatialLocationModel::Create(*this);
    if (!model.IsValid())
        return DgnDbStatus::WriteError;

    Dgn::IBriefcaseManager::Request req;
    GetDgnDb().BriefcaseManager().PrepareForModelInsert(req, *model, Dgn::IBriefcaseManager::PrepareAction::Acquire);

    return model->Insert();
    }


END_GRIDS_NAMESPACE
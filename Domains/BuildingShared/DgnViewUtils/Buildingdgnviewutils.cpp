/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "PublicApi/BuildingDgnViewUtilsApi.h"

//#include "PublicApi/BuildingUtils.h"
#include <DgnView/DgnTool.h>
#include <dgnPlatform/ViewController.h>
#include <DgnView/AccuSnap.h>
//#include "PublicApi\GeometryUtils.h"
#include <DgnPlatform/LineStyle.h>
#include <RoadRailPhysical\RoadRailPhysicalApi.h>
#include <RoadRailPhysical\RoadRailCategory.h>
#include <BuildingShared/Units/UnitConverter.h>

USING_NAMESPACE_BENTLEY_DGN
namespace DgnFx = BentleyApi::DgnClientFx;

BEGIN_BUILDING_SHARED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
BentleyStatus    BuildingDgnViewUtils::ExtractHitListFromPoint(HitListP& hitList, DgnViewportP vp, DPoint3d point)
    {
    ElementPicker&  picker = ElementLocateManager::GetManager().GetElementPicker();
    AccuSnap& snap = AccuSnap::GetInstance();

    LocateOptions   options = ElementLocateManager::GetManager().GetLocateOptions();
    options.SetHitSource(HitSource::DataPoint);

    bool wasAborted;
    double locateFactor = snap.GetSettings().searchDistance;
    double aperture = (locateFactor * vp->PixelsFromInches(ElementLocateManager::GetManager().GetApertureInches()) / 2.0) + 1.5;

    int hitCount = picker.DoPick(&wasAborted, *vp, point, aperture, nullptr, options);

    if (0 == hitCount)
        return BentleyStatus::ERROR;

    hitList = picker.GetHitList(true);
    return BentleyStatus::SUCCESS;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mykolas.Simutis                 09/2016
//--------------------------------------------------------------------------------------
void BuildingDgnViewUtils::AppendTableCell (JsonValueR jsonArr, Json::Value &&key, Json::Value &&value)
    {
    Json::Value cell;
    cell["key"] = key;
    cell["value"] = value;
    jsonArr.append (cell);
    }

END_BUILDING_SHARED_NAMESPACE
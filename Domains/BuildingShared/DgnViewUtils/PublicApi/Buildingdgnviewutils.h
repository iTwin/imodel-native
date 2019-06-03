/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <dgnPlatform/DgnModel.h>
#include <dgnPlatform/ViewController.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/DgnDomain.h>
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/RangeIndex.h>
#include <DgnPlatform/DgnViewport.h>

BEGIN_BUILDING_SHARED_NAMESPACE

//GeometryUtils is a static class for doing various operations on geometry (this is also a wrapper for all parasolid related activity)
class BuildingDgnViewUtils
{
public:     
    BUILDINGSHAREDDGNVIEWUTILS_EXPORT static BentleyStatus ExtractHitListFromPoint(Dgn::HitListP& hitListOut, Dgn::DgnViewportP vp, DPoint3d point);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Haroldas.Vitunskas              04/2017
    //---------------------------------------------------------------------------------------
    template<typename T> static bvector<RefCountedCPtr<T>> ExtractElementsFromHitList(Dgn::HitListCP hitList, const std::function< bool(RefCountedCPtr<T>) >& optionalPredicate = [](RefCountedCPtr<T> element) {return true; })
        {
        bvector<RefCountedCPtr<T>> activeElementVector;
        if (nullptr != hitList)
            {
            for (uint32_t i = 0; i < hitList->GetCount(); ++i)
                {
                if (Dgn::HitDetailP hit = hitList->GetHit(i))
                    {
                    RefCountedCPtr<T> element = dynamic_cast<T const*>(hit->GetElement().get());
                    if (element.IsValid() && std::find_if(activeElementVector.begin(), activeElementVector.end(), [&](auto el) {return el->GetElementId() == element->GetElementId(); }) == activeElementVector.end() && optionalPredicate(element))
                        activeElementVector.push_back(element);
                    }
                }
            }

        return activeElementVector;
        }
    
    BUILDINGSHAREDDGNVIEWUTILS_EXPORT static void AppendTableCell(JsonValueR jsonArr, Json::Value &&key, Json::Value &&value);
};

END_BUILDING_SHARED_NAMESPACE
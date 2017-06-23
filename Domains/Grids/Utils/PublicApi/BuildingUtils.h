/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/BuildingUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../../../../Building/ConceptBuilding/BuildingCore/PrivateApi/BuildingMacros.h"
#include <dgnPlatform/DgnModel.h>
#include <dgnPlatform/ViewController.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/Render.h>
#include "DgnPlatform/DgnDomain.h"
#include <DgnPlatform/ClipVector.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/RangeIndex.h>
#include <DgnPlatform/DgnViewport.h>
BEGIN_BUILDING_NAMESPACE

//GeometryUtils is a static class for doing various operations on geometry (this is also a wrapper for all parasolid related activity)
class BuildingUtils
    {
    private:
        static Dgn::CategorySelectorPtr CreateDefaultCategorySelector (Dgn::DgnDbR db);
        static Dgn::DisplayStyle3dPtr     CreateFloorView3dDisplayStyle (Dgn::DgnDbR db);
        
    public:
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr GetDefaultCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr CreateFloorViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr GetFloorViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr GetSiteViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr GetEgressPathViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr CreateSiteViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr CreateBuildingViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr CreateEgressPathViewCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::CategorySelectorPtr GetBuildingCategorySelector (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static Dgn::DisplayStyle3dPtr   GetFloorView3dDisplayStyle (Dgn::DgnDbR db);
        BUILDINGUTILS_EXPORT static ECN::IECInstancePtr    GetECInstance (Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId instanceId, Utf8CP ECSqlName);
        BUILDINGUTILS_EXPORT static bool                   TryExtractIndexFromName (Utf8CP name, int& indexOut);
        
        BUILDINGUTILS_EXPORT static BentleyStatus    ExtractHitListFromPoint(Dgn::HitListP& hitListOut, Dgn::DgnViewportP vp, DPoint3d point);

        BUILDINGUTILS_EXPORT static Dgn::DgnStyleId GetCenterLineStyleId (Dgn::DgnDbR dgnDb);
        BUILDINGUTILS_EXPORT static CurveVectorPtr  CreateRoadwayCurveVector (const bvector<DPoint3d>& points, double width);
        BUILDINGUTILS_EXPORT static void            AppendRoadwayGeometry (Dgn::DgnElementPtr roadway, CurveVectorPtr horizAlignment, double width);

        BUILDINGUTILS_EXPORT static BentleyStatus   InsertElement(Dgn::DgnElementPtr element);
    };

class BuildingElementsUtils
    {
    public:
        template <typename T> static bvector<RefCountedCPtr<T>> GetElementsFromStmt(BeSQLite::Statement& stmt, Dgn::DgnDbCR db)
            {
            bvector<RefCountedCPtr<T>> elements;

            while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
                {
                Dgn::DgnElementId id(stmt.GetValueId<Dgn::DgnElementId>(0));
                auto el = db.Elements().GetElement(id);

                if (auto element = dynamic_cast<T const*>(el.get()))
                    {
                    elements.push_back(element);
                    }
                }
            return elements;
            }
        template <typename T> static bvector<RefCountedCPtr<T>> GetElementsFromStmt(BeSQLite::EC::CachedECSqlStatementPtr stmt, Dgn::DgnDbCR db)
            {
            bvector<RefCountedCPtr<T>> elements;

            while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt->Step())
                {
                Dgn::DgnElementId id(stmt->GetValueId<Dgn::DgnElementId>(0));
                auto el = db.Elements().GetElement(id);

                if (auto element = dynamic_cast<T const*>(el.get()))
                    {
                    elements.push_back(element);
                    }
                }
            return elements;
            }
        BUILDINGUTILS_EXPORT static void AppendTableCell (JsonValueR jsonArr, Json::Value &&key, Json::Value &&value);
    };
END_BUILDING_NAMESPACE
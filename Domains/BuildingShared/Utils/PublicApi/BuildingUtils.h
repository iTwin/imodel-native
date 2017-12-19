/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/BuildingUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//#include <ConstraintSystem/Domain/ConstraintModelMacros.h>
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
class BuildingUtils
{
private:
    static Dgn::CategorySelectorPtr CreateDefaultCategorySelector(Dgn::DgnDbR db);
    static Dgn::DisplayStyle3dPtr CreateFloorView3dDisplayStyle(Dgn::DgnDbR db);

public:
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr GetDefaultCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr CreateFloorViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr GetFloorViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr GetSiteViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr GetEgressPathViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr CreateSiteViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr CreateBuildingViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr CreateEgressPathViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::CategorySelectorPtr GetBuildingCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::DisplayStyle3dPtr GetFloorView3dDisplayStyle(Dgn::DgnDbR db);
    BUILDINGSHAREDUTILS_EXPORT static ECN::IECInstancePtr GetECInstance(Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId instanceId, Utf8CP ECSqlName);
    BUILDINGSHAREDUTILS_EXPORT static bool TryExtractIndexFromName(Utf8CP name, int& indexOut);
    BUILDINGSHAREDUTILS_EXPORT static int ExtractNameIndexAndTemplateString(Utf8String& tmpStr);

    BUILDINGSHAREDUTILS_EXPORT static BentleyStatus ExtractHitListFromPoint(Dgn::HitListP& hitListOut, Dgn::DgnViewportP vp, DPoint3d point);

    BUILDINGSHAREDUTILS_EXPORT static Dgn::DgnStyleId GetCenterLineStyleId(Dgn::DgnDbR dgnDb);
    BUILDINGSHAREDUTILS_EXPORT static CurveVectorPtr CreateRoadwayCurveVector(const bvector<DPoint3d>& points, double width);
    BUILDINGSHAREDUTILS_EXPORT static void AppendRoadwayGeometry(Dgn::DgnElementPtr roadway, CurveVectorPtr horizAlignment, double width);

    BUILDINGSHAREDUTILS_EXPORT static BentleyStatus InsertElement(Dgn::DgnElementPtr element);

    BUILDINGSHAREDUTILS_EXPORT static int ParseStringForInt(Utf8CP string);
    BUILDINGSHAREDUTILS_EXPORT static double ParseStringForDouble(Utf8CP string);

    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfModelHasElements(Dgn::DgnModelP model);
};

class BuildingElementsUtils
{
public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                    Haroldas.Vitunskas               06/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedPtr<T>> GetModelElementsForEdit(Dgn::DgnModelCP pModel)
        {
        if (nullptr == pModel)
            return bvector<RefCountedPtr<T>>();

        bvector<RefCountedPtr<T>> result;

        for (ElementIteratorEntry elementEntry : pModel->MakeIterator())
            {
            DgnElementId id = elementEntry.GetElementId();
            auto element = pModel->GetDgnDb().Elements().GetForEdit<T>(id);
            if (element.IsValid())
                result.push_back(element);
            }

        return result;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static RefCountedCPtr<T> GetElementById(Dgn::DgnElementId id, Dgn::DgnDbCR db)
        {
        return db.Elements().Get<T>(id);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static RefCountedPtr<T> GetElementForEditById(Dgn::DgnElementId id, Dgn::DgnDbCR db)
        {
        return db.Elements().GetForEdit<T>(id);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<T> GetElementsFromStmt(BeSQLite::Statement& stmt, Dgn::DgnDbCR db, std::function<T(Dgn::DgnElementId, Dgn::DgnDbCR)> getElement)
        {
        bvector<T> elements;

        while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
            {
            Dgn::DgnElementId id(stmt.GetValueId<Dgn::DgnElementId>(0));
            auto element = getElement(id, db);

            if (element.IsValid())
                elements.push_back(element);
            }

        return elements;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedCPtr<T>> GetElementsFromStmt(BeSQLite::Statement& stmt, Dgn::DgnDbCR db)
        {
        return GetElementsFromStmt<RefCountedCPtr<T>>(stmt, db, [](Dgn::DgnElementId id, Dgn::DgnDbCR db)
            {
            return GetElementById<T>(id, db);
            });
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedPtr<T>> GetElementsFromStmtForEdit(BeSQLite::Statement& stmt, Dgn::DgnDbCR db)
        {
        return GetElementsFromStmt<RefCountedPtr<T>>(stmt, db, [](Dgn::DgnElementId id, Dgn::DgnDbCR db)
            {
            return GetElementForEditById<T>(id, db);
            });
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Nerijus.Jakeliunas                10/2016
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedCPtr<T>> GetModelElements(Dgn::DgnModelCP pModel)
        {
        return GetOtherElementsInSameModel<T>(nullptr, pModel);
        }

    //---------------------------------------------------------------------------------------
    //@bsimethod                                   Haroldas.Vitunskas               06 / 2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedCPtr<T>> GetOtherElementsInSameModel(RefCountedCPtr<T> element, Dgn::DgnModelCP pModel)
        {
        if (nullptr == pModel)
            return bvector<RefCountedCPtr<T>>();

        bvector<RefCountedCPtr<T>> result;

        Dgn::DgnElementId elementId;
        if (element.IsValid())
            elementId = element->GetElementId();

        for (Dgn::ElementIteratorEntry elementEntry : pModel->MakeIterator())
            {
            Dgn::DgnElementId id = elementEntry.GetElementId();
            if (!elementId.IsValid() || elementId != id) //If nullptr is passed, will return all elements
                {
                auto element = pModel->GetDgnDb().Elements().GetElement(id);
                if (auto spatialElement = dynamic_cast<T const*>(element.get()))
                    result.push_back(spatialElement);
                }
            }

        return result;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<T> GetElementsFromCachedStmt(BeSQLite::EC::CachedECSqlStatementPtr& stmt, Dgn::DgnDbCR db, std::function<T(Dgn::DgnElementId, Dgn::DgnDbCR db)> getElement)
        {
        bvector<T> elements;

        while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt->Step())
            {
            Dgn::DgnElementId id(stmt->GetValueId<Dgn::DgnElementId>(0));
            auto element = getElement(id, db);

            if (element.IsValid())
                elements.push_back(element);
            }
        return elements;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Tomas.Stuina                      08/2017
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedCPtr<T>> GetElementsFromStmt(BeSQLite::EC::CachedECSqlStatementPtr& stmt, Dgn::DgnDbCR db)
        {
        return GetElementsFromCachedStmt<RefCountedCPtr<T>>(stmt, db, [](Dgn::DgnElementId id, Dgn::DgnDbCR db) {return GetElementById<T>(id, db);});
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Nerijus.Jakeliunas                10/2016
    //---------------------------------------------------------------------------------------
    template <typename T> static bvector<RefCountedCPtr<T>> GetElements(Dgn::DgnDbR db, Utf8CP fullECClassName)
        {
        bvector<RefCountedCPtr<T>> elements;
        for (Dgn::ElementIteratorEntry entry : db.Elements().MakeIterator(fullECClassName))
            {
            auto element = db.Elements().Get<T>(entry.GetElementId());
            if (element.IsValid())
                {
                elements.push_back(element);
                }
            }

        return elements;
        }

    BUILDINGSHAREDUTILS_EXPORT static void AppendTableCell(JsonValueR jsonArr, Json::Value &&key, Json::Value &&value);
    BUILDINGSHAREDUTILS_EXPORT static Dgn::DgnElementId GetElementIdByParentElementAuthorityAndName(Dgn::DgnDbR db, Utf8CP authorityName, Dgn::DgnElementId parentId, Utf8CP elementName);

    //! Make iterator for all elements in element's submodel
    //! @param[in] element  The modeled element
    //! @param[in] classId  The classId of the elements that you want to iterate
    //! @return Element iterator for elements that have the given classId
    BUILDINGSHAREDUTILS_EXPORT static Dgn::ElementIterator MakeIterator(Dgn::DgnElementCR element, ECN::ECClassId classId);
};

END_BUILDING_SHARED_NAMESPACE
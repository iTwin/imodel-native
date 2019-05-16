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
class BuildingUtils
{
private:
    static Dgn::CategorySelectorPtr CreateDefaultCategorySelector(Dgn::DgnDbR db);
    static Dgn::DisplayStyle3dPtr CreateFloorView3dDisplayStyle(Dgn::DgnDbR db);
    static Utf8String GetTemplateFromName(Utf8String name, int &index);

    static Dgn::CategorySelectorPtr CreateAndInsertFloorViewCategorySelector(Dgn::DgnDbR db);
    
    static Dgn::DefinitionModelPtr CreateDefinitionModel (Dgn::DgnDbR db, Utf8CP partitionName);
    static Dgn::DefinitionModelCPtr GetDefinitionModel (Dgn::DgnDbR db, Utf8CP partitionName);

    static Dgn::DgnCode GetDefinitionPartitionCode (Dgn::DgnDbR db, Utf8CP partitionName);
    static Dgn::DefinitionPartitionCPtr CreateDefinitionPartition (Dgn::DgnDbR db, Utf8CP partitionName);
    static Dgn::DefinitionPartitionCPtr GetDefinitionPartition (Dgn::DgnDbR db, Utf8CP partitionName);

public:
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetDefaultCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetFloorViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetOrCreateAndInsertFloorViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetSiteViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetEgressPathViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr CreateSiteViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr CreateBuildingViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr CreateEgressPathViewCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::CategorySelectorPtr GetBuildingCategorySelector(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DisplayStyle3dPtr GetFloorView3dDisplayStyle(Dgn::DgnDbR db);
    BUILDINGSHAREDDGNUTILS_EXPORT static void                       SetDisplayStyleOverrides(Dgn::DisplayStyle3dR displaystyle);


    BUILDINGSHAREDDGNUTILS_EXPORT static ECN::IECInstancePtr GetECInstance(Dgn::DgnDbR db, BeSQLite::EC::ECInstanceId instanceId, Utf8CP ECSqlName);
    BUILDINGSHAREDDGNUTILS_EXPORT static bool TryExtractIndexFromName(Utf8CP name, int& indexOut);
    BUILDINGSHAREDDGNUTILS_EXPORT static int ExtractNameIndexAndTemplateString(Utf8String& tmpStr);

    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnStyleId GetCenterLineStyleId(Dgn::DgnDbR dgnDb);
    BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr CreateRoadwayCurveVector(const bvector<DPoint3d>& points, double width);
    BUILDINGSHAREDDGNUTILS_EXPORT static void AppendRoadwayGeometry(Dgn::DgnElementPtr roadway, CurveVectorPtr horizAlignment, double width);

    BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus InsertElement(Dgn::DgnElementPtr element);

    BUILDINGSHAREDDGNUTILS_EXPORT static int ParseStringForInt(Utf8CP string);
    BUILDINGSHAREDDGNUTILS_EXPORT static double ParseStringForDouble(Utf8CP string);

    BUILDINGSHAREDDGNUTILS_EXPORT static bool CheckIfModelHasElements(Dgn::DgnModelP model);

    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnCode GetUniqueElementCode(Dgn::DgnDbR db, Dgn::CodeSpecId codeSpecId, Utf8String baseName, Dgn::DgnElementId modeledElementId, int startIndex = 0);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnCode GetNamedElementCode(Dgn::DgnDbR db, Dgn::CodeSpecId codeSpecId, Utf8String baseName, Dgn::DgnElementId modeledElementId, int startIndex = 0);

    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnModelCR GetGroupInformationModel(Dgn::DgnDbR db);
    
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DefinitionModelCPtr GetOrCreateDefinitionModel (Dgn::DgnDbR db, Utf8CP partitionName);
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DefinitionPartitionCPtr GetOrCreateDefinitionPartition (Dgn::DgnDbR db, Utf8CP partitionName);

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

        for (Dgn::ElementIteratorEntry elementEntry : pModel->MakeIterator())
            {
            Dgn::DgnElementId id = elementEntry.GetElementId();
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

    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::DgnElementId GetElementIdByParentElementAuthorityAndName(Dgn::DgnDbR db, Utf8CP authorityName, Dgn::DgnElementId parentId, Utf8CP elementName);

    //! Make iterator for all elements in element's submodel
    //! @param[in] element  The modeled element
    //! @param[in] classId  The classId of the elements that you want to iterate
    //! @return Element iterator for elements that have the given classId
    BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::ElementIterator MakeIterator(Dgn::DgnElementCR element, ECN::ECClassId classId);
};

//=======================================================================================
//! The "current entry" of an ElementIdIterator
// @bsiclass                                                     Mindaugas.Butkus  03/18
//=======================================================================================
struct ElementIdIteratorEntry : Dgn::ECSqlStatementEntry
    {
    friend struct Dgn::ECSqlStatementIterator<ElementIdIteratorEntry>;
    private:
        ElementIdIteratorEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : Dgn::ECSqlStatementEntry(statement) {}
    public:
        BUILDINGSHAREDDGNUTILS_EXPORT Dgn::DgnElementId GetElementId() const; //!< Get the DgnElementId of the current element
        template <class T_ElementId> T_ElementId GetId() const { return T_ElementId(GetElementId().GetValue()); } //!< Get the DgnElementId of the current element
    };

//=======================================================================================
//! An iterator over a set of DgnElementIds, defined by a query.
// @bsiclass                                                     Shaun.Sewall      11/16
//=======================================================================================
struct ElementIdIterator : Dgn::ECSqlStatementIterator<ElementIdIteratorEntry>
    {
    //! Iterates all entries to build an unordered IdSet templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> BeSQLite::IdSet<T_ElementId> BuildIdSet()
        {
        BeSQLite::IdSet<T_ElementId> idSet;
        for (ElementIdIteratorEntry const& entry : *this)
            idSet.insert(entry.GetId<T_ElementId>());

        return idSet;
        }

    //! Iterates all entries to build an ordered bvector templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> bvector<T_ElementId> BuildIdList()
        {
        bvector<T_ElementId> idList;
        for (ElementIdIteratorEntry const& entry : *this)
            idList.push_back(entry.GetId<T_ElementId>());

        return idList;
        }

    //! Iterates all entries to populate an ordered bvector templated on DgnElementId or a subclass of DgnElementId
    template <class T_ElementId> void BuildIdList(bvector<T_ElementId>& idList)
        {
        for (ElementIdIteratorEntry const& entry : *this)
            idList.push_back(entry.GetId<T_ElementId>());
        }
    };

BUILDINGSHAREDDGNUTILS_EXPORT void BuildingElement_notifyFail(Utf8CP pOperation, Dgn::DgnElement& elm, Dgn::DgnDbStatus* stat);

END_BUILDING_SHARED_NAMESPACE
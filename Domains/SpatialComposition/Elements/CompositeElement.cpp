/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>
#include "PublicApi/CompositeElement.h"

USING_NAMESPACE_BUILDING_SHARED

BEGIN_SPATIALCOMPOSITION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ElementIterator CompositeElement::MakeIterator() const
    {
    Dgn::DgnElements& elements = GetDgnDb().Elements();
    Dgn::ElementIterator iterator = elements.MakeIterator (SPATIALCOMPOSITION_SCHEMA(SPATIALCOMPOSITION_CLASS_CompositeElement), "WHERE ComposingElement.id = ?");
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindId (1, GetElementId());
        }
    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Joana.Smitaite                  11/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator CompositeElement::MakeIterator(Utf8CP className) const
    {
    Dgn::ElementIterator iterator = GetDgnDb().Elements().MakeIterator(className, "WHERE ComposingElement=?");
    ECN::ECClassId relClassId = GetDgnDb().Schemas().GetClassId(SPATIALCOMPOSITION_SCHEMA_NAME, SPATIALCOMPOSITION_REL_CompositeComposesSubComposites);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement())
        {
        pStmnt->BindNavigationValue (1, GetElementId(), relClassId);
        }
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ElementIterator CompositeElement::MakeOverlapedElementsIterator() const
    {
    Utf8String sql = Utf8PrintfString ("SELECT TargetECInstanceId FROM %s WHERE SourceECInstanceId = ?", SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements);
    Dgn::ElementIterator iterator;
    iterator.Prepare(GetDgnDb(), sql.c_str(), 0 /* Index of ECInstanceId */);
    iterator.GetStatement()->BindId (1, GetElementId());
    return iterator;
    }


bool elementsHaveRelationship (Dgn::DgnDbR dgnDb, Dgn::DgnElementId referencingElId, Dgn::DgnElementId referencedElId, Utf8CP fullRelationshipClassName)
    {
    if (!referencingElId.IsValid () || !referencedElId.IsValid ())
        {
        return false;
        }

    Utf8String statemntString = Utf8PrintfString ("SELECT * FROM %s WHERE SourceECInstanceId=? AND TargetECInstanceId=?", fullRelationshipClassName);
    BeSQLite::EC::CachedECSqlStatementPtr statement = dgnDb.GetPreparedECSqlStatement (statemntString.c_str ());
    if (!statement.IsValid ())
        {
        return false;
        }

    statement->BindId (1, referencingElId);
    statement->BindId (2, referencedElId);

    return (BeSQLite::DbResult::BE_SQLITE_ROW == statement->Step ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/     
Dgn::DgnDbStatus CompositeElement::AddOverlapedElement (Dgn::DgnElementId overlapedElementId)
    {
    Dgn::DgnElementId id = GetElementId();
    if (!id.IsValid () || !overlapedElementId.IsValid ())
        {
        return Dgn::DgnDbStatus::BadElement;
        }

    Dgn::DgnDbR dgnDb = GetDgnDb();
    if (elementsHaveRelationship (dgnDb, id, overlapedElementId, SPATIALCOMPOSITION_SCHEMA (SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements)))
        return Dgn::DgnDbStatus::Success;

    ECN::ECClassCP relClass = dgnDb.Schemas ().GetClass (SPATIALCOMPOSITION_SCHEMA_NAME, SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements);
    ECN::ECRelationshipClassCP relationshipClass = dynamic_cast<ECN::ECRelationshipClassCP>(relClass);
    if (nullptr == relationshipClass)
        {
        return Dgn::DgnDbStatus::BadSchema;
        }

    BeSQLite::EC::ECInstanceKey rkey;
    BeSQLite::DbResult status = dgnDb.InsertLinkTableRelationship (rkey, *relationshipClass, BeSQLite::EC::ECInstanceId (id), BeSQLite::EC::ECInstanceId (overlapedElementId));
    return (BeSQLite::DbResult::BE_SQLITE_OK == status) ? Dgn::DgnDbStatus::Success : Dgn::DgnDbStatus::BadRequest;
    }


static Dgn::DgnDbStatus deleteRelationship (Dgn::DgnDbR dgnDb, Dgn::DgnElementId referencingElId, Dgn::DgnElementId referencedElId, Utf8CP fullRelationshipClassName)
    {
    if (!referencingElId.IsValid () || !referencedElId.IsValid ())
        {
        return Dgn::DgnDbStatus::BadElement;
        }

    BeSQLite::EC::ECInstanceKey rkey;
    BeSQLite::DbResult status = dgnDb.DeleteLinkTableRelationships (fullRelationshipClassName, BeSQLite::EC::ECInstanceId (referencingElId),
                                                                        BeSQLite::EC::ECInstanceId (referencedElId));

    return (BeSQLite::DbResult::BE_SQLITE_OK == status) ? Dgn::DgnDbStatus::Success : Dgn::DgnDbStatus::BadRequest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CompositeElement::RemoveOverlapedElement (Dgn::DgnElementId overlapedElementId)
    {
    Dgn::DgnDbR dgnDb = GetDgnDb();
    // unused - Dgn::DgnDbStatus status;
    Dgn::DgnElementId id = GetElementId();
    if (elementsHaveRelationship (dgnDb, id, overlapedElementId, SPATIALCOMPOSITION_SCHEMA (SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements)))
        {
        return deleteRelationship (dgnDb, id, overlapedElementId, SPATIALCOMPOSITION_SCHEMA (SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements));
        }
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CompositeElement::_LoadFromDb ()
    {
    T_Super::_LoadFromDb ();
    m_composedElementId = GetPropertyValueId<Dgn::DgnElementId> (prop_ComposingElement ());
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                   01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void CompositeElement::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);

    if (auto spatialElement = dynamic_cast<CompositeElementCP>(&source))
        {
        m_composedElementId = spatialElement->m_composedElementId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CompositeElement::_OnInsert
(
)
    {
    if (m_composedElementId.IsValid ())
        SetComposedElementId (m_composedElementId);
    CalculateProperties ();
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CompositeElement::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    CalculateProperties ();
    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Joana.Smitaite                  01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus CompositeElement::_OnDelete() const
    {
    //Preserve referential integrity of decomposed Allocation Volumes.
    Dgn::ElementIterator itor = MakeIterator(SPATIALCOMPOSITION_SCHEMA_NAME ":CompositeElement");
    Dgn::DgnDbR db = GetDgnDb();
    Dgn::DgnElementId nullParentId;

    for (Dgn::DgnElementId id : itor.BuildIdList<Dgn::DgnElementId>())
        {
        auto avPtr = db.Elements().GetForEdit<CompositeElement>(id);
        if (avPtr.IsValid())
            {
            avPtr->SetComposedElementId (nullParentId);
            db.Elements().Update (*avPtr);
            }
        }

    return __super::_OnDelete();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::Render::GeometryParams CompositeElement::_CreateGeometryParameters()
{
    return Dgn::Render::GeometryParams();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double CompositeElement::GetFootprintArea() const
    {
    return dynamic_cast<Dgn::DgnElementCP>(this)->GetPropertyValueDouble(prop_FootprintArea());
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CompositeElement::CalculateProperties()
    {
    CurveVectorPtr curve = DgnGeometryUtils::GetSliceAtZero(*this);
    if (curve.IsNull())
        {
        SetFootprintArea (0.0);
        return;
        }
        
    double area = GeometryUtils::GetCurveArea(*curve);

    SetFootprintArea(area);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompositeElement::SetFootprintShape(CurveVectorCPtr curveVector, bool updatePlacementOrigin)
    {
    BeAssert(!"CompositeElement::SetFootprintShape should be implemented in children classes");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Brien.Bastings              09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CompositeElement::DrawOutline(CurveVectorCR curves, Dgn::Render::GraphicBuilderR graphic)
    {
    if (1 > curves.size())
        {
        return;
        }

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve : curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                {
                continue;
                }

            DrawOutline(*curve->GetChildCurveVectorCP(), graphic);
            }
        }
    else if (curves.IsClosedPath())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType();

        const_cast <CurveVectorR> (curves).SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        graphic.AddCurveVector(curves, false);
        const_cast <CurveVectorR> (curves).SetBoundaryType(saveType);
        }
    else
        {
        // Open and none path types ok...
        graphic.AddCurveVector(curves, false);
        }
}

END_SPATIALCOMPOSITION_NAMESPACE

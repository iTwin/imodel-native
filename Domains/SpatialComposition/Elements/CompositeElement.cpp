#include <SpatialComposition/Elements/SpatialCompositionElementsApi.h>

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
    Dgn::DgnDbStatus status;
    Dgn::DgnElementId id = GetElementId();
    if (elementsHaveRelationship (dgnDb, id, overlapedElementId, SPATIALCOMPOSITION_SCHEMA (SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements)))
        {
        return deleteRelationship (dgnDb, id, overlapedElementId, SPATIALCOMPOSITION_SCHEMA (SPATIALCOMPOSITION_REL_CompositeOverlapsSpatialElements));
        }
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mykolas.Simutis                 06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus AllocatedVolume::_LoadFromDb ()
    {
    T_Super::_LoadFromDb ();
    m_composedElementId = GetPropertyValueId<DgnElementId> (prop_ComposingElement ());
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mykolas.Simutis                  07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AllocatedVolume::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);

    if (auto spatialElement = dynamic_cast<AllocatedVolumeCP>(&source))
        {
        m_composedElementId = spatialElement->m_composedElementId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus AllocatedVolume::_OnInsert
(
)
    {
    if (m_composedElementId.IsValid ())
        SetComposedElementId (m_composedElementId);
    CalculateProperties ();
    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus AllocatedVolume::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    CalculateProperties ();

    if (GetRelatedAllocationRequirement ().IsValid())
        if (GetRelatedAllocationRequirement()->GetTypeCost() != -1)
            {
            SetCost(UnitConverter::ToSquareFeet(GetFootprintArea ()) * GetRelatedAllocationRequirement ()->GetTypeCost ());
            }

    return T_Super::_OnUpdate (original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Wouter.Rombouts                 10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus AllocatedVolume::_OnDelete() const
    {
    //Preserve referential integrity of decomposed Allocation Volumes.
    ElementIterator itor = MakeIterator(SPATIALCOMPOSITION_SCHEMA_NAME ":CompositeVolume");
    DgnDbR db = GetDgnDb();
    Dgn::DgnElementId nullParentId;

    for (DgnElementId id : itor.BuildIdList<DgnElementId>())
        {
        auto avPtr = db.Elements().GetForEdit<AllocatedVolume>(id);
        if (avPtr.IsValid())
            {
            avPtr->SetComposedElementId (nullParentId);
            db.Elements().Update (*avPtr);
            }
        }

    return __super::_OnDelete();
    }

END_SPATIALCOMPOSITION_NAMESPACE

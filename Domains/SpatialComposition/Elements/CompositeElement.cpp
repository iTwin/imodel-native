#include <SpatialComposition/Elements/CompositeElement.h>

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

END_SPATIALCOMPOSITION_NAMESPACE

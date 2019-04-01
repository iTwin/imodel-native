/*--------------------------------------------------------------------------------------+
|
|     $Source: Elements/IClassified.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/IClassified.h"
#include "PublicApi/Classification.h"
#include "PublicApi/ClassificationSystem.h"

USING_NAMESPACE_CLASSIFICATIONSYSTEMS
USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ECN::ECRelationshipClassCR IClassified::GetRelClassIClassifiedIsClassifiedAs
(
    Dgn::DgnDbR db
)
    {
    return static_cast<ECN::ECRelationshipClassCR>(*db.Schemas().GetClass(CLASSIFICATIONSYSTEMS_SCHEMA_NAME, CLASSIFICATIONSYSTEMS_REL_IClassifiedIsClassifiedAs));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ElementIdIterator IClassified::MakeClassificationsIterator() const
    {
    Dgn::DgnDbR db = _GetAsDgnElement().GetDgnDb();
    Dgn::DgnElementId sourceId = _GetAsDgnElement().GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassIClassifiedIsClassifiedAs(db);

    return RelationshipUtils::MakeTargetIterator(db, relClass, sourceId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
ClassificationCPtr IClassified::GetClassification(ClassificationSystemCR system) const
    {
    Dgn::DgnDbR db = _GetAsDgnElement().GetDgnDb();

    Dgn::DgnElementId elemId = system.GetElementId();
    BeAssert(elemId.IsValid());

    for (ElementIdIteratorEntry classificationEntry : MakeClassificationsIterator())
        {
        ClassificationCPtr classification = Classification::Get(db, classificationEntry.GetElementId());
        if (classification.IsNull())
            continue;

        if (classification->GetClassificationSystemId() == elemId)
            return classification;
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void IClassified::AddClassification(ClassificationCR classification)
    {
    Dgn::DgnElementId sourceId = _GetAsDgnElement().GetElementId();
    BeAssert(sourceId.IsValid());

    if (IsClassifiedAs(classification))
        return;

    ClassificationSystemCPtr system = classification.GetDgnDb().Elements().Get<ClassificationSystem>(classification.GetClassificationSystemId());
    BeAssert(system.IsValid());
    if (GetClassification(*system).IsValid())
        return;

    Dgn::DgnDbR db = _GetAsDgnElement().GetDgnDb();
    Dgn::DgnElementId targetId = classification.GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassIClassifiedIsClassifiedAs(db);

    RelationshipUtils::InsertRelationship(db, relClass, sourceId, targetId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
void IClassified::RemoveClassification(ClassificationCR classification)
    {
    Dgn::DgnDbR db = _GetAsDgnElement().GetDgnDb();
    Dgn::DgnElementId sourceId = _GetAsDgnElement().GetElementId();
    Dgn::DgnElementId targetId = classification.GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassIClassifiedIsClassifiedAs(db);

    BeAssert(sourceId.IsValid());

    RelationshipUtils::DeleteRelationships(db, relClass, sourceId, targetId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                06/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IClassified::IsClassifiedAs(ClassificationCR classification) const
    {

    Dgn::DgnDbR db = _GetAsDgnElement().GetDgnDb();
    Dgn::DgnElementId sourceId = _GetAsDgnElement().GetElementId();
    Dgn::DgnElementId targetId = classification.GetElementId();
    ECN::ECRelationshipClassCR relClass = GetRelClassIClassifiedIsClassifiedAs(db);

    BeAssert(sourceId.IsValid());

    return RelationshipUtils::RelationshipExists(db, relClass, sourceId, targetId);
    }
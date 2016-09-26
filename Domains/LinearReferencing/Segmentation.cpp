/*--------------------------------------------------------------------------------------+
|
|     $Source: Segmentation.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"

enum class LinearlyReferencedLocationType : int
    {
    AtLocation = 1 << 0,
    FromToLocation = 1 << 1
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
inline LinearlyReferencedLocationType operator|(LinearlyReferencedLocationType a, LinearlyReferencedLocationType b)
    {
    return static_cast<LinearlyReferencedLocationType>(static_cast<int>(a) | static_cast<int>(b));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
inline LinearlyReferencedLocationType operator&(LinearlyReferencedLocationType a, LinearlyReferencedLocationType b)
    {
    return static_cast<LinearlyReferencedLocationType>(static_cast<int>(a) & static_cast<int>(b));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
inline void operator|=(LinearlyReferencedLocationType& a, LinearlyReferencedLocationType b)
    {
    a = static_cast<LinearlyReferencedLocationType>(static_cast<int>(a) | static_cast<int>(b));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationType GetLinearlyReferencedLocationTypesToUse(DgnDbR dgnDb, bset<DgnClassId> const& iLinearlyLocatedClassIds)
    {
    LinearlyReferencedLocationType locationType = static_cast<LinearlyReferencedLocationType>(0);
    for (auto const& classId : iLinearlyLocatedClassIds)
        {
        auto ecClassCP = dgnDb.Schemas().GetECClass(classId);
        if (!ecClassCP)
            {
            locationType = LinearlyReferencedLocationType::AtLocation | LinearlyReferencedLocationType::FromToLocation;
            break;
            }

        auto customAttrPtr = ecClassCP->GetCustomAttribute(BLR_SCHEMA_NAME, BLR_CA_ILinearlyLocatedSegmentationHints);
        if (customAttrPtr.IsNull())
            {
            locationType = LinearlyReferencedLocationType::AtLocation | LinearlyReferencedLocationType::FromToLocation;
            break;
            }

        ECValue val;
        if (ECObjectsStatus::Success != customAttrPtr->GetValue(val, BLR_CAPROP_SupportedLinearlyReferencedLocationTypes))
            {
            locationType = LinearlyReferencedLocationType::AtLocation | LinearlyReferencedLocationType::FromToLocation;
            break;
            }

        locationType |= static_cast<LinearlyReferencedLocationType>(val.GetInteger());
        }

    if (static_cast<int>(locationType) == 0)
        locationType = LinearlyReferencedLocationType::AtLocation | LinearlyReferencedLocationType::FromToLocation;

    return locationType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendILinearlyLocatedClassIdsECSQL(bset<DgnClassId> const& iLinearlyLocatedClassIds, Utf8StringR ecSql)
    {
    if (iLinearlyLocatedClassIds.empty())
        return;

    ecSql.append("AND ECClassId ");

    if (1 == iLinearlyLocatedClassIds.size())
        ecSql.append(Utf8PrintfString("= %d ", iLinearlyLocatedClassIds.begin()->GetValue()).c_str());
    else
        {
        ecSql.append("IN (");
        for (auto classId : iLinearlyLocatedClassIds)
            ecSql.append(Utf8PrintfString("%d,", classId.GetValue()).c_str());

        ecSql.erase(ecSql.at(ecSql.size() - 1)); // Removing last comma
        ecSql.append(") ");
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetAtLocationOnlyECSQL(bset<DgnClassId> const& iLinearlyLocatedClassIds, Utf8StringR ecSql, 
    NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.ECInstanceId "
        "FROM (SELECT ECInstanceId, ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

    AppendILinearlyLocatedClassIdsECSQL(iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation "
        "WHERE (LinearlyLocated.ECInstanceId = AtLocation.ElementId ");

    if (fromDistanceAlong.IsValid() && toDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart >= ? AND AtLocation.AtPosition.DistanceAlongFromStart <= ? ) ");
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        }
    else if (fromDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart >= ?) ");
        bindVals.push_back(fromDistanceAlong.Value());
        }
    else if (toDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart <= ? ) ");
        bindVals.push_back(toDistanceAlong.Value());
        }
    else
        ecSql.append(") ");

    ecSql.append("ORDER BY AtLocation.AtPosition.DistanceAlongFromStart;");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetFromToLocationOnlyECSQL(bset<DgnClassId> const& iLinearlyLocatedClassIds, Utf8StringR ecSql, 
    NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   FromToLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart, FromToLocation.ECInstanceId "
        "FROM (SELECT ECInstanceId, ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

    AppendILinearlyLocatedClassIdsECSQL(iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation "
        "WHERE (LinearlyLocated.ECInstanceId = FromToLocation.ElementId ");

    if (fromDistanceAlong.IsValid() && toDistanceAlong.IsValid())
        {
        ecSql.append("((FromToLocation.FromPosition.DistanceAlongFromStart >= ? AND FromToLocation.FromPosition.DistanceAlongFromStart <= ?) "
            "OR (FromToLocation.ToPosition.DistanceAlongFromStart >= ? AND FromToLocation.ToPosition.DistanceAlongFromStart <= ?)) ");
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        }
    else if (fromDistanceAlong.IsValid())
        {
        ecSql.append("(FromToLocation.FromPosition.DistanceAlongFromStart >= ? OR FromToLocation.ToPosition.DistanceAlongFromStart >= ?)");
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(fromDistanceAlong.Value());
        }
    else if (toDistanceAlong.IsValid())
        {
        ecSql.append("(FromToLocation.FromPosition.DistanceAlongFromStart <= ? OR FromToLocation.ToPosition.DistanceAlongFromStart <= ?) ");
        bindVals.push_back(toDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        }
    else
        ecSql.append(") ");

    ecSql.append("ORDER BY FromToLocation.FromPosition.DistanceAlongFromStart");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetAnyLocationECSQL(bset<DgnClassId> const& iLinearlyLocatedClassIds, Utf8StringR ecSql, 
    NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   coalesce(AtLocation.FromPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart), "        
        "   coalesce(AtLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart), "
        "   coalesce(AtLocation.ECInstanceId, FromToLocation.ECInstanceId) "
        "FROM ((SELECT ECInstanceId, ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

    AppendILinearlyLocatedClassIdsECSQL(iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated "
        "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation ON LinearlyLocated.ECInstanceId = AtLocation.ElementId) "
        "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation ON LinearlyLocated.ECInstanceId = FromToLocation.ElementId ");

    if (fromDistanceAlong.IsValid() && toDistanceAlong.IsValid())
        {
        ecSql.append("WHERE (AtLocation.AtPosition.DistanceAlongFromStart >= ? AND AtLocation.AtPosition.DistanceAlongFromStart <= ? ) ");
        ecSql.append("OR (FromToLocation.FromPosition.DistanceAlongFromStart >= ? AND FromToLocation.FromPosition.DistanceAlongFromStart <= ?) "
            "OR (FromToLocation.ToPosition.DistanceAlongFromStart >= ? AND FromToLocation.ToPosition.DistanceAlongFromStart <= ?) ");
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        bindVals.push_back(fromDistanceAlong.Value()); bindVals.push_back(toDistanceAlong.Value());
        }
    else if (fromDistanceAlong.IsValid())
        {
        ecSql.append("WHERE AtLocation.AtPosition.DistanceAlongFromStart >= ? ");
        ecSql.append("OR FromToLocation.FromPosition.DistanceAlongFromStart >= ? OR FromToLocation.ToPosition.DistanceAlongFromStart >= ? ");
        bindVals.push_back(fromDistanceAlong.Value()); 
        bindVals.push_back(fromDistanceAlong.Value());
        bindVals.push_back(fromDistanceAlong.Value());
        }
    else if (toDistanceAlong.IsValid())
        {
        ecSql.append("WHERE AtLocation.AtPosition.DistanceAlongFromStart <= ? ");
        ecSql.append("OR FromToLocation.FromPosition.DistanceAlongFromStart <= ? OR FromToLocation.ToPosition.DistanceAlongFromStart <= ? ");
        bindVals.push_back(toDistanceAlong.Value()); 
        bindVals.push_back(toDistanceAlong.Value());
        bindVals.push_back(toDistanceAlong.Value());
        }

    ecSql.append("ORDER BY coalesce(AtLocation.FromPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart);");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearSegment> ISegmentableLinearElement::_QuerySegments(bset<DgnClassId> const& iLinearlyLocatedClassIds, NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) const
    {
    auto& dgnDb = ToElement().GetDgnDb();
    auto locationType = GetLinearlyReferencedLocationTypesToUse(dgnDb, iLinearlyLocatedClassIds);
    
    Utf8String ecSql;
    bvector<double> bindVals;

    if (LinearlyReferencedLocationType::AtLocation == (locationType & LinearlyReferencedLocationType::AtLocation) &&
        LinearlyReferencedLocationType::FromToLocation != (locationType & LinearlyReferencedLocationType::FromToLocation))
        {
        GetAtLocationOnlyECSQL(iLinearlyLocatedClassIds, ecSql, fromDistanceAlong, toDistanceAlong, bindVals);
        }
    else if (LinearlyReferencedLocationType::AtLocation != (locationType & LinearlyReferencedLocationType::AtLocation) &&
        LinearlyReferencedLocationType::FromToLocation == (locationType & LinearlyReferencedLocationType::FromToLocation))
        {
        GetFromToLocationOnlyECSQL(iLinearlyLocatedClassIds, ecSql, fromDistanceAlong, toDistanceAlong, bindVals);
        }
    else
        {
        GetAnyLocationECSQL(iLinearlyLocatedClassIds, ecSql, fromDistanceAlong, toDistanceAlong, bindVals);
        }

    auto& dgnDbR = ToElement().GetDgnDb();
    auto stmtPtr = dgnDbR.GetPreparedECSqlStatement(ecSql.c_str());
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, ToElement().GetElementId());

    int idx = 2;
    for (auto val : bindVals)
        {
        stmtPtr->BindDouble(idx++, val);
        }

    bvector<LinearSegment> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        retVal.push_back(LinearSegment(
            stmtPtr->GetValueId<DgnElementId>(0), 
            stmtPtr->GetValueId<DgnClassId>(1), 
            stmtPtr->GetValueDouble(2), stmtPtr->GetValueDouble(3),
            stmtPtr->GetValueId<LinearlyReferencedLocationId>(4)));
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CascadeFromToLocationChangesAlgorithm::_FindFromToLocationChanges(bvector<LinearlyReferencedFromToLocationCP>& fromToLocationsChanged)
    {
    for (auto& locationId : GetReplacement()._GetLinearlyReferencedFromToLocationIdsAccessed())
        {
        auto locationCP = GetReplacement().GetLinearlyReferencedFromToLocation(locationId);
        if (!locationCP->HasChanges())
            continue;

        fromToLocationsChanged.push_back(locationCP);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CascadeFromToLocationChangesAlgorithm::_Prepare(ILinearElementSourceCR source)
    {
    if (CascadeLocationChangesAction::None == GetAction())
        return DgnDbStatus::Success;

    bvector<LinearlyReferencedFromToLocationCP> fromToLocationsChanged;
    _FindFromToLocationChanges(fromToLocationsChanged);

    if (CascadeLocationChangesAction::OnlyIfLocationsChanged == GetAction() && fromToLocationsChanged.empty())
        return DgnDbStatus::Success;

    auto iSegmentableLinearElemCP = dynamic_cast<ISegmentableLinearElementCP>(GetReplacement().GetLinearElement());
    BeAssert(iSegmentableLinearElemCP != nullptr);

    bset<DgnClassId> classIds;
    classIds.insert(GetOriginal().ToElement().GetElementClassId());
    auto linearSegmentVector = iSegmentableLinearElemCP->QuerySegments(classIds);

    return _Prepare(source, linearSegmentVector, fromToLocationsChanged);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CascadeFromToLocationChangesAlgorithm::_Prepare(ILinearElementSourceCR source,
    bvector<LinearSegment> const& existingLinearSegments, bvector<LinearlyReferencedFromToLocationCP> const& fromToLocationsChanged)
    {
    auto originElementId = GetReplacement().ToElement().GetElementId();
    for (auto iter = existingLinearSegments.begin(); iter != existingLinearSegments.end(); ++iter)
        {
        auto const& linearSegmentCR = *iter;
        if (linearSegmentCR.GetILinearlyLocatedId() != originElementId)
            continue;

        LinearlyReferencedFromToLocationCP fromToLocationCP = nullptr;

        for (auto locationCP : fromToLocationsChanged)
            {
            if (fabs(locationCP->GetOriginalFromPosition().GetDistanceAlongFromStart() - linearSegmentCR.GetStartDistanceAlong()) < DBL_EPSILON &&
                fabs(locationCP->GetOriginalToPosition().GetDistanceAlongFromStart() - linearSegmentCR.GetStopDistanceAlong()) < DBL_EPSILON)
                {
                fromToLocationCP = locationCP;
                break;
                }
            }

        if (!fromToLocationCP)
            continue;

        // If segment being processed not at the front, and its start distance along changed...
        if (iter != existingLinearSegments.begin() && 
            fabs(fromToLocationCP->GetFromPosition().GetDistanceAlongFromStart() - linearSegmentCR.GetStartDistanceAlong()) > DBL_EPSILON)
            {
            double newFromDistanceAlong = fromToLocationCP->GetFromPosition().GetDistanceAlongFromStart();

            LinearSegment const* prevSegment = iter - 1;
            if (fromToLocationCP->GetFromPosition().GetDistanceAlongFromStart() < prevSegment->GetStartDistanceAlong())
                return DgnDbStatus::BadRequest; // New start distance along can't go before previous segment's start distance along

            auto impactedDgnElementPtr = source.ToElement().GetDgnDb().Elements().GetForEdit<DgnElement>(linearSegmentCR.GetILinearlyLocatedId());
            auto linearlyLocatedP = dynamic_cast<ILinearlyLocatedP>(impactedDgnElementPtr.get());
            auto prevLocationP = linearlyLocatedP->GetLinearlyReferencedFromToLocationP(prevSegment->GetLinearlyReferencedLocationId());
            prevLocationP->GetToPositionR().SetDistanceAlongFromStart(newFromDistanceAlong);

            _AddImpactedDgnElement(impactedDgnElementPtr);
            }

        // If segment being processed not at the end, and its stop distance along changed...
        if (iter != (existingLinearSegments.end() - 1) &&
            fabs(fromToLocationCP->GetToPosition().GetDistanceAlongFromStart() - linearSegmentCR.GetStopDistanceAlong()) > DBL_EPSILON)
            {
            double newToDistanceAlong = fromToLocationCP->GetToPosition().GetDistanceAlongFromStart();

            LinearSegment const* nextSegment = iter + 1;
            if (fromToLocationCP->GetToPosition().GetDistanceAlongFromStart() > nextSegment->GetStopDistanceAlong())
                return DgnDbStatus::BadRequest; // New stop distance along can't go after next segment's stop distance along

            auto impactedDgnElementPtr = source.ToElement().GetDgnDb().Elements().GetForEdit<DgnElement>(linearSegmentCR.GetILinearlyLocatedId());
            auto linearlyLocatedP = dynamic_cast<ILinearlyLocatedP>(impactedDgnElementPtr.get());
            auto prevLocationP = linearlyLocatedP->GetLinearlyReferencedFromToLocationP(nextSegment->GetLinearlyReferencedLocationId());
            prevLocationP->GetToPositionR().SetDistanceAlongFromStart(newToDistanceAlong);

            _AddImpactedDgnElement(impactedDgnElementPtr);
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CascadeFromToLocationChangesAlgorithm::_Commit(ILinearElementSourceCR source)
    {
    DgnDbStatus status = DgnDbStatus::Success;
    for (auto& dgnElementPtr : _GetImpactedDgnElements())
        if (dgnElementPtr->Update(&status).IsNull())
            return status;

    return status;
    }
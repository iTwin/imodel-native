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
        "SELECT LinearlyLocated.ECInstanceId, AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.AtPosition.DistanceAlongFromStart "
        "FROM (SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

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
        "SELECT LinearlyLocated.ECInstanceId, FromToLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart "
        "FROM (SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

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
        "SELECT LinearlyLocated.ECInstanceId, coalesce(AtLocation.FromPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart), "
        "coalesce(AtLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart)"
        "FROM ((SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " WHERE ILinearElement = ? ");

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
        auto dgnElementCPtr = dgnDbR.Elements().GetElement(stmtPtr->GetValueId<DgnElementId>(0));
        auto linearlyLocatedCP = dynamic_cast<ILinearlyLocatedCP>(dgnElementCPtr.get());
        BeAssert(linearlyLocatedCP != nullptr);

        retVal.push_back(LinearSegment(*linearlyLocatedCP, stmtPtr->GetValueDouble(1), stmtPtr->GetValueDouble(2)));
        }

    return retVal;
    }
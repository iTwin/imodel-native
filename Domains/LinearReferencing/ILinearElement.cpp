/*--------------------------------------------------------------------------------------+
|
|     $Source: ILinearElement.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/ILinearElement.h>

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

LinearLocation::LinearLocation(ILinearlyLocatedCR linearlyLocated, double startDistanceAlong, double stopDistanceAlong) :
    m_linearlyLocatedId(linearlyLocated.ToElement().GetElementId()), m_linearlyLocatedClassId(linearlyLocated.ToElement().GetElementClassId()),
    m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong), m_locationId(LinearlyReferencedLocationId())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearLocation::operator==(LinearLocationCR rhs) const
    {
    if (m_linearlyLocatedClassId == rhs.m_linearlyLocatedClassId &&
        m_linearlyLocatedId == rhs.m_linearlyLocatedId &&
        m_locationId == rhs.m_locationId &&
        m_startDistanceAlong == rhs.m_startDistanceAlong &&
        m_stopDistanceAlong == rhs.m_stopDistanceAlong)
        {
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearElement::SetILinearElementSource(ILinearElementSourceCP linearElementSource)
    {
    if (linearElementSource)
        {
        auto relClassId = ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearElementSourceProvidesILinearElements);
        ToElementR().SetPropertyValue("ILinearElementSource", linearElementSource->ToElement().GetElementId(), relClassId);
        }
    else
        ToElementR().SetPropertyValue("ILinearElementSource", DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationType GetLinearlyReferencedLocationTypesToUse(DgnDbR dgnDb, bset<DgnClassId> const& iLinearlyLocatedClassIds)
    {
    LinearlyReferencedLocationType locationType = static_cast<LinearlyReferencedLocationType>(0);
    for (auto const& classId : iLinearlyLocatedClassIds)
        {
        auto ecClassCP = dgnDb.Schemas().GetClass(classId);
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

    ecSql.append("AND meta.ClassHasAllBaseClasses.TargetECInstanceId ");

    if (1 == iLinearlyLocatedClassIds.size())
        ecSql.append(Utf8PrintfString("= %d ", iLinearlyLocatedClassIds.begin()->GetValue()).c_str());
    else
        {
        ecSql.append("IN (");
        for (auto classId : iLinearlyLocatedClassIds)
            ecSql.append(Utf8PrintfString("%d,", classId.GetValue()).c_str());

        ecSql.erase(ecSql.begin() + ecSql.size() - 1); // Removing last comma
        ecSql.append(") ");
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetAtLocationOnlyECSQL(ILinearElement::QueryParams const& params, Utf8StringR ecSql, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.ECInstanceId "
        "FROM (SELECT il.ECInstanceId, il.ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " il, meta.ClassHasAllBaseClasses " 
        "WHERE ILinearElement.Id = ? AND meta.ClassHasAllBaseClasses.SourceECInstanceId = il.ECClassId ");

    AppendILinearlyLocatedClassIdsECSQL(params.m_iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation "
        "WHERE (LinearlyLocated.ECInstanceId = AtLocation.Element.Id ");

    Utf8String fromCompOp = params.m_fromComparisonOption == ILinearElement::ComparisonOption::Inclusive ? ">=" : ">";
    Utf8String toCompOp = params.m_toComparisonOption == ILinearElement::ComparisonOption::Inclusive ? "<=" : "<";

    if (params.m_fromDistanceAlong.IsValid() && params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? ) ");
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        }
    else if (params.m_fromDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ?) ");
        bindVals.push_back(params.m_fromDistanceAlong.Value());
        }
    else if (params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("AND AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? ) ");
        bindVals.push_back(params.m_toDistanceAlong.Value());
        }
    else
        ecSql.append(") ");

    ecSql.append("ORDER BY AtLocation.AtPosition.DistanceAlongFromStart;");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetFromToLocationOnlyECSQL(ILinearElement::QueryParams const& params, Utf8StringR ecSql, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   FromToLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart, FromToLocation.ECInstanceId "
        "FROM (SELECT il.ECInstanceId, il.ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " il, meta.ClassHasAllBaseClasses " 
        "WHERE ILinearElement.Id = ? AND meta.ClassHasAllBaseClasses.SourceECInstanceId = il.ECClassId ");

    AppendILinearlyLocatedClassIdsECSQL(params.m_iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated, "
        BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation "
        "WHERE LinearlyLocated.ECInstanceId = FromToLocation.Element.Id ");

    Utf8String fromCompOp = params.m_fromComparisonOption == ILinearElement::ComparisonOption::Inclusive ? ">=" : ">";
    Utf8String toCompOp = params.m_toComparisonOption == ILinearElement::ComparisonOption::Inclusive ? "<=" : "<";

    if (params.m_fromDistanceAlong.IsValid() && params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("AND ((FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ?) "
            "OR (FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ?) "
            "OR (FromToLocation.FromPosition.DistanceAlongFromStart <= ? AND FromToLocation.ToPosition.DistanceAlongFromStart >= ?)) ");
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        }
    else if (params.m_fromDistanceAlong.IsValid())
        {
        ecSql.append("AND (FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp); 
        ecSql.append(" ?)");
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_fromDistanceAlong.Value());
        }
    else if (params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("AND (FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ?) ");
        bindVals.push_back(params.m_toDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        }

    ecSql.append("ORDER BY FromToLocation.FromPosition.DistanceAlongFromStart");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GetAnyLocationECSQL(ILinearElement::QueryParams const& params, Utf8StringR ecSql, bvector<double>& bindVals)
    {
    ecSql.append(
        "SELECT DISTINCT LinearlyLocated.ECInstanceId, LinearlyLocated.ClassId, "
        "   coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart), "        
        "   coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart), "
        "   coalesce(AtLocation.ECInstanceId, FromToLocation.ECInstanceId) "
        "FROM (SELECT il.ECInstanceId, il.ECClassId ClassId FROM " BLR_SCHEMA(BLR_CLASS_ILinearlyLocated) " il, meta.ClassHasAllBaseClasses " 
        "WHERE ILinearElement.Id = ? AND meta.ClassHasAllBaseClasses.SourceECInstanceId = il.ECClassId ");

    AppendILinearlyLocatedClassIdsECSQL(params.m_iLinearlyLocatedClassIds, ecSql);

    ecSql.append(") LinearlyLocated "
        "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation ON LinearlyLocated.ECInstanceId = AtLocation.Element.Id "
        "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation ON LinearlyLocated.ECInstanceId = FromToLocation.Element.Id ");

    Utf8String fromCompOp = params.m_fromComparisonOption == ILinearElement::ComparisonOption::Inclusive ? ">=" : ">";
    Utf8String toCompOp = params.m_toComparisonOption == ILinearElement::ComparisonOption::Inclusive ? "<=" : "<";

    if (params.m_fromDistanceAlong.IsValid() && params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("WHERE (AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? ) ");
        ecSql.append("OR (FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ?) "
            "OR (FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? AND FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ?) "
            "OR (FromToLocation.FromPosition.DistanceAlongFromStart <= ? AND FromToLocation.ToPosition.DistanceAlongFromStart >= ? ) ");
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value()); bindVals.push_back(params.m_toDistanceAlong.Value());
        }
    else if (params.m_fromDistanceAlong.IsValid())
        {
        ecSql.append("WHERE AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? ");
        ecSql.append("OR FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(fromCompOp);
        ecSql.append(" ? ");
        bindVals.push_back(params.m_fromDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value());
        bindVals.push_back(params.m_fromDistanceAlong.Value());
        }
    else if (params.m_toDistanceAlong.IsValid())
        {
        ecSql.append("WHERE AtLocation.AtPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? ");
        ecSql.append("OR FromToLocation.FromPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart ");
        ecSql.append(toCompOp);
        ecSql.append(" ? ");
        bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_toDistanceAlong.Value());
        bindVals.push_back(params.m_toDistanceAlong.Value());
        }

    ecSql.append("ORDER BY coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart);");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearLocation> ILinearElement::_QueryLinearLocations(QueryParams const& params) const
    {
    auto& dgnDb = ToElement().GetDgnDb();
    auto locationType = GetLinearlyReferencedLocationTypesToUse(dgnDb, params.m_iLinearlyLocatedClassIds);
    
    Utf8String ecSql;
    bvector<double> bindVals;

    if (LinearlyReferencedLocationType::AtLocation == (locationType & LinearlyReferencedLocationType::AtLocation) &&
        LinearlyReferencedLocationType::FromToLocation != (locationType & LinearlyReferencedLocationType::FromToLocation))
        {
        GetAtLocationOnlyECSQL(params, ecSql, bindVals);
        }
    else if (LinearlyReferencedLocationType::AtLocation != (locationType & LinearlyReferencedLocationType::AtLocation) &&
        LinearlyReferencedLocationType::FromToLocation == (locationType & LinearlyReferencedLocationType::FromToLocation))
        {
        GetFromToLocationOnlyECSQL(params, ecSql, bindVals);
        }
    else
        {
        GetAnyLocationECSQL(params, ecSql, bindVals);
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

    bvector<LinearLocation> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        retVal.push_back(LinearLocation(
            stmtPtr->GetValueId<DgnElementId>(0), 
            stmtPtr->GetValueId<DgnClassId>(1), 
            stmtPtr->GetValueDouble(2), stmtPtr->GetValueDouble(3),
            stmtPtr->GetValueId<LinearlyReferencedLocationId>(4)));
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bset<DgnElementId> ILinearElementSource::QueryLinearElements() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(ToElement().GetDgnDb(), "SELECT TargetECInstanceId FROM " 
        BLR_SCHEMA(BLR_REL_ILinearElementSourceProvidesILinearElements) " WHERE SourceECInstanceId = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, ToElement().GetElementId());

    bset<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocated::ILinearlyLocated()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::_SetLinearElement(DgnElementId elementId)
    {
    DgnDbStatus status = ToElementR().SetPropertyValue(BLR_PROP_ILinearlyLocated_ILinearElement, elementId, 
        DgnClassId(ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearlyLocatedAlongILinearElement)));
    BeAssert(DgnDbStatus::Success == status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ILinearlyLocated::GetLinearElementId() const 
    {
    return ToElement().GetPropertyValueId<DgnElementId>(BLR_PROP_ILinearlyLocated_ILinearElement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearElementCP ILinearlyLocated::GetLinearElement() const
    {
    return dynamic_cast<ILinearElementCP>(ToElement().GetDgnDb().Elements().GetElement(GetLinearElementId()).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearlyReferencedLocationId> ILinearlyLocated::QueryLinearlyReferencedLocationIds() const
    {
    bvector<LinearlyReferencedLocationId> retVal;

    auto stmtPtr = ToElement().GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedLocation) " WHERE Element.Id = ? ORDER BY ECInstanceId;");

    stmtPtr->BindId(1, ToElement().GetElementId());

    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<LinearlyReferencedLocationId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationCP ILinearlyLocated::GetLinearlyReferencedLocation(LinearlyReferencedLocationId id) const
    {
    LinearlyReferencedLocationCP retVal = GetLinearlyReferencedAtLocation(id);
    if (!retVal)
        retVal = GetLinearlyReferencedFromToLocation(id);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationP ILinearlyLocated::GetLinearlyReferencedLocationP(LinearlyReferencedLocationId id)
    {
    LinearlyReferencedLocationP retVal = GetLinearlyReferencedAtLocationP(id);
    if (!retVal)
        retVal = GetLinearlyReferencedFromToLocationP(id);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationCP ILinearlyLocated::GetLinearlyReferencedAtLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedAtLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationP ILinearlyLocated::GetLinearlyReferencedAtLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(ToElementR(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP ILinearlyLocated::GetLinearlyReferencedFromToLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedFromToLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP ILinearlyLocated::GetLinearlyReferencedFromToLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedFromToLocation>(ToElementR(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocated::_AddLinearlyReferencedLocation(LinearlyReferencedLocationR location)
    {
    DgnElement::MultiAspect::AddAspect(ToElementR(), location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedAttribution::ILinearlyLocatedAttribution()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedElement::ILinearlyLocatedElement()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedSingleAt::ILinearlyLocatedSingleAt(CreateAtParams const& params)
    {
    m_unpersistedAtLocationPtr = LinearlyReferencedAtLocation::Create(DistanceExpression(params.m_atPosition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationCP ILinearlyLocatedSingleAt::GetSingleLinearlyReferencedAtLocation() const
    {
    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    return GetLinearlyReferencedAtLocation(m_atLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationP ILinearlyLocatedSingleAt::GetSingleLinearlyReferencedAtLocationP()
    {
    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    return GetLinearlyReferencedAtLocationP(m_atLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleAt::GetAtDistanceAlongFromStart() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedAtLocationPtr->GetAtPosition().GetDistanceAlongFromStart();

    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedAtLocation(m_atLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetAtPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleAt::SetAtDistanceAlongFromStart(double newAt)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedAtLocationPtr->GetAtPositionR().SetDistanceAlongFromStart(newAt);
        return;
        }

    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedAtLocationP(m_atLocationAspectId);
    BeAssert(locationP);

    return locationP->GetAtPositionR().SetDistanceAlongFromStart(newAt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearlyLocatedSingleFromTo::ILinearlyLocatedSingleFromTo(CreateFromToParams const& params)
    {
    m_unpersistedFromToLocationPtr = LinearlyReferencedFromToLocation::Create(DistanceExpression(params.m_fromPosition), DistanceExpression(params.m_toPosition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleFromTo::GetFromDistanceAlongFromStart() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedFromToLocationPtr->GetFromPosition().GetDistanceAlongFromStart();

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedFromToLocation(m_fromToLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetFromPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP ILinearlyLocatedSingleFromTo::GetSingleLinearlyReferencedFromToLocationP()
    {
    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    BeAssert(locationP);

    return GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP ILinearlyLocatedSingleFromTo::GetSingleLinearlyReferencedFromToLocation() const
    {
    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    return GetLinearlyReferencedFromToLocation(m_fromToLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleFromTo::SetFromDistanceAlongFromStart(double newFrom)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedFromToLocationPtr->GetFromPositionR().SetDistanceAlongFromStart(newFrom);
        return;
        }

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    BeAssert(locationP);

    return locationP->GetFromPositionR().SetDistanceAlongFromStart(newFrom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleFromTo::GetToDistanceAlongFromStart() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedFromToLocationPtr->GetToPosition().GetDistanceAlongFromStart();

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationCP = ToLinearlyLocated().GetLinearlyReferencedFromToLocation(m_fromToLocationAspectId);
    BeAssert(locationCP);

    return locationCP->GetToPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedSingleFromTo::SetToDistanceAlongFromStart(double newFrom)
    {
    if (!ToElement().GetElementId().IsValid())
        {
        m_unpersistedFromToLocationPtr->GetToPositionR().SetDistanceAlongFromStart(newFrom);
        return;
        }

    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = ToLinearlyLocated().QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    auto locationP = ToLinearlyLocatedR().GetLinearlyReferencedFromToLocationP(m_fromToLocationAspectId);
    BeAssert(locationP);

    return locationP->GetToPositionR().SetDistanceAlongFromStart(newFrom);
    }

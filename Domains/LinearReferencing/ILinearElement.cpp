/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "LinearReferencingInternal.h"
#include <LinearReferencing/ILinearElement.h>

struct QueryLinearLocationsECSQLGen
{
private:
    struct ECSQLGenImpl
    {
    public:
        ECSQLGenImpl() {}

        virtual Utf8String GenSelect() = 0;
        virtual bool SelectDistinct() { return false; }
        virtual Utf8String GenFromJoin() = 0;
        virtual Utf8String GenWhere(NullableDouble const& from, bool inclusiveFrom, NullableDouble const& to, bool inclusiveTo, bvector<double>& bindVals) = 0;
        virtual Utf8String GenOrderBy() = 0;
    }; // ECSQLGenImpl

    struct AtAndFromToECSQLGenImpl : ECSQLGenImpl
    {
    public:
        AtAndFromToECSQLGenImpl() {}

        virtual bool SelectDistinct() override { return true; }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenSelect() override
            {
            return "coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart), "
                "coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart), "
                "coalesce(AtLocation.ECInstanceId, FromToLocation.ECInstanceId) ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/

        virtual Utf8String GenFromJoin() override 
            { 
            return "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation ON LinearlyLocated.InstanceId = AtLocation.Element.Id "
                "LEFT JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation ON LinearlyLocated.InstanceId = FromToLocation.Element.Id ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenWhere(NullableDouble const& from, bool inclusiveFrom, NullableDouble const& to, bool inclusiveTo, bvector<double>& bindVals) override
            {
            Utf8String fromCompOp = inclusiveFrom ? ">=" : ">";
            Utf8String toCompOp = inclusiveTo ? "<=" : "<";

            Utf8String ecSql;
            if (from.IsValid() && to.IsValid())
                {
                ecSql.append("(AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ?) OR (FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); ecSql.append(" ?) "
                    "OR (FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); 
                ecSql.append(" ?) OR (FromToLocation.FromPosition.DistanceAlongFromStart <= ? AND FromToLocation.ToPosition.DistanceAlongFromStart >= ?) ");

                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                }
            else if (from.IsValid())
                {
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? OR FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? ");

                bindVals.push_back(from.Value());
                bindVals.push_back(from.Value());
                bindVals.push_back(from.Value());
                }
            else if (to.IsValid())
                {
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ? OR FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); ecSql.append(" ? ");

                bindVals.push_back(to.Value());
                bindVals.push_back(to.Value());
                bindVals.push_back(to.Value());
                }
            else
                {
                ecSql.append("(AtLocation.AtPosition.DistanceAlongFromStart IS NOT NULL) OR ");
                ecSql.append("(FromToLocation.FromPosition.DistanceAlongFromStart IS NOT NULL) ");
                }

            return ecSql;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenOrderBy() override
            {
            return "coalesce(AtLocation.AtPosition.DistanceAlongFromStart, FromToLocation.FromPosition.DistanceAlongFromStart)";
            }
    }; // AtAndFromToECSQLGenImpl

    struct FromToECSQLGenImpl : ECSQLGenImpl
    {
    public:
        FromToECSQLGenImpl() {}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenSelect() override
            {
            return "FromToLocation.FromPosition.DistanceAlongFromStart, FromToLocation.ToPosition.DistanceAlongFromStart, FromToLocation.ECInstanceId ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenFromJoin() override 
            { 
            return "INNER JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " FromToLocation ON LinearlyLocated.InstanceId = FromToLocation.Element.Id ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenWhere(NullableDouble const& from, bool inclusiveFrom, NullableDouble const& to, bool inclusiveTo, bvector<double>& bindVals) override
            {
            Utf8String fromCompOp = inclusiveFrom ? ">=" : ">";
            Utf8String toCompOp = inclusiveTo ? "<=" : "<";

            Utf8String ecSql;
            if (from.IsValid() && to.IsValid())
                {
                ecSql.append("AND ((FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ?) OR (FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ?) OR (FromToLocation.FromPosition.DistanceAlongFromStart <= ? AND FromToLocation.ToPosition.DistanceAlongFromStart >= ?)) ");

                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                }
            else if (from.IsValid())
                {
                ecSql.append("AND (FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp); ecSql.append(" ?)");

                bindVals.push_back(from.Value()); bindVals.push_back(from.Value());
                }
            else if (to.IsValid())
                {
                ecSql.append("AND (FromToLocation.FromPosition.DistanceAlongFromStart "); ecSql.append(toCompOp);
                ecSql.append(" ? OR FromToLocation.ToPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); ecSql.append(" ?) ");

                bindVals.push_back(to.Value()); bindVals.push_back(to.Value());
                }
            else
                {
                ecSql.append("FromToLocation.FromPosition.DistanceAlongFromStart IS NOT NULL ");
                }

            return ecSql;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenOrderBy() override
            {
            return "FromToLocation.FromPosition.DistanceAlongFromStart";
            }
    }; // FromToECSQLGenImpl

    struct AtECSQLGenImpl : ECSQLGenImpl
    {
    public:
        AtECSQLGenImpl() {}

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenSelect() override
            {
            return "AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.AtPosition.DistanceAlongFromStart, AtLocation.ECInstanceId ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenFromJoin() override 
            { 
            return "INNER JOIN " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " AtLocation ON LinearlyLocated.InstanceId = AtLocation.Element.Id ";
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenWhere(NullableDouble const& from, bool inclusiveFrom, NullableDouble const& to, bool inclusiveTo, bvector<double>& bindVals) override
            {
            Utf8String fromCompOp = inclusiveFrom ? ">=" : ">";
            Utf8String toCompOp = inclusiveTo ? "<=" : "<";

            Utf8String ecSql;
            if (from.IsValid() && to.IsValid())
                {
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp);
                ecSql.append(" ? AND AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); ecSql.append(" ? ");

                bindVals.push_back(from.Value()); bindVals.push_back(to.Value());
                }
            else if (from.IsValid())
                {
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(fromCompOp); ecSql.append(" ? ");

                bindVals.push_back(from.Value());
                }
            else if (to.IsValid())
                {
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart "); ecSql.append(toCompOp); ecSql.append(" ? ");

                bindVals.push_back(to.Value());
                }
            else
                ecSql.append("AtLocation.AtPosition.DistanceAlongFromStart IS NOT NULL ");

            return ecSql;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Diego.Diaz                      05/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual Utf8String GenOrderBy() override
            {
            return "AtLocation.AtPosition.DistanceAlongFromStart";
            }
    }; // AtECSQLGenImpl

private:
    ILinearElement::QueryParams const& m_params;
    Utf8String m_ecSQL;
    ECSQLGenImpl* m_pImpl;

private:

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddSelectClause()
        {
        Utf8String select = "SELECT ";

        if (m_pImpl->SelectDistinct())
            select.append("DISTINCT ");

        select.append("LinearlyLocated.InstanceId, LinearlyLocated.ClassId, ");
        select.append(m_pImpl->GenSelect());

        m_ecSQL.append(select);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String GenLinearlyLocated()
        {
        Utf8String linLocated = "(SELECT coalesce(Located.TargetECInstanceId, Along.SourceECInstanceId) InstanceId, "
            "coalesce(Located.TargetECClassId, Along.SourceECClassId) ClassId "
            "FROM " BLR_SCHEMA(BLR_REL_ILinearlyLocatedAlongILinearElement) " Along LEFT JOIN "
            BLR_SCHEMA(BLR_REL_ILinearLocationLocatesElement) " Located ON Along.SourceECInstanceId = Located.SourceECInstanceId "
            "WHERE Along.TargetECInstanceId = ? ";
        
        if (1 == m_params.m_iLinearlyLocatedClassIds.size())
            {
            auto classId = (*m_params.m_iLinearlyLocatedClassIds.begin()).GetValue();
            linLocated.append(Utf8PrintfString("AND ((Located.TargetECInstanceId IS NULL AND Along.SourceECClassId = %ld) ", classId));
            linLocated.append(Utf8PrintfString("OR (Located.TargetECInstanceId IS NOT NULL AND Located.TargetECClassId = %ld)) ", classId));
            }
        else if (1 < m_params.m_iLinearlyLocatedClassIds.size())
            { 
            Utf8String classIdList = "(";
            for (auto& classId : m_params.m_iLinearlyLocatedClassIds)
                {
                classIdList.append(Utf8PrintfString("%ld, ", classId.GetValue()));
                }

            classIdList.erase(linLocated.size() - 2, 2); // Removing last comma
            classIdList.append(") ");

            linLocated.append(Utf8PrintfString("AND ((Located.TargetECInstanceId IS NULL AND Along.SourceECClassId IN %s) ", classIdList.c_str()));
            linLocated.append(Utf8PrintfString("OR (Located.TargetECInstanceId NOT IS NULL AND Located.TargetECClassId IN %s)) ", classIdList.c_str()));            
            }

        linLocated.append(") LinearlyLocated ");
        return linLocated;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddFromClause(bvector<double>& bindVals)
        {
        Utf8String from = "FROM ";
        from.append(GenLinearlyLocated());
        from.append(m_pImpl->GenFromJoin());
        
        m_ecSQL.append(from);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddWhereClause(bvector<double>& bindVals)
        {
        Utf8String where = "WHERE ";
        where.append(m_pImpl->GenWhere(
            m_params.m_fromDistanceAlong,
            (m_params.m_fromComparisonOption == ILinearElement::QueryParams::ComparisonOption::Inclusive),
            m_params.m_toDistanceAlong,
            (m_params.m_toComparisonOption == ILinearElement::QueryParams::ComparisonOption::Inclusive), bindVals));

        m_ecSQL.append(where);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddOrderByClause()
        {
        Utf8String orderBy = "ORDER BY ";
        orderBy.append(m_pImpl->GenOrderBy());

        m_ecSQL.append(orderBy);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateImpl()
        {
        if (m_params.m_linearlyReferencedLocationTypeFilter == ILinearElement::QueryParams::LinearlyReferencedLocationType::Any)
            m_pImpl = new AtAndFromToECSQLGenImpl();
        else if (m_params.m_linearlyReferencedLocationTypeFilter == ILinearElement::QueryParams::LinearlyReferencedLocationType::FromTo)
            m_pImpl = new FromToECSQLGenImpl();
        else
            m_pImpl = new AtECSQLGenImpl();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void DestroyImpl()
        {
        delete m_pImpl;
        }

public:
    QueryLinearLocationsECSQLGen(ILinearElement::QueryParams const& params): m_params(params) {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Generate(Utf8StringR ecSQL, bvector<double>& bindVals)
        {
        bindVals.clear();
        m_ecSQL.clear();

        CreateImpl();
        
        AddSelectClause();
        AddFromClause(bindVals);
        AddWhereClause(bindVals);
        AddOrderByClause();

        DestroyImpl();

        ecSQL = m_ecSQL;
        }

}; // QueryLinearLocationsECSQLGen

LinearLocationReference::LinearLocationReference(ILinearlyLocatedCR linearlyLocated, double startDistanceAlong, double stopDistanceAlong) :
    m_linearlyLocatedId(linearlyLocated.ToElement().GetElementId()), m_linearlyLocatedClassId(linearlyLocated.ToElement().GetElementClassId()),
    m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong), m_locationId(LinearlyReferencedLocationId())
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LinearLocationReference::operator==(LinearLocationReferenceCR rhs) const
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
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearElement::SetSource(ILinearElementSourceCP source) 
    { 
    if (source)
        ToElementR().SetPropertyValue(BLR_PROP_ILinearElement_Source, source->ToElement().GetElementId(),
            ToElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearElementProvidedBySource));
    else
        ToElementR().SetPropertyValue(BLR_PROP_ILinearElement_Source, DgnElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LinearLocationReference> ILinearElement::_QueryLinearLocations(QueryParams const& params) const
    {
    auto& dgnDb = ToElement().GetDgnDb();
    
    Utf8String ecSql;
    bvector<double> bindVals;

    QueryLinearLocationsECSQLGen ecSqlGen(params);
    ecSqlGen.Generate(ecSql, bindVals);

    auto& dgnDbR = ToElement().GetDgnDb();
    auto stmtPtr = dgnDbR.GetPreparedECSqlStatement(ecSql.c_str());
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, ToElement().GetElementId());

    int idx = 2;
    for (auto val : bindVals)
        stmtPtr->BindDouble(idx++, val);

    bvector<LinearLocationReference> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        retVal.push_back(LinearLocationReference(
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
    stmt.Prepare(ToElement().GetDgnDb(), "SELECT SourceECInstanceId FROM " 
        BLR_SCHEMA(BLR_REL_ILinearElementProvidedBySource) " WHERE TargetECInstanceId = ?");
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
DgnDbStatus ILinearlyLocated::_InsertLinearElementRelationship()
    {
    if (!m_cachedLinearElementId.IsValid())
        return DgnDbStatus::InvalidId;

    auto& elCR = ToElement();
    auto relClassCP = elCR.GetDgnDb().Schemas().GetClass(BLR_SCHEMA_NAME, BLR_REL_ILinearlyLocatedAlongILinearElement)->GetRelationshipClassCP();

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != elCR.GetDgnDb().InsertLinkTableRelationship(insKey,
        *relClassCP,
            ECInstanceId(elCR.GetElementId().GetValue()), ECInstanceId(m_cachedLinearElementId.GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearlyLocated::_UpdateLinearElementRelationship()
    {
    if (!m_cachedLinearElementId.IsValid())
        return DgnDbStatus::Success;

    auto& elCR = ToElement();
    auto stmtDelPtr = elCR.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECClassId, ECInstanceId FROM " BLR_SCHEMA(BLR_REL_ILinearlyLocatedAlongILinearElement) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtDelPtr.IsValid());

    stmtDelPtr->BindId(1, elCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtDelPtr->Step())
        {
        if (DbResult::BE_SQLITE_OK != elCR.GetDgnDb().DeleteLinkTableRelationship(
                ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }

    return _InsertLinearElementRelationship();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ILinearlyLocated::GetLinearElementId() const
    {
    if (m_cachedLinearElementId.IsValid())
        return m_cachedLinearElementId;

    auto stmtPtr = ToElement().GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BLR_SCHEMA(BLR_REL_ILinearlyLocatedAlongILinearElement) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, ToElement().GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        m_cachedLinearElementId = stmtPtr->GetValueId<DgnElementId>(0);

    return m_cachedLinearElementId;
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
    return dynamic_cast<LinearlyReferencedLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedLocationP ILinearlyLocated::GetLinearlyReferencedLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedLocation>(ToElementR(), *LinearlyReferencedLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationCP ILinearlyLocatedMultipleAt::GetAtLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedAtLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationP ILinearlyLocatedMultipleAt::GetAtLocationP(LinearlyReferencedLocationId id)
    {
    return DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(ToElementR(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP ILinearlyLocatedMultipleFromTo::GetFromToLocation(LinearlyReferencedLocationId id) const
    {
    return dynamic_cast<LinearlyReferencedFromToLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP ILinearlyLocatedMultipleFromTo::GetFromToLocationP(LinearlyReferencedLocationId id)
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
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ILinearlyLocatedAttribution::SetAttributedElement(ILinearElementSourceCP attributedElement)
    {
    if (attributedElement)
        const_cast<DgnElementP>(&_ILinearlyLocatedToDgnElement())->SetPropertyValue(BLR_PROP_ILinearlyLocatedAttribution_AttributedElement,
            attributedElement->ToElement().GetElementId(),
            _ILinearlyLocatedToDgnElement().GetDgnDb().Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearlyLocatedAttributesElement));
    else
        const_cast<DgnElementP>(&_ILinearlyLocatedToDgnElement())->SetPropertyValue(BLR_PROP_ILinearlyLocatedAttribution_AttributedElement,
            DgnElementId(), DgnClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ILinearLocationElement::ILinearLocationElement()
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
LinearlyReferencedAtLocationCP ILinearlyLocatedSingleAt::GetAtLocation() const
    {
    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    return dynamic_cast<LinearlyReferencedAtLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), m_atLocationAspectId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedAtLocationP ILinearlyLocatedSingleAt::GetAtLocationP()
    {
    if (!m_atLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_atLocationAspectId = aspectIds.front();
        }

    return DgnElement::MultiAspect::GetP<LinearlyReferencedAtLocation>(ToElementR(), 
        *LinearlyReferencedAtLocation::QueryClass(ToElement().GetDgnDb()), m_atLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double ILinearlyLocatedSingleAt::GetAtDistanceAlongFromStart() const
    {
    if (!ToElement().GetElementId().IsValid())
        return m_unpersistedAtLocationPtr->GetAtPosition().GetDistanceAlongFromStart();

    auto locationCP = GetAtLocation();
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

    auto locationP = GetAtLocationP();
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

    auto locationCP = GetFromToLocation();
    BeAssert(locationCP);

    return locationCP->GetFromPosition().GetDistanceAlongFromStart();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationP ILinearlyLocatedSingleFromTo::GetFromToLocationP()
    {
    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    return DgnElement::MultiAspect::GetP<LinearlyReferencedFromToLocation>(ToElementR(),
        *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), m_fromToLocationAspectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
LinearlyReferencedFromToLocationCP ILinearlyLocatedSingleFromTo::GetFromToLocation() const
    {
    if (!m_fromToLocationAspectId.IsValid())
        {
        auto aspectIds = QueryLinearlyReferencedLocationIds();
        BeAssert(1 == aspectIds.size());

        m_fromToLocationAspectId = aspectIds.front();
        }

    return dynamic_cast<LinearlyReferencedFromToLocationCP>(
        DgnElement::MultiAspect::GetAspect(ToElement(), *LinearlyReferencedFromToLocation::QueryClass(ToElement().GetDgnDb()), m_fromToLocationAspectId));
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

    auto locationP = GetFromToLocationP();
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

    auto locationCP = GetFromToLocation();
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

    auto locationP = GetFromToLocationP();
    BeAssert(locationP);

    return locationP->GetToPositionR().SetDistanceAlongFromStart(newFrom);
    }

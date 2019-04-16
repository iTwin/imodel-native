/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************GetColumnsPropertyMapVisitor********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetColumnsPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if ((m_table == nullptr || m_table == &propertyMap.GetTable()) &&
        Enum::Contains(m_filter, propertyMap.GetType()))
        {
        DbColumn const& col = propertyMap.GetColumn();
        m_columns.push_back(&col);

        if (col.GetPersistenceType() == PersistenceType::Virtual)
            m_virtualColumnCount++;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetColumnsPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    if (!Enum::Contains(m_filter, propertyMap.GetType()))
        return SUCCESS;

    for (DataPropertyMap const* childPropMap : propertyMap)
        {
        if (SUCCESS != childPropMap->AcceptVisitor(*this))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetColumnsPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    if (!Enum::Contains(m_filter, propertyMap.GetType()))
        return SUCCESS;

    if (m_table != nullptr)
        {
        BeAssert(m_doNotSkipSystemPropertyMaps == false);
        SystemPropertyMap::PerTableIdPropertyMap const* dataPropMap = propertyMap.FindDataPropertyMap(*m_table);
        if (dataPropMap != nullptr)
            {
            DbColumn const& col = dataPropMap->GetColumn();
            m_columns.push_back(&col);
            if (col.GetPersistenceType() == PersistenceType::Virtual)
                m_virtualColumnCount++;
            }

        return SUCCESS;
        }

    if (m_doNotSkipSystemPropertyMaps)
        {
        for (SystemPropertyMap::PerTableIdPropertyMap const* m : propertyMap.GetDataPropertyMaps())
            {
            DbColumn const& col = m->GetColumn();
            m_columns.push_back(&col);
            if (col.GetPersistenceType() == PersistenceType::Virtual)
                m_virtualColumnCount++;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbColumn const* GetColumnsPropertyMapVisitor::GetSingleColumn() const
    {
    BeAssert(GetColumns().size() == 1);
    if (GetColumns().size() != 1)
        return nullptr;

    return GetColumns().front();
    }

//************************************GetTablesPropertyMapVisitor********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetTablesPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_tables.insert(&propertyMap.GetTable());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetTablesPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        {
        if (propertyMap.GetType() == PropertyMap::Type::Navigation)
            {
            NavigationPropertyMap const& navPropertyMap = propertyMap.GetAs<NavigationPropertyMap>();
            if (!navPropertyMap.IsComplete())
                return SUCCESS;
            }

        m_tables.insert(&propertyMap.GetTable());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus GetTablesPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_tables.insert(propertyMap.GetTables().begin(), propertyMap.GetTables().end());

    return SUCCESS;
    }

//************************************SearchPropertyMapVisitor********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SearchPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (m_propertyMapFilterCallback(propertyMap))
        m_foundPropertyMaps.push_back(&propertyMap);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SearchPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    if (m_propertyMapFilterCallback(propertyMap))
        m_foundPropertyMaps.push_back(&propertyMap);

    if (!m_recurseIntoCompoundPropertyMap(propertyMap))
        return SUCCESS;

    for (DataPropertyMap const* childPropMap : propertyMap)
        {
        if (SUCCESS != childPropMap->AcceptVisitor(*this))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SearchPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    if (m_propertyMapFilterCallback(propertyMap))
        m_foundPropertyMaps.push_back(&propertyMap);

    return SUCCESS;
    }

//************************************ToSqlPropertyMapVisitor********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ToSqlPropertyMapVisitor::ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope scope, Utf8StringCR classIdentifier, bool wrapInParentheses /*= false*/) 
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_scope(scope), m_classIdentifier(classIdentifier), m_wrapInParentheses(wrapInParentheses)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ToSqlPropertyMapVisitor::ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope scope, bool wrapInParentheses /*= false*/)
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_scope(scope), m_wrapInParentheses(wrapInParentheses)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::ConstraintECInstanceId:
                return ToNativeSql(propertyMap.GetAs<ConstraintECInstanceIdPropertyMap>());
            case PropertyMap::Type::ConstraintECClassId:
                return ToNativeSql(propertyMap.GetAs<ConstraintECClassIdPropertyMap>());
            case PropertyMap::Type::ECClassId:
                return ToNativeSql(propertyMap.GetAs<ECClassIdPropertyMap>());
            case PropertyMap::Type::ECInstanceId:
                return ToNativeSql(propertyMap.GetAs<ECInstanceIdPropertyMap>());
            default:
                BeAssert(false);
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(SingleColumnDataPropertyMap const& propertyMap) const
    {
    const bool requiresCast = RequiresCast(propertyMap);

    if (propertyMap.GetType() == PropertyMap::Type::NavigationRelECClassId)
        return ToNativeSql(propertyMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>(), requiresCast);

    Result& result = Record(propertyMap);

    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    if (m_wrapInParentheses)
        sqlBuilder.AppendParenLeft();

    if (requiresCast)
        sqlBuilder.Append("CAST(");

    sqlBuilder.AppendFullyQualified(m_classIdentifier, propertyMap.GetColumn().GetName());

    if (requiresCast)
        sqlBuilder.Append(" AS ").Append(DbColumn::TypeToSql(propertyMap.GetColumnDataType())).Append(")");

    if (m_wrapInParentheses)
        sqlBuilder.AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap, bool requiresCast) const
    {
    Result& result = Record(relClassIdPropMap);

    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    switch (m_scope)
        {
            case ECSqlScope::Select:
                BeAssert(!requiresCast && "Assignment exp must not have cast");
                //Here we refer to the class view representing the table, which is already fully prepared
                //to deal whether the respective id col is null or not. Therefore no need to inject the CASE expr
                sqlBuilder.AppendFullyQualified(m_classIdentifier, relClassIdPropMap.GetColumn().GetName());
                return SUCCESS;

            case ECSqlScope::NonSelectAssignmentExp:
                BeAssert(!requiresCast && "Assignment exp must not have cast");

                if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual) //ignore completely, no-op binders will be
                    return SUCCESS;

                sqlBuilder.AppendFullyQualified(m_classIdentifier, relClassIdPropMap.GetColumn().GetName());
                return SUCCESS;

            case ECSqlScope::NonSelectNoAssignmentExp:
            {
            BeAssert(relClassIdPropMap.GetParent() != nullptr && relClassIdPropMap.GetParent()->GetType() == PropertyMap::Type::Navigation);
            NavigationPropertyMap::IdPropertyMap const& idPropMap = relClassIdPropMap.GetParent()->GetAs<NavigationPropertyMap>().GetIdPropertyMap();

            NativeSqlBuilder idColStrBuilder;
            idColStrBuilder.AppendFullyQualified(m_classIdentifier, idPropMap.GetColumn().GetName());

            NativeSqlBuilder relClassIdColStrBuilder;
            if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual)
                relClassIdColStrBuilder.Append(relClassIdPropMap.GetDefaultClassId());
            else
                relClassIdColStrBuilder.AppendFullyQualified(m_classIdentifier, relClassIdPropMap.GetColumn().GetName());

            //wrap cast around case expression rather than the rel class id sql. Cast expressions have the affinity of the target type
            //whereas case expressions don't have an affinity and therefore behave like BLOB columns and would therefore negate the whole
            //idea of injecting the casts again which is what we want to avoid
            //No affinity behaves differently in terms of type conversions prior to comparisons
            //(see https://sqlite.org/datatype3.html#type_conversions_prior_to_comparison)
            if (requiresCast)
                sqlBuilder.Append("CAST(");

            //The RelECClassId should always be logically null if the respective NavId col is null
            sqlBuilder.AppendFormatted("CASE WHEN %s IS NULL THEN NULL ELSE %s END", idColStrBuilder.GetSql().c_str(), relClassIdColStrBuilder.GetSql().c_str());

            if (requiresCast)
                sqlBuilder.Append(" AS ").Append(DbColumn::TypeToSql(relClassIdPropMap.GetColumnDataType())).AppendParenRight();

            return SUCCESS;
            }

            default:
                BeAssert(false);
                return ERROR;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8StringCR columnExp = m_scope == ECSqlScope::Select ? propertyMap.GetAccessString() : vmap->GetColumn().GetName();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    const bool requiresCast = RequiresCast(*vmap);
    if (requiresCast)
        result.GetSqlBuilderR().Append("CAST(");

    result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, columnExp);

    if (requiresCast)
        result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(vmap->GetColumnDataType())).AppendParenRight();

    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (perTablePropMap == nullptr || perTablePropMap->GetType() != PropertyMap::Type::SystemPerTableClassId)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*perTablePropMap);
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();

    if (m_scope == ECSqlScope::Select)
        result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier.c_str(), COL_ECClassId);
    else
        {
        DbColumn const& col = perTablePropMap->GetColumn();
        if (col.GetPersistenceType() == PersistenceType::Virtual)
            result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        else
            {
            const bool requiresCast = RequiresCast(*perTablePropMap);
            if (requiresCast)
                result.GetSqlBuilderR().Append("CAST(");

            result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, col.GetName());

            if (requiresCast)
                result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(perTablePropMap->GetColumnDataType())).AppendParenRight();
            }
        }
        
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* perTablePropMap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (perTablePropMap == nullptr || perTablePropMap->GetType() != PropertyMap::Type::SystemPerTableClassId)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*perTablePropMap);

    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();


    if (m_scope == ECSqlScope::Select)
        result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, propertyMap.GetAccessString());
    else
        {
        DbColumn const& col = perTablePropMap->GetColumn();

        if (col.GetPersistenceType() == PersistenceType::Virtual)
            result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        else
            {
            const bool requiresCast = RequiresCast(*perTablePropMap);
            if (requiresCast)
                result.GetSqlBuilderR().Append("CAST(");

            result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, col.GetName());

            if (requiresCast)
                result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(perTablePropMap->GetColumnDataType())).AppendParenRight();
            }
        }

    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTableIdPropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8StringCR columnExp = m_scope == ECSqlScope::Select ? propertyMap.GetAccessString() : vmap->GetColumn().GetName();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    const bool requiresCast = RequiresCast(*vmap);
    if (requiresCast)
        result.GetSqlBuilderR().Append("CAST(");

    result.GetSqlBuilderR().AppendFullyQualified(m_classIdentifier, columnExp);

    if (requiresCast)
        result.GetSqlBuilderR().Append(" AS ").Append(DbColumn::TypeToSql(vmap->GetColumnDataType())).AppendParenRight();

    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ToSqlPropertyMapVisitor::Result& ToSqlPropertyMapVisitor::Record(SingleColumnDataPropertyMap const& propertyMap) const
    {
    m_resultSet.push_back(Result(propertyMap));
    return m_resultSet.back();
    }


//************************************SavePropertyMapVisitor*************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SavePropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (m_tableFilter && m_tableFilter->GetId() != propertyMap.GetColumn().GetTable().GetId())
        return SUCCESS;

    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRootPropertyId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), propertyMap.GetColumn().GetId()) != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SavePropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    const ECN::ECPropertyId propertyId = propertyMap.GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    for (SystemPropertyMap::PerTableIdPropertyMap const* childMap : propertyMap.GetDataPropertyMaps())
        {
        if (m_tableFilter && m_tableFilter->GetId() != childMap->GetColumn().GetTable().GetId())
            continue;

        if (m_context.InsertPropertyMap(propertyId, accessString.c_str(), childMap->GetColumn().GetId()) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

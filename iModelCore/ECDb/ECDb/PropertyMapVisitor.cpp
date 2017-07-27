/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMapVisitor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        m_tables.insert(&propertyMap.GetTable());

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
ToSqlPropertyMapVisitor::ToSqlPropertyMapVisitor(DbTable const& tableFilter, ECSqlScope scope, Utf8CP classIdentifier, bool wrapInParentheses /*= false*/) 
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_scope(scope), m_classIdentifier(classIdentifier), m_wrapInParentheses(wrapInParentheses)
    {
    if (m_classIdentifier != nullptr && Utf8String::IsNullOrEmpty(m_classIdentifier))
        m_classIdentifier = nullptr;
    }

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
    if (propertyMap.GetType() == PropertyMap::Type::NavigationRelECClassId)
        return ToNativeSql(propertyMap.GetAs<NavigationPropertyMap::RelECClassIdPropertyMap>());

    Result& result = Record(propertyMap);

    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    if (m_wrapInParentheses)
        sqlBuilder.AppendParenLeft();

    sqlBuilder.Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());

    if (m_wrapInParentheses)
        sqlBuilder.AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap) const
    {
    Result& result = Record(relClassIdPropMap);

    NativeSqlBuilder& sqlBuilder = result.GetSqlBuilderR();

    switch (m_scope)
        {
            case ECSqlScope::Select:
                //Here we refer to the class view representing the table, which is already fully prepared
                //to deal whether the respective id col is null or not. Therefore no need to inject the CASE expr
                sqlBuilder.Append(m_classIdentifier, relClassIdPropMap.GetColumn().GetName().c_str());
                return SUCCESS;

            case ECSqlScope::NonSelectAssignmentExp:
                if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual) //ignore completely, no-op binders will be
                    return SUCCESS;

                sqlBuilder.Append(m_classIdentifier, relClassIdPropMap.GetColumn().GetName().c_str());
                return SUCCESS;

            case ECSqlScope::NonSelectNoAssignmentExp:
            {
            BeAssert(relClassIdPropMap.GetParent() != nullptr && relClassIdPropMap.GetParent()->GetType() == PropertyMap::Type::Navigation);
            NavigationPropertyMap::IdPropertyMap const& idPropMap = relClassIdPropMap.GetParent()->GetAs<NavigationPropertyMap>().GetIdPropertyMap();

            NativeSqlBuilder idColStrBuilder;
            idColStrBuilder.Append(m_classIdentifier, idPropMap.GetColumn().GetName().c_str());

            NativeSqlBuilder relClassIdColStrBuilder;
            if (relClassIdPropMap.GetColumn().GetPersistenceType() == PersistenceType::Virtual)
                relClassIdColStrBuilder = NativeSqlBuilder(relClassIdPropMap.GetDefaultClassId().ToString().c_str());
            else
                relClassIdColStrBuilder.Append(m_classIdentifier, relClassIdPropMap.GetColumn().GetName().c_str());
            
            //The RelECClassId should always be logically null if the respective NavId col is null
            sqlBuilder.AppendFormatted("CASE WHEN %s IS NULL THEN NULL ELSE %s END", idColStrBuilder.ToString(), relClassIdColStrBuilder.ToString());
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
    Utf8CP columnExp = m_scope == ECSqlScope::Select ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);

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
        result.GetSqlBuilderR().Append(m_classIdentifier, COL_ECClassId);
    else
        {
        DbColumn const& col = perTablePropMap->GetColumn();

        if (col.GetPersistenceType() == PersistenceType::Virtual)
            result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        else
            result.GetSqlBuilderR().Append(m_classIdentifier, col.GetName().c_str());
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
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetAccessString().c_str());
    else
        {
        DbColumn const& col = perTablePropMap->GetColumn();

        if (col.GetPersistenceType() == PersistenceType::Virtual)
            result.GetSqlBuilderR().Append(perTablePropMap->GetAs<SystemPropertyMap::PerTableClassIdPropertyMap>().GetDefaultECClassId());
        else
            result.GetSqlBuilderR().Append(m_classIdentifier, col.GetName().c_str());
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
    Utf8CP columnExp = m_scope == ECSqlScope::Select ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ToSqlPropertyMapVisitor::Result& ToSqlPropertyMapVisitor::Record(SingleColumnDataPropertyMap const& propertyMap) const
    {
    m_resultSetByAccessString[propertyMap.GetAccessString().c_str()] = m_resultSet.size();
    m_resultSet.push_back(Result(propertyMap));
    return m_resultSet.back();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
const ToSqlPropertyMapVisitor::Result* ToSqlPropertyMapVisitor::Find(Utf8CP accessString) const
    {
    auto itor = m_resultSetByAccessString.find(accessString);
    if (itor == m_resultSetByAccessString.end())
        return nullptr;

    return &m_resultSet.at(itor->second);
    }

//************************************SavePropertyMapVisitor*************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SavePropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
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
        if (m_context.InsertPropertyMap(propertyId, accessString.c_str(), childMap->GetColumn().GetId()) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

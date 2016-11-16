/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMapVisitor.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_columns.push_back(&propertyMap.GetColumn());

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

    if (m_doNotSkipSystemPropertyMaps)
        {
        for (SystemPropertyMap::PerTablePrimitivePropertyMap const* m : propertyMap.GetDataPropertyMaps())
            {
            m_columns.push_back(&m->GetColumn());
            }

        return SUCCESS;
        }

    if (m_table == nullptr)
        return SUCCESS;

    SystemPropertyMap::PerTablePrimitivePropertyMap const* dataPropMap = propertyMap.FindDataPropertyMap(*m_table);
    if (dataPropMap != nullptr)
        m_columns.push_back(&dataPropMap->GetColumn());
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool GetColumnsPropertyMapVisitor::AllColumnsAreVirtual() const
    {
    BeAssert(!GetColumns().empty());
    bool isVirtual = true;
    for (DbColumn const* column : GetColumns())
        {
        isVirtual &= column->GetPersistenceType() == PersistenceType::Virtual;
        if (!isVirtual)
            break;
        }

    return isVirtual;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbColumn const* GetColumnsPropertyMapVisitor::GetSingleColumn() const
    {
    BeAssert(GetColumns().size() == 1);
    if (GetColumns().size() != 1)
        {
        return nullptr;
        }

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbTable const* GetTablesPropertyMapVisitor::GetSingleTable() const
    {
    BeAssert(!m_tables.empty());
    if (m_tables.size() != 1)
        return nullptr;

    return *(m_tables.begin());
    }


//************************************SearchPropertyMapVisitor********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SearchPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_foundPropertyMaps.push_back(&propertyMap);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus SearchPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    if (!Enum::Contains(m_filter, propertyMap.GetType()))
        return SUCCESS;

    if (!m_doNotAddCompoundPropertiesToResult)
        m_foundPropertyMaps.push_back(&propertyMap);

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
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_foundPropertyMaps.push_back(&propertyMap);

    return SUCCESS;
    }

//************************************ToSqlPropertyMapVisitor********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ToSqlPropertyMapVisitor::ToSqlPropertyMapVisitor(DbTable const& tableFilter, SqlTarget target, Utf8CP classIdentifier, bool wrapInParentheses /*= false*/, bool usePropertyNameAsAliasForSystemPropertyMaps /*= false*/) 
    : IPropertyMapVisitor(), m_tableFilter(tableFilter), m_target(target), m_classIdentifier(classIdentifier), m_wrapInParentheses(wrapInParentheses), m_usePropertyNameAsAliasForSystemPropertyMaps(usePropertyNameAsAliasForSystemPropertyMaps)
    {
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        BeAssert(target == SqlTarget::Table);
        BeAssert(!wrapInParentheses);
        }

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
                return ToNativeSql(static_cast<ConstraintECInstanceIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ConstraintECClassId:
                return ToNativeSql(static_cast<ConstraintECClassIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ECClassId:
                return ToNativeSql(static_cast<ECClassIdPropertyMap const&>(propertyMap));
            case PropertyMap::Type::ECInstanceId:
                return ToNativeSql(static_cast<ECInstanceIdPropertyMap const&>(propertyMap));

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
        return ToNativeSql(static_cast<NavigationPropertyMap::RelECClassIdPropertyMap const&>(propertyMap));

    Result& result = Record(propertyMap);
    if (m_wrapInParentheses)
        result.GetSqlBuilderR().AppendParenLeft();

    DbColumn const* overFlowColumn = propertyMap.GetColumn().GetMasterOverflowColumn();
    if (overFlowColumn != nullptr)
        {
        //"json_extract(<overFlowColumnMaster>, '$.<overFlowColumnSlave>')"
        if (m_target == SqlTarget::Table)
            {
            result.GetSqlBuilderR().Append("json_extract(")
                .Append(m_classIdentifier, overFlowColumn->GetName().c_str())
                .AppendComma().Append("'$.")
                .Append(propertyMap.GetColumn().GetName().c_str()).Append("')");
            }
        else
            {
            result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
            }
        }
    else
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
        }
    if (m_wrapInParentheses)
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(NavigationPropertyMap::RelECClassIdPropertyMap const& propertyMap) const
    {
    Result& result = Record(propertyMap);
    if (!propertyMap.IsVirtual())
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
        return SUCCESS;
        }

    if (m_target == SqlTarget::SelectView)
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
    else
        result.GetSqlBuilderR().Append(propertyMap.GetDefaultClassId()).AppendSpace().Append(propertyMap.GetColumn().GetName().c_str());

    return SUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8CP columnExp = m_target == SqlTarget::SelectView ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()))
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }
    
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenRight();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus ToSqlPropertyMapVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
    {
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool isVirtual = propertyMap.IsVirtual(m_tableFilter);

    Result& result = Record(*vmap);
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    if (m_target == SqlTarget::SelectView)
        result.GetSqlBuilderR().Append(m_classIdentifier, COL_ECClassId);
    else
        {
        if (isVirtual)
            result.GetSqlBuilderR().Append(propertyMap.GetDefaultECClassId());
        else
            result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());
        }
        

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || isVirtual)
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
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
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    const bool isVirtual = propertyMap.IsVirtual(m_tableFilter);
    Result& result = Record(*vmap);
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    if (m_target == SqlTarget::SelectView)
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetAccessString().c_str());
    else
        {
        if (isVirtual)
            result.GetSqlBuilderR().Append(propertyMap.GetDefaultECClassId());
        else
            result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());
        }

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || isVirtual)
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
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
    SystemPropertyMap::PerTablePrimitivePropertyMap const* vmap = propertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Result& result = Record(*vmap);
    Utf8CP columnExp = m_target == SqlTarget::SelectView ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) 
        result.GetSqlBuilderR().AppendParenLeft();
    
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()))
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
        }

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
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
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
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    for (SystemPropertyMap::PerTablePrimitivePropertyMap const* childMap : propertyMap.GetDataPropertyMaps())
        {
        if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), childMap->GetColumn().GetId()) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

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
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback GetColumnsPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        m_columns.push_back(&propertyMap.GetColumn());

    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback GetColumnsPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, propertyMap.GetType()))
        return VisitorFeedback::Next;

    return VisitorFeedback::NextSibling;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback GetColumnsPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    if (!Enum::Contains(m_filter, propertyMap.GetType()))
        return VisitorFeedback::Next;

    if (m_doNotSkipSystemPropertyMaps)
        {
        for (SystemPropertyMap::PerTablePrimitivePropertyMap const* m : propertyMap.GetDataPropertyMaps())
            {
            m_columns.push_back(&m->GetColumn());
            }
        }
    else
        {
        if (m_table)
            {
            if (SystemPropertyMap::PerTablePrimitivePropertyMap const* m = propertyMap.FindDataPropertyMap(*m_table))
                {
                m_columns.push_back(&m->GetColumn());
                }
            }
        }
    return VisitorFeedback::Next;
    }


//************************************ToSqlPropertyMapVisitor********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
bool ToSqlPropertyMapVisitor::IsAlienTable(DbTable const& table) const
    {
    if (&table != &m_tableFilter)
        {
        BeAssert(false && "PropertyMap table does not match the table filter specified.");
        m_status = ERROR;
        return true;
        }

    return false;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
SingleColumnDataPropertyMap const* ToSqlPropertyMapVisitor::FindSystemPropertyMapForTable(SystemPropertyMap const& systemPropertyMap) const
    {
    SingleColumnDataPropertyMap const* vmap = systemPropertyMap.FindDataPropertyMap(m_tableFilter);
    if (vmap == nullptr)
        {
        BeAssert(false && "Failed to find propertymap for filter table");
        m_status = ERROR;
        }

    return vmap;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
ToSqlPropertyMapVisitor::Result& ToSqlPropertyMapVisitor::Record(SingleColumnDataPropertyMap const& propertyMap) const
    {
    m_resultSetByAccessString[propertyMap.GetAccessString().c_str()] = m_resultSet.size();
    m_resultSet.push_back(Result(propertyMap));
    return m_resultSet.back();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::ToNativeSql(SingleColumnDataPropertyMap const& propertyMap) const
    {
    Result& result = Record(propertyMap);
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetColumn().GetName().c_str());
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenRight();

    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::ToNativeSql(ConstraintECInstanceIdPropertyMap const& propertyMap) const
    {
    SingleColumnDataPropertyMap const* vmap = FindSystemPropertyMapForTable(propertyMap);
    if (vmap == nullptr)
        {
        return VisitorFeedback::Cancel;
        }

    Result& result = Record(*vmap);
    auto columnExp = m_target == SqlTarget::View ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || vmap->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            {
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
            }
        }
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenRight();
    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::ToNativeSql(ECClassIdPropertyMap const& propertyMap) const
    {
    SingleColumnDataPropertyMap const* vmap = FindSystemPropertyMapForTable(propertyMap);
    if (vmap == nullptr)
        {
        return VisitorFeedback::Cancel;
        }

    Result& result = Record(*vmap);
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    if (vmap->GetColumn().GetPersistenceType() == PersistenceType::Persisted)
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());
        }
    else
        {
        if (m_target == SqlTarget::View)
            {
            result.GetSqlBuilderR().Append(m_classIdentifier, COL_ECClassId);
            }
        else
            {
            Utf8Char classIdStr[ECN::ECClassId::ID_STRINGBUFFER_LENGTH];
            propertyMap.GetDefaultECClassId().ToString(classIdStr);
            result.GetSqlBuilderR().Append(classIdStr);
            }
        }

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || vmap->GetColumn().GetPersistenceType() == PersistenceType::Virtual )
            {
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
            }
        }
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenRight();
    return VisitorFeedback::Next;
    }


//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::ToNativeSql(ConstraintECClassIdPropertyMap const& propertyMap) const
    {
    SingleColumnDataPropertyMap const* vmap = nullptr;
    if (!propertyMap.IsPersistedInDb())
        {
        //If not persisted we just need the ECClassId
        vmap = propertyMap.FindDataPropertyMap(*propertyMap.GetTables().front());
        }
    else
        vmap = FindSystemPropertyMapForTable(propertyMap);

    if (vmap == nullptr)
        {
        return VisitorFeedback::Cancel;
        }

    Result& result = Record(*vmap);
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    if (m_target == SqlTarget::View)
        {
        result.GetSqlBuilderR().Append(m_classIdentifier, propertyMap.GetAccessString().c_str());
        }
    else
        {

        if (vmap->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            {
            Utf8Char classIdStr[ECN::ECClassId::ID_STRINGBUFFER_LENGTH];
            propertyMap.GetDefaultECClassId().ToString(classIdStr);
            result.GetSqlBuilderR().Append(classIdStr);
            }
        else
            result.GetSqlBuilderR().Append(m_classIdentifier, vmap->GetColumn().GetName().c_str());
        }

    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || vmap->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            {
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
            }
        }
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenRight();
    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::ToNativeSql(ECInstanceIdPropertyMap const& propertyMap) const
    {
    SingleColumnDataPropertyMap const* vmap = FindSystemPropertyMapForTable(propertyMap);
    if (vmap == nullptr)
        {
        return VisitorFeedback::Cancel;
        }

    Result& result = Record(*vmap);
    auto columnExp = m_target == SqlTarget::View ? propertyMap.GetAccessString().c_str() : vmap->GetColumn().GetName().c_str();
    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenLeft();
    result.GetSqlBuilderR().Append(m_classIdentifier, columnExp);
    if (m_usePropertyNameAsAliasForSystemPropertyMaps)
        {
        if (!vmap->GetColumn().GetName().EqualsIAscii(propertyMap.GetAccessString()) || vmap->GetColumn().GetPersistenceType() == PersistenceType::Virtual)
            {
            result.GetSqlBuilderR().AppendSpace().Append(propertyMap.GetAccessString().c_str());
            }
        }

    if (m_wrapInParentheses) result.GetSqlBuilderR().AppendParenRight();
    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    return ToNativeSql(propertyMap);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback ToSqlPropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
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
        }

    return VisitorFeedback::Cancel;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
const ToSqlPropertyMapVisitor::Result* ToSqlPropertyMapVisitor::Find(Utf8CP accessString) const
    {
    auto itor = m_resultSetByAccessString.find(accessString);
    if (itor == m_resultSetByAccessString.end())
        return nullptr;

    return &m_resultSet.at(itor->second);
    }

//************************************SavePropertyMapVisitor*************************************

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback SavePropertyMapVisitor::_Visit(SingleColumnDataPropertyMap const& propertyMap) const
    {
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), propertyMap.GetColumn().GetId()) != SUCCESS)
        {
        BeAssert(false);
        m_status = ERROR;
        m_failedMap = &propertyMap;
        return VisitorFeedback::Cancel;
        }

    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback SavePropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    return VisitorFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
VisitorFeedback SavePropertyMapVisitor::_Visit(SystemPropertyMap const& propertyMap) const
    {
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    for (SystemPropertyMap::PerTablePrimitivePropertyMap const* childMap : propertyMap.GetDataPropertyMaps())
        {
        if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), childMap->GetColumn().GetId()) != SUCCESS)
            {
            BeAssert(false);
            m_status = ERROR;
            m_failedMap = &propertyMap;
            return VisitorFeedback::Cancel;
            }
        }

    return VisitorFeedback::Next;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

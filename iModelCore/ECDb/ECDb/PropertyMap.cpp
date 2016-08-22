﻿/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC
   
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//******************************** PropertyMapFactory *****************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
PropertyMapPtr PropertyMapFactory::CreatePropertyMap(ClassMapLoadContext& ctx, ECDbCR ecdb, ECClassCR ecClass, ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    if (!ecProperty.HasId())
        ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema(ecdb, ecProperty);

    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty != nullptr)
        {
        switch (primitiveProperty->GetType())
            {
                case PRIMITIVETYPE_Point2D:
                case PRIMITIVETYPE_Point3D:
                    return PointPropertyMap::Create(*primitiveProperty, propertyAccessString, parentPropertyMap);

                default:
                    return PrimitivePropertyMap::Create(*primitiveProperty, propertyAccessString, parentPropertyMap);
            }
        }

    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();

    // PropertyMapRule: primitives, primitive arrays , and structs map to 1 or more columns in the ECClass's main table
    if (arrayProperty != nullptr)
        {
        if (ARRAYKIND_Primitive == arrayProperty->GetKind())
            return PrimitiveArrayPropertyMap::Create(ecdb, *arrayProperty, propertyAccessString, parentPropertyMap);
        else
            {
            BeAssert(ARRAYKIND_Primitive != arrayProperty->GetKind());
            return StructArrayJsonPropertyMap::Create(ecdb, *arrayProperty->GetAsStructArrayProperty(), propertyAccessString, parentPropertyMap);
            }
        }

    if (ecProperty.GetIsStruct())
        return StructPropertyMap::Create(ctx, ecdb, *ecProperty.GetAsStructProperty(), propertyAccessString, parentPropertyMap); // The individual properties get their own binding, but we need a placeholder for the overall struct

    BeAssert(ecProperty.GetIsNavigation());

    BeAssert(parentPropertyMap == nullptr && "NavigationProperties can only be created on entity classes, so parent prop map is expected to be nullptr");
    return NavigationPropertyMap::Create(ctx, ecdb, ecClass, *ecProperty.GetAsNavigationProperty(), propertyAccessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
//static
PropertyMapPtr PropertyMapFactory::ClonePropertyMap(ECDbMapCR ecdbMap, PropertyMapCR proto, ECN::ECClassCR targetClass, PropertyMap const* parentPropertyMap)
    {
    if (ecdbMap.GetSchemaImportContext() == nullptr)
        {
        BeAssert(false && "PropertyMap::Clone must only be called during schema import");
        return nullptr;
        }

    if (auto protoMap = dynamic_cast<PointPropertyMap const*>(&proto))
        return PointPropertyMap::Clone(*protoMap, parentPropertyMap);

    if (auto protoMap = dynamic_cast<PrimitivePropertyMap const*>(&proto))
        return PrimitivePropertyMap::Clone(*protoMap, parentPropertyMap);

    if (auto protoMap = dynamic_cast<PrimitiveArrayPropertyMap const*>(&proto))
        return PrimitiveArrayPropertyMap::Clone(*protoMap, parentPropertyMap);

    if (auto protoMap = proto.GetAsStructArrayPropertyMap())
        return StructArrayJsonPropertyMap::Clone(*protoMap, parentPropertyMap);

    if (auto protoMap = dynamic_cast<StructPropertyMap const*>(&proto))
        return StructPropertyMap::Clone(ecdbMap, *protoMap, targetClass, parentPropertyMap);

    if (auto protoMap = proto.GetAsNavigationPropertyMap())
        {
        BeAssert(parentPropertyMap == nullptr && "NavigationProperties can only be created on entity classes, so parent prop map is expected to be nullptr");
        return NavigationPropertyMap::Clone(ecdbMap.GetSchemaImportContext()->GetClassMapLoadContext(), *protoMap, targetClass);
        }

    BeAssert(false && "Case is not handled");
    return nullptr;
    }

//******************************** PropertyMap *****************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan        04/2016
//---------------------------------------------------------------------------------------
void PropertyMap::_WriteDebugInfo(DebugWriter& writer) const
    {

    writer.AppendLine("ECProperty : %s, [Id=%" PRId64 "], [ECClass=%s], [Type=%s]",
                      GetProperty().GetName().c_str(),
                      GetProperty().GetId().GetValue(),
                      GetProperty().GetClass().GetName().c_str(),
                      GetProperty().GetTypeName().c_str()
                      );


    if (!m_children.IsEmpty())
        {
        if (auto b0 = writer.CreateIndentBlock())
            {
            for (auto propertyMap : m_children)
                {
                propertyMap->WriteDebugInfo(writer);
                }
            }
        }
    else
        {
        if (auto b0 = writer.CreateIndentBlock())
            {
            std::vector<DbColumn const*> columns;
            GetColumns(columns);
            for (auto column : columns)
                {
                writer.AppendLine("Column : %s, [Id=%" PRId64 "], [Table=%s], [Type=%s], [Virtual=%s], [Kind=%s] ",
                                  column->GetName().c_str(),
                                  column->GetId().GetValue(),
                                  column->GetTable().GetName().c_str(),
                                  column->TypeToSql(column->GetType()),
                                  column->GetPersistenceType() == PersistenceType::Persisted ? "false" : "true",
                                  DbColumn::KindToString(column->GetKind())
                                  );
                }
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
DbTable const* PropertyMap::GetTable() const
    {
    if (m_mappedTables.empty())
        return nullptr;
    
    if (IsSystemPropertyMap())
        return m_mappedTables.back();

    return m_mappedTables.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
bool PropertyMap::MapsToTable(DbTable const& candidateTable) const
    {
    for (DbTable const* table : m_mappedTables)
        {
        if (table == &candidateTable)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCR PropertyMap::GetRoot() const
    {
    PropertyMapCP current = this;
    while (current->m_parentPropertyMap != nullptr)
        current = current->m_parentPropertyMap;

    BeAssert(current != nullptr);
    return *current;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus PropertyMap::DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, DbColumn::Constraint::Collation& collation, 
                                               ECDbCR ecdb, ECPropertyCR ecProp, Utf8CP propAccessString)
    {
    columnName.clear();
    isNullable = true;
    isUnique = false;
    collation = DbColumn::Constraint::Collation::Default;

    ECDbPropertyMap customPropMap;
    if (ECDbMapCustomAttributeHelper::TryGetPropertyMap(customPropMap, ecProp))
        {
        if (!ecProp.GetIsPrimitive())
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                        "Failed to map ECProperty '%s:%s': only primitive ECProperties can have the custom attribute PropertyMap.",
                        ecProp.GetClass().GetFullName(), ecProp.GetName().c_str());
            return ERROR;
            }

        if (ECObjectsStatus::Success != customPropMap.TryGetColumnName(columnName))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsNullable(isNullable))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsUnique(isUnique))
            return ERROR;

        Utf8String collationStr;
        if (ECObjectsStatus::Success != customPropMap.TryGetCollation(collationStr))
            return ERROR;

        if (!DbColumn::Constraint::TryParseCollationString(collation, collationStr.c_str()))
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                       "Failed to map ECProperty '%s:%s': Custom attribute PropertyMap has an invalid value for the property 'Collation': %s",
                       ecProp.GetClass().GetFullName(), ecProp.GetName().c_str(),
                       collationStr.c_str());
            return ERROR;
            }

        }

    // PropertyMappingRule: if custom attribute PropertyMap does not supply a column name for an ECProperty, 
    // we use the ECProperty's propertyAccessString (and replace . by _)
    if (columnName.empty())
        {
        columnName.assign(propAccessString);
        columnName.ReplaceAll(".", "_");
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMap::_Save(DbClassMapSaveContext& ctx) const
    {
    std::vector<DbColumn const*> columns;
    GetColumns(columns);
    if (columns.size() == 0)
        return SUCCESS;

    const ECPropertyId rootPropertyId = GetRoot().GetProperty().GetId();
    Utf8CP accessString = GetPropertyAccessString();
    for (DbColumn const* column : columns)
        {
        if (ctx.InsertPropertyMap(rootPropertyId, accessString, column->GetId()) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
    {
    return _ToNativeSql(classIdentifier, ecsqlType, wrapInParentheses, tableFilter);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, DbTable const* tableFilter) const
    {
    std::vector<DbColumn const*> columns;
    GetColumns (columns);

    NativeSqlBuilder::List nativeSqlSnippets;
    for (DbColumn const* column : columns)
        {
        if (tableFilter != nullptr && &column->GetTable() != tableFilter)
            continue;

        BeAssert (!column->GetName ().empty());

        NativeSqlBuilder sqlSnippet;
        if (wrapInParentheses)
            sqlSnippet.AppendParenLeft();

        sqlSnippet.Append (classIdentifier, column->GetName ().c_str());

        if (wrapInParentheses)
            sqlSnippet.AppendParenRight();

        nativeSqlSnippets.push_back (std::move (sqlSnippet));
        }

    return nativeSqlSnippets;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMap::FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    return _FindOrCreateColumnsInTable(classMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::_GetColumns (std::vector<DbColumn const*>& columns) const {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      02/2015
//---------------------------------------------------------------------------------------
void PropertyMap::GetColumns(std::vector<DbColumn const*>& columns, DbTable const& table) const
    {
    std::vector<DbColumn const*> cols;
    GetColumns(cols);

    for (DbColumn const* col : cols)
        {
        if (&col->GetTable() == &table)
            columns.push_back(col);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
DbColumn const* PropertyMap::GetSingleColumn() const
    {
    std::vector<DbColumn const*> columns;
    GetColumns(columns);
    BeAssert(columns.size() == 1 && "Expecting Single Column");
    if (columns.empty() || columns.size() > 1)
        return nullptr;

    return columns.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
DbColumn const* PropertyMap::GetSingleColumn(DbTable const& table, bool alwaysFilterByTable) const
    {
    std::vector<DbColumn const*> columns;
    GetColumns(columns);

    if (columns.size() == 1 && !alwaysFilterByTable)
        return columns[0];

    DbColumn const* foundCol = nullptr;
    for (DbColumn const* col : columns)
        {
        if (&col->GetTable() == &table)
            {
            if (foundCol != nullptr)
                {
                BeAssert(false && "Cannot retrieve a single column for the given table");
                return nullptr;
                }

            foundCol = col;
            }
        }

    BeAssert(foundCol != nullptr && "PropertyMap doesn't have any columns matching the table");
    return foundCol;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
DbTable const* PropertyMap::GetSingleTable() const
    {
    std::vector<DbColumn const*> columns;
    GetColumns(columns);
    BeAssert(columns.size() > 0 && "Expecting at least one column");
    if (columns.empty())
        return nullptr;

    size_t i = 0;
    DbTable const* table = &columns.at(i++)->GetTable();
    for (; i < columns.size(); i++)
        {
        if (table != &columns.at(i)->GetTable())
            {
            BeAssert(false && "Expecting single table");
            return nullptr;
            }
        }

    return table;
    }


//************************** PropertyMapCollection ***************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection::PropertyMapCollection(PropertyMapCollection&& rhs)
    : m_dictionary(std::move(rhs.m_dictionary)), m_orderedCollection(std::move(rhs.m_orderedCollection))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection& PropertyMapCollection::operator= (PropertyMapCollection&& rhs)
    {
    if (this != &rhs)
        {
        m_dictionary = std::move(rhs.m_dictionary);
        m_orderedCollection = std::move(rhs.m_orderedCollection);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyMapCollection::AddPropertyMap(PropertyMapPtr const& propertyMap)
    {
    AddPropertyMap(propertyMap->GetPropertyAccessString(), propertyMap);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyMapCollection::AddPropertyMap(Utf8CP propertyAccessString, PropertyMapPtr const& propertyMap)
    {
    m_dictionary[propertyAccessString] = propertyMap;
    m_orderedCollection.push_back(propertyMap.get());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap(PropertyMapCP& propertyMap, Utf8CP propertyAccessString, bool recursive) const
    {
    propertyMap = nullptr;

    PropertyMapPtr propertyMapPtr = nullptr;
    const bool found = TryGetPropertyMap(propertyMapPtr, propertyAccessString, recursive);
    if (found)
        propertyMap = propertyMapPtr.get();

    return found;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap(PropertyMapPtr& propertyMap, Utf8CP propertyAccessString, bool recursive) const
    {
    propertyMap = nullptr;
    bool found = TryGetPropertyMapNonRecursively(propertyMap, propertyAccessString);
    if (found || !recursive)
        return found;

    BeAssert(recursive && !found);

    //recurse into access string and look up prop map for first member in access string
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(propertyAccessString, ".", nullptr, tokens);
    bvector<Utf8String>::const_iterator tokenIt = tokens.begin();
    bvector<Utf8String>::const_iterator tokenEndIt = tokens.end();
    return TryGetPropertyMap(propertyMap, tokenIt, tokenEndIt);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap(PropertyMapPtr& propertyMap, bvector<Utf8String>::const_iterator& accessStringTokenIterator, bvector<Utf8String>::const_iterator& accessStringTokenEndIterator) const
    {
    if (accessStringTokenIterator == accessStringTokenEndIterator)
        return false;

    Utf8StringCR currentAccessItem = *accessStringTokenIterator;

    PropertyMapPtr currentAccessPropMap = nullptr;
    bool found = TryGetPropertyMapNonRecursively(currentAccessPropMap, currentAccessItem.c_str());
    if (!found)
        return false;

    accessStringTokenIterator++;
    if (accessStringTokenIterator == accessStringTokenEndIterator)
        {
        //if there are no more tokens in the access string, the actual prop map was found
        propertyMap = currentAccessPropMap;
        return true;
        }

    //now recurse into children
    return currentAccessPropMap->GetChildren().TryGetPropertyMap(propertyMap, accessStringTokenIterator, accessStringTokenEndIterator);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMapNonRecursively(PropertyMapPtr& propertyMap, Utf8CP propertyAccessString) const
    {
    propertyMap = nullptr;
    auto it = m_dictionary.find(propertyAccessString);
    if (it != m_dictionary.end())
        {
        BeAssert(it->second.IsValid() && it->second.get() != nullptr);
        propertyMap = it->second;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   12/2013
//---------------------------------------------------------------------------------------
void PropertyMapCollection::Traverse(std::function<void(TraversalFeedback& cancel, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const
    {
    std::set<PropertyMapCollection const*> doneList;
    Traverse(doneList, *this, nodeOperation, recursive);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   12/2013
//---------------------------------------------------------------------------------------
//static 
void PropertyMapCollection::Traverse(std::set<PropertyMapCollection const*>& doneList, PropertyMapCollection const& childPropMaps, std::function<void(TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive)
    {
    if (doneList.find(&childPropMaps) != doneList.end())
        return;

    doneList.insert(&childPropMaps);
    for (PropertyMapCP propMap : childPropMaps)
        {
        TraversalFeedback feedback = TraversalFeedback::Next;
        nodeOperation(feedback, propMap);
        if (feedback == TraversalFeedback::Cancel)
            return;

        if (feedback == TraversalFeedback::NextSibling)
            continue;

        Traverse(doneList, propMap->GetChildren(), nodeOperation, recursive);
        }
    }

//**************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
StructPropertyMap::StructPropertyMap (ECN::StructECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, parentPropertyMap)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
StructPropertyMap::StructPropertyMap(ECDbMapCR ecdbMap, StructPropertyMap const& proto, ECN::ECClassCR targetClass, PropertyMap const* parentPropertyMap)
    : PropertyMap(proto, parentPropertyMap)
    {
    for (PropertyMap const* protoChild : proto.m_children)
        {
        m_children.AddPropertyMap(protoChild->GetProperty().GetName().c_str(), PropertyMapFactory::ClonePropertyMap(ecdbMap, *protoChild, targetClass, this));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
void StructPropertyMap::_GetColumns(std::vector<DbColumn const*>& columns) const 
    {
    for (auto childPropMap : m_children)
        {
        childPropMap->GetColumns (columns);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus StructPropertyMap::Initialize(ClassMapLoadContext& ctx, ECDbCR ecdb)
    {
    StructECPropertyCP structProperty = GetProperty().GetAsStructProperty();
    ECClassCR rootClass = GetProperty().GetClass();
    for (ECPropertyCP property : structProperty->GetType().GetProperties(true))
        {
        Utf8String accessString(GetPropertyAccessString());
        accessString.append(".").append(property->GetName());
        PropertyMapPtr propertyMap = PropertyMapFactory::CreatePropertyMap(ctx, ecdb, rootClass, *property, accessString.c_str(), this);
        //don't use full prop access string as key in child collection, but just the relative prop access string which is 
        //just the prop name
        if (propertyMap != nullptr)
            m_children.AddPropertyMap(property->GetName().c_str(), propertyMap);
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus StructPropertyMap::_FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    for(PropertyMap const* childPropMap : m_children)
        {
        if (SUCCESS != const_cast<PropertyMap*> (childPropMap)->FindOrCreateColumnsInTable(classMap))
            return ERROR;

        m_mappedTables.insert(m_mappedTables.end(), childPropMap->GetTables().begin(), childPropMap->GetTables().end());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StructPropertyMap::_Load(DbClassMapLoadContext const& dbClassMapLoadContext)
    {
    for (PropertyMap const* child : GetChildren())
        {
        if (SUCCESS != const_cast<PropertyMap*>(child)->Load(dbClassMapLoadContext))
            return ERROR;

        m_mappedTables.insert(m_mappedTables.end(), child->GetTables().begin(), child->GetTables().end());
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus StructPropertyMap::_Save(DbClassMapSaveContext& ctx) const
    {
    for (PropertyMap const* child : GetChildren())
        {
        if (SUCCESS != child->Save(ctx))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
PropertyMapPtr StructPropertyMap::Create(ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::StructECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    RefCountedPtr<StructPropertyMap> newPropertyMap = new StructPropertyMap(prop, propertyAccessString, parentPropertyMap);
    if (newPropertyMap->Initialize(ctx, ecdb) == BentleyStatus::SUCCESS)
        return newPropertyMap;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
Utf8String StructPropertyMap::_ToString() const 
    {
    Utf8String str;
    str.Sprintf ("StructPropertyMap: ecProperty=%s.%s\n", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str());
    str.append (" Child property maps:\n");
    for (auto childPropMap : m_children)
        {
        str.append ("\t").append (childPropMap->ToString ().c_str ()).append ("\n");
        }

    return str;
    }

//******************************** SingleColumnPropertyMap *****************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SingleColumnPropertyMap::_Load(DbClassMapLoadContext const& dbClassMapLoadContext)
    {
    BeAssert(m_column == nullptr);
    auto info = dbClassMapLoadContext.FindColumnByAccessString(GetPropertyAccessString());
    if (info == nullptr)
        {
        return ERROR;
        }
    BeAssert(info->size() == 1);
    SetColumn(*const_cast<DbColumn*>(info->front()));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    01/2016
//---------------------------------------------------------------------------------------
BentleyStatus SingleColumnPropertyMap::DoFindOrCreateColumnsInTable(ClassMap const& classMap, DbColumn::Type colType)
    {
    Utf8String colName;
    bool isNullable = true;
    bool isUnique = false;
    DbColumn::Constraint::Collation collation = DbColumn::Constraint::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(colName, isNullable, isUnique, collation, classMap.GetECDbMap().GetECDb()))
        return ERROR;
    
    DbColumn* col = classMap.GetColumnFactory().CreateColumn(*this, colName.c_str(), colType, !isNullable, isUnique, collation);
    if (col == nullptr)
        {
        BeAssert(col != nullptr);
        return ERROR;
        }

    SetColumn(*col);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    01/2016
//---------------------------------------------------------------------------------------
void SingleColumnPropertyMap::SetColumn(DbColumn const& col)
    {
    m_column = &col;
    m_mappedTables.push_back(&m_column->GetTable());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void SingleColumnPropertyMap::_GetColumns (std::vector<DbColumn const*>& columns) const
    {
    columns.push_back(m_column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2013
//---------------------------------------------------------------------------------------
bool SingleColumnPropertyMap::_IsVirtual() const
    {
    return m_column != nullptr && m_column->GetPersistenceType() == PersistenceType::Virtual;
    }

//******************************** PrimitivePropertyMap *****************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus PrimitivePropertyMap::_FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    const DbColumn::Type colType = DbColumn::PrimitiveTypeToColumnType(GetPrimitiveProperty().GetType());
    return DoFindOrCreateColumnsInTable(classMap, colType);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrimitivePropertyMap::_ToString() const
    {
    return Utf8PrintfString("PrimitivePropertyMap: ECProperty: %s.%s, Column name:%s", GetProperty().GetClass().GetFullName(),
                            GetProperty().GetName().c_str(), GetColumn().GetName().c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PointPropertyMap::PointPropertyMap(PrimitiveECPropertyCR pointProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap(pointProperty, propertyAccessString, parentPropertyMap), m_is3d(false), m_xColumn(nullptr), m_yColumn(nullptr), m_zColumn(nullptr)
    {
    switch (pointProperty.GetType())
        {
            case PRIMITIVETYPE_Point3D:
                m_is3d = true;
                break;
            case PRIMITIVETYPE_Point2D:
                m_is3d = false;
                break;

            default:
                BeAssert(false && "PointPropertyMap ctr must be called with a Point2D or Point3D ECProperty");
                break;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointPropertyMap::_Save(DbClassMapSaveContext& ctx) const
    {
    BeAssert(m_xColumn != nullptr);
    BeAssert(m_yColumn != nullptr);
    
    auto rootPropertyId = GetRoot().GetProperty().GetId();
    Utf8String accessString = GetPropertyAccessString();
    
    if (ctx.InsertPropertyMap(rootPropertyId, (accessString + ".X").c_str(), m_xColumn->GetId()) != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    if (ctx.InsertPropertyMap(rootPropertyId, (accessString + ".Y").c_str(), m_yColumn->GetId()) != SUCCESS)
        {
        BeAssert(false);
        return ERROR;
        }

    if (m_is3d)
        {
        BeAssert(m_zColumn != nullptr);
        if (ctx.InsertPropertyMap(rootPropertyId, (accessString + ".Z").c_str(), m_zColumn->GetId()) != SUCCESS)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointPropertyMap::_Load(DbClassMapLoadContext const& dbClassMapLoadContext)
    {
    BeAssert(m_xColumn == nullptr);
    BeAssert(m_yColumn == nullptr);
    BeAssert(m_zColumn == nullptr);

    /*const ECPropertyId rootPropertyId = */GetRoot().GetProperty().GetId();
    Utf8String accessString(GetPropertyAccessString());
    std::vector<DbColumn const*> const*  xPropMapping = dbClassMapLoadContext.FindColumnByAccessString((accessString + ".X").c_str());
    if (xPropMapping == nullptr)
        {
        //WIP_ECSCHEMA_UPGRADE
        return ERROR;
        }

    std::vector<DbColumn const*> const*  yPropMapping = dbClassMapLoadContext.FindColumnByAccessString((accessString + ".Y").c_str());
    if (yPropMapping == nullptr)
        {
        //WIP_ECSCHEMA_UPGRADE
        return ERROR;
        }

    std::vector<DbColumn const*> const* zPropMapping = nullptr;
    if (m_is3d)
        {
        zPropMapping = dbClassMapLoadContext.FindColumnByAccessString((accessString + ".Z").c_str());
        if (zPropMapping == nullptr)
            {
            //WIP_ECSCHEMA_UPGRADE
            return ERROR;
            }
        }
   
    return SetColumns(*(xPropMapping->front()), *(yPropMapping->front()), m_is3d ? zPropMapping->front() : nullptr);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+-
BentleyStatus PointPropertyMap::SetColumns(DbColumn const& xCol, DbColumn const& yCol, DbColumn const* zCol)
    {
    bset<DbTable const*> tables;
    m_xColumn = &xCol;
    tables.insert(&xCol.GetTable());

    m_yColumn = &yCol;
    tables.insert(&yCol.GetTable());

    m_zColumn = zCol;
    if (m_zColumn != nullptr)
        tables.insert(&m_zColumn->GetTable());

    if (tables.size() != 1)
        {
        BeAssert(false && "PropertyMapPoint should always only have columns in exactly one table");
        return ERROR;
        }

    m_mappedTables.push_back(&m_xColumn->GetTable());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PointPropertyMap::_ToString() const
    {
    if (m_is3d)
        return Utf8PrintfString("PointPropertyMap (3D): ECProperty: %s.%s, Column names: %s, %s, %s", GetProperty().GetClass().GetFullName(),
                                GetProperty().GetName().c_str(), m_xColumn->GetName().c_str(), m_yColumn->GetName().c_str(), m_zColumn->GetName().c_str());

    return Utf8PrintfString("PointPropertyMap (2D): ECProperty: %s.%s, Column names: %s, %s", GetProperty().GetClass().GetFullName(),
                                GetProperty().GetName().c_str(), m_xColumn->GetName().c_str(), m_yColumn->GetName().c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointPropertyMap::_FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    Utf8String columnName;
    bool isNullable = true;
    bool isUnique = false;
    DbColumn::Constraint::Collation collation = DbColumn::Constraint::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(columnName, isNullable, isUnique, collation, classMap.GetECDbMap().GetECDb()))
        return ERROR;

    const DbColumn::Type colType = DbColumn::Type::Real;
    bset<DbTable const*> tables;
    Utf8String xColumnName(columnName);
    xColumnName.append("_X");

    DbColumn const* xCol = classMap.GetColumnFactory().CreateColumn(*this, xColumnName.c_str(), colType, !isNullable, isUnique, collation);
    if (xCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String yColumnName(columnName);
    yColumnName.append("_Y");
    DbColumn const* yCol = classMap.GetColumnFactory().CreateColumn(*this, yColumnName.c_str(), colType, !isNullable, isUnique, collation);
    if (yCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    DbColumn const* zCol = nullptr;
    if (m_is3d)
        {
        Utf8String zColumnName(columnName);
        zColumnName.append("_Z");
        zCol = classMap.GetColumnFactory().CreateColumn(*this, zColumnName.c_str(), colType, !isNullable, isUnique, collation);
        if (zCol == nullptr)
            {
            BeAssert(false);
            return ERROR;
            }
        }

    return SetColumns(*xCol, *yCol, zCol);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PointPropertyMap::_GetColumns(std::vector<DbColumn const*>& columns) const
    {
    columns.push_back(m_xColumn);
    columns.push_back(m_yColumn);
    if (m_is3d)
        columns.push_back(m_zColumn);
    }
    

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PrimitiveArrayPropertyMap::PrimitiveArrayPropertyMap (ECDbCR ecdb, ArrayECPropertyCR arrayProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : SingleColumnPropertyMap (arrayProperty, propertyAccessString, parentPropertyMap)
    {
    BeAssert(!arrayProperty.GetIsStructArray());
    ECClassCP primitiveArrayPersistenceClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence (ecdb, arrayProperty.GetPrimitiveElementType());
    BeAssert(primitiveArrayPersistenceClass != nullptr);
    m_primitiveArrayEnabler = primitiveArrayPersistenceClass->GetDefaultStandaloneEnabler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus PrimitiveArrayPropertyMap::_FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    if (!GetProperty().GetIsPrimitiveArray())
        {
        BeAssert(false);
        return ERROR;
        }

    return DoFindOrCreateColumnsInTable(classMap, DbColumn::Type::Blob);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrimitiveArrayPropertyMap::_ToString() const
    {
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert (arrayProperty);
    
    return Utf8PrintfString("PrimitiveArrayPropertyMap: ECProperty: %s.%s, Column name=%s", GetProperty().GetClass().GetFullName(), 
                            GetProperty().GetName().c_str(), GetColumn().GetName().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      03/2016
//---------------------------------------------------------------------------------------
BentleyStatus StructArrayJsonPropertyMap::_FindOrCreateColumnsInTable(ClassMap const& classMap)
    {
    return DoFindOrCreateColumnsInTable(classMap, DbColumn::Type::Text);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      03/2016
//---------------------------------------------------------------------------------------
Utf8String StructArrayJsonPropertyMap::_ToString() const
    {
    Utf8String str;
    str.Sprintf("StructArrayJsonPropertyMap: ECProperty: %s.%s, Column name: %s", GetProperty().GetClass().GetFullName(),
                GetProperty().GetName().c_str(), GetColumn().GetName().c_str());
    return str;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
PropertyMapPtr NavigationPropertyMap::Create(ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::ECClassCR ecClass, ECN::NavigationECPropertyCR navProp, Utf8CP propertyAccessString)
    {
    if (navProp.IsMultiple())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' has a multiplicity of '%s'. ECDb only supports NavigationECProperties with a maximum multiplicity of 1.",
                                                      ecClass.GetFullName(), navProp.GetName().c_str(),
                                                      GetConstraint(navProp, NavigationEnd::To).GetCardinality().ToString().c_str());
        return nullptr;
        }

    return new NavigationPropertyMap(ctx, ecClass, navProp, propertyAccessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      12/2015
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, ECN::ECClassCR ecClass, ECN::NavigationECPropertyCR prop, Utf8CP propertyAccessString)
    : PropertyMap(prop, propertyAccessString, nullptr), m_relClassMap(nullptr), m_ecClass(ecClass)
    {
    //we need to wait with finishing the nav prop map set up to the end when all relationships have been processed
    ctx.AddNavigationPropertyMap(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, NavigationPropertyMap const& proto, ECClassCR targetClass)
    :PropertyMap(proto, nullptr), m_relClassMap(proto.m_relClassMap), m_ecClass(targetClass)
    {
    //we need to wait with finishing the nav prop map set up to the end when all relationships have been imported and mapped
    if (proto.m_relClassMap == nullptr)
        ctx.AddNavigationPropertyMap(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyMap::Postprocess(ECDbMapCR ecdbMap)
    {
    m_columns.clear();
    if (m_relClassMap != nullptr)
        {
        BeAssert(false);
        return SUCCESS;
        }

    ClassMap const* relClassMap = ecdbMap.GetClassMap(*GetNavigationProperty().GetRelationshipClass());
    if (relClassMap == nullptr || !relClassMap->IsRelationshipClassMap())
        {
        BeAssert(false && "RelationshipClassMap should not be nullptr when finishing the NavigationPropMap");
        return ERROR;
        }

    if (relClassMap->GetType() == ClassMap::Type::RelationshipLinkTable)
        {
        ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                   "Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
                                                                   GetNavigationProperty().GetClass().GetFullName(), GetNavigationProperty().GetName().c_str());
        return ERROR;
        }

    m_relClassMap = static_cast<RelationshipClassMap const*> (relClassMap);

    ClassMap const* classMap = ecdbMap.GetClassMap(m_ecClass);
    if (classMap == nullptr)
        {
        BeAssert(false && "Fail to find ClassMap that created the NavigationPropMap");
        return ERROR;
        }

    DbColumn const* fkCol = nullptr;
    std::vector<DbColumn const*> columns;
    GetConstraintMap(NavigationEnd::To).GetECInstanceIdPropMap()->GetColumns(columns, classMap->GetPrimaryTable());
    if (columns.size() == 1)
        fkCol = columns.front();

    if (fkCol == nullptr && !classMap->IsMappedToSingleTable())
        {
        GetConstraintMap(NavigationEnd::To).GetECInstanceIdPropMap()->GetColumns(columns, classMap->GetJoinedTable());
        if (columns.size() == 1)
            fkCol = columns.front();
        }

    if (fkCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }
    
    m_columns.push_back(fkCol);
    BeAssert(m_mappedTables.empty());
    m_mappedTables.push_back(&fkCol->GetTable());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
bool NavigationPropertyMap::IsSupportedInECSql(bool logIfNotSupported, ECDbCP ecdb) const
    {
    BeAssert(!logIfNotSupported || ecdb != nullptr);

    if (GetNavigationProperty().IsMultiple())
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, 
              "NavigationECProperty '%s.%s' cannot be used in ECSQL because its multiplicity is %s. Only the multiplicities %s or %s are supported.",
                                                           GetNavigationProperty().GetClass().GetFullName(), GetNavigationProperty().GetName().c_str(),
                                                           GetConstraint(GetNavigationProperty(), NavigationEnd::To).GetCardinality().ToString().c_str(),
                                                           RelationshipCardinality::ZeroOne().ToString().c_str(),
                                                           RelationshipCardinality::OneOne().ToString().c_str());
        return false;
        }
    if (m_relClassMap == nullptr)
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a virtual table.",
                                                           GetNavigationProperty().GetClass().GetFullName(), GetNavigationProperty().GetName().c_str());

        return false;
        }

    if (m_relClassMap->GetType() != ClassMap::Type::RelationshipEndTable)
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a link table.",
                                                           GetNavigationProperty().GetClass().GetFullName(), GetNavigationProperty().GetName().c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyMap::_GetColumns(std::vector<DbColumn const*>& columns) const
    {
    BeAssert(IsSupportedInECSql() && "NavProperty which is not supported in ECSQL");
    columns = m_columns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& NavigationPropertyMap::GetConstraintMap(NavigationEnd end) const
    {
    return m_relClassMap->GetConstraintMap(GetConstraintEnd(end));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipConstraintCR NavigationPropertyMap::GetConstraint(ECN::NavigationECPropertyCR navProp, NavigationEnd end)
    {
    ECRelationshipClassCP relClass = navProp.GetRelationshipClass();
    return GetConstraintEnd(navProp, end) == ECRelationshipEnd_Source ? relClass->GetSource() : relClass->GetTarget();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipEnd NavigationPropertyMap::GetConstraintEnd(ECN::NavigationECPropertyCR prop, NavigationEnd end)
    {
    const ECRelatedInstanceDirection navPropDir = prop.GetDirection();

    if (navPropDir == ECRelatedInstanceDirection::Forward && end == NavigationEnd::From ||
        navPropDir == ECRelatedInstanceDirection::Backward && end == NavigationEnd::To)
        return ECRelationshipEnd_Source;

    return ECRelationshipEnd_Target;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


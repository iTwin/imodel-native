/*--------------------------------------------------------------------------------------+
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

//******************************** PropertyMap *****************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMap::PropertyMap(ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap) : m_ecProperty(ecProperty), m_propertyAccessString(propertyAccessString), m_parentPropertyMap(parentPropertyMap), m_propertyPathId(0)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(propertyAccessString));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan          10/2015
//---------------------------------------------------------------------------------------
//static
PropertyMapPtr PropertyMap::Clone(ECDbMapCR ecdbMap, PropertyMapCR proto, ECN::ECClassCR clonedBy,  PropertyMap const* parentPropertyMap) 
    {
    if (ecdbMap.GetSchemaImportContext() == nullptr)
        {
        BeAssert(false && "PropertyMap::Clone must only be called during schema import");
        return nullptr;
        }

    if (auto protoMap = dynamic_cast<PropertyMapPoint const*>(&proto))
        {
        return new PropertyMapPoint(*protoMap, parentPropertyMap);
        }
    else if (auto protoMap = dynamic_cast<PropertyMapSingleColumn const*>(&proto))
        {
        return new PropertyMapSingleColumn(*protoMap, parentPropertyMap);
        }
    else if (auto protoMap = dynamic_cast<PropertyMapPrimitiveArray const*>(&proto))
        {
        return new PropertyMapPrimitiveArray(*protoMap, parentPropertyMap);
        }
    else if (auto protoMap = dynamic_cast<PropertyMapStructArray const*>(&proto))
        {
        return new PropertyMapStructArray(*protoMap, parentPropertyMap);
        }
    else if (auto protoMap = dynamic_cast<PropertyMapStruct const*>(&proto))
        {
        return new PropertyMapStruct(ecdbMap, *protoMap, clonedBy,  parentPropertyMap);
        }
    else if (auto protoMap = proto.GetAsNavigationPropertyMap())
        {
        return new NavigationPropertyMap(ecdbMap.GetSchemaImportContext()->GetClassMapLoadContext(), *protoMap, parentPropertyMap, clonedBy);
        }

    BeAssert(false && "Case is not handled");
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     12/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::IsVirtual () const
    {
    return _IsVirtual ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
ECDbSqlTable const* PropertyMap::GetTable() const
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
bool PropertyMap::MapsToTable(ECDbSqlTable const& candidateTable) const
    {
    for (ECDbSqlTable const* table : m_mappedTables)
        {
        if (table == &candidateTable)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext& ctx, ECDbCR ecdb, ECPropertyCR ecProperty, ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
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
                    return new PropertyMapPoint(ecProperty, propertyAccessString, parentPropertyMap);

                default:
                    return new PropertyMapSingleColumn(ecProperty, propertyAccessString, parentPropertyMap);
            }
        }

    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();

    // PropertyMapRule: primitives, primitive arrays , and structs map to 1 or more columns in the ECClass's main table
    if (arrayProperty != nullptr)
        {
        if (ARRAYKIND_Primitive == arrayProperty->GetKind())
            return new PropertyMapPrimitiveArray(ecdb, ecProperty, propertyAccessString, parentPropertyMap);
        else
            {
            BeAssert(ARRAYKIND_Primitive != arrayProperty->GetKind());
            return PropertyMapStructArray::Create(ecProperty, propertyAccessString, parentPropertyMap);
            }
        }

    if (ecProperty.GetIsStruct())
        return PropertyMapStruct::Create(ctx, ecdb, ecProperty, propertyAccessString, parentPropertyMap); // The individual properties get their own binding, but we need a placeholder for the overall struct

    BeAssert(ecProperty.GetIsNavigation());
    return NavigationPropertyMap::Create(ctx, ecdb, ecProperty, propertyAccessString, parentPropertyMap, rootClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
//static
BentleyStatus PropertyMap::DetermineColumnInfo(Utf8StringR columnName, bool& isNullable, bool& isUnique, ECDbSqlColumn::Constraint::Collation& collation, 
                                               ECDbCR ecdb, ECPropertyCR ecProp, Utf8CP propAccessString)
    {
    columnName.clear();
    isNullable = true;
    isUnique = false;
    collation = ECDbSqlColumn::Constraint::Collation::Default;

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

        if (!ECDbSqlColumn::Constraint::TryParseCollationString(collation, collationStr.c_str()))
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


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     06/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::IsECInstanceIdPropertyMap () const
    {
    return _IsECInstanceIdPropertyMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     06/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::IsSystemPropertyMap () const
    {
    return _IsSystemPropertyMap ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMap::_Save(ECDbClassMapInfo & classMapInfo) const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns(columns);
    if (columns.size() == 0)
        return SUCCESS;

    ECDbPropertyMapInfo* mapInfo = classMapInfo.CreatePropertyMap(GetRoot().GetProperty().GetId(), GetPropertyAccessString(), columns);
    if (mapInfo == nullptr)
        return ERROR;

    m_propertyPathId = mapInfo->GetPropertyPath().GetId();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMap::Save(ECDbClassMapInfo & classMapInfo) const
    {
    return _Save (classMapInfo);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCR PropertyMap::GetProperty() const
    {
    return m_ecProperty;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMap::_ToString() const
    {
    return Utf8PrintfString("PropertyMap: ecProperty=%s.%s (%s)", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str(), m_ecProperty.GetTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
Utf8CP PropertyMap::GetPropertyAccessString() const
    {
    return m_propertyAccessString.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const
    {
    return _ToNativeSql(classIdentifier, ecsqlType, wrapInParentheses, tableFilter);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses, ECDbSqlTable const* tableFilter) const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns (columns);

    NativeSqlBuilder::List nativeSqlSnippets;
    for (ECDbSqlColumn const* column : columns)
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

    return std::move (nativeSqlSnippets);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMap::FindOrCreateColumnsInTable(ClassMap& classMap, ClassMapInfo const* classMapInfo)
    {
    return _FindOrCreateColumnsInTable(classMap, classMapInfo);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMap::ToString() const
    {
    return _ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    _GetColumns (columns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      02/2015
//---------------------------------------------------------------------------------------
void PropertyMap::GetColumns(std::vector<ECDbSqlColumn const*>& columns, ECDbSqlTable const& table) const
    {
    std::vector<ECDbSqlColumn const*> cols;
    GetColumns(cols);

    for (ECDbSqlColumn const* col : cols)
        {
        if (&col->GetTable() == &table)
            columns.push_back(col);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* PropertyMap::GetSingleColumn() const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns(columns);
    BeAssert(columns.size() == 1 && "Expecting Single Column");
    if (columns.empty() || columns.size() > 1)
        return nullptr;

    return columns.front();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* PropertyMap::GetSingleColumn(ECDbSqlTable const& table, bool alwaysFilterByTable) const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns(columns);

    if (columns.size() == 1 && !alwaysFilterByTable)
        return columns[0];

    ECDbSqlColumn const* foundCol = nullptr;
    for (ECDbSqlColumn const* col : columns)
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
ECDbSqlTable const* PropertyMap::GetSingleTable() const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns(columns);
    BeAssert(columns.size() > 0 && "Expecting atleast one column");
    if (columns.empty())
        return nullptr;

    size_t i = 0;
    ECDbSqlTable const* table = &columns.at(i++)->GetTable();
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
    ECDbMap::ParsePropertyAccessString(tokens, propertyAccessString);

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
PropertyMapStruct::PropertyMapStruct (ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, parentPropertyMap)
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP PropertyMapStruct::GetPropertyMap (Utf8CP propertyName) const
    {
    for(auto childPropMap : m_children)
        {
        //Following is slow but propertyName is expected to be a accessString relative to this struct
        if (childPropMap->GetProperty().GetName().Equals (propertyName))
            return childPropMap;
        }

    Utf8String accessString = propertyName;
    auto n = accessString.find(".");
    if (n != Utf8String::npos)
        {
        Utf8String first = accessString.substr(0, n);
        Utf8String rest = accessString.substr(n + 1);
        PropertyMapCP propertyMap = nullptr;
        for(auto childPropMap : m_children)
            {
            //Following is slow but propertyName is expected to be a accessString relative to this struct
            if (childPropMap->GetProperty ().GetName ().Equals (first))
                {
                propertyMap = childPropMap;
                break;
                }
            }

        if (propertyMap != nullptr)
            {
            if(rest.empty())
                return propertyMap;

            if (auto structPropertyMap = dynamic_cast<PropertyMapStructCP>(propertyMap))
                {
                return structPropertyMap->GetPropertyMap(rest.c_str());
                } 
            }
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
void PropertyMapStruct::_GetColumns(std::vector<ECDbSqlColumn const*>& columns) const 
    {
    for (auto childPropMap : m_children)
        {
        childPropMap->GetColumns (columns);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapStruct::Initialize(ClassMapLoadContext& ctx, ECDbCR ecdb)
    {
    PRECONDITION(GetProperty().GetIsStruct() && "Expecting a struct type property", ERROR);
    ECClassCR rootClass = GetProperty().GetClass();
    StructECPropertyCP structProperty = GetProperty().GetAsStructProperty();
    for (ECPropertyCP property : structProperty->GetType().GetProperties(true))
        {
        Utf8String accessString(GetPropertyAccessString());
        accessString.append(".");
        accessString.append(property->GetName());
        PropertyMapPtr propertyMap = PropertyMap::CreateAndEvaluateMapping(ctx, ecdb, *property, rootClass, accessString.c_str(), this);
        if (propertyMap.IsValid())
            //don't use full prop access string as key in child collection, but just the relative prop access string which is 
            //just the prop name
            m_children.AddPropertyMap(property->GetName().c_str(), propertyMap);
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapStruct::_FindOrCreateColumnsInTable(ClassMap& classMap, ClassMapInfo const* classMapInfo)
    {
    for(auto childPropMap : m_children)
        {
        if (SUCCESS != const_cast<PropertyMapP> (childPropMap)->FindOrCreateColumnsInTable(classMap, classMapInfo))
            return ERROR;

        m_mappedTables.insert(m_mappedTables.end(), childPropMap->GetTables().begin(), childPropMap->GetTables().end());
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapStruct::_Load(ECDbClassMapInfo const& classMapInfo)
    {
    for (PropertyMap const* child : GetChildren())
        {
        if (SUCCESS != const_cast<PropertyMap*>(child)->Load(classMapInfo))
            return ERROR;

        m_mappedTables.insert(m_mappedTables.end(), child->GetTables().begin(), child->GetTables().end());
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapStruct::_Save(ECDbClassMapInfo& classMapInfo) const
    {
    for (PropertyMap const* child : GetChildren())
        {
        if (SUCCESS != child->Save(classMapInfo))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
PropertyMapStructPtr PropertyMapStruct::Create(ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    PRECONDITION(prop.GetIsStruct() && "Expecting a ECStruct type property", nullptr);
    PropertyMapStructPtr newPropertyMap = new PropertyMapStruct(prop, propertyAccessString, parentPropertyMap);
    if (newPropertyMap->Initialize(ctx, ecdb) == BentleyStatus::SUCCESS)
        return newPropertyMap;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
Utf8String PropertyMapStruct::_ToString() const 
    {
    Utf8String str;
    str.Sprintf ("PropertyMapStruct: ecProperty=%s.%s\n", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str());
    str.append (" Child property maps:\n");
    for (auto childPropMap : m_children)
        {
        str.append ("\t").append (childPropMap->ToString ().c_str ()).append ("\n");
        }

    return str;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapStructArray::PropertyMapStructArray (ECPropertyCR ecProperty, ECClassCR structElementType, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_structElementType (structElementType) 
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapStructArray::_Save (ECDbClassMapInfo & classMapInfo) const
    {
    ECDbMapStorage const& manager = classMapInfo.GetMapStorage ();
    auto propertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = GetPropertyAccessString ();
    ECDbPropertyPath const* propertyPath = manager.FindPropertyPath (propertyId, accessString);
    if (propertyPath == nullptr)
        propertyPath = manager.CreatePropertyPath (propertyId, accessString);

    if (propertyPath == nullptr)
        {
        BeAssert (false && "Failed to create propertyPath");
        return ERROR;
        }

    m_propertyPathId = propertyPath->GetId ();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapStructArray::_Load(ECDbClassMapInfo const& classMapInfo)
    {
    auto propertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = GetPropertyAccessString ();
    ECDbPropertyPath const* propertyPath = classMapInfo.GetMapStorage().FindPropertyPath (propertyId, accessString);
    if (propertyPath == nullptr)
        {
        BeAssert (false && "Failed to find propertyPath");
        return ERROR;
        }

    m_propertyPathId = propertyPath->GetId ();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapStructArrayPtr PropertyMapStructArray::Create (ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    StructArrayECPropertyCP structArrayProperty = ecProperty.GetAsStructArrayProperty();
    if (structArrayProperty == nullptr)
        {
        BeAssert(false && "Expecting a struct array property when using PropertyMapToTable");
        return nullptr;
        }

    ECClassCP structType = structArrayProperty->GetStructElementType();
    return new PropertyMapStructArray (ecProperty, *structType, propertyAccessString, parentPropertyMap);
    }
       
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
Utf8String PropertyMapStructArray::_ToString() const 
    {
    Utf8String str;
    str.Sprintf ("PropertyMapStructArray: ecProperty=%s.%s\r\n", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str());

    return str;
    }

//******************************** PropertyMapToColumn *****************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PropertyMapSingleColumn::PropertyMapSingleColumn (ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_primitiveProperty (ecProperty.GetAsPrimitiveProperty ()), m_column (nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2013
//---------------------------------------------------------------------------------------
bool PropertyMapSingleColumn::_IsVirtual () const
    {
    return m_column != nullptr && m_column->GetPersistenceType() == PersistenceType::Virtual;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapSingleColumn::_Load(ECDbClassMapInfo const& classMapInfo)
    {
    BeAssert(m_column == nullptr);
    ECDbPropertyMapInfo const* info = classMapInfo.FindPropertyMap(GetRoot().GetProperty().GetId(), GetPropertyAccessString());
    if (info == nullptr)
        {
        return ERROR;
        }

    SetColumn(*const_cast<ECDbSqlColumn*>(info->ExpectingSingleColumn()));
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapSingleColumn::_FindOrCreateColumnsInTable (ClassMap& classMap , ClassMapInfo const* classMapInfo)
    {
    if (!GetProperty().GetIsPrimitive())
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String colName;
    bool isNullable = true;
    bool isUnique = false;
    ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(colName, isNullable, isUnique, collation, classMap.GetECDbMap().GetECDb()))
        return ERROR;

    const PrimitiveType colType = GetProperty().GetAsPrimitiveProperty()->GetType();

    ECDbSqlColumn* col = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, colName.c_str(), colType, isNullable, isUnique, collation, nullptr);
    if (col == nullptr)
        {
        BeAssert(col != nullptr && "This actually indicates a mapping error. The method PropertyMapSingleColumn::_FindOrCreateColumnsInTable should therefore be changed to return an error.");
        return ERROR;
        }

    SetColumn(*col);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    01/2016
//---------------------------------------------------------------------------------------
void PropertyMapSingleColumn::SetColumn(ECDbSqlColumn& col)
    {
    m_column = &col;
    m_mappedTables.push_back(&m_column->GetTable());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMapSingleColumn::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    columns.push_back(m_column);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMapSingleColumn::_ToString() const
    {
    return Utf8PrintfString("PropertyMapSingleColumn: ecProperty=%s.%s, columnName=%s", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str(), m_column->GetName().c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPoint::PropertyMapPoint (ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_is3d(false), m_xColumn (nullptr), m_yColumn (nullptr), m_zColumn (nullptr)
    {
    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (!EXPECTED_CONDITION(primitiveProperty))
        return;

    switch (primitiveProperty->GetType())
        {
        case PRIMITIVETYPE_Point3D:
            m_is3d = true; 
            break;
        case PRIMITIVETYPE_Point2D: 
            m_is3d = false; 
            break;

        default:
            BeAssert(false);
            break;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapPoint::_Save (ECDbClassMapInfo & classMapInfo) const
    {
    BeAssert (m_xColumn != nullptr);
    BeAssert (m_yColumn != nullptr);

    auto rootPropertyId = GetRoot ().GetProperty ().GetId ();
    Utf8String accessString = GetPropertyAccessString ();
    auto pm = classMapInfo.CreatePropertyMap(rootPropertyId, (accessString + ".X").c_str(), {m_xColumn});
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return ERROR;
        }

    classMapInfo.CreatePropertyMap(rootPropertyId, (accessString + ".Y").c_str(), {m_yColumn});
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return ERROR;
        }

    if (m_is3d)
        {
        BeAssert (m_zColumn != nullptr);
        classMapInfo.CreatePropertyMap(rootPropertyId, (accessString + ".Z").c_str(), {m_zColumn});
        if (pm == nullptr)
            {
            BeAssert (false && "Failed to create propertymap");
            return ERROR;
            }
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapPoint::_Load(ECDbClassMapInfo const& classMapInfo)
    {
    BeAssert (m_xColumn == nullptr);
    BeAssert (m_yColumn == nullptr);
    BeAssert (m_zColumn == nullptr);

    const ECPropertyId rootPropertyId = GetRoot ().GetProperty ().GetId ();
    Utf8String accessString (GetPropertyAccessString ());
    ECDbPropertyMapInfo const* xPropMap = classMapInfo.FindPropertyMap(rootPropertyId, (accessString + ".X").c_str());
    if (xPropMap == nullptr)
        {
        BeAssert(false && "Failed to load propertymap");
        return ERROR;
        }

    ECDbPropertyMapInfo const* yPropMap = classMapInfo.FindPropertyMap(rootPropertyId, (accessString + ".Y").c_str());
    if (yPropMap == nullptr)
        {
        BeAssert(false && "Failed to load propertymap");
        return ERROR;
        }

    ECDbPropertyMapInfo const* zPropMap = nullptr;
    if (m_is3d)
        {
        zPropMap = classMapInfo.FindPropertyMap(rootPropertyId, (accessString + ".Z").c_str());
        if (zPropMap == nullptr)
            {
            BeAssert(false && "Failed to load propertymap");
            return ERROR;
            }
        }

    return SetColumns(*xPropMap->ExpectingSingleColumn(), *yPropMap->ExpectingSingleColumn(), m_is3d ? zPropMap->ExpectingSingleColumn() : nullptr);
    }

//----------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                01/2016
//+---------------+---------------+---------------+---------------+---------------+-
BentleyStatus PropertyMapPoint::SetColumns(ECDbSqlColumn const& xCol, ECDbSqlColumn const& yCol, ECDbSqlColumn const* zCol)
    {
    bset<ECDbSqlTable const*> tables;
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
Utf8String PropertyMapPoint::_ToString() const
    {
    if (m_is3d)
        return Utf8PrintfString("PropertyMapPoint (3d): ecProperty=%s.%s, columnNames=%s, %s, %s", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), m_xColumn->GetName().c_str(), m_yColumn->GetName().c_str(), m_zColumn->GetName().c_str());
    else
        return Utf8PrintfString("PropertyMapPoint (2d): ecProperty=%s.%s, columnName=%s_X, _Y", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), m_xColumn->GetName().c_str(), m_yColumn->GetName().c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapPoint::_FindOrCreateColumnsInTable(ClassMap& classMap,  ClassMapInfo const* classMapInfo)
    {
    Utf8String columnName;
    bool isNullable = true;
    bool isUnique = false;
    ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(columnName, isNullable, isUnique, collation, classMap.GetECDbMap().GetECDb()))
        return ERROR;

    bset<ECDbSqlTable const*> tables;
    Utf8String xColumnName(columnName);
    xColumnName.append("_X");
    ECDbSqlColumn const* xCol = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, xColumnName.c_str(), s_defaultCoordinateECType, isNullable, isUnique, collation, "X");
    if (xCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String yColumnName(columnName);
    yColumnName.append("_Y");
    ECDbSqlColumn const* yCol = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, yColumnName.c_str(), s_defaultCoordinateECType, isNullable, isUnique, collation, "Y");
    if (yCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ECDbSqlColumn const* zCol = nullptr;
    if (m_is3d)
        {
        Utf8String zColumnName(columnName);
        zColumnName.append("_Z");
        zCol = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, zColumnName.c_str(), s_defaultCoordinateECType, isNullable, isUnique, collation, "Z");
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
void PropertyMapPoint::_GetColumns(std::vector<ECDbSqlColumn const*>& columns) const
    {
    columns.push_back(m_xColumn);
    columns.push_back(m_yColumn);
    if (m_is3d) 
        columns.push_back(m_zColumn);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PropertyMapPrimitiveArray::PropertyMapPrimitiveArray (ECDbCR ecdb, ECPropertyCR ecProperty, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMapSingleColumn (ecProperty, propertyAccessString, parentPropertyMap)
    {
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert(arrayProperty);

    ECClassCP primitiveArrayPersistenceClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence (ecdb, arrayProperty->GetPrimitiveElementType());
    BeAssert(primitiveArrayPersistenceClass != nullptr);
    m_primitiveArrayEnabler = primitiveArrayPersistenceClass->GetDefaultStandaloneEnabler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapPrimitiveArray::_FindOrCreateColumnsInTable(ClassMap& classMap, ClassMapInfo const* classMapInfo)
    {
    if (!GetProperty().GetIsPrimitiveArray())
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String colName;
    bool isNullable = true;
    bool isUnique = false;
    ECDbSqlColumn::Constraint::Collation collation = ECDbSqlColumn::Constraint::Collation::Default;
    if (SUCCESS != DetermineColumnInfo(colName, isNullable, isUnique, collation, classMap.GetECDbMap().GetECDb()))
        return ERROR;

    //prim array is persisted as BLOB in a single col
    const PrimitiveType colType = PRIMITIVETYPE_Binary;

    ECDbSqlColumn* col = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, colName.c_str(), colType, isNullable, isUnique, collation, nullptr);
    if (col == nullptr)
        {
        BeAssert(col != nullptr && "This actually indicates a mapping error. The method PropertyMapPrimitiveArray::_FindOrCreateColumnsInTable should therefore be changed to return an error.");
        return ERROR;
        }

    SetColumn(*col);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMapPrimitiveArray::_ToString() const
    {
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert (arrayProperty);
    
    return Utf8PrintfString("PropertyMapPrimitiveArray: ecProperty=%s.%s, type=%s, columnName=%s", GetProperty().GetClass().GetFullName(), 
                            GetProperty().GetName().c_str(), ExpHelper::ToString(arrayProperty->GetPrimitiveElementType()), m_column->GetName().c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
PropertyMapPtr NavigationPropertyMap::Create(ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap, ECClassCR createdByClass)
    {
    NavigationECPropertyCP navProp = prop.GetAsNavigationProperty();
    if (navProp == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    if (navProp->IsMultiple())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' has a multiplicity of '%s'. ECDb only supports NavigationECProperties with a maximum multiplicity of 1.",
                                                      navProp->GetClass().GetFullName(), navProp->GetName().c_str(),
                                                      GetConstraint(*navProp, NavigationEnd::To).GetCardinality().ToString().c_str());
        return nullptr;
        }

    return new NavigationPropertyMap(ctx, prop, propertyAccessString, parentPropertyMap, createdByClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      12/2015
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap, ECClassCR createdByClass)
    : PropertyMap(prop, propertyAccessString, parentPropertyMap), m_navigationProperty(prop.GetAsNavigationProperty()), m_relClassMap(nullptr), m_createdByClass(&createdByClass)
    {
    BeAssert(prop.GetIsNavigation());
    
    //we need to wait with finishing the nav prop map set up to the end when all relationships have been imported and mapped
    ctx.AddNavigationPropertyMap(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, NavigationPropertyMap const& proto, PropertyMap const* parentPropertyMap, ECClassCR createdByClass)
    :PropertyMap(proto, parentPropertyMap), m_navigationProperty(proto.m_navigationProperty), m_relClassMap(proto.m_relClassMap), m_createdByClass(&createdByClass)
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

    ClassMap const* relClassMap = ecdbMap.GetClassMap(*m_navigationProperty->GetRelationshipClass());
    if (relClassMap == nullptr || !relClassMap->IsRelationshipClassMap())
        {
        BeAssert(false && "RelationshipClassMap should not be nullptr when finishing the NavigationPropMap");
        return ERROR;
        }

    if (relClassMap->GetClassMapType() == IClassMap::Type::RelationshipLinkTable)
        {
        ecdbMap.GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error,
                                                                   "Failed to map NavigationECProperty '%s.%s'. NavigationECProperties for ECRelationship that map to a link table are not supported by ECDb.",
                                                                   m_navigationProperty->GetClass().GetFullName(), m_navigationProperty->GetName().c_str());
        return ERROR;
        }

    m_relClassMap = static_cast<RelationshipClassMap const*> (relClassMap);

    BeAssert(m_createdByClass != nullptr);
    ClassMap const* classMap = ecdbMap.GetClassMap(*m_createdByClass);
    if (classMap == nullptr)
        {
        BeAssert(false && "Fail to find ClassMaps that created the NavigationPropMap");
        return ERROR;
        }

    ECDbSqlColumn const* constraintIdCol = nullptr;
    std::vector<ECDbSqlColumn const*> columns;
    GetConstraintMap(NavigationEnd::To).GetECInstanceIdPropMap()->GetColumns(columns, classMap->GetPrimaryTable());
    if (columns.size() == 1)
        constraintIdCol = columns.front();

    if (constraintIdCol == nullptr && !classMap->IsMappedToSingleTable())
        {
        GetConstraintMap(NavigationEnd::To).GetECInstanceIdPropMap()->GetColumns(columns, classMap->GetJoinedTable());
        if (columns.size() == 1)
            constraintIdCol = columns.front();
        }

    if (constraintIdCol == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }
    else
        m_columns.push_back(constraintIdCol);

    BeAssert(m_mappedTables.empty());
    m_mappedTables.push_back(&constraintIdCol->GetTable());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
bool NavigationPropertyMap::IsSupportedInECSql(bool logIfNotSupported, ECDbCP ecdb) const
    {
    BeAssert(!logIfNotSupported || ecdb != nullptr);

    if (m_navigationProperty->IsMultiple())
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, 
              "NavigationECProperty '%s.%s' cannot be used in ECSQL because its multiplicity is %s. Only the multiplicities %s or %s are supported.",
                                                           m_navigationProperty->GetClass().GetFullName(), m_navigationProperty->GetName().c_str(),
                                                           GetConstraint(*m_navigationProperty, NavigationEnd::To).GetCardinality().ToString().c_str(),
                                                           RelationshipCardinality::ZeroOne().ToString().c_str(),
                                                           RelationshipCardinality::OneOne().ToString().c_str());
        return false;
        }
    if (m_relClassMap == nullptr)
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a virtual table.",
                m_navigationProperty->GetClass().GetFullName(), m_navigationProperty->GetName().c_str());

        return false;
        }

    if (m_relClassMap->GetClassMapType() != IClassMap::Type::RelationshipEndTable)
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a link table.",
                                                           m_navigationProperty->GetClass().GetFullName(), m_navigationProperty->GetName().c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyMap::_GetColumns(std::vector<ECDbSqlColumn const*>& columns) const
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


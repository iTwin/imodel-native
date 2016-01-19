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
PropertyMapPtr PropertyMap::Clone(ECDbMapCR ecdbMap, PropertyMapCR proto, PropertyMap const* parentPropertyMap ) 
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
        return new PropertyMapStruct(ecdbMap, *protoMap, parentPropertyMap);
        }
    else if (auto protoMap = proto.GetAsNavigationPropertyMap())
        {
        return new NavigationPropertyMap(ecdbMap.GetSchemaImportContext()->GetClassMapLoadContext(), *protoMap, parentPropertyMap);
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
// @bsimethod                                                Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
bool PropertyMap::IsUnmapped () const
    {
    return _IsUnmapped ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
ECDbSqlTable const* PropertyMap::GetTable() const
    {
    ECDbSqlColumn const* firstCol = GetFirstColumn();
    if (firstCol != nullptr)
        return &firstCol->GetTable();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     01/2016
//---------------------------------------------------------------------------------------
bool PropertyMap::MapsToTable(ECDbSqlTable const& table) const
    {
    ECDbSqlTable const* mappedTable = GetTable();
    return mappedTable != nullptr && mappedTable == &table;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPtr PropertyMap::CreateAndEvaluateMapping(ClassMapLoadContext& ctx, ECDbCR ecdb, ECPropertyCR ecProperty, ECClassCR rootClass, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    if (!ecProperty.HasId())
        ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema(ecdb, ecProperty);

    // WIP_ECDB: honor the hint for non-default mappings
    ColumnInfo columnInfo = ColumnInfo::Create(ecProperty, propertyAccessString);
    if (!columnInfo.IsValid())
        return nullptr;

    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty != nullptr)
        {
        switch (primitiveProperty->GetType())
            {
                case PRIMITIVETYPE_Point2D:
                case PRIMITIVETYPE_Point3D:
                    return new PropertyMapPoint(ecProperty, propertyAccessString, columnInfo, parentPropertyMap);

                default:
                    return new PropertyMapSingleColumn(ecProperty, propertyAccessString, columnInfo, parentPropertyMap);
            }
        }

    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();

    // PropertyMapRule: primitives, primitive arrays , and structs map to 1 or more columns in the ECClass's main table
    if (arrayProperty != nullptr)
        {
        if (ARRAYKIND_Primitive == arrayProperty->GetKind())
            return new PropertyMapPrimitiveArray(ecdb, ecProperty, propertyAccessString, columnInfo, parentPropertyMap);
        else
            {
            BeAssert(ARRAYKIND_Primitive != arrayProperty->GetKind());
            return PropertyMapStructArray::Create(ecProperty, propertyAccessString, parentPropertyMap);
            }
        }

    if (ecProperty.GetIsStruct())
        return PropertyMapStruct::Create(ctx, ecdb, ecProperty, propertyAccessString, parentPropertyMap); // The individual properties get their own binding, but we need a placeholder for the overall struct

    BeAssert(ecProperty.GetIsNavigation());
    return NavigationPropertyMap::Create(ctx, ecdb, ecProperty, propertyAccessString, parentPropertyMap);
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

    if (columns.size() > 1)
        {
        BeAssert(false && "Overide this funtion for multicolumn mapping");
        return ERROR;
        }

    ECDbPropertyMapInfo* mapInfo = classMapInfo.CreatePropertyMap(GetRoot().GetProperty().GetId(), GetPropertyAccessString(), *columns.at(0));
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
NativeSqlBuilder::List PropertyMap::ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const
    {
    return _ToNativeSql(classIdentifier, ecsqlType, wrapInParentheses);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::_ToNativeSql(Utf8CP classIdentifier, ECSqlType ecsqlType, bool wrapInParentheses) const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns (columns);

    NativeSqlBuilder::List nativeSqlSnippets;
    for (auto column : columns)
        {
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
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* PropertyMap::GetFirstColumn() const
    {
    std::vector<ECDbSqlColumn const*> columns;
    GetColumns(columns);
    if (columns.empty())
        return nullptr;

    return columns.at(0);
    }


//************************** PropertyMapCollection ***************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection::PropertyMapCollection () 
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection::PropertyMapCollection (PropertyMapCollection&& rhs) 
    : m_dictionary (std::move (rhs.m_dictionary)), m_orderedCollection (std::move (rhs.m_orderedCollection))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection& PropertyMapCollection::operator= (PropertyMapCollection&& rhs)
    {
    if (this != &rhs)
        {
        m_dictionary = std::move (rhs.m_dictionary);
        m_orderedCollection = std::move (rhs.m_orderedCollection);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyMapCollection::AddPropertyMap (PropertyMapPtr const& propertyMap)
    {
    AddPropertyMap (propertyMap->GetPropertyAccessString (), propertyMap);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyMapCollection::AddPropertyMap (Utf8CP propertyAccessString, PropertyMapPtr const& propertyMap)
    {
    m_dictionary[propertyAccessString] = propertyMap;
    m_orderedCollection.push_back (propertyMap.get ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::IsEmpty () const
    {
    return Size () == 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
size_t PropertyMapCollection::Size () const
    {
    return m_orderedCollection.size ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapCP& propertyMap, Utf8CP propertyAccessString, bool recursive) const
    {
    propertyMap = nullptr;

    PropertyMapPtr propertyMapPtr = nullptr;
    const auto found = TryGetPropertyMap (propertyMapPtr, propertyAccessString, recursive);
    if (found)
        propertyMap = propertyMapPtr.get ();

    return found;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapPtr& propertyMap, Utf8CP propertyAccessString, bool recursive) const
    {
    propertyMap = nullptr;
    bool found = TryGetPropertyMapNonRecursively (propertyMap, propertyAccessString);
    if (found || !recursive)
        return found;

    BeAssert (recursive && !found);

    //recurse into access string and look up prop map for first member in access string
    bvector<Utf8String> tokens;
    ECDbMap::ParsePropertyAccessString(tokens, propertyAccessString);

    bvector<Utf8String>::const_iterator tokenIt = tokens.begin ();
    bvector<Utf8String>::const_iterator tokenEndIt = tokens.end ();
    return TryGetPropertyMap (propertyMap, tokenIt, tokenEndIt);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapPtr& propertyMap, bvector<Utf8String>::const_iterator& accessStringTokenIterator, bvector<Utf8String>::const_iterator& accessStringTokenEndIterator) const
    {
    if (accessStringTokenIterator == accessStringTokenEndIterator)
        return false;

    Utf8StringCR currentAccessItem = *accessStringTokenIterator;

    PropertyMapPtr currentAccessPropMap = nullptr;
    bool found = TryGetPropertyMapNonRecursively (currentAccessPropMap, currentAccessItem.c_str ());
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
    return currentAccessPropMap->GetChildren ().TryGetPropertyMap (propertyMap, accessStringTokenIterator, accessStringTokenEndIterator);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMapNonRecursively (PropertyMapPtr& propertyMap, Utf8CP propertyAccessString) const
    {
    propertyMap = nullptr;
    auto it = m_dictionary.find (propertyAccessString);
    if (it != m_dictionary.end ())
        {
        BeAssert (it->second.IsValid () && it->second.get () != nullptr);
        propertyMap = it->second;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   12/2013
//---------------------------------------------------------------------------------------
void PropertyMapCollection::Traverse (std::function<void (TraversalFeedback& cancel, PropertyMapCP propMap)> const& nodeOperation, bool recursive) const
    {
    std::set<PropertyMapCollection const*> doneList;
    Traverse (doneList, *this, nodeOperation, recursive);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   12/2013
//---------------------------------------------------------------------------------------
//static 
void PropertyMapCollection::Traverse (std::set<PropertyMapCollection const*>& doneList, PropertyMapCollection const& childPropMaps, std::function<void (TraversalFeedback& feedback, PropertyMapCP propMap)> const& nodeOperation, bool recursive)
    {
    if (doneList.find (&childPropMaps) != doneList.end ())
        return;

    doneList.insert (&childPropMaps);
    for (auto propMap : childPropMaps)
        {
        auto feedback = TraversalFeedback::Next;
        nodeOperation (feedback, propMap);
        if (feedback == TraversalFeedback::Cancel)
            return;

        if (feedback == TraversalFeedback::NextSibling)
            continue;

        Traverse (doneList, propMap->GetChildren (), nodeOperation, recursive);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection::const_iterator PropertyMapCollection::begin () const
    {
    return m_orderedCollection.begin();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCollection::const_iterator PropertyMapCollection::end () const
    {
    return m_orderedCollection.end ();
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
PropertyMapSingleColumn::PropertyMapSingleColumn (ECPropertyCR ecProperty, Utf8CP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_columnInfo (columnInfo), m_primitiveProperty (ecProperty.GetAsPrimitiveProperty ()), m_column (nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2013
//---------------------------------------------------------------------------------------
bool PropertyMapSingleColumn::_IsVirtual () const
    {
    return m_column != nullptr && m_column->GetPersistenceType() == PersistenceType::Virtual;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapSingleColumn::_FindOrCreateColumnsInTable (ClassMap& classMap , ClassMapInfo const* classMapInfor)
    {
    Utf8CP        columnName = m_columnInfo.GetName ();
    PrimitiveType primitiveType = m_columnInfo.GetColumnType ();
    bool          nullable = m_columnInfo.IsNullable ();
    bool          unique = m_columnInfo.IsUnique ();
    ECDbSqlColumn::Constraint::Collation collation = m_columnInfo.GetCollation ();
    m_column = classMap.FindOrCreateColumnForProperty(classMap, classMapInfor, *this, columnName, primitiveType, nullable, unique, collation, nullptr);
    BeAssert (m_column != nullptr && "This actually indicates a mapping error. The method PropertyMapSingleColumn::_FindOrCreateColumnsInTable should therefore be changed to return an error.");
    return SUCCESS;
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
        GetProperty().GetName().c_str(), m_columnInfo.GetName());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPoint::PropertyMapPoint (ECPropertyCR ecProperty, Utf8CP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_columnInfo (columnInfo), m_xColumn (nullptr), m_yColumn (nullptr), m_zColumn (nullptr)
    {
    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (!EXPECTED_CONDITION(primitiveProperty))
        return;

    m_columnInfo.SetColumnType(PRIMITIVETYPE_Double);

    switch (primitiveProperty->GetType())
        {
        case PRIMITIVETYPE_Point3D: m_is3d = true; return;
        case PRIMITIVETYPE_Point2D: m_is3d = false; return;
        }

    m_is3d = false;
    BeAssert (false && "Constructed a PropertyMapPoint for a property that is not a Point2d or Point3d");
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
    auto pm = classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".X").c_str (), *m_xColumn);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return ERROR;
        }

    classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".Y").c_str (), *m_yColumn);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return ERROR;
        }

    if (m_is3d)
        {
        BeAssert (m_zColumn != nullptr);
        classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".Z").c_str (), *m_zColumn);
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

    auto rootPropertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = Utf8String (GetPropertyAccessString ());
    auto pm = classMapInfo.FindPropertyMap (rootPropertyId, (accessString + ".X").c_str ());
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to load propertymap");
        return ERROR;
        }

    m_xColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));
    pm = classMapInfo.FindPropertyMap (rootPropertyId, (accessString + ".Y").c_str ());
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to load propertymap");
        return ERROR;
        }

    m_yColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));

    if (m_is3d)
        {
        pm = classMapInfo.FindPropertyMap (rootPropertyId, (accessString + ".Z").c_str ());
        if (pm == nullptr)
            {
            BeAssert (false && "Failed to load propertymap");
            return ERROR;
            }

        m_zColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));
        }

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMapPoint::_ToString() const
    {
    if (m_is3d)
        return Utf8PrintfString("PropertyMapPoint (3d): ecProperty=%s.%s, columnName=%s_X, _Y, _Z", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), m_columnInfo.GetName());
    else
        return Utf8PrintfString("PropertyMapPoint (2d): ecProperty=%s.%s, columnName=%s_X, _Y", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), m_columnInfo.GetName());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PropertyMapPoint::_FindOrCreateColumnsInTable(ClassMap& classMap,  ClassMapInfo const* classMapInfo)
    {
    PrimitiveType primitiveType = PRIMITIVETYPE_Double;

    Utf8CP        columnName    = m_columnInfo.GetName();
    bool          nullable      = m_columnInfo.IsNullable();
    bool          unique        = m_columnInfo.IsUnique();
    ECDbSqlColumn::Constraint::Collation collation = m_columnInfo.GetCollation();

    Utf8String xColumnName(columnName);
    xColumnName.append("_X");
    m_xColumn = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, xColumnName.c_str(), primitiveType, nullable, unique, collation, "X");

    Utf8String yColumnName(columnName);
    yColumnName.append("_Y");
    m_yColumn = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, yColumnName.c_str(), primitiveType, nullable, unique, collation, "Y");
    if (!m_is3d)
        return SUCCESS;

    Utf8String zColumnName(columnName);
    zColumnName.append("_Z");
    m_zColumn = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, zColumnName.c_str(), primitiveType, nullable, unique, collation, "Z");
    return SUCCESS;
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
PropertyMapPrimitiveArray::PropertyMapPrimitiveArray (ECDbCR ecdb, ECPropertyCR ecProperty, Utf8CP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
    : PropertyMapSingleColumn (ecProperty, propertyAccessString, columnInfo, parentPropertyMap)
    {
    BeAssert (columnInfo.GetColumnType() == PRIMITIVETYPE_Binary); 
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert(arrayProperty);

    ECClassCP primitiveArrayPersistenceClass = ECDbSystemSchemaHelper::GetClassForPrimitiveArrayPersistence (ecdb, arrayProperty->GetPrimitiveElementType());
    BeAssert(primitiveArrayPersistenceClass != nullptr);
    m_primitiveArrayEnabler = primitiveArrayPersistenceClass->GetDefaultStandaloneEnabler();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PropertyMapPrimitiveArray::_ToString() const
    {
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert (arrayProperty);
    
    return Utf8PrintfString("PropertyMapPrimitiveArray: ecProperty=%s.%s, type=%s, columnName=%s", GetProperty().GetClass().GetFullName(), 
                            GetProperty().GetName().c_str(), ExpHelper::ToString(arrayProperty->GetPrimitiveElementType()), m_columnInfo.GetName());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
PropertyMapPtr NavigationPropertyMap::Create(ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    NavigationECPropertyCP navProp = prop.GetAsNavigationPropertyCP();
    if (navProp == nullptr)
        {
        BeAssert(false);
        return nullptr;
        }

    if (navProp->IsMultiple())
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' has a multiplicity of '%s'. ECDb only supports NavigationECProperties with a maximum multiplicity of 1.",
                                                      navProp->GetClass().GetFullName(), navProp->GetName().c_str(),
                                                      GetConstraint(*navProp).GetCardinality().ToString().c_str());
        return nullptr;
        }

    return new NavigationPropertyMap(ctx, prop, propertyAccessString, parentPropertyMap);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      12/2015
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, ECN::ECPropertyCR prop, Utf8CP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap(prop, propertyAccessString, parentPropertyMap), m_navigationProperty(prop.GetAsNavigationPropertyCP()), m_relClassMap(nullptr)
    {
    BeAssert(prop.GetIsNavigation());

    //we need to wait with finishing the nav prop map set up to the end when all relationships have been imported and mapped
    ctx.AddNavigationPropertyMap(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
NavigationPropertyMap::NavigationPropertyMap(ClassMapLoadContext& ctx, NavigationPropertyMap const& proto, PropertyMap const* parentPropertyMap) :PropertyMap(proto.GetProperty(), proto.GetPropertyAccessString(), parentPropertyMap), m_navigationProperty(proto.m_navigationProperty), m_relClassMap(proto.m_relClassMap)
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
    ClassMap const* relClassMap = ecdbMap.GetClassMap(*m_navigationProperty->GetRelationshipClass());
    if (relClassMap == nullptr || !relClassMap->IsRelationshipClassMap())
        {
        BeAssert(false && "RelationshipClassMap should not be nullptr when finishing the NavigationPropMap");
        return ERROR;
        }

    m_relClassMap = static_cast<RelationshipClassMap const*> (relClassMap);
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
                                                           GetConstraint().GetCardinality().ToString().c_str(),
                                                           RelationshipCardinality::ZeroOne().ToString().c_str(),
                                                           RelationshipCardinality::OneOne().ToString().c_str());
        return false;
        }

    if (m_relClassMap->GetClassMapType() != IClassMap::Type::RelationshipEndTable)
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because its ECRelationships is mapped to a link table.",
                                                           m_navigationProperty->GetClass().GetFullName(), m_navigationProperty->GetName().c_str());
        return false;
        }


    PropertyMapRelationshipConstraintClassId const* classIdPropMap = GetConstraintMap().GetECClassIdPropMap();
    if (!classIdPropMap->IsVirtual() && classIdPropMap->IsMappedToClassMapTables())
        {
        if (logIfNotSupported)
            ecdb->GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "NavigationECProperty '%s.%s' cannot be used in ECSQL because the mapping requires an ECClassId column for the related instances.",
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

    RelationshipConstraintMap const& constraintMap = GetConstraintMap();
    constraintMap.GetECInstanceIdPropMap()->GetColumns(columns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
RelationshipConstraintMap const& NavigationPropertyMap::GetConstraintMap() const
    {
    return m_relClassMap->GetConstraintMap(GetConstraintEnd());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
ECRelationshipEnd NavigationPropertyMap::GetConstraintEnd() const
    {
    ECRelationshipClassCP relClass = m_navigationProperty->GetRelationshipClass();
    BeAssert(relClass != nullptr);
    return m_navigationProperty->GetDirection() == ECRelatedInstanceDirection::Forward ? ECRelationshipEnd::ECRelationshipEnd_Target : ECRelationshipEnd::ECRelationshipEnd_Source;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECRelationshipConstraintCR NavigationPropertyMap::GetConstraint(NavigationECPropertyCR navProp)
    {
    ECRelationshipClassCP relClass = navProp.GetRelationshipClass();
    BeAssert(relClass != nullptr);
    return navProp.GetDirection() == ECRelatedInstanceDirection::Forward ? relClass->GetTarget() : relClass->GetSource();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE


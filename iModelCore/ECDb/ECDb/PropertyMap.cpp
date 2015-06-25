﻿/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
PropertyMap::PropertyMap (ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap) : m_ecProperty (ecProperty), m_propertyAccessString (propertyAccessString), m_parentPropertyMap (parentPropertyMap), m_propertyPathId (0), m_primaryTable (primaryTable)
    {
    BeAssert (propertyAccessString);
    BeAssert (GetPrimaryTable () != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     12/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::IsVirtual () const
    {
    return _IsVirtual ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     12/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::_IsVirtual () const
    {
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
bool PropertyMap::IsUnmapped () const
    {
    return _IsUnmapped ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
bool PropertyMap::_IsUnmapped () const
    {
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECClassCR rootClass, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    {
    if (!ecProperty.HasId ())
        {
        ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema (ecDbMap.GetECDbR (), ecProperty);
        }

    // WIP_ECDB: honor the hint for non-default mappings
    ColumnInfo columnInfo = ColumnInfo::Create (ecProperty, propertyAccessString);
    if (!columnInfo.IsValid())
        return nullptr;

    auto policy = ECDbPolicyManager::GetPropertyPolicy (ecProperty, IsValidInECSqlPolicyAssertion::Get ());
    if (!policy.IsSupported ())
        {
        LOG.warningv (L"Did not map ECProperty '%ls.%ls'. %ls", rootClass.GetFullName (), propertyAccessString,
            WString (policy.GetNotSupportedMessage (), BentleyCharEncoding::Utf8).c_str ());
        return UnmappedPropertyMap::Create (ecProperty, propertyAccessString, primaryTable, parentPropertyMap);
        }

    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty != nullptr)
        {
        switch (primitiveProperty->GetType())
            {
            case PRIMITIVETYPE_Point2D: 
            case PRIMITIVETYPE_Point3D: 
                return new PropertyMapPoint (ecProperty, propertyAccessString, primaryTable, columnInfo, parentPropertyMap);

            default:                    
                return new PropertyMapToColumn (ecProperty, propertyAccessString, primaryTable, columnInfo, parentPropertyMap);
            }
        }

    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();

    // PropertyMapRule: primitives, primitive arrays , and structs map to 1 or more columns in the ECClass's main table
    if (arrayProperty != nullptr)
        {
        if (ARRAYKIND_Primitive == arrayProperty->GetKind ())
            return new PropertyMapArrayOfPrimitives (ecProperty, propertyAccessString, primaryTable, columnInfo, ecDbMap, parentPropertyMap);
        else
            {
            BeAssert (ARRAYKIND_Primitive != arrayProperty->GetKind ());
            return PropertyMapToTable::Create (ecProperty, ecDbMap, propertyAccessString, primaryTable, parentPropertyMap);
            }
        }

    BeAssert (ecProperty.GetIsStruct());
    // PropertyMapRule: embedded structs should be mapped into columns of the embedding ECClass's table, but are currently being mapped to rows in their own table, like an array of structs with one element
    return PropertyMapToInLineStruct::Create (ecProperty, ecDbMap, propertyAccessString, primaryTable,  parentPropertyMap); // The individual properties get their own binding, but we need a placeholder for the overall struct
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
bool PropertyMap::_IsECInstanceIdPropertyMap () const
    {
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     06/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::IsSystemPropertyMap () const
    {
    return _IsSystemPropertyMap ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     06/2013
//---------------------------------------------------------------------------------------
bool PropertyMap::_IsSystemPropertyMap () const
    {
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMap::_Save (ECDbClassMapInfo & classMapInfo) const
    {
    auto& children = GetChildren ();
    if (children.IsEmpty ())
        {
        std::vector<ECDbSqlColumn const*> columns;
        GetColumns (columns);
        if (columns.size () == 0)
            return BE_SQLITE_DONE;

        if (columns.size () > 1)
            {
            BeAssert (false && "Overide this funtion for multicolumn mapping");
            return BE_SQLITE_ERROR;
            }

        auto mapInfo = classMapInfo.CreatePropertyMap (GetRoot ().GetProperty ().GetId (), Utf8String (GetPropertyAccessString ()).c_str (), *columns.at (0));
        if (mapInfo != nullptr)
            m_propertyPathId = mapInfo->GetPropertyPath ().GetId ();
        else
            return BE_SQLITE_ERROR;

        return BE_SQLITE_DONE;
        }

    for (auto& child : children)
        {
        auto r = child->Save (classMapInfo);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMap::_Load (ECDbClassMapInfo const& classMapInfo)
    {
    auto& children = GetChildren ();
    if (children.IsEmpty ())
        {
        BeAssert (false && "This funtion must be overriden in derived class");
        return BE_SQLITE_ERROR;
        }
   // classMapInfo.GetMapStorage ().FindPropertyPath ("")
    for (auto child : children)
        {
 
        auto r = const_cast<PropertyMap*>(child)->Load (classMapInfo);
        if (r != BE_SQLITE_DONE)
            return r;
        }

    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMap::Save (ECDbClassMapInfo & classMapInfo) const
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
WString PropertyMap::_ToString() const
    {
    return WPrintfString(L"PropertyMap: ecProperty=%ls.%ls (%ls)", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str(), m_ecProperty.GetTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
WCharCP PropertyMap::GetPropertyAccessString() const
    {
    return m_propertyAccessString.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
Utf8CP PropertyMap::_GetColumnBaseName() const
    {
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
Utf8CP PropertyMap::GetColumnBaseName() const
    {
    return _GetColumnBaseName();
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
MapStatus PropertyMap::FindOrCreateColumnsInTable(ClassMap& classMap,ClassMapInfo const* classMapInfo)
    {
    return _FindOrCreateColumnsInTable(classMap, classMapInfo);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMap::ToString() const
    {
    return _ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMap::_FindOrCreateColumnsInTable (ClassMap& classMap , ClassMapInfo const* classMapInfo)
    {
    // Base implementation does nothing, but is implemented so PropertyMap can serve as a placeholder
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    }

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
void PropertyMapCollection::AddPropertyMap (WCharCP propertyAccessString, PropertyMapPtr const& propertyMap)
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
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapCP& propertyMap, WCharCP propertyAccessString, bool recursive) const
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
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapPtr& propertyMap, WCharCP propertyAccessString, bool recursive) const
    {
    propertyMap = nullptr;
    bool found = TryGetPropertyMapNonRecursively (propertyMap, propertyAccessString);
    if (found || !recursive)
        return found;

    BeAssert (recursive && !found);

    //recurse into access string and look up prop map for first member in access string
    bvector<WString> tokens;
    BeStringUtilities::Split(propertyAccessString, L".", NULL, tokens);

    bvector<WString>::const_iterator tokenIt = tokens.begin ();
    bvector<WString>::const_iterator tokenEndIt = tokens.end ();
    return TryGetPropertyMap (propertyMap, tokenIt, tokenEndIt);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyMapCollection::TryGetPropertyMap (PropertyMapPtr& propertyMap, bvector<WString>::const_iterator& accessStringTokenIterator, bvector<WString>::const_iterator& accessStringTokenEndIterator) const
    {
    if (accessStringTokenIterator == accessStringTokenEndIterator)
        return false;

    WStringCR currentAccessItem = *accessStringTokenIterator;

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
bool PropertyMapCollection::TryGetPropertyMapNonRecursively (PropertyMapPtr& propertyMap, WCharCP propertyAccessString) const
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
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
UnmappedPropertyMap::UnmappedPropertyMap (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap)
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapPtr UnmappedPropertyMap::Create (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    {
    return new UnmappedPropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
WString UnmappedPropertyMap::_ToString () const
    {
    WString str;
    str.Sprintf (L"UnmappedPropertyMap: ecProperty=%ls.%ls", GetProperty ().GetClass ().GetFullName (), GetProperty ().GetName ().c_str ());
    return str;
    }



//**************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
PropertyMapToInLineStruct::PropertyMapToInLineStruct (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP PropertyMapToInLineStruct::GetPropertyMap (WCharCP propertyName) const
    {

    for(auto childPropMap : m_children)
        {
        //Following is slow but propertyName is expected to be a accessString relative to this struct
        if (childPropMap->GetProperty().GetName().Equals (propertyName))
            return childPropMap;
        }

    WString accessString = propertyName;
    auto n = accessString.find(L".");
    if (n != WString::npos)
        {
        WString first = accessString.substr(0, n);
        WString rest = accessString.substr(n + 1);
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

            if (auto structPropertyMap = dynamic_cast<PropertyMapToInLineStructCP>(propertyMap))
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
void PropertyMapToInLineStruct::_GetColumns(std::vector<ECDbSqlColumn const*>& columns) const 
    {
    for (auto childPropMap : m_children)
        {
        childPropMap->GetColumns (columns);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapToInLineStruct::Initialize(ECDbMapCR map)
    {
    PRECONDITION(GetProperty().GetIsStruct() && "Expecting a struct type property", BentleyStatus::ERROR );
    auto const& rootClass = GetProperty ().GetClass ();
    auto structProperty = GetProperty().GetAsStructProperty();
    for(auto property : structProperty->GetType().GetProperties(true))
        {       
        //forClassMap.GetClass().GetDefaultStandaloneEnabler()->GetPropertyIndex(
        WString accessString = GetPropertyAccessString();
        accessString.append(L".");
        accessString.append(property->GetName());
        PropertyMapPtr propertyMap = PropertyMap::CreateAndEvaluateMapping (*property, map, rootClass, accessString.c_str (), GetPrimaryTable(), this);
        if (propertyMap.IsValid())
            //don't use full prop access string as key in child collection, but just the relative prop access string which is 
            //just the prop name
            m_children.AddPropertyMap(property->GetName ().c_str (), propertyMap);
        }

    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
MapStatus PropertyMapToInLineStruct::_FindOrCreateColumnsInTable (ClassMap& classMap,ClassMapInfo const* classMapInfo)
    {
    for(auto childPropMap : m_children)
        {
        auto status = const_cast<PropertyMapP> (childPropMap)->FindOrCreateColumnsInTable(classMap, classMapInfo);
        if (status != MapStatus::Success)
            return status;
        }

    return MapStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
PropertyMapToInLineStructPtr PropertyMapToInLineStruct::Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    {
    PRECONDITION (prop.GetIsStruct() && "Expecting a ECStruct type property", nullptr);
    auto newPropertyMap = new PropertyMapToInLineStruct (prop, propertyAccessString, primaryTable, parentPropertyMap);
    if (newPropertyMap->Initialize(ecDbMap) == BentleyStatus::SUCCESS)
        return newPropertyMap;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
WString PropertyMapToInLineStruct::_ToString() const 
    {
    WString str;
    str.Sprintf (L"PropertyMapToInlineStruct: ecProperty=%ls.%ls\n", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str());
    str.append (L" Child property maps:\n");
    for (auto childPropMap : m_children)
        {
        str.append (L"\t").append (childPropMap->ToString ().c_str ()).append (L"\n");
        }

    return str;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapToTable::PropertyMapToTable (ECPropertyCR ecProperty, ECClassCR structElementType, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap), m_structElementType (structElementType) 
    { 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMapToTable::_Save (ECDbClassMapInfo & classMapInfo) const
    {
    auto& manager = classMapInfo.GetMapStorageR ();
    auto propertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = Utf8String (GetPropertyAccessString ());
    ECDbPropertyPath const* propertyPath = manager.FindPropertyPath (propertyId, accessString.c_str ());
    if (propertyPath == nullptr)
        propertyPath = manager.CreatePropertyPath (propertyId, accessString.c_str ());

    if (propertyPath == nullptr)
        {
        BeAssert (false && "Failed to create propertyPath");
        return BE_SQLITE_ERROR;
        }

    m_propertyPathId = propertyPath->GetId ();
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMapToTable::_Load (ECDbClassMapInfo const& classMapInfo) 
    {
    auto& manager = classMapInfo.GetMapStorage ();
    auto propertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = Utf8String (GetPropertyAccessString ());
    ECDbPropertyPath const* propertyPath = manager.FindPropertyPath (propertyId, accessString.c_str ());
    if (propertyPath == nullptr)
        {
        BeAssert (false && "Failed to find propertyPath");
        return BE_SQLITE_ERROR;
        }

    m_propertyPathId = propertyPath->GetId ();
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapToTablePtr PropertyMapToTable::Create (ECPropertyCR ecProperty, ECDbMapCR ecDbMap, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, PropertyMapCP parentPropertyMap)
    {
    ECClassCP tableECType = nullptr;
    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
    if (arrayProperty)
        {
        ArrayKind kind =  arrayProperty->GetKind();
        if (kind == ARRAYKIND_Primitive)
            {
            BeAssert(false && "not yet supported");
            tableECType = &(ecDbMap.GetClassForPrimitiveArrayPersistence (arrayProperty->GetPrimitiveElementType()));
            }
        else if (kind == ARRAYKIND_Struct)
            {
            tableECType = arrayProperty->GetStructElementType();
            // Build a ClassMap for the struct
            //ClassMapPtr structMap = new ClassMap(*tableECType, ecDbMap);
            }
        }
    else
        BeAssert (false && "Expecting an array when using PropertyMapToTable");

    return new PropertyMapToTable (ecProperty, *tableECType, propertyAccessString, primaryTable, parentPropertyMap);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyMapToTable::_GetColumns(std::vector<ECDbSqlColumn const*>& columns) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMapToTable::_FindOrCreateColumnsInTable( ClassMap& classMap, ClassMapInfo const* classMapInfo)
    {
    return MapStatus::Success; // PropertyMapToTable adds no columns in the main table. The other table has a FK that points to the main table
    }
        
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
WString PropertyMapToTable::_ToString() const 
    {
    WString str;
    str.Sprintf (L"PropertyMapToTable: ecProperty=%ls.%ls\r\n", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str());

    return str;
    }

//******************************** PropertyMapToColumn *****************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PropertyMapToColumn::PropertyMapToColumn (ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap), m_columnInfo (columnInfo), m_primitiveProperty (ecProperty.GetAsPrimitiveProperty ()), m_column (nullptr)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2013
//---------------------------------------------------------------------------------------
bool PropertyMapToColumn::_IsVirtual () const
    {
    return m_column != nullptr && m_column->GetPersistenceType() == PersistenceType::Virtual;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMapToColumn::_FindOrCreateColumnsInTable (ClassMap& classMap , ClassMapInfo const* classMapInfor)
    {
    Utf8CP        columnName = m_columnInfo.GetName ();
    PrimitiveType primitiveType = m_columnInfo.GetColumnType ();
    bool          nullable = m_columnInfo.IsNullable ();
    bool          unique = m_columnInfo.IsUnique ();
    ECDbSqlColumn::Constraint::Collation collation = m_columnInfo.GetCollation ();
    m_column = classMap.FindOrCreateColumnForProperty(classMap, classMapInfor, *this, columnName, primitiveType, nullable, unique, collation, nullptr);
    BeAssert (m_column != nullptr && "This actually indicates a mapping error. The method PropertyMapToColumn::_FindOrCreateColumnsInTable should therefore be changed to return an error.");
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMapToColumn::_GetColumns (std::vector<ECDbSqlColumn const*>& columns) const
    {
    columns.push_back(m_column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
Utf8CP PropertyMapToColumn::_GetColumnBaseName() const 
    {
    Utf8String propertyName (m_ecProperty.GetName());
    if (0 == strcmp(m_columnInfo.GetName(), propertyName.c_str()))
        return nullptr;
    else
        return m_columnInfo.GetName();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMapToColumn::_ToString() const
    {
    return WPrintfString(L"PropertyMapToColumn: ecProperty=%ls.%ls, columnName=%ls", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str(), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPoint::PropertyMapPoint (ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, primaryTable, parentPropertyMap), m_columnInfo (columnInfo), m_xColumn (nullptr), m_yColumn (nullptr), m_zColumn (nullptr)
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
DbResult PropertyMapPoint::_Save (ECDbClassMapInfo & classMapInfo) const
    {
    BeAssert (m_xColumn != nullptr);
    BeAssert (m_yColumn != nullptr);

    auto rootPropertyId = GetRoot ().GetProperty ().GetId ();
    auto accessString = Utf8String (GetPropertyAccessString ());
    auto pm = classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".X").c_str (), *m_xColumn);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return BE_SQLITE_ERROR;
        }

    classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".Y").c_str (), *m_yColumn);
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to create propertymap");
        return BE_SQLITE_ERROR;
        }

    if (m_is3d)
        {
        BeAssert (m_zColumn != nullptr);
        classMapInfo.CreatePropertyMap (rootPropertyId, (accessString + ".Z").c_str (), *m_zColumn);
        if (pm == nullptr)
            {
            BeAssert (false && "Failed to create propertymap");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_DONE;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMapPoint::_Load (ECDbClassMapInfo const& classMapInfo)
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
        return BE_SQLITE_ERROR;
        }

    m_xColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));
    pm = classMapInfo.FindPropertyMap (rootPropertyId, (accessString + ".Y").c_str ());
    if (pm == nullptr)
        {
        BeAssert (false && "Failed to load propertymap");
        return BE_SQLITE_ERROR;
        }

    m_yColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));

    if (m_is3d)
        {
        pm = classMapInfo.FindPropertyMap (rootPropertyId, (accessString + ".Z").c_str ());
        if (pm == nullptr)
            {
            BeAssert (false && "Failed to load propertymap");
            return BE_SQLITE_ERROR;
            }

        m_zColumn = const_cast<ECDbSqlColumn*>(&(pm->GetColumn ()));
        }

    return BE_SQLITE_DONE;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMapPoint::_ToString() const
    {
    if (m_is3d)
        return WPrintfString(L"PropertyMapPoint (3d): ecProperty=%ls.%ls, columnName=%ls_X, _Y, _Z", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    else
        return WPrintfString(L"PropertyMapPoint (2d): ecProperty=%ls.%ls, columnName=%ls_X, _Y", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus PropertyMapPoint::_FindOrCreateColumnsInTable(ClassMap& classMap,  ClassMapInfo const* classMapInfo)
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
        return MapStatus::Success;

    Utf8String zColumnName(columnName);
    zColumnName.append("_Z");
    m_zColumn = classMap.FindOrCreateColumnForProperty(classMap, classMapInfo, *this, zColumnName.c_str(), primitiveType, nullable, unique, collation, "Z");
    return MapStatus::Success;
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
Utf8CP PropertyMapPoint::_GetColumnBaseName() const 
    {
    Utf8String propertyName (m_ecProperty.GetName());
    if (0 == strcmp(m_columnInfo.GetName(), propertyName.c_str()))
        return nullptr;
    else
        return m_columnInfo.GetName();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PropertyMapArrayOfPrimitives::PropertyMapArrayOfPrimitives (ECPropertyCR ecProperty, WCharCP propertyAccessString, ECDbSqlTable const* primaryTable, ColumnInfoCR columnInfo, ECDbMapCR ecDbMap, PropertyMapCP parentPropertyMap)
    : PropertyMapToColumn (ecProperty, propertyAccessString, primaryTable, columnInfo, parentPropertyMap)
    {
    BeAssert (columnInfo.GetColumnType() == PRIMITIVETYPE_Binary); 
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert(arrayProperty);

    ECClassCR primitiveArrayPersistenceClass = ecDbMap.GetClassForPrimitiveArrayPersistence (arrayProperty->GetPrimitiveElementType());
    m_primitiveArrayEnabler = primitiveArrayPersistenceClass.GetDefaultStandaloneEnabler();
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMapArrayOfPrimitives::_ToString() const
    {
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert (arrayProperty);
    PrimitiveType primitiveType = arrayProperty->GetPrimitiveElementType();
    return WPrintfString(L"PropertyMapArrayOfPrimitives: ecProperty=%ls.%ls, type=%ls, columnName=%ls", GetProperty().GetClass().GetFullName(), 
        GetProperty().GetName().c_str(), ECDbMap::GetPrimitiveTypeName(primitiveType), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    }

        
END_BENTLEY_SQLITE_EC_NAMESPACE


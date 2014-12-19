/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
PropertyMap::PropertyMap (ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap) : m_ecProperty (ecProperty), m_propertyAccessString (propertyAccessString), m_parentPropertyMap (parentPropertyMap)
    {
    BeAssert (propertyAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      08/2014
+---------------+---------------+---------------+---------------+---------------+------*/

ECN::ECPropertyId PropertyMap::_GetECPropertyIdForPersistence (ECClassId relativeToECClassId, ECDbR db) const
    {
    return GetProperty ().GetId ();
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
PropertyMapPtr PropertyMap::CreateAndEvaluateMapping (ECPropertyCR ecProperty, ECDbMapCR ecDbMap, ECClassCR rootClass, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    IECInstancePtr hint = ecProperty.GetCustomAttributeLocal (BSCAC_ECDbPropertyHint);
    // WIP_ECDB: honor the hint for non-default mappings

    ColumnInfo columnInfo (ecProperty, propertyAccessString, hint.get());
    DbResult r = ColumnInfo::LoadMappingInformationFromDb (columnInfo, ecProperty, ecDbMap.GetECDbR()); 
    if (r != BE_SQLITE_ROW && r != BE_SQLITE_DONE)
        {
        LOG.errorv(L"Failed to load ECDbMap information for ECProperty '%ls.%ls' from database'", rootClass.GetFullName (), propertyAccessString);
        return nullptr;
        }

    auto policy = ECDbPolicyManager::GetPropertyPolicy (ecProperty, IsValidInECSqlPolicyAssertion::Get ());
    if (!policy.IsSupported ())
        {
        LOG.warningv (L"Did not map ECProperty '%ls.%ls'. %ls", rootClass.GetFullName (), propertyAccessString,
            WString (policy.GetNotSupportedMessage (), BentleyCharEncoding::Utf8).c_str ());
        return UnmappedPropertyMap::Create (ecProperty, propertyAccessString, parentPropertyMap);
        }

    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty != nullptr)
        {
        switch (primitiveProperty->GetType())
            {
            case PRIMITIVETYPE_Point2D: 
            case PRIMITIVETYPE_Point3D: 
                return new PropertyMapPoint (ecProperty, propertyAccessString, columnInfo, parentPropertyMap);

            default:                    
                return new PropertyMapToColumn(ecProperty, propertyAccessString, columnInfo, parentPropertyMap);
            }
        }

    ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();

    // PropertyMapRule: primitives, primitive arrays , and structs map to 1 or more columns in the ECClass's main table
    if (arrayProperty != nullptr)
        {
        if (ARRAYKIND_Primitive == arrayProperty->GetKind ())
            return new PropertyMapArrayOfPrimitives (ecProperty, propertyAccessString, columnInfo, ecDbMap, parentPropertyMap);
        else
            {
            BeAssert (ARRAYKIND_Primitive != arrayProperty->GetKind ());
            if (hint.IsValid ())
                {
                ECValue strategy;
                if (ECOBJECTS_STATUS_Success == hint->GetValue (strategy, BSCAP_MapStrategy) && !strategy.IsNull () &&
                    0 == BeStringUtilities::Wcsicmp (strategy.GetString (), BSCAP_Blob))
                    LOG.warningv (L"MapStrategy '%ls' is not supported. %ls will be mapped to its own table", strategy.GetString (), ecProperty.GetName ().c_str ());
                }
            return PropertyMapToTable::Create (ecProperty, ecDbMap, propertyAccessString, parentPropertyMap);
            }
        }

    BeAssert (ecProperty.GetIsStruct());
    // PropertyMapRule: embedded structs should be mapped into columns of the embedding ECClass's table, but are currently being mapped to rows in their own table, like an array of structs with one element
    return PropertyMapToInLineStruct::Create(ecProperty, ecDbMap, propertyAccessString, parentPropertyMap); // The individual properties get their own binding, but we need a placeholder for the overall struct
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
* @bsimethod                                                    affan.khan      08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMap::Save (ECDbR ecdb) const
    {
    Utf8CP columnName = GetColumnBaseName();
    if (!columnName)
        return BE_SQLITE_DONE; // WIP_ECDB: What if it used to be different and is now changing back to default?

    ECPropertyId  propertyId = GetProperty().GetId();
    return ECDbSchemaPersistence::SetECPropertyMapColumnName (propertyId, columnName, ecdb);
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
void PropertyMap::_AddBindings( BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECEnablerCR enabler ) const 
    {
    UInt32 propertyIndex;
    ECObjectsStatus status = enabler.GetPropertyIndex(propertyIndex, GetPropertyAccessString());
    if (ECOBJECTS_STATUS_Success != status)
        { BeAssert(false); }

    Binding binding(enabler, *this, propertyIndex, 0, BINDING_NotBound, nullptr);
    bindings.push_back(binding);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyMap::_MapsToColumn (Utf8CP columnName) const
    {
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMap::_ToString() const
    {
    return WPrintfString(L"PropertyMap: ecProperty=%ls.%ls (%ls)", m_ecProperty.GetClass().GetFullName(), m_ecProperty.GetName().c_str(), m_ecProperty.GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyMap::AddBindings( BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECEnablerCR enabler ) const
    {
    _AddBindings(bindings, propertyIndexUnused, sqlIndex, enabler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
WCharCP PropertyMap::GetPropertyAccessString() const
    {
    return m_propertyAccessString.c_str();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyMap::MapsToColumn (Utf8CP columnName) const
    {
    return _MapsToColumn (columnName);
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
// @bsimethod                                                    casey.mullen      12/2012
//---------------------------------------------------------------------------------------
void PropertyMap::SetColumnBaseName (Utf8CP columnName)
    {
    _SetColumnBaseName(columnName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const
    {
    return _ToNativeSql (classIdentifier, ecsqlType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::ToNativeSql (ECDbR ecdb, DbTableCR table) const
    {
    return _ToNativeSql (ecdb, table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::_ToNativeSql (Utf8CP classIdentifier, ECSqlType ecsqlType) const
    {
    DbColumnList columns;
    GetColumns (columns);

    NativeSqlBuilder::List nativeSqlSnippets;
    for (auto column : columns)
        {
        BeAssert (!Utf8String::IsNullOrEmpty (column->GetName ()));

        NativeSqlBuilder sqlSnippet;
        sqlSnippet.Append (classIdentifier, column->GetName ());
        nativeSqlSnippets.push_back (std::move (sqlSnippet));
        }

    return std::move (nativeSqlSnippets);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMap::_ToNativeSql (ECDbR ecdb, DbTableCR table) const
    {
    //default impl is same as standard overload
    return _ToNativeSql (nullptr, ECSqlType::Select);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus PropertyMap::FindOrCreateColumnsInTable (ClassMapR classMap)
    {
    return _FindOrCreateColumnsInTable (classMap);
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
DbResult PropertyMap::_Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, IECInstanceR ecInstance) const
    {
    iBinding++; // WIP_ECD remove this default implementation when we've cleaned out all of the dead PropertyMaps
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult PropertyMap::Bind( int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, IECInstanceR ecInstance ) const 
    {
    auto stat = _Bind (iBinding, parameterBindings, statement, ecInstance);
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMap::_FindOrCreateColumnsInTable (ClassMapR classMap)
    {
    // Base implementation does nothing, but is implemented so PropertyMap can serve as a placeholder
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::_GetColumns (DbColumnList& columns) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMap::GetColumns (DbColumnList& columns) const
    {
    _GetColumns (columns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      08/2013
//---------------------------------------------------------------------------------------
DbColumnCP PropertyMap::GetFirstColumn() const
    {
    DbColumnList columns;
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
UnmappedPropertyMap::UnmappedPropertyMap (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, parentPropertyMap)
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    03/2014
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapPtr UnmappedPropertyMap::Create (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    return new UnmappedPropertyMap (ecProperty, propertyAccessString, parentPropertyMap);
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
PropertyMapToInLineStruct::PropertyMapToInLineStruct (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, parentPropertyMap)
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
void PropertyMapToInLineStruct::_GetColumns(DbColumnList& columns) const 
    {
    for (auto childPropMap : m_children)
        {
        childPropMap->GetColumns (columns);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapToInLineStruct::_ToNativeSql (ECDbR ecdb, DbTableCR table) const
    {
    NativeSqlBuilder::List nativeSqlList;
    for (auto childPropMap : m_children)
        {
        NativeSqlBuilder::List childList = childPropMap->ToNativeSql (ecdb, table);
        nativeSqlList.insert (nativeSqlList.end (), childList.begin (), childList.end ());
        }

    return std::move (nativeSqlList);
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
        PropertyMapPtr propertyMap = PropertyMap::CreateAndEvaluateMapping (*property, map, rootClass, accessString.c_str (), this);
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
MapStatus PropertyMapToInLineStruct::_FindOrCreateColumnsInTable (ClassMapR classMap)
    {
    for(auto childPropMap : m_children)
        {
        auto status = const_cast<PropertyMapP> (childPropMap)->FindOrCreateColumnsInTable(classMap);
        if (status != MapStatus::Success)
            return status;
        }

    return MapStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan     09/2013
//---------------------------------------------------------------------------------------
PropertyMapToInLineStructPtr PropertyMapToInLineStruct::Create (ECN::ECPropertyCR prop, ECDbMapCR ecDbMap, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
    {
    PRECONDITION (prop.GetIsStruct() && "Expecting a ECStruct type property", nullptr);
    auto newPropertyMap = new PropertyMapToInLineStruct(prop, propertyAccessString, parentPropertyMap);
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
PropertyMapToTable::PropertyMapToTable (ECPropertyCR ecProperty, ECClassCR structElementType, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_structElementType (structElementType) 
    { 
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapToTablePtr PropertyMapToTable::Create (ECPropertyCR ecProperty, ECDbMapCR ecDbMap, WCharCP propertyAccessString, PropertyMapCP parentPropertyMap)
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

    return new PropertyMapToTable (ecProperty, *tableECType, propertyAccessString, parentPropertyMap);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      08/2014
+---------------+---------------+---------------+---------------+---------------+------*/

ECN::ECPropertyId PropertyMapToTable::_GetECPropertyIdForPersistence (ECClassId relativeToECClassId, ECDbR db) const
    {
    auto itor = m_persistenceECPropertyIdMap.find (relativeToECClassId);
    if (itor != m_persistenceECPropertyIdMap.end ())
        return itor->second;

    auto accessString = Utf8String (GetPropertyAccessString ());
    //WIP: ECClassId of root classMap is require here.

    PropertyMapCP current = this;
    int depth = 1;
    while (current->GetParent ())
        {
        current = GetParent ();
        depth = depth + 1;
        }

    ECPropertyId propertyId = 0;
    if (depth > 1)
        propertyId = ECDbSchemaPersistence::GetECPropertyAlias (relativeToECClassId, accessString.c_str (), db);

    if (propertyId == 0)
        {
        propertyId = PropertyMap::_GetECPropertyIdForPersistence (relativeToECClassId, db);        
        }

    m_persistenceECPropertyIdMap[relativeToECClassId] = propertyId;
    return propertyId;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyMapToTable::_GetColumns(DbColumnList& columns) const
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMapToTable::_FindOrCreateColumnsInTable( ClassMapR classMap )
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
PropertyMapToColumn::PropertyMapToColumn (ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
: PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_columnInfo (columnInfo), m_primitiveProperty (ecProperty.GetAsPrimitiveProperty ())
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    12/2013
//---------------------------------------------------------------------------------------
bool PropertyMapToColumn::_IsVirtual () const
    {
    return m_column != nullptr && m_column->IsVirtual ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
MapStatus PropertyMapToColumn::_FindOrCreateColumnsInTable (ClassMapR classMap)
    {
    Utf8CP        columnName    = m_columnInfo.GetName();
    PrimitiveType primitiveType = m_columnInfo.GetColumnType();
    bool          nullable      = m_columnInfo.GetNullable();
    bool          unique        = m_columnInfo.GetUnique();
    Collate       collate       = m_columnInfo.GetCollate();
    m_column = classMap.FindOrCreateColumnForProperty(*this, columnName, primitiveType, nullable, unique, collate);
    BeAssert (m_column != nullptr && "This actually indicates a mapping error. The method PropertyMapToColumn::_FindOrCreateColumnsInTable should therefore be changed to return an error.");
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
bool PropertyMapToColumn::_MapsToColumn (Utf8CP columnName) const
    {
    return (0 == BeStringUtilities::Stricmp (columnName, m_column->GetName ()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
void PropertyMapToColumn::_GetColumns (DbColumnList& columns) const
    {
    columns.push_back(m_column.get());
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      12/2012
//---------------------------------------------------------------------------------------
void PropertyMapToColumn::_SetColumnBaseName (Utf8CP columnName)
    {
    m_columnInfo.SetColumnName (columnName);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapToColumn::_ToNativeSql (ECDbR ecdb, DbTableCR table) const
    {
    if (m_primitiveProperty->GetType () == PRIMITIVETYPE_IGeometry && !ecdb.ColumnExists (table.GetName (), m_column->GetName ()))
        return NativeSqlBuilder::List {NativeSqlBuilder ("NULL")};

    return PropertyMap::_ToNativeSql (ecdb, table);
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
void PropertyMapToColumn::_AddBindings( BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECEnablerCR enabler ) const 
    {
    UInt32 propertyIndex;
    ECObjectsStatus status = enabler.GetPropertyIndex(propertyIndex, GetPropertyAccessString());
    if (ECOBJECTS_STATUS_Success != status)
        { BeAssert(false); }

    Binding binding(enabler, *this, propertyIndex, 0, sqlIndex, m_column.get());
    bindings.push_back(binding);
    sqlIndex++;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult PropertyMapToColumn::BindECValueToParameter (Statement& statement, ECValueCR v, BindingCR binding) const
    {
    const int parameterIndex = binding.m_sqlIndex;

    if (v.IsNull())
        return statement.BindNull(parameterIndex);

    BeAssert (v.IsPrimitive());
    switch (v.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Boolean:
            return statement.BindInt(parameterIndex, v.GetBoolean()); //needswork: add BindBoolean

        case PRIMITIVETYPE_Integer:
            return statement.BindInt(parameterIndex, v.GetInteger());

        case PRIMITIVETYPE_Long:
            return statement.BindInt64(parameterIndex, v.GetLong());

        case PRIMITIVETYPE_Double:
            return statement.BindDouble(parameterIndex, v.GetDouble());

        case PRIMITIVETYPE_String:
            {
            if (v.IsUtf8 ())
                {
                return statement.BindText (parameterIndex, v.GetUtf8CP (), Statement::MAKE_COPY_Yes);
                }
            else
                {
                return statement.BindText (parameterIndex, Utf8String (v.GetString ()), Statement::MAKE_COPY_Yes);
                }
            }

        case PRIMITIVETYPE_Point2D:
            {
            // Only for the case where the Point2D was mapped to a single blob column, otherwise Point2d is mapped to two double columns
            // WIP_ECDB: start handling componentMask in case the caller only wants to bind some of the components of the point
            DPoint2d point = v.GetPoint2D();
            return statement.BindBlob (parameterIndex, &point, sizeof(DPoint2d), Statement::MAKE_COPY_Yes);
            }
        case PRIMITIVETYPE_Point3D:
            {
            // Only for the case where the Point3D was mapped to a single blob column, otherwise Point2d is mapped to three double columns
            // WIP_ECDB: start handling componentMask in case the caller only wants to bind some of the components of the point
            DPoint3d point = v.GetPoint3D();
            return statement.BindBlob (parameterIndex, &point, sizeof(DPoint3d), Statement::MAKE_COPY_Yes);
            }
        case PRIMITIVETYPE_DateTime:
            {
            bool hasMetadata = false;
            DateTime::Info metadata;
            const Int64 ceTicks = v.GetDateTimeTicks (hasMetadata, metadata);
            UInt64 jdHns = DateTime::CommonEraTicksToJulianDay (ceTicks);
            const double jd = DateTime::HnsToRationalDay (jdHns);

            return BindDateTime (jd, hasMetadata, metadata, binding, statement);
            }
        case PRIMITIVETYPE_Binary:
            {
            size_t size;
            const byte * blob = v.GetBinary (size);
            return statement.BindBlob (parameterIndex, blob, (int)size, Statement::MAKE_COPY_No);
            }

        default:
            BeAssert(false && "unrecognized type");
            return BE_SQLITE_ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult PropertyMapToColumn::_Bind( int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, IECInstanceR ecInstance ) const
    {
    BindingCR binding = parameterBindings[iBinding];
    iBinding++;

    ECValue v;
    ECObjectsStatus s = ecInstance.GetValue(v, binding.m_propertyIndex);
    if (ECOBJECTS_STATUS_Success != s)
        return BE_SQLITE_ERROR;

    return BindECValueToParameter(statement, v, binding);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PropertyMapToColumn::BindDateTime (double jd, bool hasMetadata, DateTime::Info const& metadata, BindingCR binding, BeSQLiteStatementR statement) const
    {
    if (hasMetadata)
        {
        DateTimeInfo ecPropertyMetadata;
        if (StandardCustomAttributeHelper::GetDateTimeInfo (ecPropertyMetadata, GetProperty ()) == ECOBJECTS_STATUS_Success && !ecPropertyMetadata.IsMatchedBy (metadata))
            {
            LOG.errorv ("Metadata of DateTime to bind to index %d doesn't match the metadata on the corresponding ECProperty.", binding.m_propertyIndex);
            return BE_SQLITE_ERROR;
            }
        }

    return statement.BindDouble (binding.m_sqlIndex, jd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle    02/2013
//+---------------+---------------+---------------+---------------+---------------+------
DbResult PropertyMapToColumn::GetValueDateTime (double& jd, DateTime::Info& metadata, BindingCR binding, BeSQLiteStatementR statement) const
    {
    const int columnIndex = binding.m_sqlIndex;
    jd = statement.GetValueDouble (columnIndex);

    //This takes default values for the members of the DateTimeInfo CA that are unset in the CA.
    //Defaults: Kind::Unspecified
    //          Component::DateAndTime
    ECN::DateTimeInfo customAttributeContent;
    if (ECN::StandardCustomAttributeHelper::GetDateTimeInfo (customAttributeContent, GetProperty ()) == ECOBJECTS_STATUS_Success)
        metadata = customAttributeContent.GetInfo (true);
    else
        return BE_SQLITE_ERROR;

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapPoint::PropertyMapPoint (ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, PropertyMapCP parentPropertyMap)
    : PropertyMap (ecProperty, propertyAccessString, parentPropertyMap), m_columnInfo(columnInfo)
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
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PropertyMapPoint::_ToString() const
    {
    if (m_is3d)
        return WPrintfString(L"PropertyMapPoint (3d): ecProperty=%ls.%ls, columnName=%ls.X, .Y, .Z", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    else
        return WPrintfString(L"PropertyMapPoint (2d): ecProperty=%ls.%ls, columnName=%ls.X, .Y", GetProperty().GetClass().GetFullName(), 
            GetProperty().GetName().c_str(), WString (m_columnInfo.GetName(), BentleyCharEncoding::Utf8).c_str ());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyMapPoint::_AddBindings (BindingsR bindings, UInt32 propertyIndexUnused, int& sqlIndex, ECEnablerCR enabler) const
    {
    UInt32 propertyIndex;
    ECObjectsStatus status = enabler.GetPropertyIndex (propertyIndex, GetPropertyAccessString ());
    if (ECOBJECTS_STATUS_Success != status)
        {
        BeAssert (false);
        }

    Binding xbinding (enabler, *this, propertyIndex, 1, sqlIndex, m_xColumn.get ());
    bindings.push_back (xbinding);
    sqlIndex++;

    Binding ybinding (enabler, *this, propertyIndex, 2, sqlIndex, m_yColumn.get ());
    bindings.push_back (ybinding);
    sqlIndex++;

    if (m_is3d)
        {
        Binding zbinding (enabler, *this, propertyIndex, 3, sqlIndex, m_zColumn.get ());
        bindings.push_back (zbinding);
        sqlIndex++;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MapStatus PropertyMapPoint::_FindOrCreateColumnsInTable (ClassMapR classMap)
    {
    PrimitiveType primitiveType = PRIMITIVETYPE_Double;

    Utf8CP        columnName    = m_columnInfo.GetName();
    bool          nullable      = m_columnInfo.GetNullable();
    bool          unique        = m_columnInfo.GetUnique();
    Collate       collate       = m_columnInfo.GetCollate();

    Utf8String xColumnName(columnName);
    xColumnName.append(".X");
    m_xColumn = classMap.FindOrCreateColumnForProperty(*this, xColumnName.c_str(), primitiveType, nullable, unique, collate);

    Utf8String yColumnName(columnName);
    yColumnName.append(".Y");
    m_yColumn = classMap.FindOrCreateColumnForProperty(*this, yColumnName.c_str(), primitiveType, nullable, unique, collate);

    if (!m_is3d)
        return MapStatus::Success;

    Utf8String zColumnName(columnName);
    zColumnName.append(".Z");
    m_zColumn = classMap.FindOrCreateColumnForProperty(*this, zColumnName.c_str(), primitiveType, nullable, unique, collate);
    
    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyMapPoint::_MapsToColumn( Utf8CP columnName ) const 
    {
    return (0 == BeStringUtilities::Stricmp (columnName, m_xColumn->GetName ()) ||
        0 == BeStringUtilities::Stricmp (columnName, m_yColumn->GetName ()) ||
        m_is3d && 0 == BeStringUtilities::Stricmp (columnName, m_zColumn->GetName ()));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyMapPoint::_GetColumns(DbColumnList& columns) const
    {
    columns.push_back(m_xColumn.get());
    columns.push_back(m_yColumn.get());
    if (m_is3d) 
        columns.push_back(m_zColumn.get());
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
// @bsimethod                                                    casey.mullen      12/2012
//---------------------------------------------------------------------------------------
void PropertyMapPoint::_SetColumnBaseName (Utf8CP columnName)
    {
    m_columnInfo.SetColumnName (columnName);
    Utf8String name = columnName;
    if (name.size() < 3)
        return;

    size_t len = name.size();
    Utf8CP ending = &name[len - 2];

    if ( 0 == BeStringUtilities::Stricmp(ending, ".X") ||
         0 == BeStringUtilities::Stricmp(ending, ".Y") ||
         0 == BeStringUtilities::Stricmp(ending, ".Z") )
        name[len - 2] = '\0'; // Truncate any .X, .Y, .Z

    m_columnInfo.SetColumnName (name.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult PropertyMapPoint::_Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, IECInstanceR ecInstance) const
    {
    BeAssert (static_cast<size_t> (iBinding) < parameterBindings.size ());
    UInt32 propertyIndex = parameterBindings[iBinding].m_propertyIndex;

    ECValue v;
    ECObjectsStatus s = ecInstance.GetValue(v, propertyIndex);
    if (ECOBJECTS_STATUS_Success != s)
        {
        iBinding++; // We have to move along, or get stuck in an endless loop
        return BE_SQLITE_ERROR;
        }

    if (v.IsNull())
        {
        BeAssert (static_cast<size_t> (iBinding) < parameterBindings.size ());
        DbResult stat = statement.BindNull(parameterBindings[iBinding].m_sqlIndex);
        iBinding++;
        return stat;
        }

    DPoint3d point;
    if (m_is3d)
        point = v.GetPoint3D();
    else
        {
        DPoint2d point2d = v.GetPoint2D();
        memcpy (&point, &point2d, sizeof(DPoint2d)); //we will ignore the garbage z... in the name of avoiding redundant code
        }

    for ( ; iBinding < (int)parameterBindings.size() && propertyIndex == parameterBindings[iBinding].m_propertyIndex; ++iBinding)
        {
        BindingCR binding = parameterBindings[iBinding];
        BeAssert (this == &binding.m_propertyMap); 

        DbResult r;
        if (binding.m_componentMask == 1)
            r = statement.BindDouble (binding.m_sqlIndex, point.GetComponent(0));
        else if (binding.m_componentMask == 2)
            r = statement.BindDouble (binding.m_sqlIndex, point.GetComponent(1));
        else if (m_is3d && binding.m_componentMask == 3)
            r = statement.BindDouble (binding.m_sqlIndex, point.GetComponent(2));
        else
            {
            BeAssert (false);
            r = BE_SQLITE_ERROR;
            }

        if (BE_SQLITE_OK != r)
            {
            iBinding++;
            return r;
            }
        }

    return BE_SQLITE_OK;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
PropertyMapArrayOfPrimitives::PropertyMapArrayOfPrimitives (ECPropertyCR ecProperty, WCharCP propertyAccessString, ColumnInfoCR columnInfo, ECDbMapCR ecDbMap, PropertyMapCP parentPropertyMap)
    : PropertyMapToColumn (ecProperty, propertyAccessString, columnInfo, parentPropertyMap)
    {
    BeAssert (columnInfo.GetColumnType() == PRIMITIVETYPE_Binary); 
    ArrayECPropertyCP arrayProperty = GetProperty().GetAsArrayProperty();
    BeAssert(arrayProperty);

    ECClassCR primitiveArrayPersistenceClass = ecDbMap.GetClassForPrimitiveArrayPersistence (arrayProperty->GetPrimitiveElementType());
    m_primitiveArrayEnabler = primitiveArrayPersistenceClass.GetDefaultStandaloneEnabler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
NativeSqlBuilder::List PropertyMapArrayOfPrimitives::_ToNativeSql (ECDbR ecdb, DbTableCR table) const
    {
    if (GetProperty ().GetAsArrayProperty ()->GetPrimitiveElementType () == PRIMITIVETYPE_IGeometry && !ecdb.ColumnExists (table.GetName (), m_column->GetName ()))
        return NativeSqlBuilder::List {NativeSqlBuilder ("NULL")};

    return PropertyMap::_ToNativeSql (ecdb, table);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
DbResult PropertyMapArrayOfPrimitives::_Bind (int& iBinding, BindingsCR parameterBindings, BeSQLiteStatementR statement, IECInstanceR instance) const
    {
    BindingCR binding = parameterBindings[iBinding];
    iBinding++;

    ECValue v;
    ECObjectsStatus status = instance.GetValue(v, binding.m_propertyIndex);
    PRECONDITION (status == ECOBJECTS_STATUS_Success, BE_SQLITE_ERROR);
    if (ECDbUtility::IsECValueEmpty (v))
        return statement.BindBlob(binding.m_sqlIndex, nullptr, 0, Statement::MAKE_COPY_No); //Null it out in case it was not previously cleared

    ArrayECPropertyCP arrayECProperty = GetProperty().GetAsArrayProperty();
    POSTCONDITION(arrayECProperty, BE_SQLITE_ERROR);

    PrimitiveType primitiveType = arrayECProperty->GetPrimitiveElementType();
    POSTCONDITION (v.GetArrayInfo().GetElementPrimitiveType() == primitiveType, BE_SQLITE_ERROR);
    if (primitiveType == PRIMITIVETYPE_IGeometry)
        {
        LOG.errorv ("Array of common geometry ECProperty is not supported by ECPersistence API. Use ECSqlStatement instead.");
        return BE_SQLITE_ERROR;
        }

    int nElements = v.GetArrayInfo().GetCount();
    if (nElements == 0)
        return statement.BindBlob(binding.m_sqlIndex, nullptr, 0, Statement::MAKE_COPY_No); //Null it out in case it was not previously cleared

    //Needswork: if incoming is ECDInstance and we make GetPropertyValueSize(*propertyLayout) public, this could always get the precise size
    int sizeOfPrimitive;
    if (PRIMITIVETYPE_Binary == primitiveType || PRIMITIVETYPE_String == primitiveType)
        sizeOfPrimitive = 32; // impossible to estimate, but this will help avoid some re-allocs
    else
        sizeOfPrimitive = ECValue::GetFixedPrimitiveValueSize (primitiveType);
    UInt32 anticipatedSize = nElements * sizeOfPrimitive + 6 * sizeof(UInt32); // flags, null flags, secondary offset, offset to end, plus a little extra

    StandaloneECInstancePtr persistedInstance = m_primitiveArrayEnabler->CreateInstance(anticipatedSize); // WIP_ECDB: Could be cached/shared or created more directly, with guaranteed correct size.
    BeAssert (persistedInstance.IsValid());

    // Allocate 1 more than we need, then remove it, to ensure we have some "slack" in our memory buffer.
    persistedInstance->AddArrayElements(L"PrimitiveArray", nElements + 1);
    persistedInstance->RemoveArrayElement(L"PrimitiveArray", nElements);

    //WIP_ECDB: Rather than copying them one-by-one, we could enhance MemoryInstanceSupport to allow memcpy from one to the other, if this shows up in a profiler
    for (int i = 0; i < nElements; i++)
        {
        ECValue elementValue;
        if (ECOBJECTS_STATUS_Success != instance.GetValue(elementValue, binding.m_propertyIndex, i))
            return BE_SQLITE_ERROR;

        ECObjectsStatus s = persistedInstance->SetValue((UInt32)1, elementValue, i);
        if (ECOBJECTS_STATUS_Success != s && ECOBJECTS_STATUS_PropertyValueMatchesNoChange != s)
            return BE_SQLITE_ERROR;
        }

    byte* data = const_cast<byte*>(persistedInstance->GetData());
    int bytesUsed = persistedInstance->GetBytesUsed();
    BeAssert(ECDBuffer::IsCompatibleVersion(nullptr, data, true) && "Should have a header that claims we can write it" );

    return statement.BindBlob(binding.m_sqlIndex, data, bytesUsed, Statement::MAKE_COPY_Yes);
    }
        
END_BENTLEY_SQLITE_EC_NAMESPACE


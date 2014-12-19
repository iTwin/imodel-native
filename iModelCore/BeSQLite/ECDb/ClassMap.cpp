/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMap.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


//********************* ClassDbView ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus ClassDbView::Generate (NativeSqlBuilder& viewSql, bool isPolymorphic, ECSqlPrepareContext const& preparedContext) const
    {
    if (m_classMap == nullptr)
        {
        BeAssert (false && "ClassDbView::Generate called but m_classMap is null");
        return ERROR;
        }
    
    if (m_classMap->IsUnmapped ())
        {
        BeAssert (false && "ClassDbView::Generate must not be called on unmapped class");
        return ERROR;
        }

    //ECSQL_TODO optimizeByIncludingOnlyRealTables result is optmize short queries but require some chatting with db to determine if table exist or not
    //           this feature need to be evaluated for performance before its enabled.
    return ViewGenerator::CreateView (viewSql, m_classMap->GetECDbMap (), *m_classMap, isPolymorphic, preparedContext, true /*optimizeByIncludingOnlyRealTables*/);
    }


//********************* IClassMap::NativeSqlConverter ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus IClassMap::NativeSqlConverter::GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    return _GetWhereClause (whereClauseBuilder, ecsqlType, isPolymorphicClassExp, tableAlias);
    }


//********************* IClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
IClassMap const& IClassMap::GetView (View classView) const
    {
    return _GetView (classView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection const& IClassMap::GetPropertyMaps () const
    {
    return _GetPropertyMaps ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan      09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP IClassMap::GetPropertyMap (WCharCP propertyName) const
    {
    PropertyMapCP propMap = nullptr;
    if (GetPropertyMaps ().TryGetPropertyMap (propMap, propertyName, true))
        return propMap;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool IClassMap::ContainsPropertyMapToTable () const
    {
    bool found = false;
    GetPropertyMaps ().Traverse ([&found] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        if (propMap->GetAsPropertyMapToTable () != nullptr)
            {
            found = true;
            feedback = TraversalFeedback::Cancel;
            }
        }, true);

    return found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
DbTableR IClassMap::GetTable () const
    {
    return _GetTable ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
void IClassMap::GetTables (bset<DbTableCP>& tables, bool includeDerivedClasses) const
    {
    auto insertTableDelegate = [] (bset<DbTableCP>& tables, IClassMap const& classMap)
        {
        auto const& table = classMap.GetTable ();
        if (!classMap.IsUnmapped () && !ECDbMap::IsNullTable (table))
            tables.insert (&table);
        };

    if (!includeDerivedClasses)
        {
        insertTableDelegate (tables, *this);
        return;
        }

    auto const& ecdbMap = GetECDbMap ();
    auto const& derivedClasses = ecdbMap.GetECDbR ().GetEC ().GetSchemaManager ().GetDerivedECClasses (const_cast<ECClassR> (GetClass ()));
    for (auto derivedClass : derivedClasses)
        {
        auto derivedClassMap = ecdbMap.GetClassMap (*derivedClass);
        insertTableDelegate (tables, *derivedClassMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
std::vector<IClassMap const*> IClassMap::GetDerivedClassMaps () const
    {
    auto const& ecdbMap = GetECDbMap ();

    std::vector<IClassMap const*> derivedClassMaps;
    auto const& derivedClasses = ecdbMap.GetECDbR ().GetEC ().GetSchemaManager ().GetDerivedECClasses (const_cast<ECClassR> (GetClass ()));
    for (auto derivedClass : derivedClasses)
        {
        auto derivedClassMap = ecdbMap.GetClassMap (*derivedClass);
        derivedClassMaps.push_back (derivedClassMap);
        }

    return std::move (derivedClassMaps);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ECClassCR IClassMap::GetClass () const
    {
    return _GetClass ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2014
//---------------------------------------------------------------------------------------
ECClassId IClassMap::GetParentMapClassId () const
    {
    return _GetParentMapClassId ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
IClassMap::NativeSqlConverter const& IClassMap::GetNativeSqlConverter () const
    {
    return _GetNativeSqlConverter ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
ClassDbView const& IClassMap::GetDbView () const
    {
    return _GetDbView ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
IClassMap::Type IClassMap::GetClassMapType () const
    {
    return _GetClassMapType ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
MapStrategy IClassMap::GetMapStrategy () const
    {
    return _GetMapStrategy ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbMapCR IClassMap::GetECDbMap () const
    {
    return _GetECDbMap ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IClassMap::IsUnmapped () const
    {
    BeAssert (IsDoNotMapStrategy (GetMapStrategy ()) == (GetClassMapType () == Type::Unmapped));
    return GetClassMapType () == Type::Unmapped;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsDoNotMapStrategy (MapStrategy mapStrategy)
    {
    return mapStrategy == MapStrategy::DoNotMap || mapStrategy == MapStrategy::DoNotMapHierarchy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
bool IClassMap::IsRelationshipClassMap () const
    {
    const auto type = GetClassMapType ();
    return type == Type::RelationshipEndTable || type == Type::RelationshipLinkTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
bool IClassMap::IsAbstractECClass () const
    {
    return IsAbstractECClass (GetClass ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsAbstractECClass (ECClassCR ecclass)
    {
    return !ecclass.GetIsDomainClass () && !ecclass.GetIsStruct () && !ecclass.GetIsCustomAttributeClass ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsAnyClass (ECClassCR ecclass)
    {
    return ecclass.GetSchema ().IsStandardSchema () && ecclass.GetName ().Equals (L"AnyClass");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
bool IClassMap::IsMappedToSecondaryTable () const
    {
    return IsMapToSecondaryTableStrategy (GetClass ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsMapToSecondaryTableStrategy (ECN::ECClassCR ecClass)
    {
    return ecClass.GetIsStruct ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IClassMap::ToString () const
    {
    WCharCP typeStr = nullptr;
    switch (GetClassMapType ())
        {
            case IClassMap::Type::Class:
                typeStr = L"Class";
                break;
            case IClassMap::Type::EmbeddedType:
                typeStr = L"EmbeddedType";
                break;
            case IClassMap::Type::RelationshipEndTable:
                typeStr = L"RelationshipEndTable";
                break;
            case IClassMap::Type::RelationshipLinkTable:
                typeStr = L"RelationshipLinkTable";
                break;
            case IClassMap::Type::SecondaryTable:
                typeStr = L"SecondaryTable";
                break;
            case IClassMap::Type::Unmapped:
                typeStr = L"Unmapped";
                break;
            default:
                BeAssert (false && "Update ClassMap::ToString to handle new value in enum IClassMap::Type.");
                typeStr = L"Unrecognized class map type";
                break;
        }

    WCharCP strategyName = nullptr;
    switch (GetMapStrategy ())
        {
            case MapStrategy::TablePerHierarchy:
                strategyName = BSCAV_TablePerHierarchy;
                break;
            case MapStrategy::DoNotMapHierarchy:
                strategyName = BSCAV_DoNotMapHierarchy;
                break;
            case MapStrategy::DoNotMap:
                strategyName = BSCAV_DoNotMap;
                break;
            case MapStrategy::InParentTable:
                strategyName = BSCAV_InParentTable;
                break;
            case MapStrategy::TablePerClass:
                strategyName = BSCAV_TablePerClass;
                break;
            case MapStrategy::TableForThisClass:
                strategyName = BSCAV_TableForThisClass;
                break;
            case MapStrategy::NoHint:
                strategyName = BSCAV_NoHint;
                break;
            case MapStrategy::RelationshipSourceTable:
                strategyName = BSCAV_RelationshipSourceTable;
                break;
            case MapStrategy::RelationshipTargetTable:
                strategyName = BSCAV_RelationshipTargetTable;
                break;
            case MapStrategy::SharedTableForThisClass:
                strategyName = BSCAV_SharedTableForThisClass;
                break;
            default:
                BeAssert (false && "Update ClassMap::ToString to handle new value in enum MapStrategy.");
                strategyName = L"Unrecognized map strategy";
                break;
        }

    WString str;
    str.Sprintf (L"ClassMap '%ls' - Type: %ls - Map strategy: %ls", GetClass ().GetFullName (), typeStr, strategyName);

    return Utf8String (str);
    }


//********************* ClassMap::NativeSqlConverterImpl ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
ClassMap::NativeSqlConverterImpl::NativeSqlConverterImpl (ClassMapCR classMap)
    : NativeSqlConverter (), m_classMap (classMap)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    01/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ClassMap::NativeSqlConverterImpl::_GetWhereClause (NativeSqlBuilder& whereClauseBuilder, ECSqlType ecsqlType, bool isPolymorphicClassExp, Utf8CP tableAlias) const
    {
    auto const& classMap = GetClassMap ();
    BeAssert (!classMap.IsUnmapped () && "ClassMap::NativeSqlConverterImpl::GetWhereClause not expected to be called by unmapped class map.");

    bool wasEmpty = whereClauseBuilder.IsEmpty ();

    //handle case where multiple classes are mapped to the table
    auto const& table = classMap.GetTable ();
    auto classIdColumn = table.GetClassIdColumn ();
    if (classIdColumn != nullptr)
        {
        if (wasEmpty)
            whereClauseBuilder.AppendParenLeft ();
        else
            whereClauseBuilder.Append (" AND ");

        whereClauseBuilder.Append (tableAlias, classIdColumn->GetName ()).Append (" IN (");
        whereClauseBuilder.Append (classMap.GetClass ().GetId ());
        if (isPolymorphicClassExp)
            {
            auto derivedClassMapList = classMap.GetDerivedClassMaps ();
            for (auto derivedClassMap : derivedClassMapList)
                {
                whereClauseBuilder.AppendComma ().Append (derivedClassMap->GetClass ().GetId ());
                }
            }

        whereClauseBuilder.AppendParenRight (); //right paren of IN

        if (wasEmpty)
            whereClauseBuilder.AppendParenRight (); //overall right paren
        }

    return ECSqlStatus::Success;
    }

//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap (ECClassCR ecClass, ECDbMapCR ecDbMap, MapStrategy mapStrategy, bool setIsDirty)
: IClassMap (), m_ecDbMap (ecDbMap), m_table (nullptr), m_ecClass (ecClass), m_mapStrategy (mapStrategy), 
  m_parentMapClassId (0LL), m_nativeSqlConverter (nullptr), m_dbView (nullptr), m_isDirty (setIsDirty)
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::Initialize (ClassMapInfoCR mapInfo)
    {
    const auto mapStrategy = GetMapStrategy ();
    IClassMap const* effectiveParentClassMap = mapStrategy == MapStrategy::InParentTable ? mapInfo.GetParentClassMap () : nullptr;

    auto stat = _InitializePart1 (mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    stat = _InitializePart2 (mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    return _OnInitialized ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart1 (ClassMapInfoCR mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView (*this));

    //if parent class map exists, its dbtable is reused.
    if (parentClassMap != nullptr)
        {
        PRECONDITION (!parentClassMap->IsUnmapped(), MapStatus::Error);
        m_parentMapClassId = parentClassMap->GetClass ().GetId ();
        m_table = &parentClassMap->GetTable ();
        }
    else
        {
        auto table = m_ecDbMap.FindOrCreateTable (
            mapInfo.GetTableName(),
            mapInfo.IsMapToVirtualTable (),
            mapInfo.GetECInstanceIdColumnName(), 
            IClassMap::IsMapToSecondaryTableStrategy (m_ecClass),
            mapInfo.IsMappedToExistingTable(), 
            mapInfo.IsAllowedToReplaceEmptyTableWithEmptyView()); // could be existing or to-be-created

        if (!EXPECTED_CONDITION (!table.IsNull()))
            return MapStatus::Error;

        m_table = table.get ();
        }

    //Add ECInstanceId property map
    //check if it already exists
    if (GetECInstanceIdPropertyMap () != nullptr)
        return MapStatus::Success;

    //does not exist yet
    PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create (GetSchemaManager (), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        //log and assert already done in child method
            return MapStatus::Error;

    GetPropertyMapsR ().AddPropertyMap (ecInstanceIdPropertyMap);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart2 (ClassMapInfoCR mapInfo, IClassMap const* parentClassMap)
    {
    auto stat = AddPropertyMaps (mapInfo, parentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    stat = ProcessIndices (mapInfo);
    if (stat != MapStatus::Success)
        return stat;

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_OnInitialized ()
    {
    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::AddPropertyMaps (ClassMapInfoCR mapInfo, IClassMap const* parentClassMap)
    {
    for (auto property : m_ecClass.GetProperties (true))
        {
        WCharCP propertyAccessString = property->GetName ().c_str ();
        PropertyMapPtr propMap = nullptr;
        if (&property->GetClass () != &m_ecClass && parentClassMap != nullptr)
            parentClassMap->GetPropertyMaps ().TryGetPropertyMap (propMap, propertyAccessString);

        if (propMap == nullptr)
            {
            propMap = PropertyMap::CreateAndEvaluateMapping (*property, m_ecDbMap, m_ecClass, propertyAccessString, nullptr);
            BeAssert (propMap != nullptr);
            if (propMap == nullptr)
                continue;

            propMap->FindOrCreateColumnsInTable (*this);
            BeAssert (GetPropertyMap (propertyAccessString) == nullptr && " it should not be there");
            }

        GetPropertyMapsR ().AddPropertyMap (propMap);
        }

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::ProcessIndices (ClassMapInfoCR mapInfo)
    {
    DoProcessIndices (mapInfo);
    ProcessStandardKeySpecifications (mapInfo);
    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassMap::ProcessStandardKeySpecifications(ClassMapInfoCR mapInfo)
    {
    std::set<PropertyMapCP> doneList; 
    std::set<WString> specList;
    for (StandardKeySpecificationPtr spec : mapInfo.GetStandardKeys())
         {
         BeAssert(spec->GetKeyProperties().size() > 0);
         
         if (spec->GetKeyProperties().size() == 0)
             continue;
         
         Utf8String propertyName = spec->GetKeyProperties().front();
         WString propertyNameW (propertyName.c_str(), true);
         WString typeString = StandardKeySpecification::TypeToString(spec->GetType());
         if (specList.find(typeString) != specList.end())
             continue;

         specList.insert(typeString);
         auto propertyMap = GetPropertyMap (propertyNameW.c_str());
         if (propertyMap == nullptr)
             {
             LOG.warningv(L"Column index creation is ignoring %ls on %ls because map for ECProperty '%ls' cannot be found", typeString.c_str(), GetClass().GetFullName(), WString (propertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
             continue;
             }
         //We dont want to create multiple indexes on same column.
         if (doneList.find(propertyMap) != doneList.end())
             {
             LOG.warningv(L"Ignoring %ls for property %ls.%ls. It is already part of another index.", typeString.c_str(), GetClass().GetFullName(), WString (propertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
             continue;
             }
         doneList.insert(propertyMap); 

         DbIndexPtr index = DbIndex::Create (nullptr, //name is generated when index is added 
                                    *m_table, false);
         DbColumnList columns;
         propertyMap->GetColumns (columns);
         for (auto column : columns)
             index->AddColumn (*column);

         //logs an error, if adding fails
         m_table->AddIndex (*index);
         }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassMap::DoProcessIndices (ClassMapInfoCR mapInfo)
    {
    bvector<ClassIndexInfoPtr>const& indexInfos = mapInfo.GetIndexInfo();
    for (ClassIndexInfoPtr indexInfo : indexInfos)
        {
        DbIndexPtr index = DbIndex::Create (indexInfo->GetName (), *m_table, indexInfo->GetIsUnique ());
        bool error = false;

        for (Utf8String propertyName : indexInfo->GetProperties())
            {
            WString propertyNameW (propertyName.c_str(), true);
            auto propertyMap = GetPropertyMap(propertyNameW.c_str());

            if (propertyMap == nullptr)
                {
                LOG.errorv(L"Reject user defined index on %ls. Fail to find property map for property %ls", GetClass().GetFullName(), WString (propertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                error = true;
                break;
                }

            DbColumnList columns;
            propertyMap->GetColumns (columns);
            if (0 == columns.size())
                {
                LOG.errorv(L"Reject user defined index on %ls. Fail to find column property map for property %ls. Something wrong with mapping", GetClass().GetFullName(), WString (propertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                error = true;
                break;
                }

            for (DbColumnCP column : columns)
                index->AddColumn(*(const_cast<DbColumnP>(column)));
            }

        if (!error)
            m_table->AddIndex (*index);
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2011
//------------------------------------------------------------------------------------------
DbColumnPtr ClassMap::FindOrCreateColumnForProperty (PropertyMapR propertyMap, Utf8CP requestedColumnName, PrimitiveType columnType, bool nullable, bool unique, Collate collate)
    {
    Utf8String newColumnName (requestedColumnName);
    ECPropertyCR ecProperty = propertyMap.GetProperty();
    //This can go away once we now it doesn't hornswaggle older files.
    PropertyMapCP conflictingPropertyMap = GetPropertyMapForColumnName (requestedColumnName);
    if (conflictingPropertyMap != nullptr)
        {
        newColumnName = Utf8String (ecProperty.GetClass ().GetName ().c_str ());
        newColumnName.append ("_").append (requestedColumnName);
        }
    // end of "This can go away..."

    auto column = GetTable ().GetColumns ().GetPtr (newColumnName.c_str ());
    bool createNewColumn = column == nullptr;
    if (column != nullptr)
        {
        if (column->IsCompatibleWith (columnType))
            {
            createNewColumn = false;
            if (column->GetNullable () != nullable || column->GetUnique () != unique || column->GetCollate () != collate)
                {
                LOG.warningv ("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                    " but where 'Nullable', 'Unique', or 'Collate' differs, and which will therefore be ignored for some of the properties.",
                    column->GetName (), GetTable ().GetName ());

                BeAssert (false && "A column is used by multiple property maps where property name and data type matches, "
                    " but where 'Nullable', 'Unique', or 'Collate' differs.");
                }
            }
        else
            {
            //column exists, but is not compatible -> generate alternative name
            newColumnName.Sprintf ("%s_%lld", requestedColumnName, ecProperty.GetId ());
            createNewColumn = true;

            BeAssert (!GetTable ().GetColumns ().Contains (newColumnName.c_str ()) && "Alternative column name <propname>_<propid> is not expected to pre-exist. It is expected to be unique.");
            }
        }

    if (!newColumnName.EqualsI (requestedColumnName))
        propertyMap.SetColumnBaseName (newColumnName.c_str ());

    if (!createNewColumn)
        return column;


    auto newColumn = DbColumn::Create (newColumnName.c_str (), columnType, nullable, unique, collate);
    m_table->GetColumnsR ().Add (newColumn);
    return newColumn;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
PropertyMapCP ClassMap::GetECInstanceIdPropertyMap () const
    {
    PropertyMapPtr propMap = nullptr;
    if (TryGetECInstanceIdPropertyMap (propMap))
        return propMap.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2013
//---------------------------------------------------------------------------------------
bool ClassMap::TryGetECInstanceIdPropertyMap (PropertyMapPtr& ecInstanceIdPropertyMap) const
    {
    return GetPropertyMaps ().TryGetPropertyMap (ecInstanceIdPropertyMap, PropertyMapECInstanceId::PROPERTYACCESSSTRING);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMapCP ClassMap::GetPropertyMapForColumnName (Utf8CP columnName) const
    {
    auto const& propMaps = GetPropertyMaps ();
    for (auto it = propMaps.begin (); it != propMaps.end (); ++it)
        {
        PropertyMapCP propertyMap = *it;
        BeAssert (propertyMap != nullptr);
        if (propertyMap == nullptr)
            continue;

        if (propertyMap->MapsToColumn (columnName))
            return propertyMap;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection const& ClassMap::_GetPropertyMaps () const
    {
    return m_propertyMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
PropertyMapCollection& ClassMap::GetPropertyMapsR ()
    {
    return m_propertyMaps;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbSchemaManagerCR ClassMap::GetSchemaManager () const
    {
    return GetECDbMap ().GetECDbR ().GetEC ().GetSchemaManager ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
IClassMap::Type ClassMap::_GetClassMapType () const
    {
    return Type::Class;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
ClassMap::NativeSqlConverter const& ClassMap::_GetNativeSqlConverter () const
    {
    if (m_nativeSqlConverter == nullptr)
        m_nativeSqlConverter = std::unique_ptr<NativeSqlConverter> (new NativeSqlConverterImpl (*this));

    return *m_nativeSqlConverter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
DbResult ClassMap::Save (bool includeFullGraph)
    {
    if (includeFullGraph /* save base class map*/)
        {
        for (ECClassP baseClass : GetClass ().GetBaseClasses ())
            {
            auto baseClassMap = GetECDbMap ().GetClassMapP (*baseClass);
            if (baseClassMap != nullptr)
                baseClassMap->Save (includeFullGraph);
            }
        }

    const auto classId = GetClass ().GetId ();
    ECDbSchemaPersistence::DeleteECClassMap (classId, GetECDbMap ().GetECDbR ());

    DbECClassMapInfo info;
    info.ColsInsert =
        DbECClassMapInfo::COL_ECClassId |
        DbECClassMapInfo::COL_MapStrategy;

    const bool isMapInParent = (m_mapStrategy == MapStrategy::InParentTable);
    // Key column(s)
    info.ColsNull = 0;
    info.m_ecClassId = classId;
    info.m_mapStrategy = m_mapStrategy;
    // Save Parent ECClassId if this class is map to parent
    if (isMapInParent)
        {
        info.m_mapParentECClassId = m_parentMapClassId;
        info.ColsInsert |= DbECClassMapInfo::COL_MapParentECClassId;
        }
    // save table name
    auto const& table = GetTable ();
    if (!isMapInParent && !table.IsNullTable ())
        {
        bvector<DbColumnP> primaryKeyColumns;
        table.GetPrimaryKeyTableConstraint ().GetColumns (primaryKeyColumns);
        BeAssert (primaryKeyColumns.size () != 0);

        if (strcmp (primaryKeyColumns.front ()->GetName (), ECDB_COL_ECInstanceId) != 0)
            {
            info.m_primaryKeyColumnName = primaryKeyColumns.front ()->GetName ();
            info.ColsInsert |= DbECClassMapInfo::COL_ECInstanceIdColumn;
            }
        info.m_mapToDbTable = table.GetName ();
        info.ColsInsert |= DbECClassMapInfo::COL_MapToDbTable;;
        }

    DbResult r = ECDbSchemaPersistence::InsertECClassMapInfo (GetECDbMap ().GetECDbR (), info);
    if (r != BE_SQLITE_DONE)
        return r;

    GetPropertyMaps ().Traverse ([&r, this] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        //don't save system property maps as they are handled by the class map directly
        //and don't save PropertyMapToTables
        if (propMap->IsSystemPropertyMap () || propMap->GetAsPropertyMapToTable () != nullptr)
            return;

        if (&propMap->GetProperty ().GetClass () == &m_ecClass)
            if ((r = propMap->Save (GetECDbMap ().GetECDbR ())) != BE_SQLITE_DONE)
                {
                feedback = TraversalFeedback::Cancel;
                return;
                }
        }, true);

    m_isDirty = false;
    return r;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
ECPropertyCP ClassMap::GetECProperty (ECN::ECClassCR ecClass, WCharCP propertyAccessString)
    {
    bvector<WString> tokens;
    BeStringUtilities::Split(propertyAccessString, L".", NULL, tokens);

    //for recursive lambdas, iOS requires us to efine the lambda variable before assigning the actual function to it.
    std::function<ECPropertyCP (ECClassCR, bvector<WString>&, int)> getECPropertyFromTokens;
    getECPropertyFromTokens = [&getECPropertyFromTokens] (ECClassCR ecClass, bvector<WString>& tokens, int iCurrentToken) -> ECPropertyCP
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP (tokens[iCurrentToken].c_str (), true);
        if (!ecProperty)
            return nullptr;

        if (iCurrentToken == tokens.size () - 1)
            return ecProperty; // we are the last token

        // There are more tokens... delving into an embedded struct
        StructECPropertyCP structProperty = ecProperty->GetAsStructProperty ();
        if (structProperty)
            return getECPropertyFromTokens (structProperty->GetType (), tokens, iCurrentToken + 1);

        BeAssert (false && "Any second-to-last ECProperty has to be a struct!");
        return nullptr;
        };

    return getECPropertyFromTokens (ecClass, tokens, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::GenerateParameterBindings (BindingsR parameterBindings, int firstParameterIndex) const
    {
    BeAssert (parameterBindings.size () == 0);
    // WIP_ECDB: Use same technique as in ClassMap::GenerateSelectedPropertyBindings to get the order to follow the order in the ECClass
    int sqlIndex = firstParameterIndex;

    GetPropertyMaps ().Traverse ([&parameterBindings, &sqlIndex, this] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        BeAssert (propMap != nullptr);
        if (propMap->IsSystemPropertyMap ())
            return; //ignore system props as they are not used in ECDbStatement API.

        //WIP_ECDB: remove the unused 0... the propertyIndex parameter!
        propMap->AddBindings (parameterBindings, 0, sqlIndex, *m_ecClass.GetDefaultStandaloneEnabler ());
        }, true);

    return SUCCESS;
    }


//************************** MappedTable ***************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTable::MappedTable (ECDbMapR ecDbMap, ClassMapCR classMap) : m_table (classMap.GetTable ()), m_ecDbMap (ecDbMap), m_generatedClassId (false)
    {
    m_classMaps.push_back(&classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTablePtr MappedTable::Create (ECDbMapR ecDbMap, ClassMapCR classMap)
    {
    return new MappedTable(ecDbMap, classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MappedTable::FinishTableDefinition()
    {
    //if (m_finished)
    //    return SUCCESS;
    int nOwners = 0;
    bool tablePerHierarchy = false;
    for (auto classMap : m_classMaps)
        {
        //relation end maps don't have own table
        if (!classMap->IsUnmapped() && classMap->GetClassMapType () != ClassMap::Type::RelationshipEndTable)
            {
            nOwners++;
            if (classMap->GetMapStrategy() == MapStrategy::TablePerHierarchy)
                tablePerHierarchy = true;
            }
        }

    if (tablePerHierarchy  || nOwners > 1)
        {
        if (!m_generatedClassId && m_table.GenerateClassIdColumn() != nullptr)
            m_generatedClassId = true;
        else
            return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MappedTable::AddClassMap (ClassMapCR classMap)
    {
    if (&classMap.GetTable() != &m_table)
        {
        LOG.errorv ("Attempted to add a ClassMap for table '%s' to MappedTable for table '%s'.", classMap.GetTable().GetName(), m_table.GetName());
        return ERROR;
        }
    if (std::find(m_classMaps.begin(), m_classMaps.end(), &classMap) != m_classMaps.end())
        {
        
        return SUCCESS;
        }
    m_classMaps.push_back(&classMap);
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

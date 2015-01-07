/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMap.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ENABLE_REF 1
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
ECDbSqlTable& IClassMap::GetTable () const
    {
    return _GetTable ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
void IClassMap::GetTables (bset<ECDbSqlTable const*>& tables, bool includeDerivedClasses) const
    {
    auto const& ecdbMap = GetECDbMap ();
    auto insertTableDelegate = [&ecdbMap] (bset<ECDbSqlTable const*>& tables, IClassMap const& classMap)
        {
        auto const& table = classMap.GetTable ();
        if (!classMap.IsUnmapped () && !ecdbMap.GetSQLManager ().IsNullTable(table))
            tables.insert (&table);
        };

    if (!includeDerivedClasses)
        {
        insertTableDelegate (tables, *this);
        return;
        }
    

    auto const& derivedClasses = ecdbMap.GetECDbR ().GetSchemaManager ().GetDerivedECClasses (const_cast<ECClassR> (GetClass ()));
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
    auto const& derivedClasses = ecdbMap.GetECDbR ().GetSchemaManager ().GetDerivedECClasses (const_cast<ECClassR> (GetClass ()));
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
    auto classIdColumn = table.FindColumnCP("ECClassId");
    if (classIdColumn != nullptr)
        {
        if (wasEmpty)
            whereClauseBuilder.AppendParenLeft ();
        else
            whereClauseBuilder.Append (" AND ");

        whereClauseBuilder.Append (tableAlias, classIdColumn->GetName ().c_str()).Append (" IN (");
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
        auto table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable (
            mapInfo.GetTableName(),
            mapInfo.IsMapToVirtualTable (),
            mapInfo.GetECInstanceIdColumnName(), 
            IClassMap::IsMapToSecondaryTableStrategy (m_ecClass),
            mapInfo.IsMappedToExistingTable(), 
            mapInfo.IsAllowedToReplaceEmptyTableWithEmptyView()); // could be existing or to-be-created

        if (!EXPECTED_CONDITION (table != nullptr))
            return MapStatus::Error;

        m_table = table;
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
    //! we delay the index create tell mapping finishes
    SetUserProvidedIndex (mapInfo.GetIndexInfo ());
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

         std::vector<ECDbSqlColumn const*> columns;
         propertyMap->GetColumns (columns);

         auto index = m_table->CreateIndex (nullptr);
         index->SetIsUnique (false);
         for (auto column : columns)
             index->Add (column->GetName().c_str());

         //!ECDbSqlToDo add check to make sure index is not empty
         if (!index->IsValid ())
             {
             BeAssert (false && "Index was not created correctly");
             index->Drop ();
             }
         }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassMap::CreateIndices ()
    {
    int i = 0;
    for (ClassIndexInfoPtr indexInfo : m_indexes)
        {
        i = i + 1;
        //***************************************************************************
        //Todo : Fix this code 
        //Following happen because index is recreated in case of upgrade. This should be handle
        //in someother way. 
        auto existingIndex = m_table->GetDbDef ().FindIndex (indexInfo->GetName ());
        if (existingIndex)
            {
            if (&existingIndex->GetTable () == &GetTable ())
                {
                if (existingIndex->GetColumns ().size () != indexInfo->GetProperties ().size ())
                    {
                    //ERROR
                    }
                }
            else
                {
                //ERROR
                }

            return;
            }
        //***************************************************************************

        auto index = m_table->CreateIndex (indexInfo->GetName ());
        index->SetIsUnique (indexInfo->GetIsUnique ());
        bool error = false;

        for (Utf8String classQualifiedPropertyName : indexInfo->GetProperties())
            {
           
            WString resolvePropertyName;
            Utf8String resolveClassName, resolveSchemaName;
            
            std::vector<Utf8String> parts;
            auto beginItor = classQualifiedPropertyName.begin ();
            auto itor = beginItor;
            for (; itor != classQualifiedPropertyName.end (); ++itor)
                {
                if (*itor == '.')
                    {
                    auto part = Utf8String (beginItor, itor - beginItor);
                    part.Trim ();
                    if (part.empty ())
                        {
                        BeDataAssert (false && "Qualified property name provided in ECDbIndex contain invalid format name");
                        LOG.errorv (L"Reject user defined index on %ls. Fail to find property map for property %ls", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                        error = true;
                        }

                    parts.push_back (part);
                    beginItor = itor + 1;
                    }
                }

            if (error)
                break;

            if (beginItor != itor)
                {
                parts.push_back (Utf8String (beginItor, itor - beginItor));
                }

            resolveSchemaName = Utf8String (GetClass ().GetSchema ().GetName ().c_str ());
            resolveClassName = Utf8String (GetClass ().GetName ().c_str ());
            switch (parts.size ())
                {
                case 1:
                    resolvePropertyName = WString (parts.at (0).c_str (), BentleyCharEncoding::Utf8);
                    break;
                case 2:
                    resolveClassName = parts.at (0);
                    resolvePropertyName = WString (parts.at (1).c_str (), BentleyCharEncoding::Utf8);
                    break;
                case 3:
                    resolveSchemaName = parts.at (0);
                    resolveClassName = parts.at (1);
                    resolvePropertyName = WString (parts.at (2).c_str (), BentleyCharEncoding::Utf8);
                    break;
                default:
                    {
                    BeDataAssert (false && "Qualified property name provided in ECDbIndex contain invalid format name");
                    LOG.errorv (L"Reject user defined index on %ls. Invalid format to describe property qualified name %ls", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                    error = true;
                    }
                }
            
            if (error)
                break;

            auto resolveClass = GetECDbMap ().GetECDbR ().GetSchemaManager ().GetECClass (resolveSchemaName.c_str (), resolveClassName.c_str ());
            if (resolveClass == nullptr)
                {
                LOG.errorv (L"Reject user defined index on %ls. Failed to find class associated with property %ls", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                break;
                }

            auto resolveClassMap = GetECDbMap ().GetClassMapCP (*resolveClass);
            if (resolveClassMap == nullptr)
                {
                BeAssert (false && "One reason could be that this method is called during mapping. It should be called after every thing is mapped");
                LOG.errorv (L"Reject user defined index on %ls. Failed to find classMap associated with property %ls", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                break;
                }

            if (&resolveClassMap->GetTable () != &GetTable ())
                {
                BeAssert (false && "User define class qualified property string point to a class that is mapped into a different table then current class");
                LOG.errorv (L"Reject user defined index on %ls. Property %ls belong to a class that is not mapped into table %s", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str (), WString (GetTable().GetName().c_str(), BentleyCharEncoding::Utf8).c_str ());
                break;
                }

            auto propertyMap = resolveClassMap->GetPropertyMap (resolvePropertyName.c_str());
            if (propertyMap == nullptr)
                {
                LOG.errorv (L"Rejecting index[%d] specified in ECDbClassHint on class %ls because property specified in index '%ls' doesn't exist in class or its not mapped", i, GetClass().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                error = true;
                break;
                }

            if (!propertyMap->GetProperty().GetAsPrimitiveProperty ())
                {
                LOG.errorv (L"Rejecting index[%d] specified in ECDbClassHint on class %ls because specified property is not primitive.", i, GetClass ().GetFullName ());
                error = true; // skip this index and continue with rest
                break;
                }

            switch (propertyMap->GetProperty().GetAsPrimitiveProperty ()->GetType ())
                {
                case PRIMITIVETYPE_String:
                case PRIMITIVETYPE_Boolean:
                case PRIMITIVETYPE_Integer:
                case PRIMITIVETYPE_Long:
                case PRIMITIVETYPE_DateTime:
                case PRIMITIVETYPE_Double:
                case PRIMITIVETYPE_Binary:
                case PRIMITIVETYPE_Point2D:
                case PRIMITIVETYPE_Point3D:
                    // allowed index
                    break;
                    //not supported for indexing
                case PRIMITIVETYPE_IGeometry:
                    LOG.errorv (L"Rejecting user specified index[%d] specified in ECDbClassHint on class %ls because specified property type not supported. Supported types are String, Boolean, Integer, DateTime, Double, Binary, Point2d and Point3d", i, GetClass ().GetFullName ());
                    error = true; // skip this index and continue with rest
                    break;

                default:
                    LOG.errorv (L"Rejecting user specified index[%d] specified in ECDbClassHint on class %ls because specified property type not supported. Supported types are String, Boolean, Integer, DateTime, Double and Binary", i, GetClass ().GetFullName ());
                    error = true; // skip this index and continue with rest
                    break;
                }

            std::vector<ECDbSqlColumn const*> columns;
            propertyMap->GetColumns (columns);
            if (0 == columns.size())
                {                
                LOG.errorv (L"Reject user defined index on %ls. Fail to find column property map for property %ls. Something wrong with mapping", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                error = true;
                break;
                }

            for (ECDbSqlColumn const* column : columns)
                {
                if (column->GetPersistenceType() == PersistenceType::Virtual)
                    {
                    LOG.errorv (L"Reject user defined index on %ls. One of the column assoicated with property %ls is virtual column.", GetClass ().GetFullName (), WString (classQualifiedPropertyName.c_str (), BentleyCharEncoding::Utf8).c_str ());
                    error = true;
                    break;
                    }
                index->Add(column->GetName().c_str());
                }
            }

        if (error || !index->IsValid())
            {
            index->Drop ();
            }
        }
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2011
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ClassMap::FindOrCreateColumnForProperty (PropertyMapR propertyMap, Utf8CP requestedColumnName, PrimitiveType columnType, bool nullable, bool unique, ECDbSqlColumn::Constraint::Collate collate, Utf8CP accessStringPrefix)
    {
    Utf8String newColumnName (requestedColumnName);
    ECPropertyCR ecProperty = propertyMap.GetProperty();
    auto potentialColumnType = ECDbSqlHelper::PrimitiveTypeToColumnType (columnType);
    //This can go away once we now it doesn't hornswaggle older files.
    PropertyMapCP conflictingPropertyMap = GetPropertyMapForColumnName (requestedColumnName);
    if (conflictingPropertyMap != nullptr)
        {
        newColumnName = Utf8String (ecProperty.GetClass ().GetName ().c_str ());
        newColumnName.append ("_").append (requestedColumnName);
        }
    // end of "This can go away..."

    auto column = GetTable ().FindColumnP (newColumnName.c_str ());
    bool createNewColumn = (column == nullptr);
    if (column != nullptr)
        {
        if (ECDbSqlHelper ::IsCompatiable(column->GetType (), potentialColumnType))
            {
            createNewColumn = false;
            if (GetTable ().GetOwnerType () == OwnerType::ECDb)
                {
                if (column->GetConstraint ().IsNotNull () != !nullable || column->GetConstraint ().IsUnique () != unique || column->GetConstraint ().GetCollate () != collate)
                    {
                    LOG.warningv ("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                        " but where 'Nullable', 'Unique', or 'Collate' differs, and which will therefore be ignored for some of the properties.",
                        column->GetName().c_str(), GetTable().GetName().c_str());

                    BeAssert (false && "A column is used by multiple property maps where property name and data type matches, "
                        " but where 'Nullable', 'Unique', or 'Collate' differs.");
                    }
                }
            }
        else
            {
            //column exists, but is not compatible -> generate alternative name
            newColumnName.Sprintf ("%s_%lld", requestedColumnName, ecProperty.GetId ());
            createNewColumn = true;

            BeAssert ((GetTable ().FindColumnCP (newColumnName.c_str ()) != nullptr) && "Alternative column name <propname>_<propid> is not expected to pre-exist. It is expected to be unique.");
            }
        }

    if (!newColumnName.EqualsI (requestedColumnName))
        propertyMap.SetColumnBaseName (newColumnName.c_str ());

#ifdef ENABLE_REF
    Utf8String accessString = Utf8String (propertyMap.GetPropertyAccessString ());
    if (!Utf8String::IsNullOrEmpty (accessStringPrefix))
        accessString.append (".").append (accessStringPrefix);

#endif
    if (!createNewColumn)
        {
#ifdef ENABLE_REF
        auto canEdit = column->GetTableR ().GetEditHandleR ().CanEdit ();
        if (!canEdit)
            column->GetTableR ().GetEditHandleR ().BeginEdit ();

        column->GetDependentPropertiesR ().Add (propertyMap.GetProperty ().GetClass ().GetId (), accessString.c_str ());
        
        if (!canEdit)
            column->GetTableR ().GetEditHandleR ().EndEdit ();

#endif
        return column;
        }
    

    auto newColumn = GetTable ().CreateColumn (newColumnName.c_str (), potentialColumnType);
    newColumn->GetConstraintR ().SetIsNotNull (!nullable);
    newColumn->GetConstraintR ().SetIsUnique (unique);
    newColumn->GetConstraintR ().SetCollate (collate);

#ifdef ENABLE_REF
    newColumn->GetDependentPropertiesR ().Add (propertyMap.GetProperty ().GetClass ().GetId (), accessString.c_str ());
#endif
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
    return GetECDbMap ().GetECDbR ().GetSchemaManager ();
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
    if (!isMapInParent && !GetECDbMap ().GetSQLManager ().IsNullTable (table))
        {
        std::vector<ECDbSqlColumn const*> systemColumns;
        if (GetTable ().GetFilteredColumnList (systemColumns, ECdbSystemColumnECId) == BentleyStatus::SUCCESS)
            {
            if (!systemColumns.front ()->GetName ().EqualsI (ECDB_COL_ECInstanceId))
                {
                info.m_primaryKeyColumnName = systemColumns.front ()->GetName ();
                info.ColsInsert |= DbECClassMapInfo::COL_ECInstanceIdColumn;
                }
            info.m_mapToDbTable = table.GetName ();
            info.ColsInsert |= DbECClassMapInfo::COL_MapToDbTable;
            }
        else
            {
            BeAssert (false && "ECDbSystemColumnECId column is missing");
            }
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
BentleyStatus ClassMap::GenerateParameterBindings (Bindings& parameterBindings, int firstParameterIndex) const
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
    if (m_table.GetOwnerType () == OwnerType::ECDb)
        {
        int nOwners = 0;
        bool tablePerHierarchy = false;
        for (auto classMap : m_classMaps)
            {
            //relation end maps don't have own table
            if (!classMap->IsUnmapped () && classMap->GetClassMapType () != ClassMap::Type::RelationshipEndTable)
                {
                nOwners++;
                if (classMap->GetMapStrategy () == MapStrategy::TablePerHierarchy)
                    tablePerHierarchy = true;
                }
            }

        if (tablePerHierarchy || nOwners > 1)
            {
            if (!m_generatedClassId )                
                {
                auto ecClassIdColumn = m_table.FindColumnP (ECDB_COL_ECClassId);
                if (ecClassIdColumn == nullptr)
                    {
                    const size_t insertPosition = 1;
                    ecClassIdColumn = m_table.CreateColumn (ECDB_COL_ECClassId, ECDbSqlColumn::Type::Long, insertPosition, ECdbSystemColumnECClassId, PersistenceType::Persisted);
                    if (ecClassIdColumn == nullptr)
                        return ERROR;
                    }

                for (auto classMap : m_classMaps)
                    {
                    ecClassIdColumn->GetDependentPropertiesR ().Add (classMap->GetClass ().GetId (), ECDB_COL_ECClassId);
                    }

                m_generatedClassId = true;
                }
                
            else
                return ERROR;
            }
        }

    //if (m_table.GetEditHandleR ().CanEdit ())
    //    m_table.GetEditHandleR ().EndEdit ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt MappedTable::AddClassMap (ClassMapCR classMap)
    {
    if (&classMap.GetTable() != &m_table)
        {
        LOG.errorv ("Attempted to add a ClassMap for table '%s' to MappedTable for table '%s'.", classMap.GetTable().GetName().c_str(), m_table.GetName().c_str());
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

/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMap.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ClassMap.h"
#include <algorithm>

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

    if (m_classMap->GetMapStrategy ().IsNotMapped ())
        {
        BeAssert (false && "ClassDbView::Generate must not be called on unmapped class");
        return ERROR;
        }

    //ECSQL_TODO optimizeByIncludingOnlyRealTables result is optmize short queries but require some chatting with db to determine if table exist or not
    //           this feature need to be evaluated for performance before its enabled.
    return ViewGenerator::CreateView (viewSql, m_classMap->GetECDbMap (), *m_classMap, isPolymorphic, preparedContext, true /*optimizeByIncludingOnlyRealTables*/);
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
PropertyMapCP IClassMap::GetPropertyMap (Utf8CP propertyName) const
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

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       05 / 2015
//------------------------------------------------------------------------------------------
StorageDescription const& IClassMap::GetStorageDescription () const
    {
    return GetECDbMap ().GetLightWeightMapCache ().GetStorageDescription (GetClass ().GetId ());
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
std::vector<IClassMap const*> IClassMap::GetDerivedClassMaps () const
    {
    auto const& ecdbMap = GetECDbMap ();

    std::vector<IClassMap const*> derivedClassMaps;
    auto const& derivedClasses = ecdbMap.GetECDbR ().Schemas ().GetDerivedECClasses (const_cast<ECClassR> (GetClass ()));
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
ECDbMapStrategy const& IClassMap::GetMapStrategy () const
    {
    ECDbMapStrategy const& strategy = _GetMapStrategy ();
    BeAssert(strategy.IsValid() && "MapStrategy should have been resolved by the time it is hold in a class map.");
    return strategy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2014
//---------------------------------------------------------------------------------------
ECDbMapCR IClassMap::GetECDbMap () const
    {
    return _GetECDbMap ();
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
    if (!ecclass.GetIsDomainClass() && !ecclass.GetIsStruct() && !ecclass.GetIsCustomAttributeClass())
        return true;

    //for relationship classes there is another criterion for abstractness: if one of the constraints doesn't have
    //any classes, then it is abstract. So check for that here now
    ECRelationshipClassCP relClass = ecclass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return false;
        
    return relClass->GetSource().GetClasses().empty() || relClass->GetTarget().GetClasses().empty();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2014
//---------------------------------------------------------------------------------------
//static
bool IClassMap::IsAnyClass (ECClassCR ecclass)
    {
    return ecclass.GetSchema ().IsStandardSchema () && ecclass.GetName ().Equals ("AnyClass");
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
    Utf8CP typeStr = nullptr;
    switch (GetClassMapType ())
        {
        case IClassMap::Type::Class:
            typeStr = "Class";
            break;
        case IClassMap::Type::EmbeddedType:
            typeStr = "EmbeddedType";
            break;
        case IClassMap::Type::RelationshipEndTable:
            typeStr = "RelationshipEndTable";
            break;
        case IClassMap::Type::RelationshipLinkTable:
            typeStr = "RelationshipLinkTable";
            break;
        case IClassMap::Type::SecondaryTable:
            typeStr = "SecondaryTable";
            break;
        case IClassMap::Type::Unmapped:
            typeStr = "Unmapped";
            break;
        default:
            BeAssert (false && "Update ClassMap::ToString to handle new value in enum IClassMap::Type.");
            typeStr = "Unrecognized class map type";
            break;
        }

    Utf8String str;
    str.Sprintf("ClassMap '%s' - Type: %s - Map strategy: %s", GetClass().GetFullName(), typeStr, GetMapStrategy().ToString().c_str());

    return str;
    }

//********************* ClassMap ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                06/2012
//---------------------------------------------------------------------------------------
ClassMap::ClassMap (ECClassCR ecClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty)
: IClassMap (), m_ecDbMap (ecDbMap), m_table (nullptr), m_ecClass (ecClass), m_mapStrategy (mapStrategy),
m_parentMapClassId(ECClass::UNSET_ECCLASSID), m_dbView(nullptr), m_isDirty(setIsDirty), m_columnFactory(*this), /*clang says not used - m_useSharedColumnStrategy (false),*/ m_id(0ULL)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::Initialize (ClassMapInfo const& mapInfo)
    {
    ECDbMapStrategy const& mapStrategy = GetMapStrategy ();
    IClassMap const* effectiveParentClassMap = mapStrategy.IsPolymorphicSharedTable() ? mapInfo.GetParentClassMap () : nullptr;

    auto stat = _InitializePart1 (mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;
    
    stat = _InitializePart2 (mapInfo, effectiveParentClassMap);
    if (stat != MapStatus::Success)
        return stat;

    return _OnInitialized ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_EvaluateDMLPolicy ()
    {
    DMLPolicy::Target selectTarget = DMLPolicy::Target::View;
    DMLPolicy::Target insertTarget = DMLPolicy::Target::View;
    DMLPolicy::Target updateTarget = DMLPolicy::Target::View;
    DMLPolicy::Target deleteTarget = DMLPolicy::Target::View;
 /*   auto& storagePartitions = GetStorageDescription ();
    auto& noneVirtualPartitions = storagePartitions.GetNonVirtualHorizontalPartitionIndices ();
    auto isPersisted = GetTable ().GetPersistenceType () == PersistenceType::Persisted;



    if (!isPersisted || GetMapStrategy ().GetStrategy () == ECDbMapStrategy::Strategy::ExistingTable)
        {
        selectTarget = DMLPolicy::Target::View;
        insertTarget = DMLPolicy::Target::None;
        updateTarget = DMLPolicy::Target::None;
        deleteTarget = DMLPolicy::Target::None;
        }
    else
        {
        if (noneVirtualPartitions.size () == 0)
            {
            selectTarget = DMLPolicy::Target::View;
            }
        else if (noneVirtualPartitions.size () == 1)
            {
            HorizontalPartition const* partition = GetStorageDescription ().GetHorizontalPartition (noneVirtualPartitions[0]);
            BeAssert (partition != nullptr);
            if (partition->GetClassIds ().size () == 1)
                {
                selectTarget = DMLPolicy::Target::Table;
                insertTarget = DMLPolicy::Target::Table;
                updateTarget = DMLPolicy::Target::Table;
                deleteTarget = DMLPolicy::Target::Table;
                }
            else
                {
                insertTarget = DMLPolicy::Target::Table;
                selectTarget = DMLPolicy::Target::View;
                updateTarget = DMLPolicy::Target::View;
                deleteTarget = DMLPolicy::Target::View;
                }
            }
        else
            {
            insertTarget = DMLPolicy::Target::Table;
            selectTarget = DMLPolicy::Target::View;
            updateTarget = DMLPolicy::Target::View;
            deleteTarget = DMLPolicy::Target::View;
            }
        }*/
    GetDMLPolicyR ().Set (DMLPolicy::Operation::Select, selectTarget);
    GetDMLPolicyR ().Set (DMLPolicy::Operation::Insert, insertTarget);
    GetDMLPolicyR ().Set (DMLPolicy::Operation::Update, updateTarget);
    GetDMLPolicyR ().Set (DMLPolicy::Operation::Delete, deleteTarget);

    return BentleyStatus::SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart1 (ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView (*this));

    //if parent class map exists, its dbtable is reused.
    if (parentClassMap != nullptr)
        {
        PRECONDITION (!parentClassMap->GetMapStrategy ().IsNotMapped (), MapStatus::Error);
        m_parentMapClassId = parentClassMap->GetClass ().GetId ();
        m_table = &parentClassMap->GetTable ();
        }
    else
        {
        auto table = const_cast<ECDbMapR>(m_ecDbMap).FindOrCreateTable (
            mapInfo.GetTableName (),
            mapInfo.IsMapToVirtualTable (),
            mapInfo.GetECInstanceIdColumnName (),
            IClassMap::IsMapToSecondaryTableStrategy (m_ecClass), 
            mapInfo.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::ExistingTable); 

        if (!EXPECTED_CONDITION (table != nullptr))
            return MapStatus::Error;

        m_table = table;
        }

    //Add ECInstanceId property map
    //check if it already exists
    if (GetECInstanceIdPropertyMap () != nullptr)
        return MapStatus::Success;

    //does not exist yet
    PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create (Schemas (), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        //log and assert already done in child method
        return MapStatus::Error;

    GetPropertyMapsR ().AddPropertyMap (ecInstanceIdPropertyMap);

    return MapStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2013
//---------------------------------------------------------------------------------------
MapStatus ClassMap::_InitializePart2 (ClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    auto stat = AddPropertyMaps (parentClassMap, nullptr,&mapInfo);
    if (stat != MapStatus::Success)
        return stat;
    if (mapInfo.GetClassHasCurrentTimeStampProperty() != NULL)
        {
        PropertyMapCP propertyMap = GetPropertyMap(mapInfo.GetClassHasCurrentTimeStampProperty()->GetName().c_str());
        if (propertyMap != NULL)
            {
            auto column = const_cast<ECDbSqlColumn*>(propertyMap->GetFirstColumn());
            BeAssert(column != nullptr && "TimeStamp column cannot be null");
            if (column)
                {
                //! TODO: Handle this case for shared column strategy;
                BeAssert(column->GetType() == ECDbSqlColumn::Type::DateTime);
                column->GetConstraintR().SetDefaultExpression("julianday('now')");
                Utf8String whenCondtion;
                whenCondtion.Sprintf("old.%s=new.%s", column->GetName().c_str(), column->GetName().c_str());
                Utf8String body;
                Utf8CP instanceID = GetPropertyMap("ECInstanceId")->GetFirstColumn()->GetName().c_str();
                body.Sprintf ("BEGIN UPDATE %s SET %s=julianday('now') WHERE %s=new.%s AND (%s IS NULL OR julianday('now') > %s); END", column->GetTableR ().GetName ().c_str (), column->GetName ().c_str (), instanceID, instanceID, column->GetName ().c_str (), column->GetName ().c_str ());
                Utf8String triggerName;
                triggerName.Sprintf("%s_CurrentTimeStamp", column->GetTableR().GetName().c_str());
                column->GetTableR().CreateTrigger(triggerName.c_str(), column->GetTableR(), whenCondtion.c_str(), body.c_str(), TriggerType::Create, TriggerSubType::After);
                }
            }
        }

    return ProcessStandardKeySpecifications(mapInfo) == SUCCESS ? MapStatus::Success : MapStatus::Error;
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
MapStatus ClassMap::AddPropertyMaps (IClassMap const* parentClassMap, ECDbClassMapInfo const* loadInfo,ClassMapInfo const* classMapInfo)
    {
    std::vector<ECPropertyP> propertiesToMap;
    PropertyMapPtr propMap = nullptr;

    for (auto property : m_ecClass.GetProperties (true))
        {
        Utf8CP propertyAccessString = property->GetName ().c_str ();
        propMap = nullptr;
        if (&property->GetClass () != &m_ecClass && parentClassMap != nullptr)
            parentClassMap->GetPropertyMaps ().TryGetPropertyMap (propMap, propertyAccessString);

        if (propMap == nullptr)
            propertiesToMap.push_back (property);
        else
            GetPropertyMapsR ().AddPropertyMap (propMap);
        }
    
    if (loadInfo == nullptr)
        GetColumnFactoryR ().Update ();

    for (auto property : propertiesToMap)
        {
        Utf8CP propertyAccessString = property->GetName ().c_str ();
        propMap = PropertyMap::CreateAndEvaluateMapping (*property, m_ecDbMap, m_ecClass, propertyAccessString, &GetTable(), nullptr);
        if (propMap == nullptr)
            return MapStatus::Error;

        if (GetPropertyMap (propertyAccessString) != nullptr)
            {
            BeAssert (GetPropertyMap (propertyAccessString) == nullptr && " it should not be there");
            return MapStatus::Error;
            }

        if (loadInfo == nullptr)
            {
            if (SUCCESS == propMap->FindOrCreateColumnsInTable(*this, classMapInfo))
                GetPropertyMapsR ().AddPropertyMap (propMap);
            }
        else
            {
            if (propMap->Load (*loadInfo) == SUCCESS)
                GetPropertyMapsR ().AddPropertyMap (propMap);
            }
        }

    return MapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::ProcessStandardKeySpecifications(ClassMapInfo const& mapInfo)
    {
    std::set<PropertyMapCP> doneList;
    std::set<Utf8String> specList;
    for (StandardKeySpecificationPtr spec : mapInfo.GetStandardKeys())
        {
        BeAssert(spec->GetKeyProperties().size() > 0);

        if (spec->GetKeyProperties().size() == 0)
            continue;

        Utf8String propertyName = spec->GetKeyProperties().front();
        Utf8String typeString = StandardKeySpecification::TypeToString(spec->GetType());
        if (specList.find(typeString) != specList.end())
            continue;

        specList.insert(typeString);
        auto propertyMap = GetPropertyMap(propertyName.c_str());
        if (propertyMap == nullptr)
            {
            LOG.warningv("Column index creation is ignoring %s on %s because map for ECProperty '%s' cannot be found", typeString.c_str(), GetClass().GetFullName(), propertyName.c_str());
            continue;
            }
        //We don't want to create multiple indexes on same column.
        if (doneList.find(propertyMap) != doneList.end())
            {
            LOG.warningv("Ignoring %s for property %s.%s. It is already part of another index.", typeString.c_str(), GetClass().GetFullName(), propertyName.c_str());
            continue;
            }
        doneList.insert(propertyMap);

        std::vector<ECDbSqlColumn const*> columns;
        propertyMap->GetColumns(columns);

        ECDbSqlIndex* index = m_table->CreateIndex(nullptr, false, GetClass().GetId());
        for (auto column : columns)
            index->Add(column->GetName().c_str());

        //!ECDbSqlToDo add check to make sure index is not empty
        if (!index->IsValid())
            {
            BeAssert(false && "Index was not created correctly");
            index->Drop();
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Affan.Khan                           09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClassMap::CreateUserProvidedIndices(ClassMapInfo const& classMapInfo) const
    {
    int i = 0;
    for (ClassIndexInfoPtr indexInfo : classMapInfo.GetIndexInfo())
        {
        i++;
        auto index = m_table->CreateIndex(indexInfo->GetName(), indexInfo->GetIsUnique(), GetClass().GetId());
        if (index == nullptr)
            return ERROR;

        Utf8String whereExpression;
        bool error = false;
        for (Utf8StringCR classQualifiedPropertyName : indexInfo->GetProperties())
            {
            Utf8String resolvePropertyName;
            Utf8String resolveClassName, resolveSchemaName;

            std::vector<Utf8String> parts;
            auto beginItor = classQualifiedPropertyName.begin();
            auto itor = beginItor;
            for (; itor != classQualifiedPropertyName.end(); ++itor)
                {
                if (*itor == '.')
                    {
                    auto part = Utf8String(beginItor, itor - beginItor);
                    part.Trim();
                    if (part.empty())
                        {
                        BeDataAssert(false && "Qualified property name provided in ECDbIndex contain invalid format name");
                        LOG.errorv("Reject user defined index on %s. Fail to find property map for property %s", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                        error = true;
                        }

                    parts.push_back(part);
                    beginItor = itor + 1;
                    }
                }

            if (error)
                break;

            if (beginItor != itor)
                {
                parts.push_back(Utf8String(beginItor, itor - beginItor));
                }

            resolveSchemaName = GetClass().GetSchema().GetName().c_str();
            resolveClassName = GetClass().GetName().c_str();
            switch (parts.size())
                {
                    case 1:
                        resolvePropertyName = Utf8String(parts.at(0).c_str());
                        break;
                    case 2:
                        resolveClassName = parts.at(0);
                        resolvePropertyName = Utf8String(parts.at(1).c_str());
                        break;
                    case 3:
                        resolveSchemaName = parts.at(0);
                        resolveClassName = parts.at(1);
                        resolvePropertyName = Utf8String(parts.at(2).c_str());
                        break;
                    default:
                        {
                        BeDataAssert(false && "Qualified property name provided in ECDbIndex contain invalid format name");
                        LOG.errorv("Reject user defined index on %s. Invalid format to describe property qualified name %s", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                        error = true;
                        }
                }

            if (error)
                break;

            auto resolveClass = GetECDbMap().GetECDbR().Schemas().GetECClass(resolveSchemaName.c_str(), resolveClassName.c_str());
            if (resolveClass == nullptr)
                {
                LOG.errorv("Reject user defined index on %s. Failed to find class associated with property %s", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                break;
                }

            auto resolveClassMap = GetECDbMap().GetClassMapCP(*resolveClass);
            if (resolveClassMap == nullptr)
                {
                BeAssert(false && "One reason could be that this method is called during mapping. It should be called after every thing is mapped");
                LOG.errorv("Reject user defined index on %s. Failed to find classMap associated with property %s", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                break;
                }

            if (&resolveClassMap->GetTable() != &GetTable())
                {
                BeAssert(false && "User define class qualified property string point to a class that is mapped into a different table then current class");
                LOG.errorv("Reject user defined index on %s. Property %s belong to a class that is not mapped into table %s", GetClass().GetFullName(), classQualifiedPropertyName.c_str(), GetTable().GetName().c_str());
                break;
                }

            auto propertyMap = resolveClassMap->GetPropertyMap(resolvePropertyName.c_str());
            if (propertyMap == nullptr)
                {
                LOG.errorv("Rejecting index[%d] specified in ClassMap custom attribute on class %s because property specified in index '%s' doesn't exist in class or its not mapped", i, GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                error = true;
                break;
                }

            if (!propertyMap->GetProperty().GetAsPrimitiveProperty())
                {
                LOG.errorv("Rejecting index[%d] specified in ClassMap custom attribute on class %s because specified property is not primitive.", i, GetClass().GetFullName());
                error = true; // skip this index and continue with rest
                break;
                }

            switch (propertyMap->GetProperty().GetAsPrimitiveProperty()->GetType())
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
                        LOG.errorv("Rejecting user specified index[%d] specified in ClassMap custom attribute on class %s because specified property type not supported. Supported types are String, Boolean, Integer, DateTime, Double, Binary, Point2d and Point3d", i, GetClass().GetFullName());
                        error = true; // skip this index and continue with rest
                        break;

                    default:
                        LOG.errorv("Rejecting user specified index[%d] specified in ClassMap custom attribute on class %s because specified property type not supported. Supported types are String, Boolean, Integer, DateTime, Double and Binary", i, GetClass().GetFullName());
                        error = true; // skip this index and continue with rest
                        break;
                }

            std::vector<ECDbSqlColumn const*> columns;
            propertyMap->GetColumns(columns);
            if (0 == columns.size())
                {
                LOG.errorv("Reject user defined index on %s. Fail to find column property map for property %s. Something wrong with mapping", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                error = true;
                break;
                }

            for (ECDbSqlColumn const* column : columns)
                {
                if (column->GetPersistenceType() == PersistenceType::Virtual)
                    {
                    LOG.errorv("Reject user defined index on %s. One of the column associated with property %s is virtual column.", GetClass().GetFullName(), classQualifiedPropertyName.c_str());
                    error = true;
                    break;
                    }
                if (index->Add(column->GetName().c_str()) == BentleyStatus::SUCCESS)
                    {
                    switch (indexInfo->GetWhere())
                        {
                            case EC::ClassIndexInfo::WhereConstraint::NotNull:
                                {
                                //if column is not nullable, no need to add IS NOT NULL expression to where clause of index
                                if (column->GetConstraint().IsNotNull())
                                    break;

                                if (!whereExpression.empty())
                                    whereExpression.append(" AND ");

                                whereExpression.append("[");
                                whereExpression.append(column->GetName().c_str());
                                whereExpression.append("]");
                                whereExpression.append(" IS NOT NULL");

                                break;
                                }

                            default:
                                break;
                        }
                    }
                }
            }
        index->SetAdditionalWhereExpression(whereExpression.c_str());
        if (error || !index->IsValid())
            {
            index->Drop();
            return ERROR;
            }
        }

    return SUCCESS;
    }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2011
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ClassMap::FindOrCreateColumnForProperty (ClassMapCR classMap,ClassMapInfo const* classMapInfo, PropertyMapR propertyMap, Utf8CP requestedColumnName, PrimitiveType columnType, bool nullable, bool unique, ECDbSqlColumn::Constraint::Collation collation, Utf8CP accessStringPrefix)
    {
    ColumnFactory::Specification::Strategy strategy = ColumnFactory::Specification::Strategy::CreateOrReuse;
    ColumnFactory::Specification::GenerateColumnNameOptions generateColumnNameOpts = ColumnFactory::Specification::GenerateColumnNameOptions::NameBasedOnClassIdAndCaseSaveAccessString;
    ECDbSqlColumn::Type requestedColumnType = ECDbSqlHelper::PrimitiveTypeToColumnType (columnType);
 
    if (classMap.GetMapStrategy().GetOption() == ECDbMapStrategy::Option::SharedColumns)
        {
        BeAssert(classMap.GetMapStrategy().GetStrategy() == ECDbMapStrategy::Strategy::SharedTable);
        strategy = ColumnFactory::Specification::Strategy::CreateOrReuseSharedColumn;
        requestedColumnType = ECDbSqlColumn::Type::Any; //If not set it will get set anyway
        generateColumnNameOpts = ColumnFactory::Specification::GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence;
        }
    
    auto spec = ColumnFactory::Specification 
        (        
        propertyMap, 
        strategy, 
        generateColumnNameOpts, 
        requestedColumnName, 
        requestedColumnType, 
        ECDbKnownColumns::DataColumn, 
        PersistenceType::Persisted, 
        accessStringPrefix, 
        !nullable, 
        unique, 
        collation
        );

    return GetColumnFactoryR().Configure (spec);
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
ECDbSchemaManagerCR ClassMap::Schemas () const
    {
    return GetECDbMap ().GetECDbR ().Schemas ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  12/2013
//---------------------------------------------------------------------------------------
IClassMap::Type ClassMap::_GetClassMapType () const
    {
    return Type::Class;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                           07/2012
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Save (std::set<ClassMap const*>& savedGraph)
    {
    if (savedGraph.find (this) != savedGraph.end ())
        return BentleyStatus::SUCCESS;

    savedGraph.insert (this);
    auto& mapStorage = const_cast<ECDbMapR>(m_ecDbMap).GetSQLManagerR ().GetMapStorageR ();
    std::set<PropertyMapCP> baseProperties;

    if (GetId() == 0ULL)
        {
        //auto baseClassMap = GetParentMapClassId () == 
        auto baseClass = GetECDbMap ().GetECDbR ().Schemas ().GetECClass (GetParentMapClassId ());
        auto baseClassMap = baseClass == nullptr ? nullptr : (ClassMap*)GetECDbMap ().GetClassMap (*baseClass);
        if (baseClassMap != nullptr)
            {
            auto r = baseClassMap->Save (savedGraph);
            if (r != BentleyStatus::SUCCESS)
                return r;

            for (auto propertyMap : baseClassMap->GetPropertyMaps ())
                {
                baseProperties.insert (propertyMap);
                }
            }

        
        auto mapInfo = mapStorage.CreateClassMap(GetClass().GetId(), m_mapStrategy, GetDMLPolicy().ToInt(), baseClassMap == nullptr ? ECClass::UNSET_ECCLASSID : baseClassMap->GetId());
        for (auto propertyMap : GetPropertyMaps ())
            {
            if (baseProperties.find (propertyMap) != baseProperties.end())
                continue;

            if (SUCCESS != propertyMap->Save (*mapInfo))
                return ERROR;
            }

        m_id = mapInfo->GetId ();
        }

        m_isDirty = false;
        return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    affan.khan      01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ClassMap::_Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap)
    {
    m_dbView = std::unique_ptr<ClassDbView> (new ClassDbView (*this));
    auto& pm = mapInfo.GetPropertyMaps (false);
    if (pm.empty ())
        SetTable (const_cast<ECDbSqlTable*>(GetECDbMap ().GetSQLManager ().GetNullTable ()));
    else
        SetTable (const_cast<ECDbSqlTable*>(&(pm.front ()->GetColumn ().GetTable ())));

    this->m_crudPolicy = DMLPolicy::FromInt (mapInfo.GetDMLPolicy ());

    if (GetECInstanceIdPropertyMap () != nullptr)
        return BentleyStatus::ERROR;

    PropertyMapPtr ecInstanceIdPropertyMap = PropertyMapECInstanceId::Create (Schemas (), *this);
    if (ecInstanceIdPropertyMap == nullptr)
        return BentleyStatus::ERROR;

    GetPropertyMapsR ().AddPropertyMap (ecInstanceIdPropertyMap);

    return AddPropertyMaps (parentClassMap, &mapInfo,nullptr)  == MapStatus::Success ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      11/2012
//---------------------------------------------------------------------------------------
ECPropertyCP ClassMap::GetECProperty (ECN::ECClassCR ecClass, Utf8CP propertyAccessString)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split (propertyAccessString, ".", NULL, tokens);

    //for recursive lambdas, iOS requires us to efine the lambda variable before assigning the actual function to it.
    std::function<ECPropertyCP (ECClassCR, bvector<Utf8String>&, int)> getECPropertyFromTokens;
    getECPropertyFromTokens = [&getECPropertyFromTokens] (ECClassCR ecClass, bvector<Utf8String>& tokens, int iCurrentToken) -> ECPropertyCP
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

//************************** MappedTable ***************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTable::MappedTable (ECDbMapR ecDbMap, ClassMapCR classMap) : m_table (classMap.GetTable ()), m_ecDbMap (ecDbMap), m_generatedClassIdColumn (false)
    {
    m_classMaps.push_back (&classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MappedTablePtr MappedTable::Create (ECDbMapR ecDbMap, ClassMapCR classMap)
    {
    return new MappedTable (ecDbMap, classMap);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MappedTable::FinishTableDefinition ()
    {
    if (m_table.GetOwnerType () == OwnerType::ECDb)
        {
        int nOwners = 0;
        bool polymorphicSharedTable = false;
        for (auto classMap : m_classMaps)
            {
            if (!classMap->GetMapStrategy().IsNotMapped() && classMap->GetClassMapType () != ClassMap::Type::RelationshipEndTable)
                {
                nOwners++;
                if (classMap->GetMapStrategy ().IsPolymorphicSharedTable())
                    polymorphicSharedTable = true;
                }
            }

        if (polymorphicSharedTable || nOwners > 1)
            {
            if (!m_generatedClassIdColumn)
                {
                auto ecClassIdColumn = m_table.FindColumnP (ECDB_COL_ECClassId);
                if (ecClassIdColumn == nullptr)
                    {
                    const size_t insertPosition = 1;
                    ecClassIdColumn = m_table.CreateColumn (ECDB_COL_ECClassId, ECDbSqlColumn::Type::Long, insertPosition, ECDbKnownColumns::ECClassId, PersistenceType::Persisted);
                    if (ecClassIdColumn == nullptr)
                        return ERROR;
                    }

                m_generatedClassIdColumn = true;
                }

            else
                return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MappedTable::AddClassMap (ClassMapCR classMap)
    {
    if (&classMap.GetTable () != &m_table)
        {
        LOG.errorv ("Attempted to add a ClassMap for table '%s' to MappedTable for table '%s'.", classMap.GetTable ().GetName ().c_str (), m_table.GetName ().c_str ());
        return ERROR;
        }

    if (std::find (m_classMaps.begin (), m_classMaps.end (), &classMap) != m_classMaps.end ())
        return SUCCESS;

    m_classMaps.push_back (&classMap);
    return SUCCESS;
    }

//=========================================================================================
//ColumnFactory
//=========================================================================================
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::Specification::Specification (
    PropertyMapR propertyMap,
    Specification::Strategy strategy,
    Specification::GenerateColumnNameOptions generateColumnNameOptions,
    Utf8CP columnName,
    ECDbSqlColumn::Type columnType,
    ECDbKnownColumns knownColumnId,
    PersistenceType persistenceType,
    Utf8CP accessStringPrefix,
    bool isNotNull,
    bool isUnique,
    ECDbSqlColumn::Constraint::Collation collation)
    : m_propertyMap (propertyMap), m_requestedColumnName (columnName), m_columnType (columnType), m_isNotNull (isNotNull),
    m_isUnique(isUnique), m_collation(collation), m_generateColumnNameOptions(generateColumnNameOptions),
    m_knownColumnId (knownColumnId), m_persistenceType (persistenceType), m_strategy (strategy)
    {
    m_accessString = propertyMap.GetPropertyAccessString ();
    if (!Utf8String::IsNullOrEmpty (accessStringPrefix))
        m_accessString.append (".").append (accessStringPrefix);

    if (GetStrategy () == Strategy::Create)
        {
        //Column name must be not null
        BeAssert (Utf8String::IsNullOrEmpty (columnName) == false && "Must supply a valid columnName if Strategy::Create is used");
        }
    else if (GetStrategy () == Strategy::CreateOrReuse)
        {
        //Column name must be not null
        BeAssert (Utf8String::IsNullOrEmpty (columnName) == false && "Must supply a valid columnName if Strategy::Create is used");
        }
    else if (GetStrategy () == Strategy::CreateOrReuseSharedColumn)
        {
        // Shared column does not support NOT NULL constraint
        BeAssert (isNotNull == false && "Shared column cannot enforce NOT NULL constraint.");
        BeAssert (persistenceType == PersistenceType::Persisted);
        BeAssert (knownColumnId == ECDbKnownColumns::DataColumn);
        m_generateColumnNameOptions = GenerateColumnNameOptions::NameBasedOnPropertyNameAndPropertyId;
        m_requestedColumnName.clear ();
        m_columnType = ECDbSqlColumn::Type::Any;
        m_isNotNull = false;
        }

    if (persistenceType == PersistenceType::Virtual)
        {
        m_isUnique = false;
        }
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
//static 
void  ColumnFactory::SortByLeastUsedColumnFirst (std::vector<ECDbSqlColumn const*>& columns)
    {
    std::sort (columns.begin (), columns.end (),
        [] (ECDbSqlColumn const* a, ECDbSqlColumn const* b)
        {
        return a->GetDependentProperties ().Count () < b->GetDependentProperties ().Count ();
        });
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
//static 
void  ColumnFactory::SortByMostUsedColumnFirst (std::vector<ECDbSqlColumn const*>& columns)
    {
    std::sort (columns.begin (), columns.end (),
        [] (ECDbSqlColumn const* a, ECDbSqlColumn const* b)
        {
        return a->GetDependentProperties ().Count () > b->GetDependentProperties ().Count ();
        });
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
//static 
void  ColumnFactory::SortByColumnOrderInTable (std::vector<ECDbSqlColumn const*>& columns)
    {
    std::sort (columns.begin (), columns.end (),
        [] (ECDbSqlColumn const* a, ECDbSqlColumn const* b)
        {
        BeAssert (&a->GetTable () == &b->GetTable ());
        return a->GetTable ().IndexOf (*a) < b->GetTable ().IndexOf (*b);
        });
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ColumnFactory::ColumnFactory (ClassMapCR classMap)
    :m_classMap (classMap)
    {
    Update ();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::RegisterColumnInUse(ECDbSqlColumn const& column)
    {
    columnsInUseSet.insert(column.GetFullName());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Reset ()
    {
    columnsInUseSet.clear ();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse (Utf8CP columnFullName) const
    {
    return columnsInUseSet.find (columnFullName) != columnsInUseSet.end ();
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse (Utf8CP tableName, Utf8CP columnName) const
    {
    return IsColumnInUse (ECDbSqlColumn::BuildFullName (tableName, columnName).c_str ());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::IsColumnInUse (ECDbSqlColumn const& column) const
    {
    return IsColumnInUse (column.GetFullName ().c_str ());
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
void ColumnFactory::Update ()
    {
    std::vector<ECDbSqlColumn const*> columnsInUse;
    Reset ();
    m_classMap.GetPropertyMaps ().Traverse (
        [&] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        propMap->GetColumns (columnsInUse);
        feedback = TraversalFeedback::Next;
        }, true);

    for (auto columnInUse : columnsInUse)
        {
        if (columnInUse)
            RegisterColumnInUse (*columnInUse);
        }
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
bool ColumnFactory::FindReusableSharedDataColumns (std::vector<ECDbSqlColumn const*>& columns, ECDbSqlTable const& table, ECDbSqlColumn::Constraint::Collation collation, ColumnFactory::SortBy sortby) const
    {
    for (auto column : table.GetColumns ())
        {
        if (column->GetKnownColumnId() == ECDbKnownColumns::DataColumn && column->GetType() == ECDbSqlColumn::Type::Any && collation == column->GetConstraint().GetCollation())
            {
            if (IsColumnInUse (*column) == false)
                columns.push_back (column);
            }
        }

    switch (sortby)
        {
        case SortBy::LeastUsedColumn:
            SortByLeastUsedColumnFirst (columns); break;
        case SortBy::LeftToRightColumnOrderInTable:
            SortByColumnOrderInTable (columns); break;
        case SortBy::MostUsedColumn:
            SortByMostUsedColumnFirst (columns); break;
        case SortBy::None:
            break;
        }

    return !columns.empty ();
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
const Utf8String ColumnFactory::Encode (Utf8StringCR acessString) const
    {
    Utf8String o;
    for (Utf8Char c : acessString)
        {
        if (c == '.')
            {
            o.append ("_");
            }
        else if (islower (c))
            {
            o+= c;
            }
        else
            {
            o.append(SqlPrintfString ("_%02x_", c));
            }
        }

    return o;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus ColumnFactory::ResolveColumnName (Utf8StringR resolvedColumName, ColumnFactory::Specification const& specifications, ECDbSqlTable& targetTable, ECN::ECClassId propertyLocalToClassId, int retryCount) const
    {
    if (retryCount > 0)
        {
        BeAssert (!resolvedColumName.empty ());
        resolvedColumName += SqlPrintfString ("%d", retryCount);
        return BentleyStatus::SUCCESS;
        }

    auto existingColumn = specifications.GetColumnName ().empty () ? nullptr : targetTable.FindColumnP (specifications.GetColumnName ().c_str ());
    if (existingColumn != nullptr && IsColumnInUse (*existingColumn))
        {
        switch (specifications.GetGenerateColumnNameOptions ())
            {
            case Specification::GenerateColumnNameOptions::NameBasedOnLetterFollowedByIntegerSequence:
                BeAssert (false && "Not implemented. It used for shared column only");
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnPropertyNameAndPropertyId:

                resolvedColumName.Sprintf ("%s_%lld", specifications.GetColumnName ().c_str (), specifications.GetPropertyMap ().GetProperty ().GetId ());
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnClassIdAndCaseSaveAccessString:
                {
                auto const& prefix = Encode(specifications.GetAccessString ());
                resolvedColumName.Sprintf ("C%lld_%s", propertyLocalToClassId, prefix.c_str());
                }
                break;
            case Specification::GenerateColumnNameOptions::NameBasedOnClassAndPropertyName:
                resolvedColumName = specifications.GetPropertyMap ().GetProperty ().GetClass ().GetName ().c_str ();
                resolvedColumName.append ("_").append (specifications.GetColumnName ());
                break;
            case Specification::GenerateColumnNameOptions::NeverGenerate:
                return BentleyStatus::ERROR;
            }
        }
    else
        {
        resolvedColumName = specifications.GetColumnName ();
        }

    return BentleyStatus::SUCCESS;
    }
//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateStrategy (ColumnFactory::Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    Utf8String resolvedColumnName, tmp;
    int retryCount = 0;
    if (ResolveColumnName (tmp, specifications, targetTable, propertyLocalToClassId, retryCount) == BentleyStatus::ERROR)
        {
        return nullptr;
        }

    resolvedColumnName = tmp;
    while (targetTable.FindColumnP (resolvedColumnName.c_str ()) != nullptr)
        {
        retryCount++;
        resolvedColumnName = tmp;
        if (ResolveColumnName (resolvedColumnName, specifications, targetTable, propertyLocalToClassId, retryCount) == BentleyStatus::ERROR)
            {
            return nullptr;
            }

        }

    auto canEdit = targetTable.GetEditHandle ().CanEdit ();
    if (!canEdit)
        targetTable.GetEditHandleR ().BeginEdit ();

    auto newColumn = targetTable.CreateColumn (resolvedColumnName.c_str (), specifications.GetColumnType (), specifications.GetKnownColumnId (), specifications.GetColumnPersistenceType ());
    if (newColumn == nullptr)
        {
        BeAssert (false && "Failed to create column");
        return nullptr;
        }

    newColumn->GetConstraintR ().SetIsNotNull (specifications.IsNotNull ());
    newColumn->GetConstraintR ().SetIsUnique (specifications.IsUnique ());
    newColumn->GetConstraintR ().SetCollation (specifications.GetCollation ());
   // newColumn->GetDependentPropertiesR ().Add (propertyLocalToClassId, specifications.GetAccessString ().c_str ());

    if (!canEdit)
        targetTable.GetEditHandleR ().EndEdit ();

    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//-----------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateOrReuseStrategy (Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    auto existingColumn = specifications.GetColumnName ().empty () ? nullptr : targetTable.FindColumnP (specifications.GetColumnName ().c_str ());
    if (existingColumn != nullptr && !IsColumnInUse (*existingColumn))
        {
        if (ECDbSqlHelper::IsCompatible (existingColumn->GetType (), specifications.GetColumnType ()))
            {
            if (GetTable ().GetOwnerType () == OwnerType::ECDb)
                {
                if (existingColumn->GetConstraint ().IsNotNull () != specifications.IsNotNull () || existingColumn->GetConstraint ().IsUnique () != specifications.IsUnique () || existingColumn->GetConstraint ().GetCollation () != specifications.GetCollation ())
                    {
                    LOG.warningv ("Column %s in table %s is used by multiple property maps where property name and data type matches,"
                        " but where 'Nullable', 'Unique', or 'Collation' differs, and which will therefore be ignored for some of the properties.",
                        existingColumn->GetName ().c_str (), GetTable ().GetName ().c_str ());

                    BeAssert (false && "A column is used by multiple property maps where property name and data type matches, "
                        " but where 'Nullable', 'Unique', or 'Collation' differs.");

                    return ApplyCreateStrategy (specifications, targetTable, propertyLocalToClassId);
                    }
                }

            return existingColumn;
            }
        }

    return ApplyCreateStrategy (specifications, targetTable, propertyLocalToClassId);
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::ApplyCreateOrReuseSharedColumnStrategy (Specification const& specifications, ECDbSqlTable& targetTable, ECClassId propertyLocalToClassId)
    {
    std::vector<ECDbSqlColumn const*> avaliableSharedColumns;
    if (FindReusableSharedDataColumns (avaliableSharedColumns, targetTable, specifications.GetCollation (), SortBy::LeastUsedColumn))
        {
        auto sharedColumn = const_cast<ECDbSqlColumn*>(avaliableSharedColumns.front ());
        //sharedColumn->GetDependentPropertiesR ().Add (propertyLocalToClassId, specifications.GetAccessString ().c_str ());
        return sharedColumn;
        }

    auto newColumn = targetTable.CreateColumn (nullptr, ECDbSqlColumn::Type::Any, specifications.GetKnownColumnId (), specifications.GetColumnPersistenceType ());
    if (newColumn == nullptr)
        {
        BeAssert (false && "Failed to create column");
        return nullptr;
        }

    //newColumn->GetConstraintR ().SetIsNotNull (specifications.IsNotNull ()); -- NOT NULL IS NOT SUPPORTED ON SHARED COLUMNS
    newColumn->GetConstraintR ().SetCollation (specifications.GetCollation ());
    //auto uniqueIndex = newColumn->GetTableR ().CreateIndex ((newColumn->GetFullName () + "_UNIQUE").c_str ());
    //uniqueIndex->SetIsUnique (true);
    //uniqueIndex->SetWhereExpression (SqlPrintfString ("WHERE ECClass == %lld", persistenceClassId));

    //newColumn->GetDependentPropertiesR ().Add (propertyLocalToClassId, specifications.GetAccessString ().c_str ());
    return newColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECClassId ColumnFactory::GetPersistenceClassId (Specification const& specifications) const
    {
    ECClassId propertyLocalToClassId;
    auto n = specifications.GetAccessString ().find (".");
    if (n != Utf8String::npos)
        {
        //! Get root property in given accessString.
        auto propertyP = m_classMap.GetClass ().GetPropertyP (specifications.GetAccessString ().substr (0, n).c_str ());
        if (propertyP == nullptr)
            {
            BeAssert (false && "Failed to find root property");
            return 0;
            }

        propertyLocalToClassId = propertyP->GetClass ().GetId ();
        }
    else
        {
        auto propertyP = m_classMap.GetClass ().GetPropertyP (specifications.GetAccessString ().c_str ());
        if (propertyP == nullptr)
            {
            BeAssert (false && "Failed to find root property");
            return 0;
            }

        propertyLocalToClassId = propertyP->GetClass ().GetId ();
        }

    return propertyLocalToClassId;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::Configure (Specification const& specifications, ECDbSqlTable& targetTable)
    {


    ECClassId persistenceClassId = GetPersistenceClassId (specifications);
    if (persistenceClassId == 0)
        return nullptr;

    ECDbSqlColumn* outColumn = nullptr;
    switch (specifications.GetStrategy ())
        {
        case Specification::Strategy::Create:
            outColumn = ApplyCreateStrategy (specifications, targetTable, persistenceClassId); break;
        case Specification::Strategy::CreateOrReuse:
            outColumn = ApplyCreateOrReuseStrategy (specifications, targetTable, persistenceClassId); break;
        case Specification::Strategy::CreateOrReuseSharedColumn:
            outColumn = ApplyCreateOrReuseSharedColumnStrategy (specifications, targetTable, persistenceClassId); break;
        }

    Utf8String const& className = m_classMap.GetClass ().GetName ();
    if (outColumn)
        LOG.tracev ("Property -> '%s:%s' mapped to '%s'", className.c_str(), specifications.GetAccessString ().c_str (), outColumn->GetName ().c_str ());
    else
        LOG.tracev ("Property -> '%s:%s' mapped to 'NULL'", className.c_str (), specifications.GetAccessString ().c_str ());

    RegisterColumnInUse (*outColumn);
    return outColumn;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlColumn* ColumnFactory::Configure (Specification const& specifications)
    {
    return Configure (specifications, m_classMap.GetTable ());
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       01 / 2015
//------------------------------------------------------------------------------------------
ECDbSqlTable& ColumnFactory::GetTable ()  { return m_classMap.GetTable (); }


//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
IClassMap const& PropertyMapSet::GetClassMap () const { return m_classMap; }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
const PropertyMapSet::EndPoints  PropertyMapSet::GetEndPoints () const
    {
    EndPoints endPoints;
    for (auto const& endPoint : m_orderedEndPoints)
        endPoints.push_back (endPoint.get ());

    return endPoints;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
const PropertyMapSet::EndPoints PropertyMapSet::FindEndPoints (ECDbKnownColumns filter) const
    {
    EndPoints endPoints;
    for (auto const& endPoint : m_orderedEndPoints)
        {
        if (Enum::In (endPoint->GetKnownColumnId (), filter))
            endPoints.push_back (endPoint.get ());
        }

    return endPoints;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
BentleyStatus PropertyMapSet::AddSystemEndPoint (PropertyMapSet& propertySet, IClassMap const& classMap, ECDbKnownColumns knownColumnId, ECValueCR value, ECDbSqlColumn const* column)
    {
    auto const& table = classMap.GetTable ();
    if (column == nullptr)
        column = table.GetFilteredColumnFirst (knownColumnId);

    auto const accessString = ECDbSqlColumn::ToAccessString (knownColumnId);
    if (value.IsNull ())
        {
        BeAssert (column != nullptr);
        if (column == nullptr)
            return BentleyStatus::ERROR;
        }

    BeAssert (accessString != nullptr);
    if (accessString == nullptr)
        return BentleyStatus::ERROR;

    if (column == nullptr)
        propertySet.m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint (accessString, knownColumnId, value)));
    else
        propertySet.m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint (accessString, *column, value)));

    return BentleyStatus::SUCCESS;
    }

//------------------------------------------------------------------------------------------
//@bsimethod                                                    Affan.Khan       08 / 2015
//------------------------------------------------------------------------------------------
PropertyMapSet::Ptr PropertyMapSet::Create (IClassMap const& classMap)
    {
    Ptr propertySet = Ptr (new PropertyMapSet (classMap));
    ECValue defaultValue;
    AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::ECInstanceId, defaultValue);
    AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::ECClassId, ECValue (classMap.GetClass ().GetId ()));
    if (classMap.GetClass ().GetIsStruct ())
        {
        AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::ParentECInstanceId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::ECPropertyPathId, defaultValue);
        AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::ECArrayIndex, defaultValue);
        }

    if (classMap.GetClassMapType () == IClassMap::Type::RelationshipLinkTable ||
        classMap.GetClassMapType () == IClassMap::Type::RelationshipEndTable)
        {
        RelationshipClassMapCR const& relationshipMap = static_cast<RelationshipClassMapCR>(classMap);
        auto const& sourceConstraints = relationshipMap.GetRelationshipClass ().GetSource ().GetClasses ();
        auto const& targetConstraints = relationshipMap.GetRelationshipClass ().GetTarget ().GetClasses ();
        auto sourceECInstanceIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn ();
        auto sourceECClassIdColumn = relationshipMap.GetSourceECInstanceIdPropMap ()->GetFirstColumn ();
        auto targetECInstanceIdColumn = relationshipMap.GetTargetECInstanceIdPropMap ()->GetFirstColumn ();
        auto targetECClassIdColumn = relationshipMap.GetTargetECClassIdPropMap ()->GetFirstColumn ();


        AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::SourceECInstanceId, defaultValue, sourceECInstanceIdColumn);
        auto sourceConstraintClass = sourceConstraints.at (0);
        if (!IClassMap::IsAnyClass (*sourceConstraintClass) && sourceConstraints.size () == 1)
            AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::SourceECClassId, ECValue (sourceConstraintClass->GetId ()), sourceECClassIdColumn);
        else
            AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::SourceECClassId, defaultValue, sourceECClassIdColumn);

        AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::TargetECInstanceId, defaultValue, targetECInstanceIdColumn);
        auto targetConstraintClass = targetConstraints.at (0);
        if (!IClassMap::IsAnyClass (*targetConstraintClass) && targetConstraints.size () == 1)
            AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::TargetECClassId, ECValue (targetConstraintClass->GetId ()), targetECClassIdColumn);
        else
            AddSystemEndPoint (*propertySet, classMap, ECDbKnownColumns::SourceECClassId, defaultValue, targetECClassIdColumn);
        }

    classMap.GetPropertyMaps ().Traverse ([&propertySet] (TraversalFeedback& feedback, PropertyMapCP propMap)
        {
        if (auto pm = dynamic_cast<PropertyMapPoint const*> (propMap))
            {
            std::vector<ECDbSqlColumn const*> columns;
            pm->GetColumns (columns);
            Utf8String  baseAccessString = pm->GetPropertyAccessString ();
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".X").c_str (), *columns[0], ECValue ())));
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".Y").c_str (), *columns[1], ECValue ())));
            if (pm->Is3d ())
                propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint ((baseAccessString + ".Z").c_str (), *columns[2], ECValue ())));
            }
        else if (auto pm = dynamic_cast<PropertyMapToTable const*> (propMap))
            {
            feedback = TraversalFeedback::NextSibling;
            }
        else
            {
            propertySet->m_orderedEndPoints.push_back (std::unique_ptr<EndPoint> (new EndPoint (propMap->GetPropertyAccessString (), *propMap->GetFirstColumn (), ECValue ())));
            }
        feedback = TraversalFeedback::Next;
        }, true);

    return propertySet;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

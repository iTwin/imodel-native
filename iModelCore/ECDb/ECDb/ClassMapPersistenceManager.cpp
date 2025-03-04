/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool DbMapSaveContext::IsAlreadySaved(ClassMapCR classMap) const { return m_savedClassMaps.find(classMap.GetClass().GetId()) != m_savedClassMaps.end(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbMapSaveContext::BeginSaving(ClassMapCR classMap)
    {
    if (IsAlreadySaved(classMap))
        return;

    m_savedClassMaps[classMap.GetClass().GetId()] = &classMap;
    m_editStack.push(&classMap);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DbMapSaveContext::EndSaving(ClassMapCR classMap)
    {
    if (m_editStack.top() == &classMap)
        {
        m_editStack.pop();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbClassMapSaveContext::DbClassMapSaveContext(DbMapSaveContext& ctx)
    :m_classMapContext(ctx), m_classMap(*ctx.GetCurrent())
    {
    BeAssert(ctx.GetCurrent() != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapSaveContext::TryPropertyMapExists(uint64_t& propertyMapId, ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId)
    {
    PropertyPathId propertyPathId;
    if (m_classMapContext.TryGetPropertyPathId(propertyPathId, rootPropertyId, accessString, true) != SUCCESS)
        return ERROR;

    BeAssert(propertyPathId.IsValid());
    auto sql = columnId.IsValid() ?
        "SELECT Id FROM main.ec_PropertyMap WHERE ClassId=? AND PropertyPathId=? AND ColumnId=?" :
        "SELECT Id FROM main.ec_PropertyMap WHERE ClassId=? AND PropertyPathId=?";

    CachedStatementPtr stmt = GetMapSaveContext().GetECDb().GetImpl().GetCachedSqliteStatement(sql);
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, m_classMap.GetClass().GetId()) ||
        BE_SQLITE_OK != stmt->BindId(2, propertyPathId))
        {
        BeAssert(false);
        return ERROR;
        }

    if (columnId.IsValid()) {
        if (BE_SQLITE_OK != stmt->BindId(3, columnId))
        {
        BeAssert(false);
        return ERROR;
        }
    }
    auto rc = stmt->Step();
    switch(rc)
        {
        case BE_SQLITE_ROW:
            propertyMapId = stmt->GetValueUInt64(0);
            return SUCCESS;
        case BE_SQLITE_DONE:
            propertyMapId = 0;
            return SUCCESS;
        default:
            return ERROR;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapSaveContext::InsertPropertyMap(ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId)
    {
    PropertyPathId propertyPathId;
    if (m_classMapContext.TryGetPropertyPathId(propertyPathId, rootPropertyId, accessString, true) != SUCCESS)
        return ERROR;

    BeAssert(propertyPathId.IsValid());
    BeAssert(columnId.IsValid());

    CachedStatementPtr stmt = GetMapSaveContext().GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_PropertyMap(ClassId,PropertyPathId,ColumnId,Id) VALUES(?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, m_classMap.GetClass().GetId()) ||
        BE_SQLITE_OK != stmt->BindId(2, propertyPathId) ||
        BE_SQLITE_OK != stmt->BindId(3, columnId) ||
        BE_SQLITE_OK != stmt->BindId(4, GetMapSaveContext().GetECDb().GetImpl().GetIdFactory().PropertyMap().NextId()))
        {
        BeAssert(false);
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapSaveContext::UpdatePropertyMapColumn(uint64_t propertyMapId, DbColumnId columnId)
    {

    BeAssert(propertyMapId != 0);
    BeAssert(columnId.IsValid());
    if (propertyMapId == 0)
        {
        BeAssert(false && "propertyMapId must be not zero");
        return ERROR;
        }
    CachedStatementPtr stmt = GetMapSaveContext().GetECDb().GetImpl().GetCachedSqliteStatement("UPDATE main.ec_PropertyMap SET ColumnId=? WHERE Id=?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, columnId) ||
        BE_SQLITE_OK != stmt->BindUInt64(2, propertyMapId))
        {
        BeAssert(false);
        return ERROR;
        }

    if (BE_SQLITE_DONE != stmt->Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DbMapSaveContext::InsertClassMap(ECClassId classId, MapStrategyExtendedInfo const& mapStrategyExtInfo)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_ClassMap(ClassId, MapStrategy, ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo) VALUES (?,?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, classId);
    stmt->BindInt(2, Enum::ToInt(mapStrategyExtInfo.GetStrategy()));
    if (mapStrategyExtInfo.IsTablePerHierarchy())
        {
        TablePerHierarchyInfo const& tphInfo = mapStrategyExtInfo.GetTphInfo();
        BeAssert(tphInfo.IsValid());
        if (tphInfo.GetShareColumnsMode() != TablePerHierarchyInfo::ShareColumnsMode::No)
            stmt->BindInt(3, Enum::ToInt(tphInfo.GetShareColumnsMode()));

        //uint32_t are persisted as int64 to not lose unsigned-ness
        if (!tphInfo.GetMaxSharedColumnsBeforeOverflow().IsNull())
            stmt->BindInt64(4, (int64_t) tphInfo.GetMaxSharedColumnsBeforeOverflow().Value());

        if (tphInfo.GetJoinedTableInfo() != JoinedTableInfo::None)
            stmt->BindInt(5, Enum::ToInt(tphInfo.GetJoinedTableInfo()));
        }

    const DbResult stat = stmt->Step();
    if (stat != BE_SQLITE_DONE)
        {
        BeAssert(false && "Failed to save classmap");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus DbMapSaveContext::TryGetPropertyPathId(PropertyPathId& id, ECN::ECPropertyId rootPropertyId, Utf8CP accessString, bool addIfDoesNotExist)
    {
    CachedStatementPtr stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("SELECT Id FROM main." TABLE_PropertyPath " WHERE RootPropertyId =? AND AccessString=?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }
    stmt->BindId(1, rootPropertyId);
    stmt->BindText(2, accessString, Statement::MakeCopy::No);

    if (stmt->Step() == BE_SQLITE_ROW)
        {
        id = stmt->GetValueId<PropertyPathId>(0);
        return SUCCESS;
        }

    if (!addIfDoesNotExist)
        return ERROR;

    stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_PropertyPath(RootPropertyId, AccessString, Id) VALUES(?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }

    stmt->BindId(1, rootPropertyId);
    stmt->BindText(2, accessString, Statement::MakeCopy::No);
    stmt->BindId(3, m_ecdb.GetImpl().GetIdFactory().PropertyPath().NextId());
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false);
        return ERROR;
        }

    id = DbUtilities::GetLastInsertedId<PropertyPathId>(m_ecdb);
    if (!id.IsValid())
        return ERROR;

    return SUCCESS;
    }

namespace
    {
    static std::vector<Utf8String> classIdList;
    Utf8String GetClassIdListAsString() { return BeStringUtilities::Join(classIdList, ","); }

    void CollectAllBaseClassIds(const TableSpaceSchemaManager& schemaManager, const ECClassId& classId)
        {
        auto stmt = schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT BaseClassId FROM [%s]." TABLE_ClassHierarchyCache " WHERE ClassId=?", schemaManager.GetTableSpace().GetName().c_str()).c_str());
        if (stmt == nullptr)
            {
            BeAssert(false && "Failed to get statement");
            return;
            }

        stmt->BindId(1, classId);
        while (stmt->Step() == BE_SQLITE_ROW)
            {
            const auto baseClassId = stmt->GetValueId<ECClassId>(0).ToString();
            if (std::find(classIdList.begin(), classIdList.end(), baseClassId) == classIdList.end())
                classIdList.push_back(baseClassId);
            }
        }

    bool IsMappingUnknownInBaseOrConstraintClasses(const TableSpaceSchemaManager& schemaManager, ECClassCR ecClass)
        {
        classIdList.clear();
        // Check if the class has any base class with unknown mapping
        CollectAllBaseClassIds(schemaManager, ecClass.GetId());
        
        // Check if the class has any relationship constraint class with unknown mapping
        if (const auto relClass = ecClass.GetRelationshipClassCP())
            {
            for (const auto constraintClass : relClass->GetSource().GetConstraintClasses())
                CollectAllBaseClassIds(schemaManager, constraintClass->GetId());
            for (auto& constraintClass : relClass->GetTarget().GetConstraintClasses())
                CollectAllBaseClassIds(schemaManager, constraintClass->GetId());
            }

        const auto stmt = schemaManager.GetECDb().GetImpl().GetCachedSqliteStatement(Utf8PrintfString(
            "SELECT c.Name, cm.MapStrategy FROM [%s]." TABLE_ClassMap " cm "
            "INNER JOIN [%s]." TABLE_Class " c ON c.Id = cm.ClassId "
            "WHERE cm.ClassId IN (%s)",
            schemaManager.GetTableSpace().GetName().c_str(), schemaManager.GetTableSpace().GetName().c_str(), BeStringUtilities::Join(classIdList, ",").c_str()).c_str());

        if (!stmt.IsValid())
            {
            BeAssert(false && "Failed to get statement");
            return false;
            }

        while (stmt->Step() == BE_SQLITE_ROW)
            {
            if (DbSchemaPersistenceManager::ToMapStrategy(stmt->GetValueInt(1)).IsNull())
                {
                LOG.warningv("The class '%s' has an unsupported map strategy (%d). Querying this class will return null values. The ECDb file might have been created by a new version of the software.",
                    stmt->GetValueText(0), stmt->GetValueInt(1));
                return true;
                }
            }
        return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::Load(DbClassMapLoadContext& loadContext, ClassMapLoadContext& ctx, ECDbCR ecdb, TableSpaceSchemaManager const& schemaManager, ECN::ECClassCR ecClass)
    {
    loadContext.m_classMapExists = false;
    CachedStatementPtr stmt = nullptr;
    if (schemaManager.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT MapStrategy,ShareColumnsMode,MaxSharedColumnsBeforeOverflow,JoinedTableInfo FROM main." TABLE_ClassMap " WHERE ClassId=?");
    else
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT MapStrategy,ShareColumnsMode,MaxSharedColumnsBeforeOverflow,JoinedTableInfo FROM [%s]." TABLE_ClassMap " WHERE ClassId=?", schemaManager.GetTableSpace().GetName().c_str()).c_str());

    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, ecClass.GetId());
    const DbResult stat = stmt->Step();
    switch (stat)
        {
            case BE_SQLITE_ROW:
                break;

            case BE_SQLITE_DONE:
                return SUCCESS; //does not exist yet -> m_isValid == false;

            default:
                return ERROR;
        }

    auto isMappingUnknown = false;
    Nullable<MapStrategy> mapStrategy = DbSchemaPersistenceManager::ToMapStrategy(stmt->GetValueInt(0));
    const auto isNewerECXmlVersion = ecClass.GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest);

    if (mapStrategy.IsNull())
        {
        if (isNewerECXmlVersion)
            {
            isMappingUnknown = true;
            LOG.warningv("The class '%s' has an unsupported map strategy (%d). Querying this class will return null values. The ECDb file might have been created by a new version of the software.", ecClass.GetFullName(), stmt->GetValueInt(0));
            }
        else
            {
            LOG.errorv("Failed to load class map for '%s'. It has an unsupported map strategy (%d). The ECDb file might have been created by a new version of the software.", ecClass.GetFullName(), stmt->GetValueInt(0));
            return ERROR;
            }
        }

    if (isNewerECXmlVersion)
        {
        if (isMappingUnknown || IsMappingUnknownInBaseOrConstraintClasses(schemaManager, ecClass))
            {
            loadContext.m_mapStrategyExtInfo = MapStrategyExtendedInfo(MapStrategy::UnsupportedByECVersion);
    
            if (ReadPropertyMaps(loadContext, ecdb, schemaManager, ecClass.GetId()) != SUCCESS)
                return ERROR;
            loadContext.m_classMapExists = true;
            return SUCCESS;
            }

        // Check if the class has any navigation properties in which the other end class possibly has an unknown mapping strategy
        const auto tblspaceName = schemaManager.GetTableSpace().GetName();
        const auto classIdList = GetClassIdListAsString();

        const auto relConstraintCheckStmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString(
            "SELECT rcc.ClassId AS ConstraintClassId, p.Name "
            "   FROM [%s]." TABLE_RelationshipConstraintClass " rcc "
            "       INNER JOIN [%s]." TABLE_RelationshipConstraint " rc ON rc.Id = rcc.ConstraintId "
            "       INNER JOIN [%s]." TABLE_Property " p ON rc.RelationshipClassId = p.NavigationRelationshipClassId "
            "WHERE p.ClassId IN (%s) AND ConstraintClassId NOT IN (%s) AND p.NavigationRelationshipClassId IS NOT NULL", 
            tblspaceName.c_str(), tblspaceName.c_str(), tblspaceName.c_str(), classIdList.c_str(), classIdList.c_str()).c_str());

        if (!relConstraintCheckStmt.IsValid())
            return ERROR;

        while (relConstraintCheckStmt->Step() == BE_SQLITE_ROW)
            {            
            if (const auto constraintClass = schemaManager.GetClass(relConstraintCheckStmt->GetValueId<ECClassId>(0)); constraintClass != nullptr)
                {
                if (IsMappingUnknownInBaseOrConstraintClasses(schemaManager, *constraintClass))
                    loadContext.AddPropertyToIgnoreList(relConstraintCheckStmt->GetValueText(1));
                }
            }
        }

    if (mapStrategy == MapStrategy::TablePerHierarchy)
        {
        Nullable<TablePerHierarchyInfo::ShareColumnsMode> shareColumnsMode(TablePerHierarchyInfo::ShareColumnsMode::No);
        if (!stmt->IsColumnNull(1))
            shareColumnsMode = DbSchemaPersistenceManager::ToShareColumnsMode(stmt->GetValueInt(1));

        Nullable<uint32_t> maxSharedColumnsBeforeOverflow;
        //uint32_t are persisted as int64 to not lose unsigned-ness
        if (!stmt->IsColumnNull(2))
            maxSharedColumnsBeforeOverflow = Nullable<uint32_t>((uint32_t) stmt->GetValueInt64(2));

        Nullable<JoinedTableInfo> joinedTableInfo(JoinedTableInfo::None);
        if (!stmt->IsColumnNull(3))
            joinedTableInfo = DbSchemaPersistenceManager::ToJoinedTableInfo(stmt->GetValueInt(3));

        if (shareColumnsMode.IsNull() || joinedTableInfo.IsNull())
            {
            LOG.errorv("Failed to load class map for '%s'. It has an unsupported value for ShareColumnsMode or JoinedTableInfo. The ECDb file might have been created by a new version of the software.",
                                           ecClass.GetFullName(), stmt->GetValueInt(0));
            return ERROR;
            }

        loadContext.m_mapStrategyExtInfo = MapStrategyExtendedInfo(mapStrategy.Value(), TablePerHierarchyInfo(shareColumnsMode.Value(), maxSharedColumnsBeforeOverflow, joinedTableInfo.Value()));
        }
    else
        {
        BeAssert(stmt->IsColumnNull(1) && stmt->IsColumnNull(2) && stmt->IsColumnNull(3) &&
                 "ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo cols are expected to be NULL if MapStrategy is not TablePerHierarchy");
        loadContext.m_mapStrategyExtInfo = MapStrategyExtendedInfo(mapStrategy.Value());
        }

    stmt = nullptr; //to release the statement.
    if (ReadPropertyMaps(loadContext, ecdb, schemaManager, ecClass.GetId()) != SUCCESS)
        return ERROR;

    loadContext.m_classMapExists = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb, TableSpaceSchemaManager const& schemaManager, ECClassId classId)
    {
    ctx.m_columnByAccessString.clear();
    CachedStatementPtr stmt = nullptr;
    if (schemaManager.IsMain())
        stmt = ecdb.GetImpl().GetCachedSqliteStatement("SELECT T.Name, C.Name, A.AccessString"
                                                      " FROM main.ec_PropertyMap P"
                                                      " INNER JOIN main.ec_Column C ON C.Id = P.ColumnId"
                                                      " INNER JOIN main.ec_Table T ON T.Id = C.TableId"
                                                      " INNER JOIN main.ec_PropertyPath A ON A.Id = P.PropertyPathId"
                                                      " WHERE P.ClassId = ? ORDER BY T.Name");
    else
        {
        Utf8CP tableSpace = schemaManager.GetTableSpace().GetName().c_str();
        stmt = ecdb.GetImpl().GetCachedSqliteStatement(Utf8PrintfString("SELECT T.Name, C.Name, A.AccessString"
                                                                        " FROM [%s].ec_PropertyMap P"
                                                                        " INNER JOIN [%s].ec_Column C ON C.Id = P.ColumnId"
                                                                        " INNER JOIN [%s].ec_Table T ON T.Id = C.TableId"
                                                                        " INNER JOIN [%s].ec_PropertyPath A ON A.Id = P.PropertyPathId"
                                                                        " WHERE P.ClassId = ? ORDER BY T.Name", tableSpace,
                                                                        tableSpace, tableSpace, tableSpace).c_str());
        }

    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, classId);
    const auto isMappingUnknown = ctx.GetMapStrategy().GetStrategy() == MapStrategy::UnsupportedByECVersion;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP tableName = stmt->GetValueText(0);
        Utf8CP columName = stmt->GetValueText(1);
        Utf8CP accessString = stmt->GetValueText(2);

        DbTable const* table = schemaManager.GetDbSchema().FindTable(tableName);
        if (table == nullptr)
            {
            if (isMappingUnknown)
                continue;
            return ERROR;
            }

        DbColumn const* column = table->FindColumn(columName);
        if (column == nullptr)
            {
            if (isMappingUnknown)
                continue;
            return ERROR;
            }

        ctx.m_columnByAccessString[accessString].push_back(column);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> const* DbClassMapLoadContext::FindColumnByAccessString(Utf8StringCR accessString) const
    {
    auto itor = m_columnByAccessString.find(accessString);
    if (itor != m_columnByAccessString.end())
        return &(itor->second);

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

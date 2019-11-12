/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool DbMapSaveContext::IsAlreadySaved(ClassMapCR classMap) const { return m_savedClassMaps.find(classMap.GetClass().GetId()) != m_savedClassMaps.end(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void DbMapSaveContext::BeginSaving(ClassMapCR classMap)
    {
    if (IsAlreadySaved(classMap))
        return;

    m_savedClassMaps[classMap.GetClass().GetId()] = &classMap;
    m_editStack.push(&classMap);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void DbMapSaveContext::EndSaving(ClassMapCR classMap)
    {
    if (m_editStack.top() == &classMap)
        {
        m_editStack.pop();
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
DbClassMapSaveContext::DbClassMapSaveContext(DbMapSaveContext& ctx)
    :m_classMapContext(ctx), m_classMap(*ctx.GetCurrent())
    {
    BeAssert(ctx.GetCurrent() != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DbClassMapSaveContext::InsertPropertyMap(ECPropertyId rootPropertyId, Utf8CP accessString, DbColumnId columnId)
    {
    PropertyPathId propertyPathId;
    if (m_classMapContext.TryGetPropertyPathId(propertyPathId, rootPropertyId, accessString, true) != SUCCESS)
        return ERROR;

    BeAssert(propertyPathId.IsValid());
    BeAssert(columnId.IsValid());

    CachedStatementPtr stmt = GetMapSaveContext().GetECDb().GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_PropertyMap(ClassId,PropertyPathId,ColumnId) VALUES(?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, m_classMap.GetClass().GetId()) ||
        BE_SQLITE_OK != stmt->BindId(2, propertyPathId) ||
        BE_SQLITE_OK != stmt->BindId(3, columnId))
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
// @bsimethod                                                    Affan.Khan        10/2014
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
// @bsimethod                                                    Affan.Khan        10/2014
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

    stmt = m_ecdb.GetImpl().GetCachedSqliteStatement("INSERT INTO main.ec_PropertyPath(RootPropertyId, AccessString) VALUES(?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }

    stmt->BindId(1, rootPropertyId);
    stmt->BindText(2, accessString, Statement::MakeCopy::No);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
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

    Nullable<MapStrategy> mapStrategy = DbSchemaPersistenceManager::ToMapStrategy(stmt->GetValueInt(0));
    if (mapStrategy.IsNull())
        {
        LOG.errorv("Failed to load class map for '%s'. It has an unsupported map strategy (%d). The ECDb file might have been created by a new version of the software.",
                                       ecClass.GetFullName(), stmt->GetValueInt(0));
        return ERROR;
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
// @bsimethod                                                    Affan.Khan        10/2014
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
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP tableName = stmt->GetValueText(0);
        Utf8CP columName = stmt->GetValueText(1);
        Utf8CP accessString = stmt->GetValueText(2);

        DbTable const* table = schemaManager.GetDbSchema().FindTable(tableName);
        if (table == nullptr)
            return ERROR;

        DbColumn const* column = table->FindColumn(columName);
        if (column == nullptr)
            return ERROR;

        ctx.m_columnByAccessString[accessString].push_back(column);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> const* DbClassMapLoadContext::FindColumnByAccessString(Utf8StringCR accessString) const
    {
    auto itor = m_columnByAccessString.find(accessString);
    if (itor != m_columnByAccessString.end())
        return &(itor->second);

    return nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

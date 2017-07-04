/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapPersistenceManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool DbMapSaveContext::IsAlreadySaved(ClassMapCR classMap) const
    {
    return m_savedClassMaps.find(classMap.GetClass().GetId()) != m_savedClassMaps.end();
    }

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

    CachedStatementPtr stmt = GetMapSaveContext().GetECDb().GetCachedStatement("INSERT INTO ec_PropertyMap(Id,ClassId,PropertyPathId,ColumnId) VALUES(?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    BeInt64Id pmId;
    if (GetMapSaveContext().GetECDb().GetImpl().GetSequence(IdSequences::Key::PropertyMapId).GetNextValue(pmId))
        return ERROR;

    if (BE_SQLITE_OK != stmt->BindId(1, pmId) ||
        BE_SQLITE_OK != stmt->BindId(2, m_classMap.GetClass().GetId()) ||
        BE_SQLITE_OK != stmt->BindId(3, propertyPathId) ||
        BE_SQLITE_OK != stmt->BindId(4, columnId))
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_ClassMap(ClassId, MapStrategy, ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo) VALUES (?,?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, classId);
    stmt->BindInt(2, Enum::ToInt(mapStrategyExtInfo.GetStrategy()));
    if (mapStrategyExtInfo.GetStrategy() == MapStrategy::TablePerHierarchy)
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
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_PropertyPath WHERE RootPropertyId =? AND AccessString = ?");
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

    if (m_ecdb.GetImpl().GetSequence(IdSequences::Key::PropertyPathId).GetNextValue(id) != BE_SQLITE_OK)
        {
        BeAssert(false);
        return ERROR;
        }

    stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_PropertyPath(Id, RootPropertyId, AccessString) VALUES(?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement");
        return ERROR;
        }
    stmt->BindId(1, id);
    stmt->BindId(2, rootPropertyId);
    stmt->BindText(3, accessString, Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_DONE)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::Load(DbClassMapLoadContext& loadContext, ClassMapLoadContext& ctx, ECDbCR ecdb, ECN::ECClassCR ecClass)
    {
    loadContext.m_isValid = false;
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT MapStrategy,ShareColumnsMode,MaxSharedColumnsBeforeOverflow,JoinedTableInfo FROM ec_ClassMap WHERE ClassId=?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, ecClass.GetId());
    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    const MapStrategy mapStrategy = Enum::FromInt<MapStrategy>(stmt->GetValueInt(0));
    if (mapStrategy == MapStrategy::TablePerHierarchy)
        {
        const TablePerHierarchyInfo::ShareColumnsMode shareColumnsMode = stmt->IsColumnNull(1) ? TablePerHierarchyInfo::ShareColumnsMode::No : Enum::FromInt<TablePerHierarchyInfo::ShareColumnsMode>(stmt->GetValueInt(1));
        Nullable<uint32_t> maxSharedColumnsBeforeOverflow;
        //uint32_t are persisted as int64 to not lose unsigned-ness
        if (!stmt->IsColumnNull(2))
            maxSharedColumnsBeforeOverflow = Nullable<uint32_t>((uint32_t) stmt->GetValueInt64(2));

        const JoinedTableInfo joinedTableInfo = stmt->IsColumnNull(3) ? JoinedTableInfo::None : Enum::FromInt<JoinedTableInfo>(stmt->GetValueInt(3));
        loadContext.m_mapStrategyExtInfo = MapStrategyExtendedInfo(TablePerHierarchyInfo(shareColumnsMode, maxSharedColumnsBeforeOverflow, joinedTableInfo));
        }
    else
        {
        BeAssert(stmt->IsColumnNull(1) && stmt->IsColumnNull(2) && stmt->IsColumnNull(3) &&
                 "ShareColumnsMode, MaxSharedColumnsBeforeOverflow, JoinedTableInfo cols are expected to be NULL if MapStrategy is not TablePerHierarchy");
        loadContext.m_mapStrategyExtInfo = MapStrategyExtendedInfo(mapStrategy);
        }

    stmt = nullptr; //to release the statement.
    if (ReadPropertyMaps(loadContext, ecdb, ecClass.GetId()) != SUCCESS)
        return ERROR;

    loadContext.m_isValid = true;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb, ECClassId classId)
    {
    ctx.m_columnByAccessString.clear();
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT T.Name, C.Name, A.AccessString"
                                                      " FROM ec_PropertyMap P"
                                                      "     INNER JOIN ec_Column C ON C.Id = P.ColumnId"
                                                      "     INNER JOIN ec_Table T ON T.Id = C.TableId"
                                                      "     INNER JOIN ec_PropertyPath A ON A.Id = P.PropertyPathId"
                                                      " WHERE P.ClassId = ? ORDER BY T.Name");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, classId);
    DbSchema const& dbSchema = ecdb.Schemas().GetDbMap().GetDbSchema();

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP tableName = stmt->GetValueText(0);
        Utf8CP columName = stmt->GetValueText(1);
        Utf8CP accessString = stmt->GetValueText(2);

        DbTable const* table = dbSchema.FindTable(tableName);
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

/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapPersistenceManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  08/2016
//---------------------------------------------------------------------------------------
//static
/*BentleyStatus ClassMapPersistenceManager::Save(SaveContext& ctx, ClassMapCR classMap)
    {
    if (ctx.IsAlreadySaved(classMap) || classMap.GetId().IsValid())
        return SUCCESS;

    ClassMapId baseClassMapid;
    std::set<PropertyMapCP> baseProperties;
    ctx.BeginSaving(classMap);
    if (classMap.GetBaseClassId().IsValid())
        {
        ECClassCP baseClass = ctx.GetECDb().Schemas().GetECClass(classMap.GetBaseClassId());
        if (baseClass == nullptr)
            {
            BeAssert(false && "Failed to find baseclass");
            return ERROR;
            }

        ClassMap* baseClassMap = const_cast<ClassMap*> (ctx.GetECDbMap().GetClassMap(*baseClass));
        if (baseClassMap == nullptr)
            {
            BeAssert(false && "Failed to find baseClass map");
            return ERROR;
            }

        if (!baseClassMap->GetId().IsValid())
            {
            if (SUCCESS != Save(ctx, *baseClassMap))
                return ERROR;
            }

        for (PropertyMapCP propertyMap : baseClassMap->GetPropertyMaps())
            baseProperties.insert(propertyMap);

        baseClassMapid = baseClassMap->GetId();
        }

    if (ctx.InsertClassMap(m_id, GetClass().GetId(), GetMapStrategy(), baseClassMapid) != SUCCESS)
        {
        m_id = ClassMapId();
        BeAssert(false);
        return ERROR;
        }

    DbClassMapSaveContext classMapSaveContext(ctx);
    for (PropertyMapCP propertyMap : GetPropertyMaps())
        {
        if (baseProperties.find(propertyMap) != baseProperties.end())
            continue;

        if (SUCCESS != propertyMap->Save(classMapSaveContext))
            return ERROR;
        }

    m_isDirty = false;
    ctx.EndSaving(*this);
    return SUCCESS;

    }

*/

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
    ECDbCR ecdb = GetMapSaveContext().GetECDb();
    CachedStatementPtr stmt = ecdb.GetCachedStatement("INSERT INTO ec_PropertyMap(ClassMapId, PropertyPathId, ColumnId) VALUES (?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, m_classMap.GetId());
    stmt->BindId(2, propertyPathId);
    stmt->BindId(3, columnId);
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
BentleyStatus DbMapSaveContext::InsertClassMap(ClassMapId& classMapId, ECClassId classId, ECDbMapStrategy const& mapStrategy, ClassMapId baseClassMapId)
    {
    CachedStatementPtr stmt = m_ecdb.GetCachedStatement("INSERT INTO ec_ClassMap(Id, ParentId, ClassId, MapStrategy, MapStrategyOptions, MapStrategyMinSharedColumnCount, MapStrategyAppliesToSubclasses) VALUES (?,?,?,?,?,?,?)");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    if (BE_SQLITE_OK != m_ecdb.GetECDbImplR().GetClassMapIdSequence().GetNextValue(classMapId))
        {
        BeAssert(false);
        return ERROR;
        }

    stmt->BindId(1, classMapId);
    if (!baseClassMapId.IsValid())
        stmt->BindNull(2);
    else
        stmt->BindId(2, baseClassMapId);

    stmt->BindId(3, classId);
    stmt->BindInt(4, Enum::ToInt(mapStrategy.GetStrategy()));
    stmt->BindInt(5, Enum::ToInt(mapStrategy.GetOptions()));
    const int minSharedColCount = mapStrategy.GetMinimumSharedColumnCount();
    if (minSharedColCount != ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT)
        stmt->BindInt(6, minSharedColCount);

    stmt->BindInt(7, mapStrategy.AppliesToSubclasses() ? 1 : 0);

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
#ifdef WIP_ECDB_PERF
    auto itor = m_properytPathCache.find(std::make_tuple(rootPropertyId, accessString));
    if (itor != m_properytPathCache.end())
        {
        id = (*itor).second;
        return SUCCESS;
        }
#endif
    auto stmt = m_ecdb.GetCachedStatement("SELECT Id FROM ec_PropertyPath  WHERE RootPropertyId =? AND AccessString = ?");
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

    if (m_ecdb.GetECDbImplR().GetPropertyPathIdSequence().GetNextValue(id) != BE_SQLITE_OK)
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

#ifdef WIP_ECDB_PERF
    m_properytPathCache.insert(std::make_pair(std::make_pair(rootPropertyId, accessString), id));
#endif

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
std::vector<DbColumn const*> const* DbClassMapLoadContext::FindColumnByAccessString(Utf8CP accessString) const
    {
    auto itor = m_columnByAccessString.find(accessString);
    if (itor != m_columnByAccessString.end())
        return &(itor->second);

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::ReadPropertyMaps(DbClassMapLoadContext& ctx, ECDbCR ecdb)
    {
    if (!ctx.GetClassMapId().IsValid())
        return ERROR;

    ctx.m_columnByAccessString.clear();
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT  T.Name TableName, C.Name ColumnName , A.AccessString"
                                                      " FROM ec_PropertyMap P"
                                                      "     INNER JOIN ec_Column C ON C.Id = P.ColumnId"
                                                      "     INNER JOIN ec_Table T ON T.Id = C.TableId"
                                                      "     INNER JOIN ec_PropertyPath A ON A.Id = P.PropertyPathId"
                                                      " WHERE P.ClassMapId = ? ORDER BY T.Name");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }

    stmt->BindId(1, ctx.GetClassMapId());
    DbSchema const& dbSchema = ecdb.GetECDbImplR().GetECDbMap().GetDbSchema();

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        Utf8CP  tableName = stmt->GetValueText(0);
        Utf8CP  columName = stmt->GetValueText(1);
        Utf8CP  accessString = stmt->GetValueText(2);

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
BentleyStatus DbClassMapLoadContext::SetBaseClassMap(ClassMapCR classMap)
    {
    if (classMap.GetClass().GetId() != m_baseClassId)
        {
        BeAssert(false);
        return ERROR;
        }

    m_baseClassMap = &classMap;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus DbClassMapLoadContext::Load(DbClassMapLoadContext& loadContext, ECDbCR ecdb, ECN::ECClassId classId)
    {
    loadContext.m_isValid = false;
    CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT A.Id, b.ClassId, A.MapStrategy, A.MapStrategyOptions, A.MapStrategyMinSharedColumnCount, A.MapStrategyAppliesToSubclasses FROM ec_ClassMap A LEFT JOIN  ec_ClassMap B ON B.Id = A.ParentId  WHERE A.ClassId = ?");
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to get statement");
        return ERROR;
        }
    stmt->BindId(1, classId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return ERROR;

    loadContext.m_baseClassMap = nullptr;
    loadContext.m_classMapId = stmt->GetValueId<ClassMapId>(0);
    loadContext.m_baseClassId = stmt->IsColumnNull(1) ? ECN::ECClassId() : stmt->GetValueId<ECN::ECClassId>(1);
    const int minSharedColCount = stmt->IsColumnNull(4) ? ECDbClassMap::MapStrategy::UNSET_MINIMUMSHAREDCOLUMNCOUNT : stmt->GetValueInt(5);
    if (SUCCESS != loadContext.m_mapStrategy.Assign(Enum::FromInt<ECDbMapStrategy::Strategy>(stmt->GetValueInt(2)),
                                                    Enum::FromInt<ECDbMapStrategy::Options>(stmt->GetValueInt(3)),
                                                    minSharedColCount,
                                                    stmt->GetValueInt(5) == 1))
        {
        BeAssert(false && "Found invalid persistence values for ECDbMapStrategy");
        return ERROR;
        }

    if (DbClassMapLoadContext::ReadPropertyMaps(loadContext, ecdb) != SUCCESS)
        return ERROR;

    loadContext.m_isValid = true;
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

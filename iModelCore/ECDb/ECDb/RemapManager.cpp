/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

void RemapManager::CollectRemapInfosFromNewProperty(ECN::ECClassCR ecClass, Utf8StringCR propertyName, ECN::ECPropertyId id)
    {
    auto classId = m_schemaManager.GetClassId(ecClass);
    auto& remapInfo = GetOrAddRemapInfo(classId, ecClass.GetName(), ecClass.GetSchema().GetName());
    remapInfo.TrackAddedProperty(propertyName, id);
    }

void RemapManager::CollectRemapInfosFromModifiedBaseClass(ECClassCR derivedClass, ECClassCR newBaseClass)
    {
    auto derivedClassId = m_schemaManager.GetClassId(derivedClass);
    auto& remapInfo = GetOrAddRemapInfo(derivedClassId, derivedClass.GetName(), derivedClass.GetSchema().GetName());
    auto baseClassId = m_schemaManager.GetClassId(newBaseClass);
    remapInfo.TrackAddedBaseClass(newBaseClass.GetName(), baseClassId, newBaseClass.GetSchema().GetName());
    }

void RemapManager::CollectRemapInfosFromDeletedPropertyOverride(ECN::ECPropertyId propertyId)
    {
    //ColumnKind 4 indicates shared columns
    Utf8CP sql = R"sqlstatement(
SELECT [rootSchema].[Name], [rootClass].[Name], [rootClass].[Id], [s].[Name], [c].[Name], [c].[Id], [ecp].[Id], [ecp].[Name], [pp].[AccessString], [tab].[Name], [col].[Name], [pm].[ClassId]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
    JOIN [ec_Class] [c] on [c].[Id] = [pm].[ClassId]
    JOIN [ec_Class] [rootClass] on [rootClass].[Id] = [ecp].[ClassId]
    JOIN [ec_Schema] [s] on [s].[Id] = [c].[SchemaId]
    JOIN [ec_Schema] [rootSchema] on [rootSchema].[Id] = [rootClass].[SchemaId]
WHERE [ecp].[Id] = ? AND [col].[ColumnKind] = 4
    )sqlstatement";

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(sql);
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement to collect remap infos for deleted property");
        return;
        }

    if (BE_SQLITE_OK != stmt->BindId(1, propertyId))
        {
        BeAssert(false);
        return;
        }

    while (stmt->Step() == BE_SQLITE_ROW)
        {
        ECClassId classId =  stmt->GetValueId<ECClassId>(11);

        auto& cleanedInfos = GetOrAddCleanedMappingInfo(classId);
        cleanedInfos.emplace_back();
        auto& cleanedInfo = cleanedInfos.back(); //in c++ 17 emplace_back() already returns the reference, but we don't fully support that yet in our build chain.


        cleanedInfo.m_rootSchemaName = stmt->GetValueText(0);
        cleanedInfo.m_rootClassName = stmt->GetValueText(1);
        cleanedInfo.m_rootClassId = stmt->GetValueId<ECClassId>(2); //refers to the class that defines the property
        cleanedInfo.m_schemaName = stmt->GetValueText(3);
        cleanedInfo.m_className = stmt->GetValueText(4);
        cleanedInfo.m_classId = classId;

        cleanedInfo.m_propertyId = propertyId;
        cleanedInfo.m_propertyName = stmt->GetValueText(7);
        cleanedInfo.m_accessString = stmt->GetValueText(8);
        cleanedInfo.m_tableName = stmt->GetValueText(9);
        cleanedInfo.m_columnName = stmt->GetValueText(10);
        cleanedInfo.m_fullOldColumnIdentifier = RemapManager::GetFullColumnIdentifier(cleanedInfo.m_tableName, cleanedInfo.m_columnName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus RemapManager::CleanModifiedMappings()
    {
    ECDB_PERF_LOG_SCOPE("Schema import> Clean modified property mappings");
    std::set<ECPropertyId> cleanedPropertyIds; //hold a unique list of rootPropertyIds that have been cleaned, so we can purge orphans later
    for (auto& [key, remapInfo] : m_remapInfos)
        {
        for(auto& addedProperty : remapInfo.m_addedProperties)
            {
            if(CleanAddedProperty(remapInfo, addedProperty, cleanedPropertyIds) != SUCCESS)
                return ERROR;
            }

        for(auto& addedBaseClass : remapInfo.m_addedBaseClasses)
            {
            if(CleanAddedBaseClass(remapInfo, addedBaseClass, cleanedPropertyIds) != SUCCESS)
                return ERROR;
            }
        }

    Utf8CP cleanPropertyPathSql = R"sqlstatement(
DELETE from [ec_PropertyPath]
WHERE [ec_PropertyPath].[RootPropertyId] = ? AND NOT EXISTS (SELECT 1 FROM [ec_PropertyMap] [pm] WHERE [pm].[PropertyPathId] = [ec_PropertyPath].[Id])
    )sqlstatement";

    CachedStatementPtr cleanPropertyPathStmt = m_ecdb.GetCachedStatement(cleanPropertyPathSql);
    if (cleanPropertyPathStmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement to clean orphan propertypath entries");
        return ERROR;
        }

    for(auto& cleanedPropertyId : cleanedPropertyIds)
        {
        cleanPropertyPathStmt->Reset();
        cleanPropertyPathStmt->ClearBindings();
        if (BE_SQLITE_OK != cleanPropertyPathStmt->BindId(1, cleanedPropertyId))
            {
            BeAssert(false);
            return ERROR;
            }

        if (BE_SQLITE_DONE != cleanPropertyPathStmt->Step())
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to clean property path orphans: %s",
                m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        int affectedRows = m_ecdb.GetModifiedRowCount();
        if(affectedRows >= 1)
            LOG.infov("Cleaned up %" PRIi32 " orphan row(s) in PropertyPath for property %d", affectedRows, cleanedPropertyId.GetValue());
        }

    return SUCCESS;
    }

BentleyStatus RemapManager::CleanAddedProperty(RemapInfosForClass& remapInfo, AddedProperty& addedProperty, std::set<ECPropertyId>& cleanedPropertyIds)
    { //clean that property by name in all of the derived classes
    return CleanPropertyDownTheHierarchyByName(remapInfo.m_classId, addedProperty.m_name, false, cleanedPropertyIds);
    }

BentleyStatus RemapManager::CleanPropertyDownTheHierarchyByName(ECClassId classId, Utf8StringCR propertyName, bool includeClassInCleanup, std::set<ECPropertyId>& cleanedPropertyIds)
    {
    //ColumnKind 4 indicates shared columns
    Utf8CP getInfoSql = R"sqlstatement(
SELECT [rootSchema].[Name], [rootClass].[Name], [rootClass].[Id], [s].[Name], [c].[Name], [c].[Id], [ecp].[Id], [ecp].[Name], [pp].[AccessString], [tab].[Name], [col].[Name], [pm].[ClassId]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
    JOIN [ec_Class] [c] on [c].[Id] = [pm].[ClassId]
    JOIN [ec_Class] [rootClass] on [rootClass].[Id] = [ecp].[ClassId]
    JOIN [ec_Schema] [s] on [s].[Id] = [c].[SchemaId]
    JOIN [ec_Schema] [rootSchema] on [rootSchema].[Id] = [rootClass].[SchemaId]
WHERE [ecp].[Name] = ? AND [col].[ColumnKind] = 4 AND [pm].[ClassId] IN (SELECT [ClassId] FROM [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?)
    )sqlstatement";

    Utf8CP getInfoIncludeClassInCleanupSql = R"sqlstatement(
SELECT [rootSchema].[Name], [rootClass].[Name], [rootClass].[Id], [s].[Name], [c].[Name], [c].[Id], [ecp].[Id], [ecp].[Name], [pp].[AccessString], [tab].[Name], [col].[Name], [pm].[ClassId]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
    JOIN [ec_Class] [c] on [c].[Id] = [pm].[ClassId]
    JOIN [ec_Class] [rootClass] on [rootClass].[Id] = [ecp].[ClassId]
    JOIN [ec_Schema] [s] on [s].[Id] = [c].[SchemaId]
    JOIN [ec_Schema] [rootSchema] on [rootSchema].[Id] = [rootClass].[SchemaId]
WHERE [ecp].[Name] = ?1 AND [col].[ColumnKind] = 4 AND [pm].[ClassId] IN (SELECT ?2 as [ClassId] UNION ALL SELECT [ClassId] FROM [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?2)
    )sqlstatement";

    CachedStatementPtr getInfoStmt = m_ecdb.GetCachedStatement(includeClassInCleanup ? getInfoIncludeClassInCleanupSql : getInfoSql);
    if (getInfoStmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement to obtain property maps for cleaning property by name");
        return ERROR;
        }

    Utf8CP deleteSql = R"sqlstatement(
DELETE FROM [ec_PropertyMap] WHERE [Id] IN(
SELECT [pm].[Id]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
WHERE [ecp].[Name] = ? AND [col].[ColumnKind] = 4 AND [pm].[ClassId] IN (SELECT [ClassId] FROM [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?))
    )sqlstatement";
    Utf8CP deleteIncludeClassSql = R"sqlstatement(
DELETE FROM [ec_PropertyMap] WHERE [Id] IN(
SELECT [pm].[Id]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
WHERE [ecp].[Name] = ? AND [col].[ColumnKind] = 4 AND [pm].[ClassId] IN (SELECT ?2 as [ClassId] UNION ALL SELECT [ClassId] FROM [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?2))
    )sqlstatement";
    CachedStatementPtr deleteStmt = m_ecdb.GetCachedStatement(includeClassInCleanup ? deleteIncludeClassSql : deleteSql);
    if (deleteStmt == nullptr)
        {
        BeAssert(false && "Failed to prepare delete statement for cleaning property by name");
        return ERROR;
        }

    if (BE_SQLITE_OK != getInfoStmt->BindText(1, propertyName, Statement::MakeCopy::No))
        {
        BeAssert(false);
        return ERROR;
        }
    if (BE_SQLITE_OK != getInfoStmt->BindId(2, classId))
        {
        BeAssert(false);
        return ERROR;
        }

    int results = 0;
    while (getInfoStmt->Step() == BE_SQLITE_ROW)
        {
        results++;
        ECClassId classId =  getInfoStmt->GetValueId<ECClassId>(11);
        ECPropertyId propertyId =  getInfoStmt->GetValueId<ECPropertyId>(6); //equals root property id

        cleanedPropertyIds.insert(propertyId);
        auto& cleanedInfos = GetOrAddCleanedMappingInfo(classId);
        cleanedInfos.emplace_back(); //in c++ 17 emplace_back() already returns the reference, but we don't fully support that yet in our build chain.
        auto& cleanedInfo = cleanedInfos.back();

        cleanedInfo.m_rootSchemaName = getInfoStmt->GetValueText(0);
        cleanedInfo.m_rootClassName = getInfoStmt->GetValueText(1);
        cleanedInfo.m_rootClassId = getInfoStmt->GetValueId<ECClassId>(2); //refers to the class that defines the property
        cleanedInfo.m_schemaName = getInfoStmt->GetValueText(3);
        cleanedInfo.m_className = getInfoStmt->GetValueText(4);
        cleanedInfo.m_classId = classId;

        cleanedInfo.m_propertyId = propertyId;
        cleanedInfo.m_propertyName = getInfoStmt->GetValueText(7);
        cleanedInfo.m_accessString = getInfoStmt->GetValueText(8);
        cleanedInfo.m_tableName = getInfoStmt->GetValueText(9);
        cleanedInfo.m_columnName = getInfoStmt->GetValueText(10);
        cleanedInfo.m_fullOldColumnIdentifier = RemapManager::GetFullColumnIdentifier(cleanedInfo.m_tableName, cleanedInfo.m_columnName);
        }

    if(results == 0)
        return SUCCESS; //no need to try and clean up mappings if there are none

    if (BE_SQLITE_OK != deleteStmt->BindText(1, propertyName, Statement::MakeCopy::No))
        {
        BeAssert(false);
        return ERROR;
        }
    if (BE_SQLITE_OK != deleteStmt->BindId(2, classId))
        {
        BeAssert(false);
        return ERROR;
        }

    if (BE_SQLITE_DONE != deleteStmt->Step())
        {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to clean mappings. Could not delete mapping for ECClass %s Property %s: %s",
            classId.ToString().c_str(), propertyName.c_str(), m_ecdb.GetLastError().c_str());
        return ERROR;
        }

    int affectedRows = m_ecdb.GetModifiedRowCount();
    if(affectedRows != results)
        {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cleaning up deleted %" PRIi32 " mapping(s) but expected to clean %" PRIi32 " for property %s.", affectedRows, results, propertyName.c_str());
        return ERROR;
        }

    LOG.infov("Cleaned up %" PRIi32 " mapping(s) for added property %s", affectedRows, propertyName.c_str());
    return SUCCESS;
    }

BentleyStatus RemapManager::CleanAddedBaseClass(RemapInfosForClass& remapInfo, AddedBaseClass& addedBaseClass, std::set<ECPropertyId>& cleanedPropertyIds)
    {
    //Parameters: 1 BaseClass Name, 2 BaseClass SchemaName, 3 DerivedClass Name, 4 DerivedClass SchemaName
    Utf8CP detectPropertiesToRemapSql = R"sqlstatement(
        WITH
    [baseClassColumns]([ColumnId]) AS(
        SELECT DISTINCT [cn].[Id]
        FROM   [ec_Property] [pt]
               JOIN [ec_Cache_ClassHierarchy] [ch] ON [ch].[ClassId] = [pt].[ClassId]
               JOIN [ec_Class] [baseCl] ON [baseCl].[Id] = [ch].[BaseClassId]
               JOIN [ec_Schema] [baseSchema] ON [baseSchema].[Id] = [baseCl].[SchemaId]
               JOIN [ec_Class] [cl] ON [cl].[Id] = [ch].[ClassId]
               JOIN [ec_Propertypath] [pp] ON [pp].[RootPropertyId] = [pt].[Id]
               JOIN [ec_PropertyMap] [pm] ON [pm].[PropertyPathId] = [pp].[Id]
                    AND [pm].[ClassId] = [cl].[Id]
               JOIN [ec_Column] [cn] ON [cn].[Id] = [pm].[ColumnId] AND [cn].[ColumnKind] = 4
        WHERE  [baseCl].[Name] = ?
          AND  [baseSchema].[Name] = ?
          AND [cn].ColumnKind = 4
    ),
    [derivedClassColumns]([PropertyMapId], [ColumnId]) AS(
        SELECT
               [pm].[Id],
               [pm].[ColumnId]
        FROM   [ec_Property] [pt]
               JOIN [ec_Cache_ClassHierarchy] [ch] ON [ch].[ClassId] = [pt].[ClassId]
               JOIN [ec_Class] [baseCl] ON [baseCl].[Id] = [ch].[BaseClassId]
               JOIN [ec_Schema] [baseSchema] ON [baseSchema].[Id] = [baseCl].[SchemaId]
               JOIN [ec_Class] [cl] ON [cl].[Id] = [ch].[ClassId]
               JOIN [ec_Propertypath] [pp] ON [pp].[RootPropertyId] = [pt].[Id]
               JOIN [ec_PropertyMap] [pm] ON [pm].[PropertyPathId] = [pp].[Id]
                    AND [pm].[ClassId] = [cl].[Id]
               JOIN [ec_Column] [cn] ON [cn].[Id] = [pm].[ColumnId] AND [cn].[ColumnKind] = 4
        WHERE  [baseCl].[Name] = ?
          AND  [baseSchema].[Name] = ?
          AND [cn].[ColumnKind] = 4
    ),
    [propertiesWhichNeedRemapping]([PropertyMapId]) AS(
        SELECT DISTINCT [PropertyMapId]
        FROM   [derivedClassColumns]
        WHERE  [ColumnId] IN (SELECT [ColumnId] FROM [baseClassColumns])
    )
SELECT
       DISTINCT [pp].[RootPropertyId] [RootPropertyId],[pm].[ClassId] [MappedClassId], [c].[Name], [p].[Name]
FROM   [propertiesWhichNeedRemapping] [remap]
       JOIN [ec_PropertyMap] [pm] ON [pm].[id] = [remap].[PropertyMapId]
       JOIN [ec_PropertyPath] [pp] ON [pp].[Id] = [pm].[PropertyPathId]
       JOIN [ec_Property] [p] on [pp].[RootPropertyId] = [p].[Id]
       JOIN [ec_Class] [c] on [pm].[ClassId] = [c].[Id]
    )sqlstatement";

    //ColumnKind 4 indicates shared columns
    Utf8CP collectInfoSql = R"sqlstatement(
SELECT [rootSchema].[Name], [rootClass].[Name], [rootClass].[Id], [s].[Name], [c].[Name], [c].[Id], [ecp].[Id], [ecp].[Name], [pp].[AccessString], [tab].[Name], [col].[Name], [pm].[ClassId]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
    JOIN [ec_Class] [c] on [c].[Id] = [pm].[ClassId]
    JOIN [ec_Class] [rootClass] on [rootClass].[Id] = [ecp].[ClassId]
    JOIN [ec_Schema] [s] on [s].[Id] = [c].[SchemaId]
    JOIN [ec_Schema] [rootSchema] on [rootSchema].[Id] = [rootClass].[SchemaId]
WHERE [ecp].[Id] = ?1 AND [pm].[ClassId] IN (SELECT ?2 as [ClassId] UNION ALL SELECT [ClassId] from [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?2) AND [col].[ColumnKind] = 4
    )sqlstatement";

    Utf8CP cleanMappingsSql = R"sqlstatement(
DELETE FROM [ec_PropertyMap] WHERE [Id] IN(
SELECT [pm].[Id]
    FROM [ec_PropertyMap] [pm]
    JOIN [ec_Property] [ecp] on [ecp].[Id] = [pp].[RootPropertyId]
    JOIN [ec_PropertyPath] [pp] on [pp].[Id] = [pm].[PropertyPathId]
    JOIN [ec_Column] [col] on [col].[Id] = [pm].[ColumnId]
    JOIN [ec_Table] [tab] on [col].[TableId] = [tab].[Id]
WHERE [ecp].[Id] = ?1 AND [pm].[ClassId] IN (SELECT ?2 as [ClassId] UNION ALL SELECT [ClassId] from [ec_Cache_ClassHierarchy] WHERE [BaseClassId] = ?2) AND [col].[ColumnKind] = 4)
    )sqlstatement";


    CachedStatementPtr detectPropertiesToRemapStmt = m_ecdb.GetCachedStatement(detectPropertiesToRemapSql);
    if (detectPropertiesToRemapStmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != detectPropertiesToRemapStmt->BindText(1, addedBaseClass.m_name, Statement::MakeCopy::No))
        return ERROR;
    if (BE_SQLITE_OK != detectPropertiesToRemapStmt->BindText(2, addedBaseClass.m_schemaName, Statement::MakeCopy::No))
        return ERROR;
    if (BE_SQLITE_OK != detectPropertiesToRemapStmt->BindText(3, remapInfo.m_className, Statement::MakeCopy::No))
        return ERROR;
    if (BE_SQLITE_OK != detectPropertiesToRemapStmt->BindText(4, remapInfo.m_schemaName, Statement::MakeCopy::No))
        return ERROR;

    while (detectPropertiesToRemapStmt->Step() == BE_SQLITE_ROW)
        {
        ECPropertyId rootPropertyId = detectPropertiesToRemapStmt->GetValueId<ECPropertyId>(0);
        ECClassId classId = detectPropertiesToRemapStmt->GetValueId<ECClassId>(1);
        Utf8String className = detectPropertiesToRemapStmt->GetValueText(2);
        Utf8String propertyName = detectPropertiesToRemapStmt->GetValueText(3);
        LOG.infov("Property %s:%s needs to be remapped...", className.c_str(), propertyName.c_str());

        CachedStatementPtr collectInfoStmt = m_ecdb.GetCachedStatement(collectInfoSql);
        if (collectInfoStmt == nullptr)
            return ERROR;

        if (BE_SQLITE_OK != collectInfoStmt->BindId(1, rootPropertyId))
            return ERROR;
        if (BE_SQLITE_OK != collectInfoStmt->BindId(2, classId))
            return ERROR;

        int results = 0;
        while (collectInfoStmt->Step() == BE_SQLITE_ROW)
            {
            results++;
            cleanedPropertyIds.insert(rootPropertyId);
            auto mappedClassId = collectInfoStmt->GetValueId<ECClassId>(11); //class used in the property map
            auto& cleanedInfos = GetOrAddCleanedMappingInfo(mappedClassId);
            cleanedInfos.emplace_back(); //in c++ 17 emplace_back() already returns the reference, but we don't fully support that yet in our build chain.
            auto& cleanedInfo = cleanedInfos.back();

            cleanedInfo.m_rootSchemaName = collectInfoStmt->GetValueText(0);
            cleanedInfo.m_rootClassName = collectInfoStmt->GetValueText(1);
            cleanedInfo.m_rootClassId  = collectInfoStmt->GetValueId<ECClassId>(2); //refers to the class that defines the property
            cleanedInfo.m_schemaName = collectInfoStmt->GetValueText(3);
            cleanedInfo.m_className = collectInfoStmt->GetValueText(4);
            cleanedInfo.m_classId = mappedClassId;

            cleanedInfo.m_propertyId = rootPropertyId;
            cleanedInfo.m_propertyName = collectInfoStmt->GetValueText(7);
            cleanedInfo.m_accessString = collectInfoStmt->GetValueText(8);
            cleanedInfo.m_tableName = collectInfoStmt->GetValueText(9);
            cleanedInfo.m_columnName = collectInfoStmt->GetValueText(10);
            cleanedInfo.m_fullOldColumnIdentifier = RemapManager::GetFullColumnIdentifier(cleanedInfo.m_tableName, cleanedInfo.m_columnName);
            }

        CachedStatementPtr cleanMappingsStmt = m_ecdb.GetCachedStatement(cleanMappingsSql);
        if (cleanMappingsStmt == nullptr)
            {
            BeAssert(false && "Failed to prepare statement for cleaning maps of new base class");
            return ERROR;
            }
        if (BE_SQLITE_OK != cleanMappingsStmt->BindId(1, rootPropertyId))
            return ERROR;
        if (BE_SQLITE_OK != cleanMappingsStmt->BindId(2, classId))
            return ERROR;

        if (BE_SQLITE_DONE != cleanMappingsStmt->Step())
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to clean mappings for new base class property. Could not delete mapping for ECClass %s.%s: %s",
                remapInfo.m_schemaName.c_str(), remapInfo.m_className.c_str(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }

        int affectedRows = m_ecdb.GetModifiedRowCount();
        if(affectedRows != results)
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cleaning up deleted %" PRIi32 " mapping(s) but expected to clean %" PRIi32 " for property %d on class %s:%s.", affectedRows, results, rootPropertyId.GetValue(), remapInfo.m_schemaName.c_str(), remapInfo.m_className.c_str());
            return ERROR;
            }
        }

    //here we detect properties that were once mapped directly, and now come from the base class, so they turn into overrides. We need to clean their mappings and move the data
    Utf8CP detectNewPropertyOverridesSql = R"sqlstatement(
WITH
[propertyNamesUpTheHierarchy]([PropertyName]) as(
    Select distinct [p].[Name]
    FROM [ec_Property] [p]
    JOIN [ec_Class] [c] on [p].[ClassId] = [c].[Id]
    WHERE [c].[Id] IN (Select ?1 as [BaseClassId] UNION ALL Select [BaseClassId] from [ec_cache_ClassHierarchy] where [ClassId] = ?1)
),
[mappedPropertyNamesDownTheHierarchy]([PropertyName]) as(
    Select distinct [p].[Name]
    FROM [ec_Property] [p]
    JOIN [ec_PropertyPath] [pp] on [pp].[RootPropertyId] = [p].[Id]
    JOIN [ec_PropertyMap] [pm] on [pm].[PropertyPathId] = [pp].[Id]
    JOIN [ec_Class] [c] on [p].[ClassId] = [c].[Id]
    WHERE [c].[Id] IN (Select ?2 as [ClassId] UNION ALL Select [ClassId] from [ec_cache_ClassHierarchy] where [BaseClassId] = ?2)
)
SELECT [PropertyName]
 FROM [mappedPropertyNamesDownTheHierarchy]
 WHERE [PropertyName] in [propertyNamesUpTheHierarchy]
    )sqlstatement";
    CachedStatementPtr detectPropertyOverridesStmt = m_ecdb.GetCachedStatement(detectNewPropertyOverridesSql);
    if (detectPropertyOverridesStmt == nullptr)
        return ERROR;

    if (BE_SQLITE_OK != detectPropertyOverridesStmt->BindId(1, addedBaseClass.m_id))
        return ERROR;
    if (BE_SQLITE_OK != detectPropertyOverridesStmt->BindId(2, remapInfo.m_classId))
        return ERROR;

    while (detectPropertyOverridesStmt->Step() == BE_SQLITE_ROW)
        {
        Utf8String propertyName = detectPropertyOverridesStmt->GetValueText(0);
        if(CleanPropertyDownTheHierarchyByName(remapInfo.m_classId, propertyName, true, cleanedPropertyIds) != SUCCESS)
            return ERROR;
        }

    return SUCCESS;
    }

BentleyStatus RemapManager::EnsureInvolvedSchemasAreLoaded(bvector<ECSchemaCP> const& schemasToMap)
    {
    /*
    This method ensures that all schemas that we touch in remapping are being loaded before schema mapping takes place.
    Schema Mapping relies on .DerivedClasses() to navigate to the leaf classes, so we have to make sure the necessary schemas are fully loaded.
    */
    if(m_cleanedMappingInfo.empty())
        return SUCCESS;
    IdSet<ECClassId> classIdsToMap;
    IdSet<ECSchemaId> schemasIdsToMap;
    for(ECSchemaCP schema : schemasToMap)
        {
        if(schema->HasId())
            schemasIdsToMap.insert(schema->GetId());
        }
    for(auto& pair : m_cleanedMappingInfo)
        {
        auto classId = pair.first;
        classIdsToMap.insert(classId);
        }
        Utf8CP complementarySchemaIdsSql = R"sqlstatement(
SELECT DISTINCT [s].[Name] from [ec_Class] c
    JOIN [ec_Schema] [s] on [s].[Id] = [c].[SchemaId]
    WHERE NOT InVirtualSet(?,[c].[SchemaId]) AND InVirtualSet(?, [c].[Id])
    )sqlstatement";
    CachedStatementPtr complementarySchemaIdsStmt = m_ecdb.GetCachedStatement(complementarySchemaIdsSql);
    if (complementarySchemaIdsStmt == nullptr)
        return ERROR;
    if(BE_SQLITE_OK != complementarySchemaIdsStmt->BindVirtualSet(1, schemasIdsToMap))
        return ERROR;
    if(BE_SQLITE_OK != complementarySchemaIdsStmt->BindVirtualSet(2, classIdsToMap))
        return ERROR;
    while (complementarySchemaIdsStmt->Step() == BE_SQLITE_ROW)
        {
        Utf8String schemaName = complementarySchemaIdsStmt->GetValueText(0);
        ECSchemaCP schema = m_ecdb.Schemas().GetSchema(schemaName);
        if(schema == nullptr)
            return ERROR;
        }
    return SUCCESS;
    }

BentleyStatus RemapManager::RestoreAndProcessCleanedPropertyMaps(SchemaImportContext& ctx)
    {
    if(m_cleanedMappingInfo.empty())
        return SUCCESS;

    DbMapSaveContext saveCtx(m_ecdb);

    for(auto& pair : m_cleanedMappingInfo)
        {
        auto classId = pair.first;
        auto ecClass = m_schemaManager.GetClass(classId);
        if(ecClass == nullptr)
            {
            BeAssert(false && "Failed to find class");
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to find class %s for ensuring persisted property maps.", pair.second[0].m_className.c_str());
            return ERROR;
            }

        auto classMap = m_schemaManager.GetClassMap(*ecClass);
        if(classMap == nullptr)
            {
            BeAssert(false && "Failed to find class map");
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to find class map for class %s for ensuring persisted property maps.", ecClass->GetFullName());
            return ERROR;
            }
        saveCtx.BeginSaving(*classMap);
        DbClassMapSaveContext classMapContext(saveCtx);
        SavePropertyMapVisitor saveVisitor(classMapContext, SavePropertyMapVisitor::Action::SkipIfExists);

        auto& propertyMaps = classMap->GetPropertyMaps();
        for(auto& cleaned : pair.second)
            {
            auto map = propertyMaps.Find(cleaned.m_accessString.c_str());
            if(map == nullptr)
                {
                BeAssert(false && "Failed to find property map");
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to find property map for access string %s on class %s for ensuring persisted property maps.", cleaned.m_accessString.c_str(), ecClass->GetFullName());
                return ERROR;
                }

            if(map->AcceptVisitor(saveVisitor) != SUCCESS)
                {
                BeAssert(false && "Failed to find property map");
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to make sure property map for access string %s on class %s is persisted.", cleaned.m_accessString.c_str(), ecClass->GetFullName());
                return ERROR;
                }
            PropertyMap::Type propMapType = map->GetType();
            if(!Enum::Contains(PropertyMap::Type::SingleColumnData, propMapType))
                { // we always expect this to be single column because the access string is fully qualified
                BeAssert(false && "Failed to cast property map");
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed check if property map for access string %s on class %s is a single data property map.", cleaned.m_accessString.c_str(), ecClass->GetFullName());
                return ERROR;
                }
            auto& singleColumnMap = map->GetAs<SingleColumnDataPropertyMap>();
            Utf8String tableName = singleColumnMap.GetTable().GetName();
            Utf8String columnName = singleColumnMap.GetColumn().GetName();
            bool isSameTableName = tableName.EqualsI(cleaned.m_tableName);
            bool isSameColumnName = columnName.EqualsI(cleaned.m_columnName);
            if(isSameTableName && isSameColumnName)
                continue;//we ended up in the same column, nothing to move

            auto& target = GetOrAddRemappedColumnInfo(classId);
            target.emplace_back(cleaned); //in c++ 17 emplace_back() already returns the reference, but we don't fully support that yet in our build chain.
            auto& remapInfo = target.back();
            remapInfo.m_newColumnName = columnName;
            remapInfo.m_newTableName = tableName;
            remapInfo.m_isSameColumnName = isSameColumnName;
            remapInfo.m_isSameTableName = isSameTableName;
            remapInfo.m_fullNewColumnIdentifier = GetFullColumnIdentifier(tableName, columnName);
            }
        saveCtx.EndSaving(*classMap);
        }

    return SUCCESS;
    }

BentleyStatus RemapManager::UpgradeExistingECInstancesWithRemappedProperties()
    {
    ECDB_PERF_LOG_SCOPE( "Schema import> Upgrading existing ECInstances with remapped properties to different columns");
    for(auto& pair : m_remappedColumns)
        {
        auto classId = pair.first;
        auto& remapInfos = pair.second;

        if(remapInfos.empty())
            continue; //nothing to remap

        LOG.infov("Updating existing instances for class %s: %" PRIuPTR " columns to remap...", remapInfos[0].m_cleanedMapping.m_className.c_str(), remapInfos.size());

        //Hold the remaining remap infos to sort in a map
        std::map<Utf8String, RemappedColumnInfo*> remainingRemapsByOldColumnId;
        std::transform(remapInfos.begin(), remapInfos.end(), std::inserter(remainingRemapsByOldColumnId, remainingRemapsByOldColumnId.end()),
               [](RemappedColumnInfo &i) { return std::make_pair(i.m_cleanedMapping.m_fullOldColumnIdentifier, &i); });
        BeAssert(remapInfos.size() == remainingRemapsByOldColumnId.size());

        std::vector<RemappedColumnInfo*> sortedUpdates;
        std::vector<std::vector<RemappedColumnInfo*>> circularUpdates;
        if(SortRemapInfos(sortedUpdates, circularUpdates, remainingRemapsByOldColumnId) != SUCCESS)
            {
            return ERROR;
            }

        if(UpdateRemappedData(sortedUpdates) != SUCCESS)
            {
            return ERROR;
            }

        if(UpdateRemappedCircularData(circularUpdates) != SUCCESS)
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

BentleyStatus RemapManager::SortRemapInfos(std::vector<RemappedColumnInfo*>& sortedInfos, std::vector<std::vector<RemappedColumnInfo*>>& circularUpdates, std::map<Utf8String, RemappedColumnInfo*>& remainingInfos)
    {
    if(!CheckIfSortingIsNeeded(remainingInfos))
        {
        for(auto& it : remainingInfos) { sortedInfos.push_back(it.second); }
        remainingInfos.clear();
        return SUCCESS;
        }

    SortRemappedColumnInfos(sortedInfos,remainingInfos);

    if (!remainingInfos.empty())
        {
        LOG.infov("SortRemapInfos> There are circular property column updates. Affected columns: %" PRIuPTR, remainingInfos.size());
        SortCircularRemappedColumnInfos(circularUpdates,remainingInfos);
        }

    if (!remainingInfos.empty())
        {
        Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "SortRemapInfos> There remaining column updates which could not be arranged into a valid order. Aborting. Remaining items: %" PRIuPTR, remainingInfos.size());
        return ERROR;
        }

    return SUCCESS;
    }

bool RemapManager::CheckIfSortingIsNeeded(std::map<Utf8String, RemappedColumnInfo*>& unsortedInfos)
    {
    for(auto& info : unsortedInfos)
        {
        if(unsortedInfos.count(info.second->m_fullNewColumnIdentifier) >= 1)
            return true; //if there is any entry which uses our new column as its old column we have to sort.
        }

    return false;
    }

void RemapManager::SortRemappedColumnInfos(std::vector<RemappedColumnInfo*>& sortedInfos, std::map<Utf8String, RemappedColumnInfo*>& remainingItemsToSort)
    {
    bool successInLastRun = true;
    // we repeat this process until we find no more valid entries to move
    while(successInLastRun && !remainingItemsToSort.empty())
        {
        successInLastRun = false;
        //iterate the map, either remove the item and push it to sortedInfos or step over it
        for (auto it = remainingItemsToSort.begin(); it != remainingItemsToSort.end() /* not hoisted */; /* no increment */)
            {
            if (remainingItemsToSort.count(it->second->m_fullNewColumnIdentifier) == 0) //we have no items that feed on our new column
                {
                sortedInfos.push_back(it->second);
                it = remainingItemsToSort.erase(it);
                successInLastRun = true;
                }
            else
                {
                ++it;
                }
            }
        }
    }

void RemapManager::SortCircularRemappedColumnInfos(std::vector<std::vector<RemappedColumnInfo*>>& circularUpdates, std::map<Utf8String, RemappedColumnInfo*>& remainingItemsToSort)
    {
    bool successInLastRun = true;
    // we repeat this process until we find no more valid entries to move
    while(successInLastRun && !remainingItemsToSort.empty())
        {
        successInLastRun = false;

        std::vector<RemappedColumnInfo*> circularUpdate;
        auto firstItem = remainingItemsToSort.begin()->second;
        Utf8String firstId = firstItem->m_cleanedMapping.m_fullOldColumnIdentifier;
        Utf8String nextId = firstItem->m_fullNewColumnIdentifier;
        circularUpdate.push_back(firstItem);
        remainingItemsToSort.erase(firstItem->m_cleanedMapping.m_fullOldColumnIdentifier);
        while(true)
            {
            auto nextPtr = remainingItemsToSort.find(nextId);

            if(nextPtr == remainingItemsToSort.end())
                { //failed to find the next element of the circle
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Could not finish resolving circular property remapping. Failed to find the next node after %s. Property: %s", nextId.c_str(), firstItem->ToString().c_str());
                for(auto& v : circularUpdate)
                    { //re-add the items we have taken so far
                    remainingItemsToSort[v->m_cleanedMapping.m_fullOldColumnIdentifier] = v;
                    }
                return; //return early, we cannot recover from this. Outer method will fail if items are left in the remainingItems
                }

            RemappedColumnInfo* next = nextPtr->second;
            circularUpdate.push_back(next);
            nextId = next->m_fullNewColumnIdentifier;
            remainingItemsToSort.erase(next->m_cleanedMapping.m_fullOldColumnIdentifier);

            if(nextId.EqualsI(firstId))
                {
                circularUpdates.push_back(circularUpdate);
                successInLastRun = true;
                break;
                }
            }
        }
    }

BentleyStatus RemapManager::UpdateRemappedData(std::vector<RemappedColumnInfo*>& infos)
    {
    if(infos.empty())
        return SUCCESS;

    for (auto remapInfo : infos)
        {
        LOG.infov("Moving data for property %s from column %s to column %s.", remapInfo->ToString().c_str(), remapInfo->m_cleanedMapping.m_fullOldColumnIdentifier.c_str(), remapInfo->m_fullNewColumnIdentifier.c_str());
        // Move data from old column to new column
        if (remapInfo->m_isSameTableName)
            {
                {
                auto rc = m_ecdb.TryExecuteSql(SqlPrintfString("UPDATE [%s] SET [%s]=[%s] WHERE([ECClassId]=%s)",
                                                                remapInfo->m_newTableName.c_str(),
                                                                remapInfo->m_newColumnName.c_str(),
                                                                remapInfo->m_cleanedMapping.m_columnName.c_str(),
                                                                remapInfo->m_cleanedMapping.m_classId.ToHexStr().c_str()));
                if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
                    {
                    Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to update remapped property columns for %s. Error: %s", remapInfo->ToString().c_str(), m_ecdb.GetLastError().c_str());
                    return ERROR;
                    }
                }

                {
                auto rc = m_ecdb.TryExecuteSql(SqlPrintfString("UPDATE [%s] SET [%s]=NULL WHERE([ECClassId]=%s)",
                                                                remapInfo->m_newTableName.c_str(),
                                                                remapInfo->m_cleanedMapping.m_columnName.c_str(),
                                                                remapInfo->m_cleanedMapping.m_classId.ToHexStr().c_str()));
                if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
                    {
                    Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to clean old column for remapped property %s. Error: %s", remapInfo->ToString().c_str(), m_ecdb.GetLastError().c_str());
                    return ERROR;
                    }
                }
            }
        else
            { // move between tables
            {
            Utf8String oldInstanceIdColumn = GetInstanceIdColumnName(remapInfo->m_cleanedMapping.m_tableName);
            Utf8String newInstanceIdColumn = GetInstanceIdColumnName(remapInfo->m_newTableName);
            if(oldInstanceIdColumn.empty() || newInstanceIdColumn.empty())
                {
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to find instance id column for table %s or %s.", remapInfo->m_newTableName.c_str(),
                    remapInfo->m_cleanedMapping.m_tableName.c_str());
                return ERROR;
                }

            auto rc = m_ecdb.TryExecuteSql(SqlPrintfString("UPDATE [%s] SET [%s] = (SELECT [%s] FROM [%s] WHERE [%s].[%s] = [%s].[%s]) WHERE([ECClassId]=%s)",
                                                            remapInfo->m_newTableName.c_str(),
                                                            remapInfo->m_newColumnName.c_str(),
                                                            remapInfo->m_cleanedMapping.m_columnName.c_str(),
                                                            remapInfo->m_cleanedMapping.m_tableName.c_str(),
                                                            remapInfo->m_cleanedMapping.m_tableName.c_str(),
                                                            oldInstanceIdColumn.c_str(),
                                                            remapInfo->m_newTableName.c_str(),
                                                            newInstanceIdColumn.c_str(),
                                                            remapInfo->m_cleanedMapping.m_classId.ToHexStr().c_str()));
            if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
                {
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to execute remap property statement to move property data into different table. Property %s. Error: %s", remapInfo->ToString().c_str(),
                    m_ecdb.GetLastError().c_str());
                return ERROR;
                }
            }

            {
            auto rc = m_ecdb.TryExecuteSql(SqlPrintfString("UPDATE [%s] SET [%s] = NULL WHERE([ECClassId]=%s)",
                                                            remapInfo->m_cleanedMapping.m_tableName.c_str(),
                                                            remapInfo->m_cleanedMapping.m_columnName.c_str(),
                                                            remapInfo->m_cleanedMapping.m_classId.ToHexStr().c_str()));
            if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
                {
                Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to execute update statement for removing remapped property data from old column. Property %s. Error: %s",
                    remapInfo->ToString().c_str(), m_ecdb.GetLastError().c_str());
                return ERROR;
                }
            }
            } // move between tables
        } //for each remap info

    return SUCCESS;
    }

bool RemapManager::CheckIfAllUpdatesAreWithinSameTable(std::vector<std::vector<RemappedColumnInfo*>>& infos)
    {
    for(auto& update : infos)
        {
        if(!std::all_of(update.begin(), update.end(), [](RemappedColumnInfo* colInfo) { return colInfo->m_isSameTableName; }))
            {
            return false;
            }
        }

    return true;
    }

BentleyStatus RemapManager::UpdateRemappedCircularData(std::vector<std::vector<RemappedColumnInfo*>>& infos)
    {
    if(infos.empty())
        return SUCCESS;

    if(!CheckIfAllUpdatesAreWithinSameTable(infos))
        {
        Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Cannot perform update on instance data for remapped properties. This update requires moving data between columns in a circular way across tables, which is not supported.");
        // Swapping Columns between multiple tables in sqlite has to be done in a different way. Probably use a temporary table/column or in-memory to preserve data of one column
        // move the rest, and then put the temporary data where it belongs. This is covered in test: SchemaRemapTestFixture, SwapColumnsWithOverflow
        return ERROR;
        }

    for(auto& update : infos)
        {
        if(update.empty())
            continue;

        auto first = update.front();
        Statement stmt;
        SqlPrintfString prefix("UPDATE [%s] SET [%s]=[%s]", first->m_cleanedMapping.m_tableName.c_str(), first->m_newColumnName.c_str(), first->m_cleanedMapping.m_columnName.c_str());
        Utf8PrintfString updateStmt(prefix.GetUtf8CP());
        if(update.size() > 1)
            {
            auto start = update.begin() + 1; // start from second element
            for (auto p = start; p != update.end(); p++)
                {
                auto item = *p;
                SqlPrintfString assignmentStmt(", [%s]=[%s]", item->m_newColumnName.c_str(), item->m_cleanedMapping.m_columnName.c_str());
                updateStmt.append(assignmentStmt.GetUtf8CP());
                }
            }
        SqlPrintfString classIdPart(" WHERE([ECClassId]=%s)", first->m_cleanedMapping.m_classId.ToHexStr().c_str());
        updateStmt.append(classIdPart.GetUtf8CP());
        auto rc = m_ecdb.TryExecuteSql(updateStmt.c_str());
        if (rc != BE_SQLITE_OK && rc != BE_SQLITE_DONE)
            {
            Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECDbIssue, "Failed to update remapped property columns (circular) for %s. Error: %s", first->ToString().c_str(), m_ecdb.GetLastError().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

Utf8String RemapManager::GetInstanceIdColumnName(Utf8StringCR tableName)
    {
    Utf8CP sql = "select [col].[Name] FROM [ec_Table] [tab] JOIN [ec_Column] [col] on [tab].[Id] = [col].[TableId] WHERE [tab].[Name] = ? AND [col].[ColumnKind] = 1";

    CachedStatementPtr stmt = m_ecdb.GetCachedStatement(sql);
    if (stmt == nullptr)
        {
        BeAssert(false && "Failed to prepare statement to obtain instance id column");
        return "";
        }

    if (BE_SQLITE_OK != stmt->BindText(1, tableName, Statement::MakeCopy::No))
        {
        BeAssert(false);
        return "";
        }

    if (stmt->Step() != BE_SQLITE_ROW)
        {
        BeAssert(false && "Failed to find instance id column");
        return "";
        }

    return stmt->GetValueText(0);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*
Check README_Transform.md for information about the logic here
*/

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RemapManager final
    {
private:
    struct AddedProperty
        {
        Utf8String m_name;
        ECN::ECPropertyId m_id;

        explicit AddedProperty(Utf8StringCR name, ECN::ECPropertyId id)
        : m_name(name), m_id(id) {}
        };

    struct AddedBaseClass
        {
        //all three fields refer to the added base class
        Utf8String m_schemaName;
        Utf8String m_name;
        ECN::ECClassId m_id;

        explicit AddedBaseClass(Utf8StringCR name, ECN::ECClassId id, Utf8StringCR schemaName)
        : m_name(name), m_id(id), m_schemaName(schemaName) {}
        };

    struct RemapInfosForClass
        {
    public:
        Utf8String m_schemaName;
        Utf8String m_className;
        ECN::ECClassId m_classId;
        std::vector<AddedProperty> m_addedProperties;
        std::vector<AddedBaseClass> m_addedBaseClasses;

        explicit RemapInfosForClass(Utf8StringCR schemaName, Utf8StringCR className, ECN::ECClassId classId)
        : m_schemaName(schemaName), m_className(className), m_classId(classId), m_addedProperties(), m_addedBaseClasses() {}

        void TrackAddedProperty(Utf8StringCR propertyName, ECN::ECPropertyId id) 
            {
            m_addedProperties.emplace_back(propertyName, id);
            }

        void TrackAddedBaseClass(Utf8StringCR name, ECN::ECClassId id, Utf8StringCR schemaName) 
            {
            m_addedBaseClasses.emplace_back(name, id, schemaName);
            }
        };

    struct CleanedMappingInfo // Holds the information about a mapping that has been cleaned up
        {
    public:
        Utf8String m_rootSchemaName;
        Utf8String m_rootClassName;
        ECN::ECClassId m_rootClassId; //refers to the class that defines the property
        Utf8String m_schemaName;
        Utf8String m_className;
        ECN::ECClassId m_classId; //refers to the actual class (may be derived class of rootClassId)

        ECN::ECPropertyId m_propertyId;
        Utf8String m_propertyName;
        Utf8String m_accessString;
        Utf8String m_tableName;
        Utf8String m_columnName;
        Utf8String m_fullOldColumnIdentifier; //format for these identifiers is <tablename>:<columnname>

        Utf8String ToString() const
            {
            return Utf8PrintfString("%s.%s.%s",
                m_schemaName.c_str(), m_className.c_str(), m_accessString.c_str());
            }
        };

    struct RemappedColumnInfo // Holds the information about an old and new column assignment
        {
    public:
        CleanedMappingInfo& m_cleanedMapping;
        Utf8String m_newTableName;
        Utf8String m_newColumnName;
        bool m_isSameTableName;
        bool m_isSameColumnName;
        Utf8String m_fullNewColumnIdentifier;

        RemappedColumnInfo(CleanedMappingInfo& mapping) : m_cleanedMapping(mapping) { }

        Utf8String ToString() const
            {
            return m_cleanedMapping.ToString();
            }
        };

//General methods and fields
private:
    MainSchemaManager const& m_schemaManager;
    ECDbCR m_ecdb;
    IssueDataSource const& Issues() const { return m_ecdb.GetImpl().Issues(); }
public:
    RemapManager(ECDbCR ecdb) : m_schemaManager(ecdb.Schemas().Main()), m_ecdb(ecdb), m_remapInfos() { }
    ~RemapManager() {}

private:
    std::map<ECN::ECClassId, RemapInfosForClass> m_remapInfos;
    RemapInfosForClass& GetOrAddRemapInfo(ECN::ECClassId id, Utf8StringCR className, Utf8StringCR schemaName)
        {
        auto [ insertedIt, success ] = m_remapInfos.try_emplace(id, schemaName, className, id);
        return insertedIt->second;
        };
public:
    void CollectRemapInfosFromNewProperty(ECN::ECClassCR ecClass, Utf8StringCR propertyName, ECN::ECPropertyId id);
    void CollectRemapInfosFromModifiedBaseClass(ECN::ECClassCR derivedClass, ECN::ECClassCR newBaseClass);
    void CollectRemapInfosFromDeletedPropertyOverride(ECN::ECPropertyId propertyId);

//Clear Modified Mappings
private:
    std::map<ECN::ECClassId, std::vector<CleanedMappingInfo>> m_cleanedMappingInfo;
    std::vector<CleanedMappingInfo>& GetOrAddCleanedMappingInfo(ECN::ECClassId id)
        {
        auto [ insertedIt, success ] = m_cleanedMappingInfo.try_emplace(id);
        return insertedIt->second;
        };
    BentleyStatus CleanPropertyDownTheHierarchyByName(ECN::ECClassId classId, Utf8StringCR propertyName, bool includeClassInCleanup, std::set<ECN::ECPropertyId>& cleanedPropertyIds);
    BentleyStatus CleanAddedProperty(RemapInfosForClass& remapInfo, AddedProperty& addedProperty, std::set<ECN::ECPropertyId>& cleanedPropertyIds);
    BentleyStatus CleanAddedBaseClass(RemapInfosForClass& remapInfo, AddedBaseClass& addedBaseClass, std::set<ECN::ECPropertyId>& cleanedPropertyIds);
    
public:
    BentleyStatus CleanModifiedMappings();
    BentleyStatus EnsureInvolvedSchemasAreLoaded(bvector<ECN::ECSchemaCP> const& schemasToMap);
    BentleyStatus RestoreAndProcessCleanedPropertyMaps(SchemaImportContext& ctx);

//move data
private:
    std::map<ECN::ECClassId, std::vector<RemappedColumnInfo>> m_remappedColumns;
    std::vector<RemappedColumnInfo>& GetOrAddRemappedColumnInfo(ECN::ECClassId id)
        {
        auto [ insertedIt, success ] = m_remappedColumns.try_emplace(id);
        return insertedIt->second;
        };
    static Utf8String GetFullColumnIdentifier(Utf8StringCR tableName, Utf8StringCR columnName)
        {
        Utf8PrintfString idStr("%s:%s", tableName.c_str(), columnName.c_str());
        return idStr;
        };

    bool CheckIfSortingIsNeeded(std::map<Utf8String, RemappedColumnInfo*>& unsortedInfos);
    void SortRemappedColumnInfos(std::vector<RemappedColumnInfo*>& sortedInfos, std::map<Utf8String, RemappedColumnInfo*>& remainingItemsToSort);
    void SortCircularRemappedColumnInfos(std::vector<std::vector<RemappedColumnInfo*>>& circularUpdates, std::map<Utf8String, RemappedColumnInfo*>& remainingItemsToSort);
    BentleyStatus SortRemapInfos(std::vector<RemappedColumnInfo*>& sortedInfos, std::vector<std::vector<RemappedColumnInfo*>>& circularUpdates, std::map<Utf8String, RemappedColumnInfo*>& remainingInfos);
    BentleyStatus UpdateRemappedData(std::vector<RemappedColumnInfo*>& infos, SchemaImportContext& ctx);
    bool CheckIfAllUpdatesAreWithinSameTable(std::vector<std::vector<RemappedColumnInfo*>>& infos);
    BentleyStatus UpdateRemappedCircularData(std::vector<std::vector<RemappedColumnInfo*>>& infos, SchemaImportContext& ctx);
    Utf8String GetInstanceIdColumnName(Utf8StringCR tableName);

public:
    BentleyStatus UpgradeExistingECInstancesWithRemappedProperties(SchemaImportContext& ctx);
    };



END_BENTLEY_SQLITE_EC_NAMESPACE

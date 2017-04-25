/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapColumnFactory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECDb.h>
#include "DbSchema.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ClassMap;
struct RelationshipClassEndTableMap;
struct PropertyMap;

//======================================================================================
// @bsiclass                                                     Affan.Khan      01/2015
//ColumnMapping Steps for ECSchema Import/Update
//1. For none TPH/SharedColumn is not documented here.
//2. For TPH/SharedColumn Algorithm is as following
//    a. Find deepest derived classes of the ECClass that is about to be mapped called it X int a list DL<ClassMap>.
//    b. Find all the mixins that is implemented by X or one of its derived Classes and put it in map LM<MixIN,ClassMap>
//    c. Remove any mixin from LM if its implemented by DL.
//    d. Any mixin that is not resolved into a classMap
//    a. Find an implementation of Mixin by tranversing its derive classes until we find a ClassMap that implements it and is int the same table as X.
//    b. For MixIn in LM that is not resolved by previous step traverse its baseClasses until we find a baseClass that has a implementation in same table as X.
//    e. Find all the relationships that will be stored in same table as X and has X as end point of the relationship. Store it in list RM
//    c. Go over each ClassMap in DL and add its property map to a map CM<AccessString,DbColumn>
//    d. Go over each MixIn map and add property map for mixin to CM.
//    e. Go over each EndTable relationship ClassMap in RM and add its ECInstanceId,ECClassId propertyMap in CM that is map to same table as X.
//    f. return CM.
//===============+===============+===============+===============+===============+======
struct ClassMapColumnFactory final : NonCopyableClass
    {
    private: 
        //Find list of columns with access string that cannot be used by current classmap
        struct UsedColumnFinder final
            {
            typedef std::map<Utf8String, DbColumn const*> ColumnMap;
            private:
                std::set<ClassMap const*> m_deepestClassMapped;//Set of deepest classmap in traversed hierarchy.
                std::set<ECN::ECEntityClassCP> m_mixins; //Set of identitifed mixin during traversing class hierarchy
                std::set<ECN::ECClassCP> m_primaryHierarchy; //Set of classes that is part of pirmary hierarchy that is already traversed.
                std::set<RelationshipClassEndTableMap const*> m_endTableRelationship; //Final list of relationship for the context class
                std::map<ECN::ECEntityClassCP, ClassMap const*> m_mixinImpls; // Final list of mixIn classes implementation
                std::set<DbTable const*> m_contextMapTableSet; //fast cache for context class tables
                ClassMap const& m_classMap; //Context Class for which to find used columns

                UsedColumnFinder(ClassMap const& classMap);
                ClassMap const* GetClassMap(ECN::ECClassCR) const;
                bool IsMappedIntoContextClassMapTables(ClassMap const&) const;
                bool IsMappedIntoContextClassMapTables(PropertyMap const&) const;
                BentleyStatus ResolveMixins();
                ClassMap const* ResolveMixin(ECN::ECClassCR);
                void ResolveBaseMixin(ECN::ECClassCR currentClass);
                BentleyStatus TraverseClassHierarchy(ECN::ECClassCR, ClassMap const*);
                BentleyStatus FindRelationshipEndTableMaps();
                BentleyStatus Execute(ColumnMap&);
                BentleyStatus QueryRelevantMixIns();
            public:
                static BentleyStatus Find(ColumnMap&, ClassMap const&);
            };


        ClassMap const& m_classMap;
        mutable std::map<Utf8String, std::set<DbColumn const*>, CompareIUtf8Ascii> m_usedColumnMap;
        mutable std::set<DbColumn const*> m_usedColumnSet;
        bool m_usesSharedColumnStrategy;
        int m_sharedColumnCount;
        mutable std::vector<ClassMap const*> m_compoundFilter;
        DbTable* GetOverflowTable() const;
        void Initialize() const;

        ECN::ECClassId GetPersistenceClassId(ECN::ECPropertyCR, Utf8StringCR accessString) const;
        BentleyStatus ResolveColumnName(Utf8StringR resolvedColumName, Utf8StringCR requestedColumnName, ECN::ECClassId, int retryCount) const;

        DbColumn* CreateColumn(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplyDefaultStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&, Utf8StringCR accessString) const;
        DbColumn* ApplySharedColumnStrategy(ECN::ECPropertyCR, DbColumn::Type, DbColumn::CreateParams const&) const;

        bool TryFindReusableSharedDataColumn(DbColumn const*& reusableColumn) const;
        bool IsColumnInUseByClassMap(DbColumn const& column) const;
        bool IsCompatible(DbColumn const& avaliableColumn, DbColumn::Type type, DbColumn::CreateParams const& param) const;

        void AddColumnToCache(DbColumn const&, Utf8StringCR) const;
        void SetupCompoundFilter(bset<const ClassMap*> const* additionalFilter) const;
        void RemoveCompoundFilter() const;

        DbTable& GetTable() const;
        ECDbCR GetECDb() const;
        struct ColumnReservationInfo
            {
            private:
                ECN::ECPropertyCR m_property;
                int m_createdColumnCount;
                int m_reservedColumnsCount;
                int m_reusedColumnCount;
                DbTable* m_overflowTable;
            public:
                ColumnReservationInfo(ECN::ECPropertyCR property, int reservedColumns, int reusedColumn, int createdColumns)
                    :m_property(property),m_reservedColumnsCount(reservedColumns), m_createdColumnCount(createdColumns), m_reusedColumnCount(reusedColumn), m_overflowTable(nullptr)
                    {}

                ColumnReservationInfo(ECN::ECPropertyCR property, int reservedColumns, DbTable& overflowTable)
                    :m_property(property), m_reservedColumnsCount(reservedColumns), m_createdColumnCount(0), m_reusedColumnCount(0), m_overflowTable(&overflowTable)
                    {}
                ~ColumnReservationInfo(){}
                int GetReservedColumnCount() const { m_reservedColumnsCount; }
                int GetCreatedColumnCount() const { m_createdColumnCount; }
                int GetReusedColumnCount() const { return m_reusedColumnCount; }
                ECN::ECPropertyCR GetProperty() const { return m_property; }
                DbTable* GetOverflowTable () const { return m_overflowTable; }
                void AllocateNew() { BeAssert(m_createdColumnCount > 0);  m_createdColumnCount--; }
                void AllocateExisting() { BeAssert(m_reusedColumnCount > 0); m_reusedColumnCount--; }
                static int MaxColumnsRequiredToPersistAProperty(ECN::ECPropertyCR ecProperty);
            };

        mutable std::unique_ptr<ColumnReservationInfo> m_columnReservationInfo;
        //! This method determine how many avaliable columns is present in current table.
        //! It presume current table GetTable() is either JoinedTable or PrimaryTable.
        BentleyStatus TryGetAvaliableColumns(int& sharedColumnThatCanBeCreated, int& sharedColumnThatCanBeReused) const;
    public:
        explicit ClassMapColumnFactory(ClassMap const& classMap);
        //This function either creates a column or grabs an existing column
        bool UsesSharedColumnStrategy() const { return m_usesSharedColumnStrategy; }
        BentleyStatus BeignSharedColumnBlock(Utf8CP propertyName) const;
        BentleyStatus EndSharedColumnBlock() const;
        DbColumn* AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bset<const ClassMap*> const* additionalFilter = nullptr) const;
        void Refresh() const { m_usedColumnMap.clear(); m_usedColumnSet.clear(); Initialize(); }
        void Debug() const;

    };


END_BENTLEY_SQLITE_EC_NAMESPACE

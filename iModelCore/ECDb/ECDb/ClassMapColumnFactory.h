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
        mutable std::vector<ClassMap const*> m_compoundFilter;
                
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

    public:
        explicit ClassMapColumnFactory(ClassMap const& classMap);
        //This function either creates a column or grabs an existing column
        DbColumn* AllocateDataColumn(ECN::ECPropertyCR property, DbColumn::Type type, DbColumn::CreateParams const& param, Utf8StringCR accessString, bset<const ClassMap*> const* additionalFilter = nullptr) const;
        void Refresh() const { m_usedColumnMap.clear(); m_usedColumnSet.clear(); Initialize(); }
        void Debug() const;
    };


END_BENTLEY_SQLITE_EC_NAMESPACE

/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapAnalyser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Describe which command to generate in sql
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
enum class SqlOption
    {
    Create,
    CreateIfNotExist,
    Drop,
    DropIfExists
    };

//=======================================================================================
//! Allow to build a sql trigger
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct SqlTriggerBuilder
    {
    public:
        enum class Condition
            {
            After,
            Before,
            InsteadOf
            };

        enum class Type
            {
            Insert,
            Update,
            UpdateOf,
            Delete
            };
        //=======================================================================================
        //! Describe collection of trigger with create/delete
        // @bsiclass                                               Affan.Khan          09/2015
        //+===============+===============+===============+===============+===============+======
        struct TriggerList
            {
            typedef std::vector<SqlTriggerBuilder> List;
            private:
                List m_list;

            public:
                TriggerList();
                SqlTriggerBuilder& Create(SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary);
                List const& GetTriggers() const;
                void Delete(SqlTriggerBuilder const& trigger);
            };

    private:
        NativeSqlBuilder m_name;
        NativeSqlBuilder m_when;
        NativeSqlBuilder m_body;
        NativeSqlBuilder m_on;
        bool m_temprory;
        Type m_type;
        Condition m_condition;
        std::vector<Utf8String> m_ofColumns;

    public:
        SqlTriggerBuilder() {}
        SqlTriggerBuilder(Type type, Condition condition, bool temprary);
        SqlTriggerBuilder(SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder(SqlTriggerBuilder const& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder const& rhs);
        NativeSqlBuilder& GetNameBuilder();
        NativeSqlBuilder& GetWhenBuilder();
        NativeSqlBuilder& GetBodyBuilder();
        NativeSqlBuilder& GetOnBuilder();
        Utf8CP GetName() const;
        Utf8CP GetWhen() const;
        Utf8CP GetBody() const;
        Utf8CP GetOn() const;
        bool IsTemporary() const;
        bool IsValid() const;
        Utf8String ToString(SqlOption option, bool escape) const;
        bool IsEmpty() const { return m_body.IsEmpty(); }
    };

//=======================================================================================
//! Describe a sql view
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct SqlViewBuilder
    {
    private:
        NativeSqlBuilder m_name;
        NativeSqlBuilder::List m_selectList;
        bool m_isTmp;
        bool m_isNullView;
        Utf8String m_sqlComment;

    public:
        SqlViewBuilder();
        void MarkAsNullView();
        bool IsNullView() const;
        NativeSqlBuilder& GetNameBuilder();
        NativeSqlBuilder& AddSelect();
        void SetComment(Utf8CP comment) { m_sqlComment.assign(comment); }
        bool IsEmpty() const;
        bool IsValid() const;
        Utf8CP GetName() const;
        bool IsTemporary() const;
        Utf8String ToString(SqlOption option, bool escape = false, bool useUnionAll = true) const;
    };

//=======================================================================================
//! Allow to analyse ECDbMap and build trigger/view necessary for operations
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct ECDbMapAnalyser
    {
    struct Class;
    struct Relationship;
    struct Storage
        {
    private:
        DbTable const& m_table;
        std::set<Class*> m_classes;
        std::set<Relationship*> m_relationships;
        std::map<Storage*, std::set<Relationship*>> m_cascades;

    public:
        explicit Storage(DbTable const& table) :m_table(table){}

        DbTable const& GetTable() const { return m_table; }
        bool IsVirtual() const { return m_table.GetPersistenceType() == PersistenceType::Virtual; }
        std::set<Class*> & GetClassesR() { return m_classes; }
        std::set<Class*> const& GetClasses() const { return m_classes; }
        std::set<Relationship*> & GetRelationshipsR() { return m_relationships; }
        std::map<Storage*, std::set<Relationship*>> & CascadesTo() { return m_cascades; }
        };

    //=======================================================================================
    //! Wraps up a concept of ECClass
    // @bsiclass                                               Affan.Khan          09/2015
    //+===============+===============+===============+===============+===============+======
    struct Class
        {
        private:
            ClassMapCR m_classMap;
            Storage& m_storage;
            bool m_inQueue;
            Class* m_parent;
            Utf8String m_name;
            std::map <Storage const*, std::set<Class const*>> m_partitions;
        public:
            Class(ClassMapCR classMap, Storage& storage, Class* parent);
            Utf8CP GetSqlName() const;
            Storage const& GetStorage() const { return m_storage; }
            ClassMapCR GetClassMap() const;
            void SetParent(Class& cl) { m_parent = &cl; }
            std::map <Storage const*, std::set<Class const*>>& GetPartitionsR();
            bool InQueue() const;
            void Done();
            std::vector<Storage const*> GetNoneVirtualStorages() const;
            bool RequireView() const;
        };

    //=======================================================================================
    //! Wrap up concept of a ECRelationshipClass
    // @bsiclass                                               Affan.Khan          09/2015
    //+===============+===============+===============+===============+===============+======
    struct Relationship : Class
        {
        enum class EndType
            {
            From, To
            };

        enum class PersistanceLocation
            {
            From, To, Self
            };

        struct EndInfo
            {
            private:
                Utf8CP m_accessString;
                DbColumn const* m_column;
                EndInfo(EndInfo const&);
                EndInfo& operator = (EndInfo const&);
            public:
                EndInfo(Utf8CP accessString, DbColumn const&);
                EndInfo(PropertyMapCR);
                EndInfo(PropertyMapCR, Storage const&, DbColumn::Kind);
                EndInfo(EndInfo const&& rhs);
                EndInfo();
                DbColumn const& GetColumn() const;
            };

        struct EndPoint
            {
            private:
                std::set<Class*> m_classes;
                PropertyMapCP m_ecid;
                ECClassIdRelationshipConstraintPropertyMap const* m_classId;
                Relationship const& m_parent;
            public:
                EndPoint(Relationship const& parent, EndType);
                std::set<Class*>& GetClassesR();
                std::set<Storage const*> GetStorages() const;
                PropertyMapCP GetInstanceId() const;
                ECClassIdRelationshipConstraintPropertyMap const* GetClassId() const;
                EndInfo GetResolvedInstanceId(Storage const& forStorage) const;
            };

        private:
            EndPoint m_from;
            EndPoint m_to;
            ForeignKeyDbConstraint::ActionType m_onDeleteAction;
            ForeignKeyDbConstraint::ActionType m_onUpdateAction;

        public:
            Relationship(RelationshipClassMapCR, Storage&, Class* parent);
            RelationshipClassMapCR GetRelationshipClassMap() const;
            PersistanceLocation GetPersistanceLocation() const;
            bool RequireCascade() const;
            bool IsLinkTable() const;
            EndPoint& From();
            EndPoint& To();
            EndPoint& ForeignEnd() { BeAssert(!IsLinkTable()); return GetPersistanceLocation() == PersistanceLocation::From ? From() : To(); }
            EndPoint& ReferencedEnd() { BeAssert(!IsLinkTable()); return GetPersistanceLocation() == PersistanceLocation::To ? From() : To(); }
            bool IsHolding() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Holding; }
            bool IsReferencing() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Referencing; }
            bool IsEmbedding() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Embedding; }
            bool IsMarkedForCascadeDelete() const { BeAssert(!IsLinkTable()); return m_onDeleteAction == ForeignKeyDbConstraint::ActionType::Cascade; }
            bool IsMarkedForCascadeUpdate() const { BeAssert(!IsLinkTable()); return m_onUpdateAction == ForeignKeyDbConstraint::ActionType::Cascade; }
        };

    private:
        //=======================================================================================
        //! Hold view information about those classes which required a view for polymorphic delete
        //! and update
        // @bsiclass                                               Affan.Khan          09/2015
        //+===============+===============+===============+===============+===============+======
        struct ViewInfo
            {
            private:
                SqlTriggerBuilder::TriggerList m_triggers;
                SqlViewBuilder m_view;
            public:
                ViewInfo() {}
                ViewInfo(ViewInfo const& rhs) :m_triggers(rhs.m_triggers), m_view(rhs.m_view) {}
                ViewInfo(ViewInfo const&& rhs) :m_triggers(std::move(rhs.m_triggers)), m_view(std::move(rhs.m_view)) {}
                ViewInfo& operator = (ViewInfo const& rhs)
                    {
                    if (this != &rhs)
                        {
                        m_triggers = rhs.m_triggers;
                        m_view = rhs.m_view;
                        }
                    return *this;
                    }
                ViewInfo& operator = (ViewInfo const&& rhs)
                    {
                    if (this != &rhs)
                        {
                        m_triggers = std::move(rhs.m_triggers);
                        m_view = std::move(rhs.m_view);
                        }
                    return *this;
                    }
                SqlViewBuilder& GetViewR() { return m_view; }
                SqlTriggerBuilder::TriggerList& GetTriggersR() { return m_triggers; }
                SqlViewBuilder const& GetView() const { return m_view; }
                SqlTriggerBuilder::TriggerList const& GetTriggers() const { return m_triggers; }

            };

        ECDbMapR m_map;
        mutable std::map<ECN::ECClassId, std::set<ECN::ECClassId>> m_derivedClassLookup;
        std::map<ECN::ECClassId, std::unique_ptr<Class>> m_classes;
        std::map<ECN::ECClassId, std::unique_ptr<Relationship>> m_relationships;
        std::map<Utf8CP, std::unique_ptr<Storage>, CompareIUtf8Ascii> m_storage;
        std::map<Class const*, ViewInfo> m_viewInfos;

    private:
        Storage& GetStorage(Utf8CP tableName);
        Storage& GetStorage(ClassMapCR classMap) { return GetStorage(classMap.GetJoinedTable().GetName().c_str()); }
        Class& GetClass(ClassMapCR);
        Relationship& GetRelationship(RelationshipClassMapCR);
        BentleyStatus AnalyseClass(ClassMapCR);
        BentleyStatus AnalyseRelationshipClass(RelationshipClassMapCR);
        void GetClassIds(std::vector<ECN::ECClassId>& rootClassIds, std::vector<ECN::ECClassId>& rootRelationshipClassIds) const;
        std::set<ECN::ECClassId> const& GetDerivedClassIds(ECN::ECClassId baseClassId) const;
        void SetupDerivedClassLookup();
        ClassMapCP GetClassMap(ECN::ECClassId classId) const;
        SqlViewBuilder BuildView(Class& nclass);
        BentleyStatus BuildPolymorphicDeleteTrigger(Class&);
        BentleyStatus BuildPolymorphicUpdateTrigger(Class&);
        static NativeSqlBuilder GetClassFilter(std::pair<ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>> const& partition);
        DbResult ApplyChanges();
        DbResult ExecuteDDL(Utf8CP sql);
        ViewInfo* GetViewInfoForClass(Class const&);

    public:
        explicit ECDbMapAnalyser(ECDbMapR ecdbMap) : m_map(ecdbMap) {}
        BentleyStatus Analyse(bool applyChanges);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
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
                TriggerList (List const&& list);
                TriggerList ();
                SqlTriggerBuilder& Create (SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary);
                List const& GetTriggers () const;
                void Delete (SqlTriggerBuilder const& trigger);
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
        SqlTriggerBuilder (){}
        SqlTriggerBuilder (Type type, Condition condition, bool temprary);
        SqlTriggerBuilder (SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder (SqlTriggerBuilder const& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder const& rhs);
        NativeSqlBuilder& GetNameBuilder ();
        NativeSqlBuilder& GetWhenBuilder ();
        NativeSqlBuilder& GetBodyBuilder ();
        NativeSqlBuilder& GetOnBuilder ();
        Type GetType () const;
        Condition GetCondition () const;
        std::vector<Utf8String> const& GetUpdateOfColumns () const;
        std::vector<Utf8String>& GetUpdateOfColumnsR ();
        Utf8CP GetName () const;
        Utf8CP GetWhen () const;
        Utf8CP GetBody () const;
        Utf8CP GetOn () const;
        bool IsTemporary () const;
        bool IsValid () const;
        Utf8String ToString (SqlOption option, bool escape) const;
        bool IsEmpty () const { return m_body.IsEmpty (); }
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
        SqlViewBuilder ();
        void MarkAsNullView ();
        bool IsNullView () const;
        NativeSqlBuilder& GetNameBuilder ();
        void SetTemporary (bool tmp);
        NativeSqlBuilder& AddSelect ();
        void SetComment(Utf8CP comment) { m_sqlComment.assign(comment); }
        bool IsEmpty () const;
        bool IsValid () const;
        Utf8CP GetName () const;
        bool IsTemporary () const;
        bool IsCompound () const;
        Utf8String ToString (SqlOption option, bool escape = false, bool useUnionAll = true) const;
    };

//=======================================================================================
//! Allow to analyse ECDbMap and build trigger/view necessary for operations
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct ECDbMapAnalyser
    {
    struct Class;
    struct Struct;
    struct Relationship;
    struct Storage
        {
        typedef std::unique_ptr<Storage> Ptr;
        private:

            ECDbSqlTable const& m_table;
            std::set<Class*> m_classes;
            std::set<Relationship*> m_relationships;
            std::map<Storage*, std::set<Relationship*>> m_cascades;
            std::set<Struct*> m_structCascades;
            SqlTriggerBuilder::TriggerList m_triggers;

        public:
            Storage(ECDbSqlTable const& table);
            ECDbSqlTable const& GetTable() const;
            bool IsVirtual() const;
            std::set<Class*> & GetClassesR();
            std::set<Class*> const& GetClasses() const { return m_classes; }
            std::set<Relationship*> & GetRelationshipsR();
            std::map<Storage*, std::set<Relationship*>> & CascadesTo();
            std::set<Struct* > &StructCascadeTo()
                {
                return m_structCascades;
                }
            SqlTriggerBuilder::TriggerList& GetTriggerListR();
            SqlTriggerBuilder::TriggerList const& GetTriggerList() const;
            void HandleCascadeLinkTable(std::vector<Relationship*> const& relationships);
            void Generate();
        };
    //=======================================================================================
    //! Wraps up a concept of ECClass
    // @bsiclass                                               Affan.Khan          09/2015
    //+===============+===============+===============+===============+===============+======
    struct Class
        {
        typedef  std::unique_ptr<Class> Ptr;
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
            Storage& GetStorageR();
            Storage const& GetStorage() const
                {
                return m_storage;
                }
            ClassMapCR GetClassMap() const;
            Class* GetParent();
            void SetParent(Class& cl) { m_parent = &cl; }
            std::map <Storage const*, std::set<Class const*>>& GetPartitionsR();
            bool InQueue() const;
            void Done();
            std::vector<Storage const*> GetNoneVirtualStorages() const;
            bool IsAbstract() const;
            bool RequireView() const;
        };
    //=======================================================================================
    //! Wrap up concept of a sturct array
    // @bsiclass                                               Affan.Khan          09/2015
    //+===============+===============+===============+===============+===============+======
    struct Struct : Class
        {
        typedef  std::unique_ptr<Struct> Ptr;
        public:
            Struct(ClassMapCR classMap, Storage& storage, Class* parent)
                :Class(classMap, storage, parent)
                {}
        };

    //=======================================================================================
    //! Wrap up concept of a ECRelationshipClass
    // @bsiclass                                               Affan.Khan          09/2015
    //+===============+===============+===============+===============+===============+======
    struct Relationship : Class
        {
        typedef  std::unique_ptr<Relationship> Ptr;
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
                ECDbSqlColumn const* m_column;
                EndInfo(EndInfo const&);
                EndInfo& operator = (EndInfo const&);
            public:
                EndInfo(Utf8CP accessString, ECDbSqlColumn const& column);
                EndInfo(PropertyMapCR map);
                EndInfo(PropertyMapCR map, Storage const& storage, ColumnKind columnType);
                EndInfo(EndInfo const&& rhs);
                EndInfo();
                EndInfo& operator = (EndInfo const&& rhs);
                Utf8CP GetAccessString() const;
                ECDbSqlColumn const& GetColumn() const;
            };

        struct EndPoint
            {
            private:
                std::set<Class*> m_classes;
                PropertyMapCP m_ecid;
                ECClassIdRelationshipConstraintPropertyMap const* m_classId;
                EndType m_type;
                Relationship const& m_parent;
            public:
                EndPoint(Relationship const& parent, EndType type);
                std::set<Class*>& GetClassesR();
                std::set<Storage const*> GetStorages() const;
                PropertyMapCP GetInstanceId() const;
                ECClassIdRelationshipConstraintPropertyMap const* GetClassId() const;
                EndType GetEnd() const;
                bool Contains(Class const& constraintClass) const;
                EndInfo GetResolvedInstanceId(Storage const& forStorage) const;
                EndInfo GetResolvedClassId(Storage const& forStorage) const;

            };

        private:
            EndPoint m_from;
            EndPoint m_to;
            ForeignKeyActionType m_onDeleteAction;
            ForeignKeyActionType m_onUpdateAction;

        public:
            Relationship(RelationshipClassMapCR classMap, Storage& storage, Class* parent);
            RelationshipClassMapCR GetRelationshipClassMap() const;
            PersistanceLocation GetPersistanceLocation() const;
            bool RequireCascade() const;
            bool IsLinkTable() const;
            EndPoint& From();
            EndPoint& To();
            EndPoint& ForeignEnd()
                {
                BeAssert(!IsLinkTable());
                return GetPersistanceLocation() == PersistanceLocation::From ? From() : To();
                }
            EndPoint& ReferencedEnd()
                {
                BeAssert(!IsLinkTable());
                return GetPersistanceLocation() == PersistanceLocation::To ? From() : To();
                }
            bool IsHolding() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Holding; }
            bool IsReferencing() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Referencing; }
            bool IsEmbedding() const { return GetRelationshipClassMap().GetRelationshipClass().GetStrength() == ECN::StrengthType::Embedding; }
            bool IsMarkedForCascadeDelete() const
                {
                BeAssert(!IsLinkTable());
                return m_onDeleteAction == ForeignKeyActionType::Cascade;
                }
            bool IsMarkedForCascadeUpdate() const
                {
                BeAssert(!IsLinkTable());
                return m_onUpdateAction == ForeignKeyActionType::Cascade;
                }

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
                ViewInfo(ViewInfo const& rhs)
                    :m_triggers(rhs.m_triggers), m_view(rhs.m_view)
                    {
                    }
                ViewInfo(ViewInfo const&& rhs)
                    :m_triggers(std::move(rhs.m_triggers)), m_view(std::move(rhs.m_view))
                    {
                    }
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

        mutable std::map<ECN::ECClassId, std::set<ECN::ECClassId>> m_derivedClassLookup;
        ECDbMapR m_map;
        std::map<ECN::ECClassId, Class::Ptr> m_classes;
        std::map<ECN::ECClassId, Relationship::Ptr> m_relationships;
        std::map<Utf8CP, Storage::Ptr, CompareIUtf8> m_storage;
        std::map<Class const*, ViewInfo> m_viewInfos;

    private:
        ECDbMapR GetMapR() { return m_map; }
        ECDbMapCR GetMap() const { return m_map; }
        Storage& GetStorage(Utf8CP tableName);
        Storage& GetStorage(ClassMapCR classMap);
        Class& GetClass(ClassMapCR classMap);
        Relationship&  GetRelationship(RelationshipClassMapCR classMap);
        BentleyStatus AnalyseRelationshipClass(RelationshipClassMapCR ecRelationshipClassMap);
        std::vector<ECN::ECClassId> GetRootClassIds() const;
        std::vector<ECN::ECClassId> GetRelationshipClassIds() const;
        std::set<ECN::ECClassId> const& GetDerivedClassIds(ECN::ECClassId baseClassId) const;
        ClassMapCP GetClassMap(ECN::ECClassId classId) const;
        void SetupDerivedClassLookup();
        void ProcessEndTableRelationships();
        void ProcessLinkTableRelationships();
        SqlViewBuilder BuildView(Class& nclass);
        BentleyStatus BuildPolymorphicDeleteTrigger(Class& nclass);
        BentleyStatus BuildPolymorphicUpdateTrigger(Class& nclass);
        void HandleLinkTable(Storage* fromStorage, std::map<Storage*, std::set<ECDbMapAnalyser::Relationship*>> const& relationshipsByStorage, bool isFrom);
        static NativeSqlBuilder GetClassFilter(std::pair<ECDbMapAnalyser::Storage const*, std::set<ECDbMapAnalyser::Class const*>> const& partition);
        DbResult ApplyChanges();
        DbResult ExecuteDDL(Utf8CP sql);
        ViewInfo* GetViewInfoForClass(Class const& nclass);

    public:
        explicit ECDbMapAnalyser(ECDbMapR ecdbMap) : m_map(ecdbMap) {}
        BentleyStatus Analyse(bool applyChanges);
    };


//=======================================================================================
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct ECClassViewGenerator : NonCopyableClass
    {
    private:
        ECDbMapCR m_map;

    private:
        BentleyStatus BuildRelationshipJoinIfAny(NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint) const;
        BentleyStatus BuildEndTableRelationshipView(NativeSqlBuilder::List& viewSql, RelationshipClassMapCR classMap) const;
        BentleyStatus BuildClassView(SqlViewBuilder& viewBuilder, ClassMap const& classMap) const;

        static BentleyStatus BuildSelectionClause(NativeSqlBuilder& viewSql, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool nullValue);
        static BentleyStatus BuildSystemSelectionClause(NativeSqlBuilder::List& fragments, ClassMapCR baseClassMap, ClassMapCR classMap, bool nullValue);

        static BentleyStatus BuildPrimitivePropertyExpression(NativeSqlBuilder& viewSql, PropertyMap const&, Utf8CP tablePrefix, bool nullValue);
        static BentleyStatus BuildPointPropertyExpression(NativeSqlBuilder& viewSql, PointPropertyMap const& propertyMap, Utf8CP tablePrefix, bool nullValue);
        static BentleyStatus BuildStructPropertyExpression(NativeSqlBuilder& viewSql, StructPropertyMap const& propertyMap, Utf8CP tablePrefix, bool nullValue);
        static BentleyStatus BuildPropertyExpression(NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool nullValue);

        static BentleyStatus BuildECClassIdConstraintExpression(NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool nullValue);
        static BentleyStatus BuildECInstanceIdConstraintExpression(NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool nullValue);

        static BentleyStatus BuildColumnExpression(NativeSqlBuilder::List& colExpList, bool isNullValue, Utf8CP tablePrefix, Utf8CP columnName, Utf8CP castTargetType, Utf8CP columnAlias);

        void DropExistingViews() const;

        static Utf8String BuildViewClassName(ECN::ECClassCR ecClass);
        static Utf8String BuildSchemaQualifiedClassName(ECN::ECClassCR ecClass);
        static Utf8CP GetECClassIdPrimaryTableAlias(ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }

        static Utf8CP DetermineCastTargetType(ECDbSqlColumn const&, ECN::ECPropertyCR);
        static Utf8CP DetermineCastTargetType(ECDbSqlColumn const&, ECDbSqlColumn::Type targetType);

        static void CollectDerivedEndTableRelationships(std::set<RelationshipClassEndTableMap const*>& childMaps, RelationshipClassMapCR classMap);

    public:
        explicit ECClassViewGenerator(ECDbMapCR map) : m_map(map) {}
        BentleyStatus BuildViews(std::vector<ClassMap const*> const& classMaps) const;
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
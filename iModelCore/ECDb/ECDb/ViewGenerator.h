/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ViewGenerator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlPrepareContext;
enum class SqlOption
    {
    Create,
    CreateIfNotExist,
    Drop,
    DropIfExists
    };


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
        struct TriggerList
            {
            typedef std::vector<SqlTriggerBuilder> List;
            private:
                List m_list;
            public:
                TriggerList (List const&& list)
                    :m_list (std::move (list))
                    {
                    }

                TriggerList ()
                    {}
                SqlTriggerBuilder& Create (SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
                    {
                    m_list.push_back (SqlTriggerBuilder (type, condition, temprary));
                    return m_list.back ();
                    }
                List const& GetTriggers () const { return m_list; }
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
        bool IsTemprory () const;
        bool IsValid () const;
        const Utf8String ToString (SqlOption option, bool escape) const;
        bool IsEmpty () const { return m_body.IsEmpty (); }
    };
struct SqlViewBuilder
    {
    private:
        NativeSqlBuilder m_name;
        NativeSqlBuilder::List m_selectList;
        bool m_isTmp;
        bool m_isNullView;
    public:
        SqlViewBuilder ()
            :m_isTmp (false), m_isNullView (false)
            {
            }
        void MarkAsNullView ()
            {
            m_isNullView = true;
            }

        bool IsNullView () const { return m_isNullView; }
        NativeSqlBuilder& GetNameBuilder ()  { return m_name; }
        void SetTemprory (bool tmp) { m_isTmp = tmp; }
        NativeSqlBuilder& AddSelect ()
            {
            m_selectList.push_back (NativeSqlBuilder ());
            return m_selectList.back ();
            }
        bool IsEmpty () const
            {
            return m_selectList.empty () && m_name.IsEmpty ();
            }
        bool IsValid () const
            {
            if (m_name.IsEmpty ())
                {
                BeAssert (false && "Must specify a view name");
                return false;
                }

            if (m_selectList.empty ())
                {
                BeAssert (false && "View must have atleast one select statement");
                return false;
                }

            return true;
            }
        Utf8CP GetName () const{ return m_name.ToString (); }
        bool IsTemprory () const { return m_isTmp; }
        bool IsCompound () { return m_selectList.size () > 1; }
        const Utf8String ToString (SqlOption option, bool escape = false, bool useUnionAll = true) const
            {
            if (!IsValid ())
                {
                BeAssert (false && "view specification is not valid");
                }

            NativeSqlBuilder sql;
            if (option == SqlOption::Drop || option == SqlOption::DropIfExists)
                {
                sql.Append ("DROP VIEW ").AppendIf (option == SqlOption::DropIfExists, "IF EXISTS ").AppendEscapedIf (escape, GetName ()).Append (";");
                }
            else
                {
                sql.AppendLine ("--### WARNING: SYSTEM GENERATED VIEW. DO NOT CHANGE THIS VIEW IN ANYWAY. ####");
                sql.Append ("CREATE ").AppendIf (IsTemprory (), "TEMP ").Append ("VIEW ").AppendIf (option == SqlOption::CreateIfNotExist, "IF NOT EXISTS ").AppendEscapedIf (escape, GetName ()).AppendEOL ();
                sql.Append ("AS").AppendEOL ();
                for (auto& select : m_selectList)
                    {
                    if (&select != &m_selectList.front ())
                        sql.AppendTAB ().Append ("UNION ").AppendIf (useUnionAll, "ALL").AppendEOL ();

                    sql.AppendTAB (2).AppendLine (select.ToString ());
                    }
                sql.Append (";");
                }

            return sql.ToString ();
            }
    };

  
struct NClass;
struct NRelationship;
struct NConstraint
    {
    typedef std::unique_ptr<NConstraint> Ptr;
    enum class Type
        {
        Relationship, StructArray
        };

    private:
        NClass const& m_class;
        SqlTriggerBuilder::TriggerList m_triggers;

    public:
        virtual Type GetType () const = 0;
        NClass const& GetClass () const;
        NConstraint (NClass const& nclass);
        SqlTriggerBuilder::TriggerList& GetTriggersR () { return m_triggers; }
    };

struct NRelationshipConstraint : NConstraint
    {
    typedef std::unique_ptr<NRelationshipConstraint> Ptr;
    typedef std::map<ECN::ECClassId, ECDbMap::LightWeightMapCache::RelationshipEnd> RelationshipEndByClassIds;
    private:
        RelationshipEndByClassIds m_constraintClasses;
        NRelationshipConstraint (NRelationship const& nclass);

    public:
        void SetEnd (ECN::ECClassId classId, ECDbMap::LightWeightMapCache::RelationshipEnd end);
        virtual NConstraint::Type GetType () const override;
        NRelationship const& GetRelationship () const;
        static Ptr Create (NRelationship const& nclass);
        RelationshipEndByClassIds const& GetRelationshipEnds () const { return m_constraintClasses; }
    };

struct NStructArrayConstraint : NConstraint
    {
    typedef std::unique_ptr<NStructArrayConstraint> Ptr;
    std::set<ECN::ECClassId> m_constraintClasses;
    private:
        NStructArrayConstraint (NClass const& view);

    public:
        virtual NConstraint::Type GetType () const override;
        void SetEnd (ECN::ECClassId classId);
        static Ptr Create (NClass const& nclass);
    };

struct NTable
    {
    typedef std::unique_ptr<NTable> Ptr;
    private:
        ECDbSqlTable const& m_table;
        std::set<ECN::ECClassId> m_classIds;
        std::set<ECN::ECClassId> m_relationshipIds;
        std::map < ECN::ECClassId, NConstraint::Ptr> m_constraints;
        NTable (ECDbSqlTable const& table, std::set<ECN::ECClassId> const& classIds);
        SqlTriggerBuilder::TriggerList m_triggers;
    public:
        std::set<ECN::ECClassId> const& GetClassIds () const { return m_classIds; }
        std::set<ECN::ECClassId> const&  GetRelationshipIds () const { return m_relationshipIds; }
        std::set<ECN::ECClassId>&  GetRelationshipIdsR ()  { return m_relationshipIds; }
        std::map < ECN::ECClassId, NConstraint::Ptr> const& GetConstraints () const { return m_constraints; }
        NConstraint* FindConstraint (ECN::ECClassId constraintClassId);
        void AppendRelationshipConstraint (NRelationship const& relationship, ECN::ECClassId constraintClassId, ECDbMap::LightWeightMapCache::RelationshipEnd end);
        void AppendStructArrayConstraint (NClass const& structClass, ECN::ECClassId constraintClassId);
        bool StoreMany () const;
        bool StoreOne () const;
        ECDbSqlTable const& GetTable () const { return m_table; }
        SqlTriggerBuilder::TriggerList& GetTriggersR () { return m_triggers; }
        static Ptr Create (ECDbSqlTable const& table, std::vector<ECN::ECClassId> const& classIds);
    };

struct NClass
    {
    typedef std::unique_ptr<NClass> Ptr;
    typedef std::map<NTable*, std::set<ECN::ECClassId>> HorizontalParititions;
    enum class Type
        {
        Class, Relationship
        }; 
    private:
        ECN::ECClassId m_classId;
        HorizontalParititions m_classesPerTable;
        SqlTriggerBuilder::TriggerList m_triggers;
        SqlViewBuilder m_viewBuilder;
    protected:
        NClass (ECN::ECClassId classId);
    public:
        virtual Type GetType () const;
        ECN::ECClassId GetClassId () const;
        bool IsCompound () const;
        bool IsEmpty () const;
        static Ptr Create (ECN::ECClassId classId);
        HorizontalParititions const& GetPartitions () const;
        HorizontalParititions & GetPartitionsR ();
        SqlTriggerBuilder::TriggerList& GetTriggersR () { return m_triggers; }
        SqlViewBuilder & GetViewBuilderR () { return m_viewBuilder; }
    };

struct NRelationship : NClass
    {
    typedef std::unique_ptr<NRelationship> Ptr;
    private:
        ECDbMap::LightWeightMapCache::RelationshipType m_type;
        NRelationship (ECN::ECClassId classId, ECDbMap::LightWeightMapCache::RelationshipType type);
    public:
        virtual Type GetType () const override;
        ECDbMap::LightWeightMapCache::RelationshipType GetMapType () const;
        bool IsLinkTable () const;
        static Ptr Create (ECN::ECClassId classId, ECDbMap::LightWeightMapCache::RelationshipType type);
    };



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
            Storage (ECDbSqlTable const& table);
            ECDbSqlTable const& GetTable () const;
            bool IsVirtual () const;
            std::set<Class*> & GetClassesR ();
            std::set<Relationship*> & GetRelationshipsR ();
            std::map<Storage*, std::set<Relationship*>> & CascadesTo ();
            std::set<Struct* > &StructCascadeTo ()
                {
                return m_structCascades;
                }
            SqlTriggerBuilder::TriggerList& GetTriggerListR ();
            SqlTriggerBuilder::TriggerList const& GetTriggerList () const;
            void HandleStructArray ();
            void HandleCascadeLinkTable (std::vector<Relationship*> const& relationships);
            void Generate ();
        };
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
            Class (ClassMapCR classMap, Storage& storage, Class* parent);
            Utf8CP GetSqlName () const;
            Storage& GetStorageR ();
            Storage const& GetStorage () const 
                {
                return m_storage;
                }
            ClassMapCR GetClassMap () const;
            Class* GetParent ();
            std::map <Storage const*, std::set<Class const*>>& GetPartitionsR ();
            bool InQueue () const;
            void Done ();
            std::vector<Storage const*> GetNoneVirtualStorages () const;
            bool IsAbstract () const;
            bool RequireView () const;
        };

    struct Struct : Class
        {
        typedef  std::unique_ptr<Struct> Ptr;
        public:
            Struct (ClassMapCR classMap, Storage& storage, Class* parent)
                :Class (classMap, storage, parent)
                {}
        };
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
        struct EndPoint
            {
            private:
                std::set<Class*> m_classes;
                PropertyMapCP m_ecid;
                PropertyMapCP m_classId;
                EndType m_type;
            public:
                EndPoint (RelationshipClassMapCR map, EndType type);
                std::set<Class*>& GetClassesR ();
                std::set<Storage const*> GetStorages () const;

                PropertyMapCP GetInstanceId () const;
                PropertyMapCP GetClassId () const;
                EndType GetEnd () const;
            };
        private:
            EndPoint m_from;
            EndPoint m_to;
        public:
            Relationship (RelationshipClassMapCR classMap, Storage& storage, Class* parent);
            RelationshipClassMapCR GetRelationshipClassMap () const;
            PersistanceLocation GetPersistanceLocation () const;
            bool RequireCascade () const;
            bool IsLinkTable () const;
            EndPoint& From ();
            EndPoint& To ();
            EndPoint& ForeignEnd ()
                {
                BeAssert (!IsLinkTable ());
                return GetPersistanceLocation () == PersistanceLocation::From ? From () : To ();
                }
            EndPoint& PrimaryEnd ()
                {
                BeAssert (!IsLinkTable ());
                return GetPersistanceLocation () == PersistanceLocation::To ? From () : To ();
                }   
            bool IsHolding () const { return GetRelationshipClassMap ().GetRelationshipClass ().GetStrength () == ECN::StrengthType::STRENGTHTYPE_Holding; }
            bool IsReferencing () const { return GetRelationshipClassMap ().GetRelationshipClass ().GetStrength () == ECN::StrengthType::STRENGTHTYPE_Referencing; }
            bool IsEmbedding () const { return GetRelationshipClassMap ().GetRelationshipClass ().GetStrength () == ECN::StrengthType::STRENGTHTYPE_Embedding; }
            
        };
    private:
        struct ViewInfo
            {
            private:
                SqlTriggerBuilder m_deleteTrigger;
                SqlTriggerBuilder m_updateTrigger;
                SqlViewBuilder m_view;
            public: 
                ViewInfo (){}
                ViewInfo (ViewInfo const& rhs)
                    :m_deleteTrigger (rhs.m_deleteTrigger), m_updateTrigger (rhs.m_updateTrigger), m_view (rhs.m_view)
                    {
                    }
                ViewInfo (ViewInfo const&& rhs)
                    :m_deleteTrigger (std::move (rhs.m_deleteTrigger)), m_updateTrigger (std::move (rhs.m_updateTrigger)), m_view (std::move (rhs.m_view))
                    {
                    }
                ViewInfo& operator = (ViewInfo const& rhs)
                        {
                        if (this != &rhs)
                            {
                            m_deleteTrigger = rhs.m_deleteTrigger;
                            m_updateTrigger = rhs.m_updateTrigger;
                            m_view = rhs.m_view;
                            }
                        return *this;
                        }
                ViewInfo& operator = (ViewInfo const&& rhs)
                    {
                    if (this != &rhs)
                        {
                        m_deleteTrigger = std::move(rhs.m_deleteTrigger);
                        m_updateTrigger = std::move (rhs.m_updateTrigger);
                        m_view = std::move (rhs.m_view);
                        }
                    return *this;
                    }
                SqlViewBuilder& GetViewR () { return m_view; }
                SqlTriggerBuilder& GetDeleteTriggerR () { return m_deleteTrigger; }
                SqlTriggerBuilder& GetUpdateTriggerR () { return m_updateTrigger; }
                SqlViewBuilder const& GetView () const { return m_view; }
                SqlTriggerBuilder const& GetDeleteTrigger () const{ return m_deleteTrigger; }
                SqlTriggerBuilder const& GetUpdateTrigger ()const { return m_updateTrigger; }

            };

        mutable std::map<ECN::ECClassId, std::set<ECN::ECClassId>> m_derivedClassLookup;
        ECDbMapR m_map;
        std::map<ECN::ECClassId, Class::Ptr> m_classes;
        std::map<ECN::ECClassId, Relationship::Ptr> m_relationships;
        std::map<Utf8CP, Storage::Ptr, CompareIUtf8> m_storage;
        std::map<Class const*, ViewInfo> m_viewInfos;
    private:
        ECDbMapR GetMapR () { return m_map; }
        ECDbMapCR GetMap () const { return m_map; } 
        Storage& GetStorage (Utf8CP tableName);
        Storage& GetStorage (ClassMapCR classMap);
        Class& GetClass (ClassMapCR classMap);
        Relationship&  GetRelationship (RelationshipClassMapCR classMap);
        BentleyStatus AnalyseClass (ClassMapCR ecClassMap);
        void AnalyseStruct (Class& classInfo);
        BentleyStatus AnalyseRelationshipClass (RelationshipClassMapCR ecRelationshipClassMap);
        const std::vector<ECN::ECClassId> GetRootClassIds () const;
        const std::vector<ECN::ECClassId> GetRelationshipClassIds () const;
        std::set<ECN::ECClassId> const& GetDerivedClassIds (ECN::ECClassId baseClassId) const;
        ClassMapCP GetClassMap (ECN::ECClassId classId) const;
        void SetupDerivedClassLookup ();
        void ProcessEndTableRelationships ();
        void ProcessLinkTableRelationships ();
        SqlViewBuilder BuildView (Class& nclass);
        SqlTriggerBuilder BuildPolymorphicDeleteTrigger (Class& nclass);
        SqlTriggerBuilder BuildPolymorphicUpdateTrigger (Class& nclass);
        void HandleLinkTable (Storage* fromStorage, std::map<Storage*, std::set<ECDbMapAnalyser::Relationship*>> const& relationshipsByStorage, bool isFrom);

        DbResult ApplyChanges ();
        DbResult ExecuteDDL (Utf8CP sql);
        DbResult UpdateHoldingView ();
        ViewInfo* GetViewInfoForClass (Class const& nclass);
    public:
        ECDbMapAnalyser (ECDbMapR ecdbMap);
        BentleyStatus Analyser (bool applyChanges);
    };
struct ECDbViewGenerator
    {
    private:
        std::map<Utf8CP, NTable::Ptr, CompareIUtf8> m_tables;
        std::map<ECN::ECClassId, std::unique_ptr<NClass>> m_classes;
    
        NClass* FindClass (ECN::ECClassId classId, bool add);
        ECDbMapCR m_map;
    private:
        const std::set<ECDbSqlTable const*> GetEndTables (ECN::ECRelationshipClassCR relationship, ECN::ECRelationshipEnd end);
        const NativeSqlBuilder CreateFilterList (std::set<ECN::ECClassId> const& allSet, std::set<ECN::ECClassId> const& subSet, Utf8CP columnName = nullptr) const;
        BentleyStatus CreateCascadeDeleteRelationshipTrigger (NRelationship& relationship);
        BentleyStatus CreateDeleteTriggerForConstraints (NTable& table);
        void BuildView (NClass& nclass);
        BentleyStatus BuildHoldingView (NativeSqlBuilder& viewSql);
    public:
        ECDbViewGenerator (ECDbMapCR map)
            :m_map (map)
            {
            }
        void BuildGraph ();


       
        BentleyStatus ComputeView ()
            {
            for (auto& i : m_classes)
                {
                if (i.second->IsCompound ())
                    {
                    BuildView (*i.second); // Will recive DELETE/UPDATE trigger
                    }
                }
            for (auto& i : m_tables)
                {
                auto table = i.second.get ();
                for (auto relationshipId : table->GetRelationshipIds())
                    {
                    auto relationship = static_cast<NRelationship*>(FindClass (relationshipId, false));
                    BeAssert (relationship != nullptr);
                    if (CreateCascadeDeleteRelationshipTrigger (*relationship) != BentleyStatus::SUCCESS)
                        return BentleyStatus::ERROR;
                    }

                if (CreateDeleteTriggerForConstraints (*table) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            
            Utf8String all;
            NativeSqlBuilder b;
            BuildHoldingView (b);
            if (m_map.GetECDbR ().ExecuteSql (b.ToString ()) != DbResult::BE_SQLITE_OK)
                {
                printf ("%s\n", m_map.GetECDbR ().GetLastError ());
                printf (b.ToString ());
                return BentleyStatus::ERROR;
                }

            all.append (b.ToString ());
            for (auto& i : m_classes)
                {
                auto nclass = i.second.get ();
                if (!nclass->GetViewBuilderR ().IsEmpty ())
                    {
                    Utf8String sql;
                    sql.append (nclass->GetViewBuilderR ().ToString (SqlOption::DropIfExists, true));
                    sql.append (nclass->GetViewBuilderR ().ToString (SqlOption::CreateIfNotExist, true));
                    if (m_map.GetECDbR ().ExecuteSql (sql.c_str ()) != DbResult::BE_SQLITE_OK)
                        {
                        printf ("%s\n", m_map.GetECDbR ().GetLastError());
                        printf (sql.c_str ());
                        return BentleyStatus::ERROR;
                        }
                    }
                }
            for (auto& i : m_tables)
                {
                auto table = i.second.get ();
                if (table->GetTable ().GetPersistenceType () == PersistenceType::Virtual)
                    continue;

                for (auto& trigger : table->GetTriggersR ().GetTriggers())
                    {
                    Utf8String sql;
                    sql.append (trigger.ToString (SqlOption::DropIfExists, true));
                    sql.append (trigger.ToString (SqlOption::CreateIfNotExist,true));
                    if (m_map.GetECDbR ().ExecuteSql (sql.c_str ()) != DbResult::BE_SQLITE_OK)
                        {
                        printf ("%s\n", m_map.GetECDbR ().GetLastError ());
                        printf (sql.c_str ());
                        return BentleyStatus::ERROR;
                        }
                    }
                }
            return BentleyStatus::SUCCESS;
            }

    };
//==========================================================================================================================================
//==========================================================================================================================================
//==========================================================================================================================================




    //

struct SqlClassPersistenceMethod : NonCopyableClass
{
private:
    SqlViewBuilder m_viewBuilder;
    NativeSqlBuilder m_tableName;
    NativeSqlBuilder m_rowFilter;
    std::vector<SqlTriggerBuilder> m_triggerBuilderList;
    ClassMapCR m_classMap;
    bool m_finish;
public:
    SqlClassPersistenceMethod (ClassMapCR classMap)
        :m_classMap (classMap), m_finish (false)
        {}

    ~SqlClassPersistenceMethod ()
        {}
    ClassMapCR GetClassMap () const { return m_classMap; }
    NativeSqlBuilder& GetTableNameBuilder () { return m_tableName; }
    NativeSqlBuilder& GetRowFilterBuilder () { return m_rowFilter; }
    SqlViewBuilder& GetViewBuilder () { return m_viewBuilder; }
    bool IsFinished () const { return m_finish; }
    void MarkAsFinish () { m_finish = true; }
    SqlTriggerBuilder& AddTrigger (SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary)
        {
        m_triggerBuilderList.push_back (SqlTriggerBuilder (type, condition, temprary));
        return m_triggerBuilderList.back ();
        }
    Utf8CP GetAffectedTargetId (DMLPolicy::Operation op) const
        {
        auto target = GetClassMap().GetDMLPolicy ().Get (op);
        if (target == DMLPolicy::Target::None)
            return "";

        if (target == DMLPolicy::Target::Table)
            return GetClassMap ().GetTable ().GetName ().c_str();

        return const_cast<SqlClassPersistenceMethod*>(this)->GetViewBuilder ().GetNameBuilder ().ToString ();
        }

    std::vector<SqlTriggerBuilder> const& GetTriggerBuilderList () { return m_triggerBuilderList; }
    void ResetTriggers ()
        {
        m_triggerBuilderList.clear ();
        }
    Utf8String ToString (SqlOption option, bool escape = false)
        {
        NativeSqlBuilder sql;
        sql.AppendLine ("--#-----------------------View--------------------------");
        sql.AppendLine (m_viewBuilder.ToString (option, escape).c_str());

        if (!m_triggerBuilderList.empty ())
            sql.AppendLine ("--#-----------------------Trigger-----------------------");
        for (auto& trigger : m_triggerBuilderList)
            {
            sql.AppendLine (trigger.ToString (option, escape).c_str());
            }

        return sql.ToString();
        }
};






        
//=======================================================================================
// @bsiclass                                               Affan.Khan           06/2015
//+===============+===============+===============+===============+===============+======
struct SqlGenerator
    {

    private:
        ECDbMapR m_map;
        std::map<ECN::ECClassId, std::unique_ptr<SqlClassPersistenceMethod>> m_scpms;
    private:
        const std::vector<ClassMapCP> GetEndClassMaps (ECN::ECRelationshipClassCR relationship, ECN::ECRelationshipEnd end);
         BentleyStatus BuildHoldingConstraint (NativeSqlBuilder& stmt, RelationshipClassMapCR classMap);
         BentleyStatus BuildEmbeddingConstraint (NativeSqlBuilder& stmt, RelationshipClassMapCR classMap);

         SqlClassPersistenceMethod* GetClassPersistenceMethod (ClassMapCR classMap);
         BentleyStatus BuildHoldingView (NativeSqlBuilder& sql);

         BentleyStatus FindRelationshipReferences (bmap<RelationshipClassMapCP, ECDbMap::LightWeightMapCache::RelationshipEnd>& relationships, ClassMapCR classMap);
         void CollectDerivedEndTableRelationships (std::set<RelationshipClassEndTableMapCP>& childMaps, RelationshipClassMapCR classMap);
        
         BentleyStatus BuildDerivedFilterClause (Utf8StringR filter, ECDb& db, ECN::ECClassId baseClassId);
         Utf8CP GetECClassIdPrimaryTableAlias (ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }
         BentleyStatus BuildECInstanceIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildECClassIdConstraintExpression (NativeSqlBuilder::List& fragments, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint, bool topLevel);
         BentleyStatus BuildEndTableRelationshipView (NativeSqlBuilder::List& viewSql, RelationshipClassMapCR classMap);

         BentleyStatus BuildDeleteTriggersForDerivedClasses (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForMe (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForEndTableMe (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggersForRelationships (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggers (SqlClassPersistenceMethod& scpm);
         BentleyStatus BuildDeleteTriggerForStructArrays (SqlClassPersistenceMethod& scpm);

         BentleyStatus BuildPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapCR propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildColumnExpression (NativeSqlBuilder::List& viewSql, Utf8CP tablePrefix, Utf8CP columnName, Utf8CP accessString, bool addECPropertyPathAlias, bool nullValue, bool escapeColumName = true);
         BentleyStatus BuildPointPropertyExpression (NativeSqlBuilder& viewSql, PropertyMapPoint const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildPrimitivePropertyExpression (NativeSqlBuilder& viewSql, PropertyMapToColumn const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildStructPropertyExpression(NativeSqlBuilder& viewSql, PropertyMapToInLineStruct const& propertyMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildSystemSelectionClause (NativeSqlBuilder::List& fragments, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildSelectionClause (NativeSqlBuilder& viewSql, ClassMapCR baseClassMap, ClassMapCR classMap, Utf8CP tablePrefix, bool addECPropertyPathAlias, bool nullValue);
         BentleyStatus BuildClassView (SqlClassPersistenceMethod& scpm);

         BentleyStatus DropViewIfExists (ECDbR map, Utf8CP viewName);
    public:
        SqlGenerator (ECDbMapR map):m_map (map) {}
        static Utf8String BuildViewClassName (ECN::ECClassCR ecClass);
         BentleyStatus BuildViewInfrastructure (std::set<ClassMap const*>& classMaps);
         static Utf8String BuildSchemaQualifiedClassName (ECN::ECClassCR ecClass);

    };

/*=================================================================================**//**
* @bsiclass                                                     Affan.Khan       07/2013
+===============+===============+===============+===============+===============+======*/
struct ViewGenerator
    { 
public:
    //! Name of the ECClassId column in the generated view
    static Utf8CP const ECCLASSID_COLUMNNAME;

private:
    struct ViewMember
        {
    private:
        std::vector<IClassMap const*> m_classMaps;
        DbMetaDataHelper::ObjectType m_storageType;
    public:
        std::vector<IClassMap const*>& GetClassMaps () { return m_classMaps; }
        DbMetaDataHelper::ObjectType GetStorageType() const { return m_storageType;}

        ViewMember()
            :m_storageType(DbMetaDataHelper::ObjectType::Table)
            {
            }
        ViewMember (DbMetaDataHelper::ObjectType storageType, IClassMap const& classMap)
            :m_storageType(storageType)
            {
            m_classMaps.push_back(&classMap);
            }
        };

    typedef bmap<ECDbSqlTable const*, ViewMember> ViewMemberByTable; 
    static BentleyStatus ComputeViewMembers (ViewMemberByTable& viewMembers, ECDbMapCR map, ECN::ECClassCR ecClass, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables, bool ensureDerivedClassesAreLoaded);
    static BentleyStatus GetRootClasses (std::vector<IClassMap const*>& rootClasses, ECDbR db);
    static BentleyStatus GetViewQueryForChild (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, const std::vector<IClassMap const*>& childClassMap, IClassMap const& baseClassMap, bool isPolymorphic);
    //! Relationship polymorphic query
    static BentleyStatus CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, bool isPolymorphic, bool optimizeByIncludingOnlyRealTables);
    static BentleyStatus CreateViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, RelationshipClassEndTableMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationshipClassLinkTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationshipClassEndTableMap (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullViewForRelationship (NativeSqlBuilder& viewSql, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& relationMap, IClassMap const& baseClassMap);
    static BentleyStatus CreateNullView (NativeSqlBuilder& viewSql, ECSqlPrepareContext const& prepareContext, IClassMap const& classMap);
    static BentleyStatus GetAllChildRelationships (std::vector<RelationshipClassMapCP>& relationshipMaps, ECDbMapCR map, ECSqlPrepareContext const& prepareContext, IClassMap const& baseRelationMap);
    static Utf8CP GetECClassIdPrimaryTableAlias (ECN::ECRelationshipEnd endPoint) { return endPoint == ECN::ECRelationshipEnd::ECRelationshipEnd_Source ? "SourceECClassPrimaryTable" : "TargetECClassPrimaryTable"; }

    static BentleyStatus BuildRelationshipJoinIfAny (NativeSqlBuilder& sqlBuilder, RelationshipClassMapCR classMap, ECN::ECRelationshipEnd endPoint);
    //! Append view prop map list separated by comma.
    static BentleyStatus AppendViewPropMapsToQuery (NativeSqlBuilder& viewQuery, ECDbR ecdb, ECSqlPrepareContext const& prepareContext, ECDbSqlTable const& table, std::vector<std::pair<PropertyMapCP, PropertyMapCP>> const& viewPropMaps, bool forNullView = false);

    static BentleyStatus AppendSystemPropMaps (NativeSqlBuilder& viewQuery, ECDbMapCR ecdbMap, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap);
    static BentleyStatus AppendSystemPropMapsToNullView (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, RelationshipClassMapCR relationMap, bool endWithComma);
    static BentleyStatus AppendConstraintClassIdPropMap (NativeSqlBuilder& viewQuery, ECSqlPrepareContext const& prepareContext, PropertyMapRelationshipConstraint const& propMap, ECDbMapCR ecdbMap, RelationshipClassMapCR relationMap, ECN::ECRelationshipConstraintCR constraint);

    //! Return prop maps of child base on parent map. So only prop maps that make up baseClass properties are selected.
    static BentleyStatus GetPropertyMapsOfDerivedClassCastAsBaseClass (std::vector<std::pair<PropertyMapCP, PropertyMapCP>>& propMaps, ECSqlPrepareContext const& prepareContext, IClassMap const& baseClassMap, IClassMap const& childClassMap, bool skipSystemProperties, bool embededStatement);

    static void LoadDerivedClassMaps (std::map<ECN::ECClassId, IClassMap const *>& viewClasses, ECDbMapCR map, IClassMap const* classMap);
    static void CreateSystemClassView (NativeSqlBuilder &viewSql, std::map<ECDbSqlTable const*, std::vector<IClassMap const*>> &tableMap, std::set<ECDbSqlTable const*> &tableToIncludeEntirly, bool forStructArray, ECSqlPrepareContext const& prepareContext);

public:

    //! Create a SQLite polymorphic SELECT query for a given classMap
    //! @param viewSql [out] Output SQL for view
    //! @param map [in] ECDbMap instance
    //! @param classMap [in] Source classMap for which to generate view
    //! @param isPolymorphicQuery [in] if true return a polymorphic view of ECClass else return a non-polymorphic view. Intend to be use by ECSQL "ONLY <ecClass>"
    //! @param structArrayProperty [in] specify structArrayProperty for which struct classmap view is needed
    //! @param optimizeByIncludingOnlyRealTables [in] Enabling would produce small length views but it does take a little long to generate
    //! @return The number of relevant relationships found
    //! @remarks Only work work normal ECClasses but not relationship. It also support query over ecdb.Instances
    static BentleyStatus CreateView (NativeSqlBuilder& viewSql, ECDbMapCR map, IClassMap const& classMap, bool isPolymorphicQuery, ECSqlPrepareContext const& prepareContext, bool optimizeByIncludingOnlyRealTables);
    enum class SystemViewType
        {
        Class,
        RelationshipClass,
        StructArray
        };

    };

END_BENTLEY_SQLITE_EC_NAMESPACE

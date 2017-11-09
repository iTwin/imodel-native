/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ChangeSummaryV2.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECInstanceId.h>
#include <BeSQLite/BeSQLite.h>
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct InstancesTableV2;
struct ValuesTableV2;
struct ChangeExtractorV2;

//=======================================================================================
//! Utility to interpret a set of changes to the database as EC instances. 
//! 
//! @remarks The utility iterates over the raw Sqlite changes to consolidate and extract 
//! information on the contained ECInstances and ECRelationshipInstances. 
//! 
//! Internally two passes are made over the changes with the @ref ChangeIterator. The
//! second pass allows consolidation of instances and relationship instances that may be 
//! spread across multiple tables. The results are stored in temporary tables, and this 
//! allows iteration and queries of the changes as EC instances. 
//! 
//! @see ChangeSet, ChangeIterator
//! @ingroup ECDbGroup
//! @bsiclass                                                 Ramanujam.Raman      07/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ChangeSummaryV2 : NonCopyableClass
    {
    //! DbOpcodes that can be bitwise combined to pass as arguments to query methods
    enum class QueryDbOpcode
        {
        None = 0,
        Insert = 1,
        Delete = 1 << 1,
        Update = 1 << 2,
        All = Insert | Delete | Update,
        InsertUpdate = Insert | Update,
        InsertDelete = Insert | Delete,
        UpdateDelete = Update | Delete
        };

    //! Options to control extraction of the change summary
    struct Options final
        {
        private:
            bool m_includeRelationshipInstances;
        public:
            Options() : m_includeRelationshipInstances(true) {}
            void SetIncludeRelationshipInstances(bool value) { m_includeRelationshipInstances = value; }
            bool GetIncludeRelationshipInstances() const { return m_includeRelationshipInstances; }
        };

    struct Instance;
    struct InstanceIterator;
    struct ValueIterator;

    //! Represents a changed instance
    struct Instance
        {
        private:
            ChangeSummaryV2 const* m_changeSummary = nullptr;
            ECN::ECClassId m_classId;
            ECInstanceId m_instanceId;
            DbOpcode m_dbOpcode;
            int m_indirect;
            Utf8String m_tableName;
            mutable ECSqlStatement m_valuesTableSelect;

            void SetupValuesTableSelectStatement(Utf8CP accessString) const;

        public:
            Instance() {}

            Instance(ChangeSummaryV2 const& changeSummary, ECN::ECClassId classId, ECInstanceId instanceId, DbOpcode dbOpcode, int indirect, Utf8StringCR tableName) :
                m_changeSummary(&changeSummary), m_classId(classId), m_instanceId(instanceId), m_dbOpcode(dbOpcode), m_indirect(indirect), m_tableName(tableName)
                {}

            Instance(Instance const& other) { *this = other; }

            ECDB_EXPORT Instance& operator=(Instance const& other);

            //! Get the class id of the changed instance
            ECN::ECClassId GetClassId() const { return m_classId; }

            //! Get the instance id of the changed instance
            ECInstanceId GetInstanceId() const { return m_instanceId; }

            //! Get the DbOpcode of the changed instance that indicates that the instance was inserted, updated or deleted.
            DbOpcode GetDbOpcode() const { return m_dbOpcode; }

            //! Get the flag indicating if the change was "indirectly" caused by a database trigger or other means. 
            int GetIndirect() const { return m_indirect; }

            //! Get the name of the primary table containing the instance
            Utf8StringCR GetTableName() const { return m_tableName; }

            //! Returns true if the instance is valid. 
            bool IsValid() const { return m_instanceId.IsValid(); }

            //! Returns true if the value specified by the accessString exists
            ECDB_EXPORT bool ContainsValue(Utf8CP accessString) const;

            //! Get a specific changed value
            ECDB_EXPORT DbDupValue GetOldValue(Utf8CP accessString) const;

            //! Get a specific changed value
            ECDB_EXPORT DbDupValue GetNewValue(Utf8CP accessString) const;

        };

    typedef Instance const& InstanceCR;
    private:
        ECDbCR m_ecdb;
        bool m_isValid = false;
        InstancesTableV2* m_instancesTable;
        ValuesTableV2* m_valuesTable;
        ChangeExtractorV2* m_changeExtractor;
        ECInstanceId m_changesetId;

        void Initialize();
        Utf8String FormatInstanceIdStr(ECInstanceId) const;
        Utf8String FormatClassIdStr(ECN::ECClassId) const;

        BentleyStatus DeleteSummaryEntry();
        BentleyStatus CreateSummaryEntry();

    public:
        //! Construct a ChangeSummaryV2 from a BeSQLite ChangeSet
        ECDB_EXPORT explicit ChangeSummaryV2(ECDbCR);

        //! Destructor
        ECDB_EXPORT virtual ~ChangeSummaryV2();

        //! Populate the ChangeSummaryV2 from the contents of a BeSQLite ChangeSet
        //! @remarks The ChangeSummaryV2 needs to be new or freed before this call. 
        //! @see MakeIterator, GetInstancesTableName
        ECDB_EXPORT BentleyStatus FromChangeSet(BeSQLite::IChangeSet& changeSet, Options const& options = Options());

        //! Free the data held by this ChangeSummaryV2.
        //! @note After this call the ChangeSet becomes invalid. Need not be called if used only once - 
        //! the destructor will automatically call Free. 
        ECDB_EXPORT void Free();

        //! Determine whether this ChangeSet holds extracted data or not.
        bool IsValid() const { return m_isValid; }

        //! Get the Db used by this change set
        ECDbCR GetDb() const { return m_ecdb; }



                //! Check if the change summary contains a specific instance
        ECDB_EXPORT bool ContainsInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

        //! Get a specific changed instance
        ECDB_EXPORT Instance GetInstance(ECN::ECClassId classId, ECInstanceId instanceId) const;

        //! Return changeset id fromc ChangeSummaryV2::Changeset::Id
        ECDB_EXPORT ECInstanceId GetId() const { return m_changesetId; }

        Utf8String ConstructWhereInClause(QueryDbOpcode queryDbOpcodes) const; //! @private
        ECDB_EXPORT static BentleyStatus GetMappedPrimaryTable(Utf8StringR tableName, bool& isTablePerHierarcy, ECN::ECClassCR ecClass, ECDbCR ecdb); //!< @private
    };

typedef ChangeSummaryV2 const& ChangeSummaryV2CR;
typedef ChangeSummaryV2 const* ChangeSummaryV2CP;

ENUM_IS_FLAGS(ChangeSummaryV2::QueryDbOpcode);

END_BENTLEY_SQLITE_EC_NAMESPACE

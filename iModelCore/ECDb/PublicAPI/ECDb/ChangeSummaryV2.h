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
    enum class Stage
        {
        Unset = 0,
        Old = 1,
        New = 2,
        };

    enum class Operation
        {
        Inserted = 1,
        Deleted = 1 << 1,
        Updated = 1 << 2
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


    private:
        ECDbCR m_ecdb;
        bool m_isValid = false;
        InstancesTableV2* m_instancesTable;
        ValuesTableV2* m_valuesTable;
        ChangeExtractorV2* m_changeExtractor;
        ECInstanceId m_changesetId;
        bool m_autoCleanUp;
        void Initialize();
        Utf8String FormatInstanceIdStr(ECInstanceId) const;
        Utf8String FormatClassIdStr(ECN::ECClassId) const;

        BentleyStatus DeleteSummaryEntry();
        BentleyStatus CreateSummaryEntry();

    public:
        //! Construct a ChangeSummaryV2 from a BeSQLite ChangeSet
        ECDB_EXPORT explicit ChangeSummaryV2(ECDbCR, bool autoCleanUp);

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

        //! Return changeset id fromc ChangeSummaryV2::Changeset::Id
        ECDB_EXPORT ECInstanceId GetId() const { return m_changesetId; }

    };

typedef ChangeSummaryV2 const& ChangeSummaryV2CR;
typedef ChangeSummaryV2 const* ChangeSummaryV2CP;


END_BENTLEY_SQLITE_EC_NAMESPACE

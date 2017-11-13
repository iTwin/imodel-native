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
struct InstanceChangeManager;
struct PropertyValueChangeManager;
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
    enum class OpCode
        {
        Insert = 1,
        Delete = 2,
        Update = 4,
        };

    struct Options final
        {
        private:
            bool m_includeRelationshipInstances = true;

        public:
            explicit Options(bool includeRelationshipInstances = true) : m_includeRelationshipInstances(includeRelationshipInstances) {}
            bool IncludeRelationshipInstances() const { return m_includeRelationshipInstances; }
        };


    private:
        ECDbCR m_ecdb;
        InstanceChangeManager* m_instanceChangeManager = nullptr;
        PropertyValueChangeManager* m_propertyValueChangeManager = nullptr;
        ChangeExtractorV2* m_changeExtractor = nullptr;
        ECInstanceId m_changeSummaryId;
        bool m_isValid = false;

        BentleyStatus Initialize();
        Utf8String FormatInstanceIdStr(ECInstanceId) const;
        Utf8String FormatClassIdStr(ECN::ECClassId) const;

        BentleyStatus DeleteSummaryEntry();
        BentleyStatus CreateSummaryEntry();

        Utf8String ConstructWhereInClause(OpCode opcodes) const;

    public:
        //! Construct a ChangeSummaryV2 from a BeSQLite ChangeSet
        explicit ChangeSummaryV2(ECDbCR ecdb) : m_ecdb(ecdb) {}

        //! Destructor
        virtual ~ChangeSummaryV2() { Free(); }

        //! Get the Db used by this change set
        ECDbCR GetECDb() const { return m_ecdb; }

        //! Populate the ChangeSummaryV2 from the contents of a BeSQLite ChangeSet
        //! @remarks The ChangeSummaryV2 needs to be new or freed before this call. 
        //! @see MakeIterator, GetInstancesTableName
        ECDB_EXPORT BentleyStatus FromChangeSet(BeSQLite::IChangeSet& changeSet, Options const& options = Options());

        //! Return changeset id from ChangeSummaryV2::Changeset::Id
        ECDB_EXPORT ECInstanceId GetId() const { return m_changeSummaryId; }

        //! Free the data held by this ChangeSummaryV2.
        //! @note After this call the ChangeSet becomes invalid. Need not be called if used only once - 
        //! the destructor will automatically call Free. 
        ECDB_EXPORT void Free();

        //! Determine whether this ChangeSet holds extracted data or not.
        bool IsValid() const { return m_isValid; }


        //! Return changeset id fromc ChangeSummaryV2::Changeset::Id
        ECInstanceId GetId() const { return m_changesetId; }

    };

ENUM_IS_FLAGS(ChangeSummaryV2::OpCode);

END_BENTLEY_SQLITE_EC_NAMESPACE

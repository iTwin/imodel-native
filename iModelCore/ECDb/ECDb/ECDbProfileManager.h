/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include "ECDbProfileUpgrader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      12/2012
//+===============+===============+===============+===============+===============+======
struct ECDbProfileManager
    {
private:
    //=======================================================================================
    // @bsiclass                                                 Krischan.Eberle      07/2013
    //+===============+===============+===============+===============+===============+======
    struct ProfileCreator
        {
    private:
        //non-instantiable class
        ProfileCreator ();
        ~ProfileCreator ();
        static DbResult CreateECProfileTables (Db& db);
        static DbResult CreateTableECSchema (Db& db);
        static DbResult CreateTableECClass (Db& db);
        static DbResult CreateTableECClassMap (Db& db);
        static DbResult CreateTableBaseClass (Db& db);
        static DbResult CreateTableECProperty (Db& db);
        static DbResult CreateTableECRelationshipConstraint (Db& db);
        static DbResult CreateTableECRelationshipConstraintClass (Db& db);
        static DbResult CreateTableECRelationshipConstraintClassKeyProperty(Db& db);
        static DbResult CreateTableCustomAttribute (Db& db);
        static DbResult CreateTableSchemaReferences (Db& db);
        static DbResult CreateTableECPropertyMap (Db& db);
        static DbResult CreateTablePropertyPath (Db& db);
        static DbResult CreateTableTable (Db& db);
        static DbResult CreateTableColumn (Db& db);
        static DbResult CreateTableIndex (Db& db);
        static DbResult CreateTableIndexColumn (Db& db);
        static DbResult CreateTableForeignKey (Db& db);
        static DbResult CreateTableForeignKeyColumn (Db& db);

    public:
        static DbResult Create (ECDbR ecdb);
        };


    typedef std::vector<std::unique_ptr<ECDbProfileUpgrader>> ECDbProfileUpgraderSequence;
    static Utf8CP const PROFILENAME;
    static const PropertySpec PROFILEVERSION_PROPSPEC;
    //! Oldest version supported by this version of ECDb.
    static const SchemaVersion MINIMUM_SUPPORTED_VERSION;

    static ECDbProfileUpgraderSequence s_upgraderSequence;

    //non-instantiable class
    ECDbProfileManager ();
    ~ECDbProfileManager ();

    //! Reads the version of the ECDb profile of the given ECDb file
    //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
    //! ECDb file, i.e. does not have the ECDb profile
    static DbResult ReadProfileVersion (SchemaVersion& profileVersion, ECDbCR ecdb, Savepoint& defaultTransaction);
    static DbResult AssignProfileVersion (ECDbR ecdb);

    //! Version of the ECDb Profile expected by the ECDb API.
    static SchemaVersion GetExpectedProfileVersion ();
    //! Minimum version of the ECDb profile which can still be auto-upgraded to the latest profile version.
    static SchemaVersion GetMinimumAutoUpgradableProfileVersion ();

    static ECDbProfileUpgrader const& GetLatestUpgrader ();

    static ECDbProfileManager::ECDbProfileUpgraderSequence::const_iterator GetUpgraderSequenceFor (SchemaVersion const& currentProfileVersion);
    static ECDbProfileManager::ECDbProfileUpgraderSequence const& GetUpgraderSequence ();

public:
    //! Creates the ECDb profile in the specified ECDb file.
    //! @remarks In case of success the outermost transaction is committed.
    //!     In case of error, the outermost transaction is rolled back.
    //! @param[in] ecdb ECDb file handle to create the profile in.
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult CreateECProfile (ECDbR ecdb);

    //! Upgrades the ECDb profile in the specified ECDb file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @remarks In case an upgrade was necessary and the upgrade was successful,
    //! the outermost transaction is committed. In case of
    //! error, the outermost transaction is rolled back. 
    //! @param[in] ecdb ECDb file handle to upgrade
    //! @param[in] openParams Open params passed by the client
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult UpgradeECProfile (ECDbR ecdb, Db::OpenParams const& openParams);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
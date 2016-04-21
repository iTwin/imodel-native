/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLiteProfileManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <BeSQLite/BeSQLite.h>
#include <vector>

BEGIN_BENTLEY_SQLITE_NAMESPACE

struct BeSQLiteProfileUpgrader;

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      10/2014
//+===============+===============+===============+===============+===============+======
struct BeSQLiteProfileManager
    {
private:
    BeSQLiteProfileManager();
    ~BeSQLiteProfileManager();


    //! Version of the BeSQLite Profile expected by the BeSQLite API.
    static SchemaVersion GetExpectedVersion() { return SchemaVersion(BEDB_CURRENT_VERSION_Major, BEDB_CURRENT_VERSION_Minor, BEDB_CURRENT_VERSION_Sub1, BEDB_CURRENT_VERSION_Sub2); }
    //! Minimum version of the BeSQLite profile which can still be auto-upgraded to the latest profile version.
    static SchemaVersion GetMinimumSupportedVersion() { return SchemaVersion(BEDB_SUPPORTED_VERSION_Major, BEDB_SUPPORTED_VERSION_Minor, BEDB_SUPPORTED_VERSION_Sub1, BEDB_SUPPORTED_VERSION_Sub2); }

    static void GetUpgraderSequence(std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>>&, SchemaVersion const& currentProfileVersion);

    public:
        //! Upgrades the BeSQLite profile in the specified BeSQLite file to the latest version (if the file's profile
        //! is not up-to-date).
        //! @param[in] db BeSQLite file handle to upgrade
        //! @return BE_SQLITE_OK if successful. Error code otherwise.
        static DbResult UpgradeProfile(DbR db);

        //! Reads the version of the BeSQLite profile of the given BeSQLite file
        //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
        //! BeSQLite file, i.e. does not have the BeSQLite profile
        static DbResult ReadProfileVersion(SchemaVersion& profileVersion, DbR db);

        //! Saves the software's expected profile version in the specified BeSQLite file.
        //! @param[in] db BeSQLite file handle to save profile version to
        //! @return BE_SQLITE_OK if successful. Error code otherwise.
        static DbResult AssignProfileVersion(DbR db);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      10/2014
//+===============+===============+===============+===============+===============+======
struct BeSQLiteProfileUpgrader
    {
private:
    virtual DbResult _Upgrade(DbR db) const = 0;

public:
    virtual ~BeSQLiteProfileUpgrader() {}

    DbResult Upgrade(DbR db) const { return _Upgrade(db); }
    };


END_BENTLEY_SQLITE_NAMESPACE
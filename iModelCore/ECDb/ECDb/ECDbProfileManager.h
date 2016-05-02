/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    //non-instantiable class
    ECDbProfileManager();
    ~ECDbProfileManager();

    static DbResult CreateECProfileTables(ECDbCR);
    //! Reads the version of the ECDb profile of the given ECDb file
    //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
    //! ECDb file, i.e. does not have the ECDb profile
    static DbResult ReadProfileVersion(SchemaVersion& profileVersion, ECDbCR, Savepoint& defaultTransaction);
    //! @param onProfileCreation true if this method is called during profile creation. false if 
    //! called during profile upgrade
    static DbResult AssignProfileVersion(ECDbR, bool onProfileCreation);

    //! Expected version of the ECDb profile for this version of the ECDb API.
    static SchemaVersion GetExpectedVersion() { return SchemaVersion(3, 6, 0, 0); }
    //! Minimum version of the ECDb profile which can still be auto-upgraded to the latest profile version.
    static SchemaVersion GetMinimumSupportedVersion() { return SchemaVersion(3, 0, 0, 0); }

    static void GetUpgraderSequence(std::vector<std::unique_ptr<ECDbProfileUpgrader>>&, SchemaVersion const& currentProfileVersion);

    static PropertySpec GetProfileVersionPropertySpec() { return PropertySpec("SchemaVersion", "ec_Db"); }
    static PropertySpec GetInitialProfileVersionPropertySpec() { return PropertySpec("InitialSchemaVersion", "ec_Db"); }

public:
    //! Creates the ECDb profile in the specified ECDb file.
    //! @remarks In case of success the outermost transaction is committed.
    //!     In case of error, the outermost transaction is rolled back.
    //! @param[in] ecdb ECDb file handle to create the profile in.
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult CreateECProfile(ECDbR ecdb);

    //! Upgrades the ECDb profile in the specified ECDb file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @remarks In case an upgrade was necessary and the upgrade was successful,
    //! the outermost transaction is committed. In case of
    //! error, the outermost transaction is rolled back. 
    //! @param[in] ecdb ECDb file handle to upgrade
    //! @param[in] openParams Open params passed by the client
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult UpgradeECProfile(ECDbR ecdb, Db::OpenParams const& openParams);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
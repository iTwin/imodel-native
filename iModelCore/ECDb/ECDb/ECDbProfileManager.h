/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    static SchemaVersion GetExpectedVersion() { return SchemaVersion(3, 104, 0, 0); }
    //! Minimum version of the ECDb profile which can still be auto-upgraded to the latest profile version.
    static SchemaVersion GetMinimumSupportedVersion() { return SchemaVersion(3, 104, 0, 0); }

    static DbResult RunUpgraders(ECDbCR, SchemaVersion const& currentProfileVersion);

    static PropertySpec GetProfileVersionPropertySpec() { return PropertySpec("SchemaVersion", "ec_Db"); }
    static PropertySpec GetInitialProfileVersionPropertySpec() { return PropertySpec("InitialSchemaVersion", "ec_Db"); }

public:
    //! Creates the ECDb profile in the specified ECDb file.
    //! @remarks In case of success the outermost transaction is committed.
    //!     In case of error, the outermost transaction is rolled back.
    //! @param[in] ecdb ECDb file handle to create the profile in.
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult CreateProfile(ECDbR ecdb);

    //! Upgrades the ECDb profile in the specified ECDb file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @remarks In case an upgrade was necessary and the upgrade was successful,
    //! the outermost transaction is committed. In case of
    //! error, the outermost transaction is rolled back. 
    //! @param[in] ecdb ECDb file handle to upgrade
    //! @param[in] openParams Open params passed by the client
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult UpgradeProfile(ECDbR ecdb, Db::OpenParams const& openParams);

    //! Checks the compatibility of the ECDb profile of the specified file to be opened with the current version of the ECDb API.
    //!
    //! @see BeSQLite::Db::OpenBeSQLiteDb for the compatibility contract for Bentley SQLite profiles.
    //! @param[out] fileIsAutoUpgradable Returns true if the file's ECDb profile version indicates that it is old, but auto-upgradeable.
    //!             false otherwise.
    //!             This method does @b not perform auto-upgrades. The out parameter just indicates to calling code
    //!             whether it has to perform the auto-upgrade or not.
    //! @param[out] actualProfileVersion the retrieved actual profile version
    //! @param[in]  openModeIsReadonly true if the file is going to be opened in read-only mode. false if
    //!             the file is going to be opened in read-write mode.
    //! @return     BE_SQLITE_OK if ECDb profile can be opened in the requested mode, i.e. the compatibility contract is matched.
    //!             BE_SQLITE_Error_ProfileTooOld if file's ECDb profile is too old to be opened by this API.
    //!             This error code is also returned if the file is old but not too old to be auto-upgraded.
    //!             Check @p fileIsAutoUpgradable to tell whether the file is auto-upgradeable and not.
    //!             BE_SQLITE_Error_ProfileTooNew if file's profile is too new to be opened by this API.
    //!             BE_SQLITE_Error_ProfileTooNewForReadWrite if file's profile is too new to be opened read-write, i.e. @p openModeIsReadonly is false
    static DbResult CheckProfileVersion(bool& fileIsAutoUpgradable, SchemaVersion& actualProfileVersion, ECDbCR ecdb, bool openModeIsReadOnly);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
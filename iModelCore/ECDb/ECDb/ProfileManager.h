/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ProfileManager.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"
#include "ProfileUpgrader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//for ECDb related entries in the be_Prop table
#define ECDB_PROPSPEC_NAMESPACE "ec_Db"

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      12/2012
//+===============+===============+===============+===============+===============+======
struct ProfileManager final
    {
private:
    ProfileManager() = delete;
    ~ProfileManager() = delete;

    static DbResult CreateProfileTables(ECDbCR);
    //! @param onProfileCreation true if this method is called during profile creation. false if 
    //! called during profile upgrade
    static DbResult AssignProfileVersion(ECDbR, bool onProfileCreation);

    //! Minimum version of the ECDb profile which can still be auto-upgraded to the latest profile version.
    static ProfileVersion GetMinimumSupportedVersion() { return ProfileVersion(4, 0, 0, 0); }

    static DbResult RunUpgraders(ECDbCR, ProfileVersion const& actualFileProfileVersion);

    static PropertySpec GetProfileVersionPropertySpec() { return PropertySpec("SchemaVersion", ECDB_PROPSPEC_NAMESPACE); }
    static PropertySpec GetInitialProfileVersionPropertySpec() { return PropertySpec("InitialSchemaVersion", ECDB_PROPSPEC_NAMESPACE); }

public:
    //! Expected version of the ECDb profile for this version of the ECDb API.
    static ProfileVersion GetExpectedVersion() { return ProfileVersion(4, 0, 0, 2); }

    static ProfileState CheckProfileVersion(ProfileVersion& fileProfileVersion, ECDbCR);


    //! Reads the version of the ECDb profile of the given ECDb file
    //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
    //! ECDb file, i.e. does not have the ECDb profile
    static DbResult ReadProfileVersion(ProfileVersion& profileVersion, ECDbCR);

    //! Creates the ECDb profile in the specified ECDb file.
    //! @remarks In case of success the outermost transaction is committed.
    //!     In case of error, the outermost transaction is rolled back.
    //! @param[out] version the profile version
    //! @param[in] ecdb ECDb file handle to create the profile in.
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult CreateProfile(ProfileVersion& version, ECDbR ecdb);

    //! Upgrades the ECDb profile in the specified ECDb file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @remarks In case an upgrade was necessary and the upgrade was successful,
    //! the outermost transaction is committed. In case of
    //! error, the outermost transaction is rolled back. 
    //! @param[out] newVersion the new version after the upgrade has happened
    //! @param[in] ecdb ECDb file handle to upgrade
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult UpgradeProfile(ProfileVersion& newVersion, ECDbR ecdb);

    static bset<Utf8CP, CompareIUtf8Ascii> GetECDbSchemaNames()
        { 
        bset<Utf8CP, CompareIUtf8Ascii> names;
        names.insert("ECDbChange");
        names.insert("ECDbFileInfo");
        names.insert("ECDbMap");
        names.insert("ECDbMeta");
        names.insert("ECDbSchemaPolicies");
        names.insert("ECDbSystem");
        return names;
        }

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
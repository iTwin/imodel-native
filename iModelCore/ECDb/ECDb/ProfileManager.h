/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ProfileUpgrader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//for ECDb related entries in the be_Prop table
#define ECDB_PROPSPEC_NAMESPACE "ec_Db"

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ProfileManager final
    {
private:
    ECDbR m_ecdb;
    mutable ProfileVersion m_profileVersion = ProfileVersion(0, 0, 0, 0);

    //! Reads the version of the ECDb profile of the given ECDb file
    //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
    //! ECDb file, i.e. does not have the ECDb profile
    DbResult ReadProfileVersion() const;

    DbResult CreateProfileTables() const;
    //! @param onProfileCreation true if this method is called during profile creation. false if 
    //! called during profile upgrade
    DbResult AssignProfileVersion(bool onProfileCreation) const;

    DbResult RunUpgraders(ProfileVersion const& versionBeforeUpgrade) const;

    static PropertySpec GetProfileVersionPropertySpec() { return PropertySpec("SchemaVersion", ECDB_PROPSPEC_NAMESPACE); }
    static PropertySpec GetInitialProfileVersionPropertySpec() { return PropertySpec("InitialSchemaVersion", ECDB_PROPSPEC_NAMESPACE); }

public:
    explicit ProfileManager(ECDbR ecdb): m_ecdb(ecdb) {}

    ProfileState CheckProfileVersion() const;

    //! Creates the ECDb profile in the specified ECDb file.
    //! @remarks In case of success the outermost transaction is committed.
    //!     In case of error, the outermost transaction is rolled back.
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    DbResult CreateProfile() const;

    //! Upgrades the ECDb profile in the specified ECDb file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @remarks In case an upgrade was necessary and the upgrade was successful,
    //! the outermost transaction is committed. In case of
    //! error, the outermost transaction is rolled back. 
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    DbResult UpgradeProfile() const;

    //! Returns the profile version of the current file. 
    //! Note this method must only be called after having called CheckProfileVersion, CreateProfile or UpgradeProfile.
    ProfileVersion const& GetProfileVersion() const { BeAssert(!m_profileVersion.IsEmpty()); return m_profileVersion; }

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
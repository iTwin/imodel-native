/*--------------------------------------------------------------------------------------+
|
|     $Source: BeSQLiteProfileManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    typedef std::vector<std::unique_ptr<BeSQLiteProfileUpgrader>> UpgraderSequence;
    static Utf8CP const PROFILENAME;
    //! Version of the BeSQLite Profile expected by the BeSQLite API.
    static const SchemaVersion s_expectedProfileVersion;
    //! Minimum version of the BeSQLite profile which can still be auto-upgraded to the latest profile version.
    static const SchemaVersion s_minimumAutoUpgradableProfileVersion;

    static UpgraderSequence s_upgraderSequence;

    //non-instantiable class
    BeSQLiteProfileManager ();
    ~BeSQLiteProfileManager ();

    static UpgraderSequence::const_iterator GetUpgraderSequenceFor (SchemaVersion const& currentProfileVersion);
    static UpgraderSequence const& GetUpgraderSequence ();

public:
    //! Upgrades the BeSQLite profile in the specified BeSQLite file to the latest version (if the file's profile
    //! is not up-to-date).
    //! @param[in] db BeSQLite file handle to upgrade
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult UpgradeProfile (DbR db);

    //! Reads the version of the BeSQLite profile of the given BeSQLite file
    //! @return BE_SQLITE_OK in case of success or error code if the SQLite database is no
    //! BeSQLite file, i.e. does not have the BeSQLite profile
    static DbResult ReadProfileVersion (SchemaVersion& profileVersion, DbR db);

    //! Saves the software's expected profile version in the specified BeSQLite file.
    //! @param[in] db BeSQLite file handle to save profile version to
    //! @return BE_SQLITE_OK if successful. Error code otherwise.
    static DbResult AssignProfileVersion (DbR db);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      10/2014
//+===============+===============+===============+===============+===============+======
struct BeSQLiteProfileUpgrader
    {
private:
    virtual SchemaVersion _GetTargetVersion () const = 0;
    virtual DbResult _Upgrade (DbR db) const = 0;

public:
    virtual ~BeSQLiteProfileUpgrader ()
        {}

    SchemaVersion GetTargetVersion () const;
    DbResult Upgrade (DbR db) const;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      10/2014
//+===============+===============+===============+===============+===============+======
struct BeSQLiteProfileUpgrader_3101 : BeSQLiteProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual SchemaVersion _GetTargetVersion () const override
        {
        return SchemaVersion (3, 1, 0, 1);
        }
    virtual DbResult _Upgrade (DbR db) const override;
    };

END_BENTLEY_SQLITE_NAMESPACE
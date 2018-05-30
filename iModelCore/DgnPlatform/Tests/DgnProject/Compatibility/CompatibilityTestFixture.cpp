/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/CompatibilityTestFixture.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTestFixture.h"

#include <Bentley/Bentley.h>
#include <Bentley/BeDirectoryIterator.h>

//================================Profile================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
Profile::Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name) 
    : m_type(type), m_versionPropertySpec("SchemaVersion", nameSpace), m_name(name)
    {
    m_profileSeedFolder = ProfileManager::Get().GetSeedFolder();
    m_profileSeedFolder.AppendSeparator().AppendUtf8(m_name);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Profile::Init() const
    {
    if (!m_profileSeedFolder.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(m_profileSeedFolder.GetName());
        if (status != BeFileNameStatus::Success)
            {
            BeAssert(status == BeFileNameStatus::Success);
            return ERROR;
            }
        }

    return _Init();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName Profile::GetPathForNewTestFile(Utf8CP testFileName) const
    {
    BeFileName path(GetSeedFolder());
    path.AppendToPath(BeFileName(GetExpectedVersion().ToString())).AppendToPath(BeFileName(testFileName));
    return path;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus Profile::ReadProfileVersion(ProfileVersion& schemaVersion, Db const& db, PropertySpec const& spec)
    {
    Utf8String profileVersionJson;
    if (BE_SQLITE_ROW != db.QueryProperty(profileVersionJson, spec))
        return ERROR;

    if (profileVersionJson.empty())
        return ERROR;

    schemaVersion.FromJson(profileVersionJson.c_str());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileType Profile::ParseProfileType(Utf8CP str)
    {
    if (BeStringUtilities::StricmpAscii(PROFILE_NAME_BEDB, str) == 0)
        return ProfileType::BeDb;

    if (BeStringUtilities::StricmpAscii(PROFILE_NAME_ECDB, str) == 0)
        return ProfileType::ECDb;

    if (BeStringUtilities::StricmpAscii(PROFILE_NAME_DGNDB, str) == 0)
        return ProfileType::DgnDb;

    BeAssert(false && "Unknown profile name");
    return ProfileType::BeDb;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbProfile::_Init() const
    {
    if (!ECDb::IsInitialized())
        {
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);

        ECDb::Initialize(temporaryDir, &applicationSchemaDir);
        srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
        }

    ECDb db;
    if (BE_SQLITE_OK != db.CreateNewDb(BEDB_MemoryDb))
        return ERROR;

    return ReadProfileVersion(m_expectedVersion, db, m_versionPropertySpec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus BeDbProfile::_Init() const
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    Db db;
    if (BE_SQLITE_OK != db.CreateNewDb(BEDB_MemoryDb))
        return ERROR;

    return ReadProfileVersion(m_expectedVersion, db, m_versionPropertySpec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DgnDbProfile::_Init() const
    {
    if (!DgnDb::IsInitialized())
        {
        BeFileName applicationSchemaDir;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

        BeFileName temporaryDir;
        BeTest::GetHost().GetOutputRoot(temporaryDir);

        DgnDb::Initialize(temporaryDir, &applicationSchemaDir);
        srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
        }

    BeFileName temporaryDir;
    BeTest::GetHost().GetTempDir(temporaryDir);
    temporaryDir.AppendUtf8("temp");

    CreateDgnDbParams temp("SchemaVersion");
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, temporaryDir, temp);
    if (db == nullptr)
        return ERROR;

    return ReadProfileVersion(m_expectedVersion, *db, m_versionPropertySpec);
    }

//=====================================ProfileManager====================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager* ProfileManager::s_singleton = new ProfileManager();

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
Profile& ProfileManager::GetProfile(ProfileType type) const
    {
    auto it = m_profiles.find(type);
    if (it != m_profiles.end())
        return *it->second;

    std::unique_ptr<Profile> profile = nullptr;
    switch (type)
        {
            case ProfileType::BeDb:
                profile = BeDbProfile::Create();
                break;

            case ProfileType::ECDb:
                profile = ECDbProfile::Create();
                break;

            case ProfileType::DgnDb:
                profile = DgnDbProfile::Create();
                break;

            default:
                BeAssert(false && "Unhandled ProfileType value. Update the code");
                break;
        }

    BeAssert(profile != nullptr && "Failed to create profile");
    Profile* profileP = profile.get();
    m_profiles[type] = std::move(profile);
    return *profileP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName const& ProfileManager::GetSeedFolder() const 
    {
    if(m_testSeedFolder.empty())
        {
        BeTest::GetHost().GetOutputRoot(m_testSeedFolder);
        m_testSeedFolder.PopDir().AppendSeparator().append(L"SeedData");
        }

    return m_testSeedFolder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName const& ProfileManager::GetOutFolder() const
    {
    if (m_outFolder.empty())
        BeTest::GetHost().GetOutputRoot(m_outFolder);

    return m_outFolder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState ProfileManager::GetFileProfileState(BeFileNameCR filePath) const
    {
    ProfileType fileProfileType = Profile::ParseProfileType(Utf8String(filePath.GetExtension()).c_str());
    ProfileVersion const& expectedVersion = GetProfile(fileProfileType).GetExpectedVersion();

    BeFileName fileVersionFolder = filePath.GetDirectoryName();
    // This is needed because the sscanf does not search the entire file path for the format %d.%d.%d.%d. It only looks for the string
    // to explicitly match. So drop the version directory and append the parse format to the end.
    BeFileName parseFormat(fileVersionFolder);
    parseFormat.PopDir().AppendSeparator().append(WString(VERSION_PARSE_FORMAT, BentleyCharEncoding::Utf8));
    ProfileVersion fileProfileVersion(0,0,0,0);
    fileProfileVersion.FromString(fileVersionFolder.GetNameUtf8().c_str(), parseFormat.GetNameUtf8().c_str());
    BeAssert(!fileProfileVersion.IsEmpty());

    const int compareRes = fileProfileVersion.CompareTo(expectedVersion);
    if (compareRes == 0)
        return ProfileState::Current;

    if (compareRes < 0)
        return ProfileState::Older;

    return ProfileState::Newer;
    }

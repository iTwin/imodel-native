/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ProfileManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTests.h"
#include "ProfileManager.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN

//================================Profile================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
Profile::Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name, BeFileNameCR seedFolder) : m_type(type), m_versionPropertySpec("SchemaVersion", nameSpace), m_name(name)
    {
    m_profileSeedFolder = seedFolder;
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
// @bsimethod                                    Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<TestFile> Profile::GetAllVersionsOfTestFile(Utf8CP testFileName, bool logFoundFiles) const
    {
    BeFileName profileSeedFolder(GetSeedFolder());
    bvector<BeFileName> matches;
    BeDirectoryIterator::WalkDirsAndMatch(matches, profileSeedFolder, WString(testFileName, BentleyCharEncoding::Utf8).c_str(), true);
    std::vector<TestFile> testFiles;
    for (BeFileNameCR match : matches)
        {
        Utf8String testFileName(match.GetFileNameAndExtension().c_str());
        BeFileName profileVersionFolderName = match.GetDirectoryName();
        //just get folder name without path
        if (profileVersionFolderName.EndsWith(L"/") || profileVersionFolderName.EndsWith(L"\\"))
            profileVersionFolderName.erase(profileVersionFolderName.size() - 1, 1);

        const size_t separatorPos = profileVersionFolderName.find_last_of(L"/\\");
        if (separatorPos != BeFileName::npos)
            profileVersionFolderName.erase(0, separatorPos + 1);

        bvector<Utf8String> profileVersions;
        BeStringUtilities::Split(profileVersionFolderName.GetNameUtf8().c_str(), "_", profileVersions);
        BeAssert(!profileVersions.empty());
        ProfileVersion bedbVersion(0, 0, 0, 0), ecdbVersion(0, 0, 0, 0), dgndbVersion(0, 0, 0, 0);
        bedbVersion.FromString(profileVersions.back().c_str());
        if (profileVersions.size() == 3)
            {
            dgndbVersion.FromString(profileVersions[0].c_str());
            ecdbVersion.FromString(profileVersions[1].c_str());
            }
        else if (profileVersions.size() == 2)
            ecdbVersion.FromString(profileVersions[0].c_str());

        testFiles.push_back(TestFile(testFileName, match, bedbVersion, ecdbVersion, dgndbVersion));
        }

    if (logFoundFiles && LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        if (testFiles.empty())
            LOG.info("Found test files: none.");
        else
            {
            Utf8String msg("Found test files:\r\n");
            for (TestFile const& testFile : testFiles)
                {
                msg.append(testFile.ToString()).append("\r\n");
                }
            LOG.info(msg.c_str());
            }
        }

    return testFiles;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName Profile::GetPathForNewTestFile(Utf8CP testFileName) const
    {
    BeFileName path(GetSeedFolder());
    path.AppendToPath(BeFileName(GetExpectedVersion().ToString()));
    for (ProfileVersion const& includedProfileVersion : m_expectedIncludedProfileVersions)
        {
        path.AppendString(L"_").AppendToPath(BeFileName(includedProfileVersion.ToString()));
        }

    path.AppendToPath(BeFileName(testFileName));
    return path;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion Profile::ReadProfileVersion(Db const& db) const
    {
    Utf8String profileVersionJson;
    if (BE_SQLITE_ROW != db.QueryProperty(profileVersionJson, m_versionPropertySpec) || profileVersionJson.empty())
        return ProfileVersion(0, 0, 0, 0);

    return ProfileVersion(profileVersionJson.c_str());
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

    m_expectedVersion = ReadProfileVersion(db);

    m_expectedIncludedProfileVersions.push_back(ProfileManager::Get().GetProfile(ProfileType::BeDb).GetExpectedVersion());
    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
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

    m_expectedVersion = ReadProfileVersion(db);
    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
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

    CreateDgnDbParams temp("ProfileVersion");
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, temporaryDir, temp);
    if (db == nullptr)
        return ERROR;

    m_expectedVersion = ReadProfileVersion(*db);

    m_expectedIncludedProfileVersions.push_back(ProfileManager::Get().GetProfile(ProfileType::BeDb).GetExpectedVersion());
    m_expectedIncludedProfileVersions.push_back(ProfileManager::Get().GetProfile(ProfileType::ECDb).GetExpectedVersion());

    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
    }

//=====================================TestFile====================================
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/18
//+---------------+---------------+---------------+---------------+---------------+------
TestFile::TestFile(Utf8StringCR name, BeFileName const& path, ProfileVersion const& bedbVersion, ProfileVersion const& ecdbVersion, ProfileVersion const& dgndbVersion) : m_name(name), m_path(path), m_bedbVersion(bedbVersion), m_ecdbVersion(ecdbVersion), m_dgndbVersion(dgndbVersion) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   05/18
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestFile::ToString() const
    {

    Utf8String versionString;
    if (!m_dgndbVersion.IsEmpty())
        versionString.Sprintf(PROFILE_NAME_DGNDB " %s | " PROFILE_NAME_ECDB " %s | " PROFILE_NAME_BEDB " %s", m_dgndbVersion.ToString().c_str(), m_ecdbVersion.ToString().c_str(), m_bedbVersion.ToString().c_str());
    else if (!m_ecdbVersion.IsEmpty())
        versionString.Sprintf(PROFILE_NAME_ECDB " %s | " PROFILE_NAME_BEDB " %s", m_ecdbVersion.ToString().c_str(), m_bedbVersion.ToString().c_str());
    else
        versionString.Sprintf(PROFILE_NAME_BEDB " %s", m_bedbVersion.ToString().c_str());

    return Utf8PrintfString("%s | %s | %s", m_name.c_str(), versionString.c_str(), m_path.GetNameUtf8().c_str());
    }

//=====================================ProfileManager====================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager* ProfileManager::s_singleton = new ProfileManager();


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager::ProfileManager()
    {
    BeTest::GetHost().GetOutputRoot(m_testSeedFolder);
    m_testSeedFolder.PopDir().AppendSeparator().append(L"SeedData");

    m_profiles[ProfileType::BeDb] = BeDbProfile::Create(m_testSeedFolder);
    m_profiles[ProfileType::ECDb] = ECDbProfile::Create(m_testSeedFolder);
    m_profiles[ProfileType::DgnDb] = DgnDbProfile::Create(m_testSeedFolder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
Profile& ProfileManager::GetProfile(ProfileType type) const
    {
    auto it = m_profiles.find(type);
    BeAssert(it != m_profiles.end() && "Unhandled ProfileType value. Update the code");
    return *it->second;
    }

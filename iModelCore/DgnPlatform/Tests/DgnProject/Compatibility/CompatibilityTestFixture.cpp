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
std::vector<BeFileName> Profile::GetAllVersionsOfTestFile(Utf8CP testFileName) const
    {
    std::vector<BeFileName> files;
    for (ProfileVersion const& ver : ReadProfileVersionFromDisk())
        {
        BeFileName fl;
        if (SUCCESS != GetTestFile(fl, testFileName, ver))
            {
            LOG.warningv("Seed file %s/%s.%s is not found or created", ver.ToString().c_str(), testFileName, m_name);
            continue;
            }

        files.push_back(fl);
        }

    return files;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Profile::GenerateAllSeedFiles() const
    {
    for (auto& key : m_testFiles)
        {
        if (SUCCESS != GenerateSeedFile(*key.second, m_expectedVersion))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Profile::GenerateSeedFile(TestFile& testFile, ProfileVersion const& actualVersion) const
    {
    if (testFile.IsResolved())
        return SUCCESS;

    if (actualVersion != m_expectedVersion)
        {
        LOG.errorv("File  %s/%s.%s does not exist and cannot be created. Current profile version is %s", actualVersion.ToString().c_str(), testFile.GetFileName(), m_name, m_expectedVersion.ToString().c_str());
        BeAssert(false && "Only create seed file for expected profile version");
        return ERROR;
        }

    //Create the file

    BeDirectoryIterator it(m_profileSeedFolder);
    do
        {
        BeFileName entry;
        bool isDir = false;
        if (it.GetCurrentEntry(entry, isDir, false) == SUCCESS && isDir)
            {
            ProfileVersion version(0, 0, 0, 0);
            version.FromString(entry.GetNameUtf8().c_str());
            if (version.CompareTo(actualVersion) > 0)
                {
                BeFileName filePath(m_profileSeedFolder);
                filePath.AppendSeparator().AppendUtf8(version.ToString().c_str());

                filePath.AppendSeparator().AppendUtf8(testFile.GetFileName());
                filePath.AppendExtension(WString(m_name, BentleyCharEncoding::Utf8).c_str());
                testFile.SetResolvedFileName(filePath);
                return SUCCESS;
                }
            }
        } while (it.ToNext() == SUCCESS);

    BeFileName filePath(m_profileSeedFolder);
    filePath.AppendToPath(BeFileName(actualVersion.ToString()));
    if (!filePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(filePath.GetName()))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    filePath.AppendSeparator().AppendUtf8(testFile.GetFileName());
    filePath.AppendExtension(WString(m_name, BentleyCharEncoding::Utf8).c_str());
    if (!filePath.DoesPathExist())
        {
        testFile.CreatePhysical();
        if (!filePath.DoesPathExist())
            {
            LOG.errorv("Fail to create %s", filePath.GetNameUtf8().c_str());
            BeAssert(false && "Failed to create seed file");
            return ERROR;
            }
        }

    testFile.SetResolvedFileName(filePath);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Profile::GetTestFile(BeFileName& copyTestFile, Utf8CP testFileName, ProfileVersion const& ver) const
    {
    copyTestFile.clear();
    auto itor = m_testFiles.find(testFileName);
    if (itor == m_testFiles.end())
        {
        LOG.errorv("File %s/%s.%s is not registered", ver.ToString().c_str(), testFileName, m_name);
        BeAssert(false && "Failed to find file");
        return ERROR;
        }

    TestFile& testFile = *itor->second;
    if (SUCCESS != GenerateSeedFile(testFile, ver))
        return ERROR;

    // Create copy of the file
    copyTestFile = ProfileManager::Get().GetOutFolder();
    copyTestFile.AppendSeparator().AppendUtf8(m_name);
    copyTestFile.AppendSeparator().AppendUtf8(ver.ToString().c_str());
    if (!copyTestFile.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(copyTestFile.GetName()))
            {
            BeAssert(false);
            return ERROR;
            }
        }

    copyTestFile.AppendSeparator().AppendUtf8(testFile.GetFileName());
    copyTestFile.AppendExtension(WString(m_name, BentleyCharEncoding::Utf8).c_str());
    if (copyTestFile.DoesPathExist())
        {
        if (copyTestFile.BeDeleteFile() != BeFileNameStatus::Success)
            {
            LOG.errorv("Fail to delete %s", copyTestFile.GetNameUtf8().c_str());
            BeAssert(false && "Failed to delete copy file");
            return ERROR;
            }
        }

    if (BeFileName::BeCopyFile(testFile.GetResolvedFilePath(), copyTestFile) != BeFileNameStatus::Success)
        {
        LOG.errorv("Fail to copy file %s to %s", testFile.GetResolvedFilePath().GetNameUtf8().c_str(), copyTestFile.GetNameUtf8().c_str());
        BeAssert(false && "Failed to create copy seed file");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<ProfileVersion> const&  Profile::ReadProfileVersionFromDisk() const
    {
    if (!m_versionList.empty())
        return m_versionList;

    BeFileName fl(m_profileSeedFolder);
    if (fl.DoesPathExist())
        {
        if (!GenerateAllSeedFiles())
            {
            BeAssert(false && "Failed to generate profile");
            return m_versionList;
            }
        }
    BeDirectoryIterator it(fl);
    do
        {
        BeFileName entry;
        bool isDir;
        if (it.GetCurrentEntry(entry, isDir, false) == 0 && isDir)
            {
            ProfileVersion ver(0, 0, 0, 0);
            ver.FromString(entry.GetNameUtf8().c_str());
            m_versionList.push_back(ver);
            }
        } while (it.ToNext() == 0);
        

    std::sort(m_versionList.begin(), m_versionList.end());
    return m_versionList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
void Profile::RegisterTestFile(TestFile& testFile)
    {
    Utf8CP fileName = testFile.GetFileName();

    if (m_testFiles.end() != m_testFiles.find(fileName))
        return;

    m_testFiles[fileName] = &testFile;
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

    m_expectedVersion = db->GetProfileVersion();
    return ERROR;
    }

//=====================================ProfileManager====================================

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager ProfileManager::s_singleton;

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

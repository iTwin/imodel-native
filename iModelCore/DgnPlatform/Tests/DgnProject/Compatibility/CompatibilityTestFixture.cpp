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

//================================ProfileManager::Profile================================
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager::Profile::Profile(Kind type, Utf8CP nameSpace, Utf8CP schemaVersionProp, Utf8CP fileExtension)
    :m_type(type), m_versionPropertySpec(schemaVersionProp, nameSpace), m_fileExtension(fileExtension), m_expectedVersion(0, 0, 0, 0), m_initalized(false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion const& ProfileManager::Profile::GetExpectedVersion() const
    {
    if (m_expectedVersion.IsEmpty())
        {
        Init();
        DbResult r = _GetSchemaVersion(m_expectedVersion, m_versionPropertySpec);
        if (r != BE_SQLITE_OK)
            {
            BeAssert(false && "Failed to query schema version");
            }
        }

    BeAssert(!m_expectedVersion.IsEmpty());
    return m_expectedVersion;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName const& ProfileManager::Profile::GetSeedFolder() const
    {
    if (m_profileSeedFolder.empty())
        {
        m_profileSeedFolder = ProfileManager::GetInstance().GetSeedFolder();
        m_profileSeedFolder.AppendSeparator().AppendUtf8(m_fileExtension.c_str());
        if (!m_profileSeedFolder.DoesPathExist())
            {
            BeFileNameStatus status = BeFileName::CreateNewDirectory(m_profileSeedFolder.GetName());
            if (status != BeFileNameStatus::Success)
                {
                BeAssert(status == BeFileNameStatus::Success);
                }
            }
        }

    return m_profileSeedFolder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<BeFileName> ProfileManager::Profile::GetCopyOfAllVersionOfTestFile(Utf8CP name) const
    {
    Init();
    std::vector<BeFileName> files;
    for (BeVersion ver : ReadProfileVersionFromDisk())
        {
        BeFileName fl;
        if (!GetCopyOfTestFile(fl, name, ver))
            {
            LOG.warningv("Seed file %s/%s.%s is not found or created", ver.ToString().c_str(), name, m_fileExtension.c_str());
            continue;
            }

        files.push_back(fl);
        }

    return files;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
bool ProfileManager::Profile::GenerateAllSeedFiles() const
    {
    for (auto& key : m_testFiles)
        {
        TestFile* testFile = key.second;
        if (GenerateSeedFile(testFile, GetExpectedVersion()))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
bool ProfileManager::Profile::GenerateSeedFile(TestFile* testFile, BeVersion const& ver) const
    {
    if (testFile->IsResolved())
        return true;

    if (ver != GetExpectedVersion())
        {
        LOG.errorv("File  %s/%s.%s does not exist and cannot be created. Currect profile version is %s", ver.ToString().c_str(), testFile->GetFileName(), m_fileExtension.c_str(), GetExpectedVersion().ToString().c_str());
        BeAssert(false && "Only create seed file for expected profile version");
        return false;
        }

    //Create the file
    BeFileName& fl = testFile->m_resolvedFileName;
    fl = GetSeedFolder(ver);
    if (!fl.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(fl.GetName());
        if (status != BeFileNameStatus::Success)
            {
            BeAssert(status == BeFileNameStatus::Success);
            return false;
            }
        }

    fl.AppendSeparator().AppendUtf8(testFile->GetFileName());
    fl.AppendExtension(WString(m_fileExtension.c_str(), BentleyCharEncoding::Utf8).c_str());
    if (!fl.DoesPathExist())
        {
        Init();
        testFile->Create();
        if (!fl.DoesPathExist())
            {
            LOG.errorv("Fail to create %s", fl.GetNameUtf8().c_str());
            BeAssert(false && "Failed to create seed file");
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
bool  ProfileManager::Profile::GetCopyOfTestFile(BeFileName& copyTestFile, Utf8CP name, BeVersion const& ver) const
    {
    copyTestFile.clear();
    auto itor = m_testFiles.find(name);
    if (itor == m_testFiles.end())
        {
        LOG.errorv("File %s/%s.%s is not registered", ver.ToString().c_str(), name, m_fileExtension.c_str());
        BeAssert(false && "Failed to find file");
        return false;
        }

    TestFile* testFile = itor->second;
    if (!GenerateSeedFile(testFile, ver))
        return false;

    // Create copy of the file
    copyTestFile = ProfileManager::GetInstance().GetOutFolder();
    copyTestFile.AppendSeparator().AppendUtf8(m_fileExtension.c_str());
    copyTestFile.AppendSeparator().AppendUtf8(ver.ToString().c_str());
    if (!copyTestFile.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(copyTestFile.GetName());
        if (status != BeFileNameStatus::Success)
            {
            BeAssert(status == BeFileNameStatus::Success);
            return false;
            }
        }

    copyTestFile.AppendSeparator().AppendUtf8(testFile->GetFileName());
    copyTestFile.AppendExtension(WString(m_fileExtension.c_str(), BentleyCharEncoding::Utf8).c_str());
    if (copyTestFile.DoesPathExist())
        {
        if (copyTestFile.BeDeleteFile() != BeFileNameStatus::Success)
            {
            LOG.errorv("Fail to delete %s", copyTestFile.GetNameUtf8().c_str());
            BeAssert(false && "Failed to delete copy file");
            return false;
            }
        }

    if (BeFileName::BeCopyFile(testFile->GetResolvedFilePath(), copyTestFile) != BeFileNameStatus::Success)
        {
        LOG.errorv("Fail to copy file %s to %s", testFile->GetResolvedFilePath().GetNameUtf8().c_str(), copyTestFile.GetNameUtf8().c_str());
        BeAssert(false && "Failed to create copy seed file");
        return false;
        }

    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName ProfileManager::Profile::GetSeedFolder(BeVersion const& ver) const
    {
    BeFileName fl = GetSeedFolder();
    fl.AppendSeparator().AppendUtf8(ver.ToString().c_str());  
    return fl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<BeVersion> const&  ProfileManager::Profile::ReadProfileVersionFromDisk() const
    {
    if (!m_verList.empty())
        return m_verList;

    BeFileName fl = GetSeedFolder();
    if (fl.DoesPathExist())
        {
        if (!GenerateAllSeedFiles())
            {
            BeAssert(false && "Failed to generate profile");
            return m_verList;
            }
        }
    BeDirectoryIterator it(fl);
    do
        {
        BeFileName entry;
        bool isDir;
        if (it.GetCurrentEntry(entry, isDir, false) == 0 && isDir)
            {
            BeVersion ver;
            ver.FromString(entry.GetNameUtf8().c_str());
            m_verList.push_back(ver);
            }
        } while (it.ToNext() == 0);
        

    std::sort(m_verList.begin(), m_verList.end());
    return m_verList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
bool ProfileManager::Profile::Register(TestFile* testFile)
    {
    BeAssert(testFile != nullptr);
    if (testFile == nullptr || m_testFiles.end() != m_testFiles.find(testFile->GetFileName()))
        return false;

    return m_testFiles.insert(std::make_pair(testFile->GetFileName(), testFile)).second;
    }

//================================ProfileManager::_ECDb==================================
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ProfileManager::_ECDb::_GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const
    {
    ECDb db;
    DbResult r = db.CreateNewDb(BEDB_MemoryDb);
    if (BE_SQLITE_OK != r)
        return r;

    Utf8String schemaVersionJSon;
    r = db.QueryProperty(schemaVersionJSon, spec);
    if (BE_SQLITE_ROW != r)
        return r;

    if (schemaVersionJSon.empty())
        return BE_SQLITE_ERROR;

    schemaVersion.FromJson(schemaVersionJSon.c_str());
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
void ProfileManager::_ECDb::_Init() const
    {
    if (ECDb::IsInitialized())
        return;

    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);

    ECDb::Initialize(temporaryDir, &applicationSchemaDir);
    srand((uint32_t) (BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
    }

//================================ProfileManager::_BeDb==================================
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ProfileManager::_BeDb::_GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const
    {
    Db db;
    DbResult r = db.CreateNewDb(BEDB_MemoryDb);
    if (BE_SQLITE_OK != r)
        return r;

    Utf8String schemaVersionJSon;
    r = db.QueryProperty(schemaVersionJSon, spec);
    if (BE_SQLITE_ROW != r)
        return r;

    if (schemaVersionJSon.empty())
        return BE_SQLITE_ERROR;

    schemaVersion.FromJson(schemaVersionJSon.c_str());
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
void ProfileManager::_BeDb::_Init() const
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);
    }

//================================ProfileManager::_DgnDb=================================
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ProfileManager::_DgnDb::_GetSchemaVersion(ProfileVersion& schemaVersion, PropertySpec const& spec) const
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetTempDir(temporaryDir);
    temporaryDir.AppendUtf8("temp");

    CreateDgnDbParams temp("SchemaVersion");
    DbResult r;
    DgnDbPtr db = DgnDb::CreateDgnDb(&r, temporaryDir, temp);
    if (BE_SQLITE_OK != r)
        return r;

    schemaVersion = db->GetProfileVersion();
    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
void ProfileManager::_DgnDb::_Init() const
    {
    if (DgnDb::IsInitialized())
        return;

    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);

    DgnDb::Initialize(temporaryDir, &applicationSchemaDir);
    srand((uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
    }

//=====================================ProfileManager====================================
//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager::ProfileManager()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager::Profile* ProfileManager::GetProfile(Kind kind)
    {
    if (kind == Kind::PROFILE_BeDb)
        return &m_bedb;

    if (kind == Kind::PROFILE_ECDb)
        return &m_ecdb;

    if (kind == Kind::PROFILE_DgnDb)
        return &m_dgndb;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager& ProfileManager::GetInstance()
    {
    static ProfileManager profileManager;
    return profileManager;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
ProfileManager::Profile* ProfileManager::GetProfile(Utf8CP name)
    {
    if (BeStringUtilities::StricmpAscii(name, "bedb") == 0)
        return GetProfile(PROFILE_BeDb);

    if (BeStringUtilities::StricmpAscii(name, "ecdb") == 0)
        return GetProfile(PROFILE_ECDb);

    if (BeStringUtilities::StricmpAscii(name, "dgndb") == 0)
        return GetProfile(PROFILE_DgnDb);

    BeAssert(false);
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
void ProfileManager::Register(TestFile* tf)
    {
    BeAssert(tf != nullptr);
    if (tf == nullptr)
        return;

    bool success = false;
    if (Profile* profile = GetProfile(tf->GetProfileName()))
        success = profile->Register(tf);

    BeAssert(success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName const& ProfileManager::GetSeedFolder() const 
    {
    if(m_testSeedFolder.empty())
        {
        BeTest::GetHost().GetDocumentsRoot(m_testSeedFolder);
        m_testSeedFolder.AppendSeparator().AppendUtf8("compatibility");
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
int ProfileManager::CompareFileVersion(BeFileName const& in)
    {
    Utf8String ext(in.GetExtension());
    Utf8String ver(in.GetDirectoryName().GetNameUtf8());
    BeVersion currentVersion = ProfileManager::GetInstance().GetProfile(ext.c_str())->GetExpectedVersion();

    // This is needed because the sscanf does not search the entire file path for the format %d.%d.%d.%d. It only looks for the string
    // to explicitly match. So drop the version directory and append the VersionParseFormat to the end.
    BeFileName pathForVersionParseFormat = in.GetDirectoryName();
    pathForVersionParseFormat.PopDir().AppendSeparator();

    Utf8String parseFormat(pathForVersionParseFormat.GetNameUtf8());
    parseFormat.append(VERSION_PARSE_FORMAT);

    BeVersion fileVer;
    fileVer.FromString(ver.c_str(), parseFormat.c_str());

    BeAssert(!fileVer.IsEmpty());
    if (currentVersion < fileVer)
        return 1;

    if (currentVersion > fileVer)
        return -1;

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan.Khan                        03/18
//+---------------+---------------+---------------+---------------+---------------+------
bool CompatibilityTestFixture::HasNewProfile(BeFileName const& fileName) { return ProfileManager::CompareFileVersion(fileName) > 0; }
bool CompatibilityTestFixture::HasOldProfile(BeFileName const& fileName) { return ProfileManager::CompareFileVersion(fileName) < 0; }
bool CompatibilityTestFixture::HasCurrentProfile(BeFileName const& fileName) { return ProfileManager::CompareFileVersion(fileName) == 0; }

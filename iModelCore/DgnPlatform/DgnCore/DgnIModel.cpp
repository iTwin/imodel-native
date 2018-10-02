/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnIModel.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <BeSQLite/IModelDb.h>
#include <Bentley/PerformanceLogger.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnDbStatus performEmbeddedProjectVersionChecks(DbResult& dbResult, Db& db, BeBriefcaseBasedId id)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty(versionString, DgnEmbeddedProjectProperty::ProfileVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::VersionTooOld;

    ProfileVersion actualDgnVersion(0, 0, 0, 0);
    actualDgnVersion.FromJson(versionString.c_str());
    ProfileVersion expectedDgnVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);
    ProfileVersion minimumUpgradableDgnVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    const ProfileState profileState = Db::CheckProfileVersion(expectedDgnVersion, actualDgnVersion, minimumUpgradableDgnVersion, "DgnDb");
    if (profileState.IsError())
        return DgnDbStatus::InvalidProfileVersion;

    if (profileState.GetCanOpen() == ProfileState::CanOpen::No || (profileState.GetCanOpen() == ProfileState::CanOpen::Readonly && !db.IsReadonly()))
        {
        if (profileState.IsNewer())
            return DgnDbStatus::VersionTooNew;

        return DgnDbStatus::VersionTooOld;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnDbStatus performPackageVersionChecks(DbResult& dbResult, Db& db)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty(versionString, PackageProperty::ProfileVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::VersionTooOld;

    ProfileVersion actualPackageVersion(0,0,0,0);
    actualPackageVersion.FromJson(versionString.c_str());
    ProfileVersion expectedPackageVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    ProfileVersion minimumUpgradablePackageVersion(PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    const ProfileState profileState = Db::CheckProfileVersion(expectedPackageVersion, actualPackageVersion, minimumUpgradablePackageVersion, "Package");
    if (profileState.IsError())
        return DgnDbStatus::InvalidProfileVersion;

    if (profileState.GetCanOpen() == ProfileState::CanOpen::No || (profileState.GetCanOpen() == ProfileState::CanOpen::Readonly && !db.IsReadonly()))
        {
        if (profileState.IsNewer())
            return DgnDbStatus::VersionTooNew;

        return DgnDbStatus::VersionTooOld;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnDbStatus DgnIModel::ExtractUsingDefaults(DbResult& dbResult, BeFileNameCR dgndbFile, BeFileNameCR packageFile, bool overwriteExisting, ICompressProgressTracker* progress)
    {
    PERFLOG_START("Core", "Uncompressing the DgnDb");

    if (!BeFileName::DoesPathExist(packageFile))
        return DgnDbStatus::FileNotFound;

    Utf8CP dbName = NULL;
    Db  db;
    Db::OpenParams openParams(Db::OpenMode::Readonly);

    Utf8String   utf8Name;
    //  Make dbName match the name + extension of the package, but with the trailing z removed.
    WString   temp;
    WString   tempExt;
    packageFile.ParseName(NULL, NULL, &temp, &tempExt);
    utf8Name.Sprintf("%s.%s", Utf8String(temp.c_str()).c_str(), Utf8String(tempExt.c_str()).c_str());
    utf8Name.resize(utf8Name.size()-1);
    dbName = utf8Name.c_str();

    if ((dbResult = db.OpenBeSQLiteDb(packageFile, openParams)) != BE_SQLITE_OK)
        return DgnDbStatus::SQLiteError;

    DgnDbStatus schemaStatus = performPackageVersionChecks(dbResult, db);
    if (DgnDbStatus::Success != schemaStatus)
        return DgnDbStatus::BadSchema;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeBriefcaseBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);

    if (!id.IsValid())
        {
        DbEmbeddedFileTable::Iterator iterator = embeddedFiles.MakeIterator();
        //  If there is only 1 use it regardless of name.
        if (iterator.QueryCount() != 1)
            {
            dbResult = BE_SQLITE_NOTFOUND;
            return DgnDbStatus::SQLiteError;
            }

        DbEmbeddedFileTable::Iterator::Entry entry = iterator.begin();
        utf8Name = entry.GetNameUtf8();
        dbName = utf8Name.c_str();

        id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);
        }

    if (!id.IsValid() || strcmp("dgndb", fileType.c_str()))
        {
        //  If there is only 1 use it regardless of name.
        dbResult = BE_SQLITE_NOTFOUND;
        return DgnDbStatus::SQLiteError;
        }

    if (DgnDbStatus::Success != (schemaStatus = performEmbeddedProjectVersionChecks(dbResult, db, id)))
        return schemaStatus;

    if (BeFileName::DoesPathExist(dgndbFile.GetName()))
        {
        if (!overwriteExisting)
            return DgnDbStatus::FileAlreadyExists;

        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(dgndbFile))
            return DgnDbStatus::FileAlreadyExists;
        }

    Utf8String  u8outputName(dgndbFile.GetName());
    if ((dbResult = embeddedFiles.ExportDbFile(u8outputName.c_str(), dbName, progress)) != BE_SQLITE_OK)
        return DgnDbStatus::SQLiteError;

    PERFLOG_FINISH("Core", "Uncompressing the DgnDb");
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnDbStatus DgnIModel::Extract(BeSQLite::DbResult& dbResult, Utf8CP outputDirectory, Utf8CP dbName, BeFileNameCR packageFile, bool overwriteExisting, ICompressProgressTracker* progress)
    {
    if (!BeFileName::DoesPathExist(packageFile))
        return DgnDbStatus::FileNotFound;

    BeSQLite::Db        db;
    Db::OpenParams      openParams(Db::OpenMode::Readonly);

    Utf8String   utf8Name;
    if (NULL == dbName)
        {
        //  Make dbName match the name + extension of the package, but with the trailing z removed.
        WString   temp;
        WString   tempExt;
        packageFile.ParseName(NULL, NULL, &temp, &tempExt);
        utf8Name.Sprintf("%ls.%ls", temp.c_str(), tempExt.c_str());
        utf8Name.resize(utf8Name.size()-1);
        dbName = utf8Name.c_str();
        }

    if ((dbResult = db.OpenBeSQLiteDb(packageFile, openParams)) != BE_SQLITE_OK)
        return DgnDbStatus::SQLiteError;

    Utf8String versionString;
    dbResult = db.QueryProperty(versionString, PackageProperty::ProfileVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::InvalidProfileVersion;

    ProfileVersion packageVersion(0,0,0,0);
    packageVersion.FromJson(versionString.c_str());
    ProfileVersion currentPackageVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    ProfileVersion supportedPackageVersion(PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    if (packageVersion.CompareTo(supportedPackageVersion, ProfileVersion::VERSION_MajorMinor) < 0)
        return DgnDbStatus::VersionTooOld;

    if (packageVersion.CompareTo(currentPackageVersion, ProfileVersion::VERSION_MajorMinor) > 0)
        return DgnDbStatus::VersionTooNew;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeBriefcaseBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);
    if (!id.IsValid() || strcmp("dgndb", fileType.c_str()))
        {
        dbResult = BE_SQLITE_NOTFOUND;
        return DgnDbStatus::SQLiteError;
        }

    dbResult = db.QueryProperty(versionString, DgnEmbeddedProjectProperty::ProfileVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::InvalidProfileVersion;

    ProfileVersion profileVersion(0, 0, 0, 0);
    profileVersion.FromJson(versionString.c_str());
    ProfileVersion currentVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);
    ProfileVersion supportedVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    if (profileVersion.CompareTo(supportedVersion, ProfileVersion::VERSION_MajorMinor) < 0)
        return DgnDbStatus::VersionTooOld;

    if (profileVersion.CompareTo(currentVersion, ProfileVersion::VERSION_MajorMinor) > 0)
        return DgnDbStatus::VersionTooNew;

    WString wcOutputDirectory;
    WString wcDbName;

    wcOutputDirectory.AssignUtf8(outputDirectory);
    BeFileName::AppendSeparator(wcOutputDirectory);
    wcDbName.AssignUtf8(dbName);

    BeFileName  outputFileName;
    WString dev, dir, name, ext;
    BeFileName::ParseName(&dev, &dir, NULL, NULL, wcOutputDirectory.c_str());
    BeFileName::ParseName(NULL, NULL, &name, &ext, wcDbName.c_str());

    outputFileName.BuildName(dev.c_str(), dir.c_str(), name.c_str(), ext.c_str());

    if (BeFileName::DoesPathExist(outputFileName))
        {
        if (!overwriteExisting)
            return DgnDbStatus::FileAlreadyExists;

        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(outputFileName))
            return DgnDbStatus::FileAlreadyExists;
        }

    Utf8String  u8outputName(outputFileName.GetName());

    if ((dbResult = embeddedFiles.ExportDbFile(u8outputName.c_str(), dbName, progress)) != BE_SQLITE_OK)
        return DgnDbStatus::SQLiteError;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static void copyProjectPropertyToIModel(BeSQLite::Db& package, DgnDb& sourceProject, PropertySpecCR sourceSpec, PropertySpecCR targetSpec, BeBriefcaseBasedId&embeddedProjectId)
    {
    Utf8String  propertyStr;

    if (sourceProject.QueryProperty(propertyStr, sourceSpec) != BE_SQLITE_ROW)
        return;  //  Nothing to copy

    package.SavePropertyString(targetSpec, propertyStr, embeddedProjectId.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DbResult DgnIModel::Create(BeFileNameCR packageFile, BeFileNameCR dgndbFile, CreateIModelParams createParams)
    {
    BeFileName dgndbJournalFile(dgndbFile);
    dgndbJournalFile.append(L"-journal");
    if (BeFileName::DoesPathExist(dgndbJournalFile))
        {
        LOGF("Error creating package for [%ls] -- a journal file exists.", dgndbFile.c_str());
        return BE_SQLITE_CORRUPT;
        }

    DbResult result;
    DgnDbPtr  sourceProject = DgnDb::OpenDgnDb(&result, dgndbFile, DgnDb::OpenParams(Db::OpenMode::Readonly));
    if (!sourceProject.IsValid())
        return result;

    Utf8String  dgndbFileUtf8(dgndbFile.GetName());

    WString     base;
    WString     ext;
    dgndbFile.ParseName(NULL, NULL, &base, &ext);

    WString     embeddedName;
    BeFileName::BuildName(embeddedName, NULL, NULL, base.c_str(), ext.c_str());
    Utf8String  embeddedUtf8(embeddedName.c_str());

    createParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    if (createParams.m_overwriteExisting && BeFileName::DoesPathExist(packageFile))
        {
        if (BeFileNameStatus::Success != BeFileName::BeDeleteFile(packageFile))
            {
            LOGE("Unable to create DgnPackage because '%ls' cannot be deleted.", packageFile.GetName());
            return BE_SQLITE_ERROR_FileExists;
            }
        }

    BeSQLite::Db db;
    result = db.CreateNewDb(packageFile, createParams.GetGuid(), createParams);
    if (BE_SQLITE_OK != result)
        return result;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    DbResult importResult;
    embeddedFiles.ImportDbFile(importResult, embeddedUtf8.c_str(), dgndbFileUtf8.c_str(), "dgndb", nullptr, nullptr, createParams.m_chunkSize, true);
    if (BE_SQLITE_OK != importResult)
        {
        db.AbandonChanges();
        db.CloseDb();
        packageFile.BeDeleteFile();
        return importResult;
        }

    //  Imported the file into a compressed format.  Now copy properties.
    BeBriefcaseBasedId id = embeddedFiles.QueryFile(embeddedUtf8.c_str(), NULL, NULL, NULL, NULL);

    DgnViewId defaultViewId;
    if (BE_SQLITE_ROW != sourceProject->QueryProperty(&defaultViewId, (uint32_t)sizeof(defaultViewId), DgnViewProperty::DefaultView()))
        {
        auto iter = ViewDefinition::MakeIterator(*sourceProject);
        if (iter.begin() != iter.end())
            defaultViewId = (*iter.begin()).GetId();
        }

    if (defaultViewId.IsValid())  //  This should be valid unless the project has no views
        {
        auto view = sourceProject->Elements().Get<ViewDefinition>(defaultViewId);
        if (view.IsValid())
            {
            Render::ImageSource thumbnail = view->ReadThumbnail();
            if (thumbnail.IsValid())
                {
                auto& data = thumbnail.GetByteStream();
                result = db.SaveProperty(DgnEmbeddedProjectProperty::Thumbnail(), data.GetData(), data.GetSize(), id.GetValue());
                BeAssert(BE_SQLITE_OK == result);
                }
            }
        }

    ProfileVersion schemaVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    db.SavePropertyString(PackageProperty::ProfileVersion(), schemaVersion.ToJson());
    db.SaveCreationDate();

    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::ProfileVersion(), DgnEmbeddedProjectProperty::ProfileVersion(), id);
    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::Name(), DgnEmbeddedProjectProperty::Name(), id);
    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::Description(), DgnEmbeddedProjectProperty::Description(), id);
    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::Client(), DgnEmbeddedProjectProperty::Client(), id);
    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::LastEditor(), DgnEmbeddedProjectProperty::LastEditor(), id);
    copyProjectPropertyToIModel(db, *sourceProject, Properties::CreationDate(), DgnEmbeddedProjectProperty::CreationDate(), id);
    
    // Set the ExpirationDate of the .imodel file itself. This will be returned by Db::GetExpirationDate.
    DateTime expdate;
    sourceProject->QueryExpirationDate(expdate);
    if (expdate.IsValid()) //only set date if source project has it set
        db.SaveExpirationDate(expdate);

    return BE_SQLITE_OK;
    }

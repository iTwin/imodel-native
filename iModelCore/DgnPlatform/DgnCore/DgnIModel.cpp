/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnIModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <BeSQLite/IModelDb.h>
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnDbStatus performEmbeddedProjectVersionChecks(DbResult& dbResult, Db& db, BeRepositoryBasedId id)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty(versionString, DgnEmbeddedProjectProperty::SchemaVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::VersionTooOld;

    DgnVersion actualDgnProfileVersion(0,0,0,0);
    actualDgnProfileVersion.FromJson(versionString.c_str());
    DgnVersion expectedDgnProfileVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);
    DgnVersion minimumAutoUpgradableDgnProfileVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    bool isProfileAutoUpgradable = false; //unused as this method is not attempting to auto-upgrade
    dbResult = Db::CheckProfileVersion(isProfileAutoUpgradable, expectedDgnProfileVersion, actualDgnProfileVersion, minimumAutoUpgradableDgnProfileVersion, db.IsReadonly(), "DgnDb");
    switch (dbResult)
        {
        case BE_SQLITE_ERROR_ProfileTooOld:
            return DgnDbStatus::VersionTooOld;

        case BE_SQLITE_ERROR_ProfileTooNew:
            return DgnDbStatus::VersionTooNew;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
static DgnDbStatus performPackageVersionChecks(DbResult& dbResult, Db& db)
    {
    Utf8String versionString;
    dbResult = db.QueryProperty(versionString, PackageProperty::SchemaVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::VersionTooOld;

    SchemaVersion actualPackageSchemaVersion(0,0,0,0);
    actualPackageSchemaVersion.FromJson(versionString.c_str());
    SchemaVersion expectedPackageVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    SchemaVersion minimumAutoUpgradablePackageVersion(PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    bool isProfileAutoUpgradable = false; //unused as this method is not attempting to auto-upgrade
    dbResult = Db::CheckProfileVersion(isProfileAutoUpgradable, expectedPackageVersion, actualPackageSchemaVersion, minimumAutoUpgradablePackageVersion, db.IsReadonly(), "Package");
    switch (dbResult)
        {
        case BE_SQLITE_ERROR_ProfileTooOld:
            return DgnDbStatus::VersionTooOld;

        case BE_SQLITE_ERROR_ProfileTooNew:
            return DgnDbStatus::VersionTooNew;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2013
//---------------------------------------------------------------------------------------
DgnDbStatus DgnIModel::ExtractUsingDefaults(DbResult& dbResult, BeFileNameCR dgndbFile, BeFileNameCR packageFile, bool overwriteExisting, ICompressProgressTracker* progress)
    {
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
        return DgnDbStatus::InvalidFileSchema;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeRepositoryBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);

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
    dbResult = db.QueryProperty(versionString, PackageProperty::SchemaVersion());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::InvalidSchemaVersion;

    SchemaVersion packageSchemaVersion(0,0,0,0);
    packageSchemaVersion.FromJson(versionString.c_str());
    SchemaVersion currentPackageVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    SchemaVersion supportedPackageVersion(PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    if (packageSchemaVersion.CompareTo(supportedPackageVersion, SchemaVersion::VERSION_MajorMinor) < 0)
        return DgnDbStatus::VersionTooOld;

    if (packageSchemaVersion.CompareTo(currentPackageVersion,   SchemaVersion::VERSION_MajorMinor) > 0)
        return DgnDbStatus::VersionTooNew;

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles();
    Utf8String  fileType;
    BeRepositoryBasedId id = embeddedFiles.QueryFile(dbName, NULL, NULL, NULL, &fileType);
    if (!id.IsValid() || strcmp("dgndb", fileType.c_str()))
        {
        dbResult = BE_SQLITE_NOTFOUND;
        return DgnDbStatus::SQLiteError;
        }

    dbResult = db.QueryProperty(versionString, DgnEmbeddedProjectProperty::SchemaVersion(), id.GetValue());
    if (BE_SQLITE_ROW != dbResult)
        return DgnDbStatus::InvalidSchemaVersion;

    DgnVersion dgnSchemaVersion(0,0,0,0);
    dgnSchemaVersion.FromJson(versionString.c_str());
    DgnVersion currentVersion(DGNDB_CURRENT_VERSION_Major, DGNDB_CURRENT_VERSION_Minor, DGNDB_CURRENT_VERSION_Sub1, DGNDB_CURRENT_VERSION_Sub2);
    DgnVersion supportedVersion(DGNDB_SUPPORTED_VERSION_Major, DGNDB_SUPPORTED_VERSION_Minor, 0, 0);

    if (dgnSchemaVersion.CompareTo(supportedVersion, DgnVersion::VERSION_MajorMinor) < 0)
        return DgnDbStatus::VersionTooOld;

    if (dgnSchemaVersion.CompareTo(currentVersion,   DgnVersion::VERSION_MajorMinor) > 0)
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
static void copyProjectPropertyToIModel(BeSQLite::Db& package, DgnDb& sourceProject, PropertySpecCR sourceSpec, PropertySpecCR targetSpec, BeRepositoryBasedId&embeddedProjectId)
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
        LOG.fatalv(L"Error creating package for [%ls] -- a journal file exists.", dgndbFile.c_str());
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
            LOG.errorv(L"Unable to create DgnPackage because '%ls' cannot be deleted.", packageFile.GetName());
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
    BeRepositoryBasedId id = embeddedFiles.QueryFile(embeddedUtf8.c_str(), NULL, NULL, NULL, NULL);

    DgnViewId defaultViewID;
    if (BE_SQLITE_ROW != sourceProject->QueryProperty(&defaultViewID, (uint32_t)sizeof(defaultViewID), DgnViewProperty::DefaultView()))
        {
        Statement firstViewStatement;
        firstViewStatement.Prepare(*sourceProject, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_View) " LIMIT 1");

        if (BE_SQLITE_ROW == firstViewStatement.Step())
            defaultViewID = firstViewStatement.GetValueId<DgnViewId>(0);
        }

    if (defaultViewID.IsValid())  //  This should be valid unless the project has no views
        {
        Statement stmt;
        result = stmt.Prepare(*sourceProject, "SELECT SubId from be_Prop where Namespace = 'dgn_View' AND Name = 'Thumbnail' AND Id=? LIMIT 1");
        BeAssert(BE_SQLITE_OK == result);
        stmt.BindInt64(1, defaultViewID.GetValue());
        result = stmt.Step();
        if (BE_SQLITE_ROW == result)
            {
            uint32_t imageSize;
            uint32_t resolution = (uint32_t)stmt.GetValueInt(0);
            result = sourceProject->QueryPropertySize(imageSize, DgnViewProperty::Thumbnail(), defaultViewID.GetValue(), resolution);
            BeAssert(BE_SQLITE_ROW == result);

            ScopedArray<Byte>   thumbnail(imageSize);
            result = sourceProject->QueryProperty(thumbnail.GetData(), imageSize, DgnViewProperty::Thumbnail(), defaultViewID.GetValue(), resolution);
            if (BE_SQLITE_ROW != result)
                { BeAssert(false); }

            result = db.SaveProperty(DgnEmbeddedProjectProperty::Thumbnail(), thumbnail.GetData(), imageSize, id.GetValue(), resolution);
            BeAssert(BE_SQLITE_OK == result);
            }
        }

    SchemaVersion  schemaVersion(PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    db.SavePropertyString(PackageProperty::SchemaVersion(), schemaVersion.ToJson());
    db.SaveCreationDate();

    copyProjectPropertyToIModel(db, *sourceProject, DgnProjectProperty::SchemaVersion(), DgnEmbeddedProjectProperty::SchemaVersion(), id);
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


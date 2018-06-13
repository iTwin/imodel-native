/*--------------------------------------------------------------------------------------+
|
|     $Source: FakeServer/FakeServer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <FakeServer/FakeServer.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::CreateiModelFromSeed(WCharCP seedFilePath, WCharCP serverPath)
    {
    BeFileName seedPathFile(seedFilePath);
    
    BeFileName fileName = seedPathFile.GetBaseName();
    BeFileName servPathFile(serverPath);
    if (!servPathFile.DoesPathExist())
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(servPathFile.GetName()))
            return BeFileNameStatus::UnknownError;
    
    servPathFile.AppendToPath(fileName);
    BeFileNameStatus stat = BeFileName::BeCopyFile(seedPathFile, servPathFile);
    if (BeFileNameStatus::Success != stat)
        return BeFileNameStatus::UnknownError;
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::CreateiModel(BeFileName serverPath, WCharCP seedFile)
    {
    BeFileName servPathFile(serverPath);
    BeFileNameStatus stat = BeFileName::CreateNewDirectory(servPathFile);
    if (stat != BeFileNameStatus::Success)
        return stat;
    servPathFile.AppendToPath(seedFile);
    servPathFile.append(L".bim");

    CreateDgnDbParams params("MockServerFile");
    DbResult res;
    DgnDb::CreateDgnDb(&res, servPathFile, params);
    if (res != DbResult::BE_SQLITE_OK)
        return BeFileNameStatus::UnknownError;
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::DeleteAlliModels(WCharCP serverPath)
    {
    BeFileNameStatus stat = BeFileName::EmptyAndRemoveDirectory(serverPath);
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteDir;
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::DeleteiModel(Utf8String serverPath, Utf8String filename)
    {
    BeFileName servPath(serverPath);
    servPath.AppendToPath(BeFileName(filename));
    BeFileNameStatus stat = BeFileName::EmptyAndRemoveDirectory(servPath.GetName());
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteFile;
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::DownloadiModel(BeFileName downloadPath, CharCP serverPath, CharCP fileToDownload)
    {
    BeFileName basePath = downloadPath.GetDirectoryName();
    if (!BeFileName::DoesPathExist(basePath.GetWCharCP()))
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(basePath.GetWCharCP()))
            return BeFileNameStatus::UnknownError;

    BeFileName imodelPathFile(downloadPath);
    BeFileName servPathFile(serverPath);
    servPathFile.AppendA("\\");
    servPathFile.AppendA(fileToDownload);

    BeFileNameStatus stat = BeFileName::BeCopyFile(servPathFile.GetName(), imodelPathFile.GetName());
    if (BeFileNameStatus::Success != stat)
        return BeFileNameStatus::UnknownError;
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameStatus FakeServer::AcquireiModel()
    {
    return BeFileNameStatus::Success;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Farhad.Kabir    11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr FakeServer::AcquireBriefcase(DbResult &res, WCharCP filePath, WCharP file) 
    {
    BeFileName filename(filePath);
    filename.AppendToPath(file);
    return DgnDb::OpenDgnDb(&res, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    }


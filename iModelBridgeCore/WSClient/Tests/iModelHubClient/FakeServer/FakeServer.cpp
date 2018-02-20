#include "FakeServer.h"


BeFileNameStatus FakeServer::CreateiModelFromSeed(WCharCP seedFilePath, WCharCP serverPath, WCharP seedFile)
    {
    //here place the seed file on mockserver and return a mock response
    BeFileName seedPathFile(seedFilePath);
    BeFileName servPathFile(serverPath);
    seedPathFile.AppendToPath(seedFile);
    servPathFile.AppendToPath(seedFile);
    BeFileNameStatus stat = BeFileName::BeCopyFile(seedPathFile, servPathFile);
    if (BeFileNameStatus::Success != stat)
        return BeFileNameStatus::UnknownError;
    return BeFileNameStatus::Success;
    }
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
BeFileNameStatus FakeServer::DeleteAlliModels(WCharCP serverPath)
    {
    BeFileNameStatus stat = BeFileName::EmptyAndRemoveDirectory(serverPath);
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteDir;
    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::DeleteiModel(Utf8String serverPath, Utf8String filename)
    {
    BeFileName servPath(serverPath);
    servPath.AppendToPath(BeFileName(filename));
    BeFileNameStatus stat = BeFileName::EmptyAndRemoveDirectory(servPath.GetName());
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteFile;
    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::DownloadiModel(BeFileName downloadPath, CharCP serverPath, CharCP fileToDownload)
    {

    if (!downloadPath.DoesPathExist())
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(downloadPath.GetName()))
            return BeFileNameStatus::UnknownError;

    BeFileName imodelPathFile(downloadPath);
    BeFileName servPathFile(serverPath);
    imodelPathFile.AppendA("\\");
    servPathFile.AppendA("\\");
    imodelPathFile.AppendA(fileToDownload);
    servPathFile.AppendA(fileToDownload);
    imodelPathFile.append(L".bim");
    servPathFile.append(L".bim");

    BeFileNameStatus stat = BeFileName::BeCopyFile(servPathFile.GetName(), imodelPathFile.GetName());
    if (BeFileNameStatus::Success != stat)
        return BeFileNameStatus::UnknownError;
    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::AcquireiModel()
    {
    return BeFileNameStatus::Success;
    }
DgnDbPtr FakeServer::AcquireBriefcase(DbResult &res, WCharCP filePath, WCharP file) 
    {
    BeFileName filename(filePath);
    filename.AppendToPath(file);
    return DgnDb::OpenDgnDb(&res, filename, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    }


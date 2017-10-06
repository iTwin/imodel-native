#include "FakeServer.h"

#ifdef  _WIP_
BeFileNameStatus FakeServer::CreateFakeServer(WCharCP path) 
    {
    if (BeFileName::DoesPathExist(path))
        return BeFileNameStatus::AlreadyExists;
    if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(path))
        return BeFileNameStatus::CantCreate;
    if (!BeFileName::IsDirectory(path))
        return BeFileNameStatus::IllegalName;
    return BeFileNameStatus::Success;
    }

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
BeFileNameStatus FakeServer::CreateiModel(WCharCP serverPath, WCharP seedFile)
    {

    BeFileName servPathFile(serverPath);
    servPathFile.AppendToPath(seedFile);

    CreateDgnDbParams params("MockServerFile");
    DbResult res;
    DgnDb::CreateDgnDb(&res, servPathFile, params);

    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::DeleteAlliModels(WCharCP serverPath)
    {
    BeFileNameStatus stat = BeFileName::EmptyAndRemoveDirectory(serverPath);
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteDir;
    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::DeleteiModel(WCharCP serverPath, WCharCP filename)
    {
    BeFileName servPath(serverPath);
    servPath.AppendToPath(filename);
    BeFileNameStatus stat = BeFileName::BeDeleteFile(servPath);
    if (stat != BeFileNameStatus::Success)
        return BeFileNameStatus::CantDeleteFile;
    return BeFileNameStatus::Success;
    }
BeFileNameStatus FakeServer::DownloadiModel(WCharCP downloadPath, WCharCP serverPath, WCharP fileToDownload)
    {
    BeFileName imodelPathFile(downloadPath);
    BeFileName servPathFile(serverPath);
    imodelPathFile.AppendToPath(fileToDownload);
    servPathFile.AppendToPath(fileToDownload);
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
#endif // _WIP_
#include "stdafx.h"
#include "DataSourceFile.h"
#include "DataSourceAccount.h"


DataSourceFile::DataSourceFile(DataSourceAccount *sourceAccount, const SessionName &session) : DataSource(sourceAccount, session)
{

}

DataSourceFile::~DataSourceFile(void)
{

}


DataSourceStatus DataSourceFile::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    DataSourceURL                url;
    std::wstring                filePath;
    //std::ios_base::open_mode    streamMode = std::ios_base::binary;
    BeFileAccess streamMode = BeFileAccess::ReadWrite;
    BeFileSharing streamSharing = BeFileSharing::None;
    DataSourceStatus            status;

    status = Super::open(sourceURL, sourceMode);
    if (status.isFailed())
        return status;

    if(getAccount() == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);

    getURL(url);
    
    url.getFilePath(filePath);

    switch (sourceMode)
    {
    case DataSourceMode_Write:
    case DataSourceMode_Write_Segmented:
        {
        //streamMode |= std::ios_base::out;
        streamMode = BeFileAccess::Write;
        streamSharing = BeFileSharing::None;

        // strip filename from path
        auto directoryPath = BeFileName::GetDirectoryName(filePath.c_str());
        if (!BeFileName::DoesPathExist(directoryPath.c_str()))
            {
            // Path does not exist, create it
            if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(directoryPath.c_str()))
                {
                return DataSourceStatus(DataSourceStatus::Status_Error_Not_File_Path);
                }
            }

        if (!BeFileName::DoesPathExist(filePath.c_str()))
            {
            // File does not exist, create it
            if (BeFileStatus::Success != stream.Create(filePath.c_str()))
                {
                return DataSourceStatus(DataSourceStatus::Status_Not_Found);
                }
            }
        break;
        }
    case DataSourceMode_Read:
    default:
        {
        //streamMode |= std::ios_base::in;
        streamMode = BeFileAccess::Read;
        streamSharing = BeFileSharing::Read;
        break;
        }
    }

    if (stream.IsOpen() || BeFileStatus::Success == stream.Open(filePath.c_str(), streamMode, streamSharing))
        {
        return DataSourceStatus();
        }

    //stream.open(filePath, streamMode);
    //
    //if (stream.is_open())
    //{
    //    return DataSourceStatus();
    //}

    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
}

DataSourceStatus DataSourceFile::close(void)
{
    //stream.close();
    stream.Close();

    return DataSourceStatus();
}

//DataSource::DataSize DataSourceFile::getSize(void)
//{
//    DataPtr        originalPtr;
//    DataSize    length;
//                                                            // Take a copy of the current file pointer so it can be restored
//    originalPtr = getStream().tellg();
//                                                            // Seek to end of file
//    try
//    {
//        getStream().seekg(0, getStream().end);
//    }
//    catch (...)
//    {
//        DataSourceStatus(Status_Error_Seek);
//
//        return 0;
//    }
//                                                            // Get file ptr at end of file
//    length = getStream().tellg();
//                                                            // Restore original file ptr
//    std::streampos pos = originalPtr;
//    getStream().seekg(pos);
//
//                                                            // Return the length of the file
//    return length;
//}

/*
DataSourceStatus DataSourceFile::read(Buffer * dest, DataSize destSize)
{
    DataSize    size;

    if ((size = getSize()) > destSize)
        return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

    getStream().read(reinterpret_cast<char *>(dest), size);

    if(getStream())
        return DataSourceStatus();

    return DataSourceStatus(DataSourceStatus::Status_Error_Read);
}
*/

DataSourceStatus DataSourceFile::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize inputSize)
{
    if (inputSize > destSize)
        return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

    UInt32 outputSize = 0;
    if (inputSize > 0)
    {
        //getStream().read(reinterpret_cast<char *>(dest), inputSize);
        if (BeFileStatus::Success == getStream().Read(reinterpret_cast<char *>(dest), &outputSize, inputSize))
            {
            return DataSourceStatus(DataSourceStatus::Status_Error_EOF);
            }
        }
    else
    {
        //std::streampos originalPosition = getStream().tellg();
        //
        //getStream().seekg(0, getStream().end);
        //DataSize fileSize = getStream().tellg();
        //getStream().seekg(originalPosition);
        //
        //DataSize sizeToRead = fileSize - originalPosition;
        DataSize sizeToRead = 0;
        getStream().GetSize(sizeToRead);

        if(sizeToRead > destSize)
            return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

        //getStream().read(reinterpret_cast<char *>(dest), sizeToRead);
        if (BeFileStatus::Success != getStream().Read(reinterpret_cast<char *>(dest), &outputSize, sizeToRead))
            {
            return DataSourceStatus(DataSourceStatus::Status_Error_EOF);
            }
        }

    //readSize = getStream().gcount();
    readSize = outputSize;

    if(inputSize > 0 && readSize != inputSize)
        return DataSourceStatus(DataSourceStatus::Status_Error_EOF);

    return DataSourceStatus();
}

DataSourceStatus DataSourceFile::write(const Buffer * source, DataSize inputSize)
{
    UInt32 bytesWritten = 0;
    if (BeFileStatus::Success == getStream().Write(&bytesWritten, reinterpret_cast<const char *>(source), inputSize))
    {
        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error_Write);
}

//DataSourceStatus DataSourceFile::move(DataPtr position)
//{
//    std::streampos pos = position;
//
//    try
//    {
//        getStream().seekg(pos);
//    }
//    catch (...)
//    {
//        return DataSourceStatus(DataSourceStatus::Status_Error_Seek);
//    }
//
//    return DataSourceStatus();
//}

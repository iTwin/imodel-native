/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/AustraliaUploader/AustraliaUploader.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/bset.h>

#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatformTools/RealityDataService.h>
#include <RealityAdmin/RealityDataHandler.h>
#include <CCApi/CCPublic.h>

#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <zlib/zip/unzip.h>
#include <zlib/zip/zip.h>

#define MAX_FILENAME                512
#define READ_SIZE                   8192

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

bool memorySaverMode = false;
Utf8String metadata = "";
Utf8String dataset = "";
Utf8String description = "";

static void progressFunc(Utf8String filename, double fileProgress, double repoProgress)
    {
    std::cout << Utf8PrintfString("Upload percent : %3.0f%%\r", repoProgress * 100.0);
    }

static void statusFunc(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(ErrorCode > 0)
        std::cout << Utf8PrintfString("Curl error code : %d \n %s", ErrorCode, pMsg);
    else if(ErrorCode < 0)
        std::cout << pMsg;
    }

Utf8String MakeBuddiCall(int region)
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed\n" << endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running\n" << endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in\n" << endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA\n" << endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session\n" << endl;
        CCApi_FreeApi(api);
        return "";
        }

    wchar_t* buddiUrl;
    UINT32 strlen = 0;

    if (region > 100)
        {
        CCApi_GetBuddiRegionUrl(api, L"RealityDataServices", region, NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiRegionUrl(api, L"RealityDataServices", region, buddiUrl, &strlen);
        }
    else
        {
        CCApi_GetBuddiUrl(api, L"RealityDataServices", NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiUrl(api, L"RealityDataServices", buddiUrl, &strlen);
        }

    char* charServer = new char[strlen];
    wcstombs(charServer, buddiUrl, strlen);

    CCApi_FreeApi(api);

    return Utf8String(charServer);
    }

bvector<GeoPoint2d> Extents2Footprint(bvector<GeoPoint2d> extents)
    {
    bvector<GeoPoint2d> footprint = bvector<GeoPoint2d>();
    footprint.push_back(GeoPoint2d::From(extents[0].longitude, extents[0].latitude));
    footprint.push_back(GeoPoint2d::From(extents[0].longitude, extents[1].latitude));
    footprint.push_back(GeoPoint2d::From(extents[1].longitude, extents[1].latitude));
    footprint.push_back(GeoPoint2d::From(extents[1].longitude, extents[0].latitude));
    footprint.push_back(GeoPoint2d::From(extents[0].longitude, extents[0].latitude));

    return footprint;
    }

int ZipFile(BeFileNameR fileName)
    {
    char src[MAX_FILENAME];

    wcstombs(src, fileName.c_str(), fileName.size());
    
    BeFileName  zipFileName = fileName.GetDirectoryName();
    zipFileName.AppendToPath(WPrintfString(L"%s%s", fileName.GetFileNameWithoutExtension().c_str(), L".zip").c_str());

    // First need to create a new db
    /*BeFileName  fileToZip = fileName.GetDirectoryName();
    fileToZip.AppendToPath(fileName.GetFileNameWithoutExtension());
    BeFileName::CreateNewDirectory(fileToZip.GetName());
    fileToZip.AppendToPath(fileName.GetFileNameAndExtension());*/

    Utf8String nameInZip = BeFileName(fileName.GetFileNameAndExtension()).GetNameUtf8();

    zipFile zf = zipOpen(zipFileName.GetNameUtf8().c_str(), APPEND_STATUS_CREATE);

    int status = zipOpenNewFileInZip(zf, nameInZip.c_str(), nullptr, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

    BeFile file;
    status += (StatusInt)file.Open(fileName, BeFileAccess::Read);

    bvector<Byte> fileBytes;
    status += (StatusInt)file.ReadEntireFile(fileBytes);
    status += zipWriteInFileInZip(zf, fileBytes.data(), (int)fileBytes.size());
    status += zipCloseFileInZip(zf);

    Utf8CP comment = "Compressed Australian Terrain data";
    status += zipClose(zf, comment);
    file.Close();

    if (status == 0) //no errors encountered
        {
        /*if (memorySaverMode)
            BeFileName::BeDeleteFile(fileName.GetName());*/
        fileName = zipFileName;
        }
    else
        {
        std::cout << zipFileName.GetNameUtf8() << " failed to zip\n" << std::endl;
        }

    return status;
    }

bool UnZipFile(BeFileName pi_strSrc, BeFileName& pi_strDest)
    {
    char src[MAX_FILENAME];
    char dest[MAX_FILENAME];

    wcstombs(src, pi_strSrc.GetName(), pi_strSrc.size());
    wcstombs(dest, pi_strDest.GetName(), pi_strDest.size());

    src[pi_strSrc.size()] = 0;
    dest[pi_strDest.size()] = 0;

    if (!strstr(src, ".zip"))
        return false;

    unzFile uf = unzOpen(src);
    if (nullptr == uf)
        return false;

    unz_global_info unzGlobalInfo;
    if (unzGetGlobalInfo(uf, &unzGlobalInfo) != 0)
        return false;

    uLong i;
    char read_buffer[READ_SIZE];
    BeFileName tmpName;
    for (i = 0; i < unzGlobalInfo.number_entry; ++i)
        {
        unz_file_info file_info;
        char filename[MAX_FILENAME];
        if (unzGetCurrentFileInfo(
            uf,
            &file_info,
            filename,
            MAX_FILENAME,
            NULL, 0, NULL, 0) != UNZ_OK)
            return false;

        char fullpath[MAX_FILENAME];
        sprintf(fullpath, "%s%s", dest, filename);

        const size_t fullpath_length = strlen(fullpath);
        WString pathString(fullpath, BentleyCharEncoding::Utf8);
        if (!pathString.EndsWithI(L"/")) //a file
            {
            tmpName = BeFileName(fullpath);
            if (!tmpName.DoesPathExist())
                BeFileName::CreateNewDirectory(tmpName.GetDirectoryName().c_str());

            if (unzOpenCurrentFile(uf) != UNZ_OK)
                return false;

            FILE *out = fopen(fullpath, "wb");
            if (out == NULL)
                return false;

            int status = UNZ_OK;
            do
                {
                status = unzReadCurrentFile(uf, read_buffer, READ_SIZE);
                if (status < 0)
                    return false;
                if (status > 0)
                    fwrite(read_buffer, status, 1, out);
                } while (status > 0);
            fclose(out);
            // Set the date to original file
            DateTime fileTime(DateTime::Kind::Local, (uint16_t)file_info.tmu_date.tm_year, (uint8_t)(file_info.tmu_date.tm_mon + 1), (uint8_t)file_info.tmu_date.tm_mday, (uint8_t)file_info.tmu_date.tm_hour, (uint8_t)file_info.tmu_date.tm_min, (uint8_t)file_info.tmu_date.tm_sec);
            time_t fileModifTime = (time_t)(file_info.dosDate);
            fileTime.ToUnixMilliseconds(fileModifTime);
            fileModifTime /= 1000;

            BeFileName::SetFileTime(WString(fullpath, true).c_str(), &fileModifTime, &fileModifTime);
            }
        else // a folder
            {
            if (!BeFileName::DoesPathExist(pathString.c_str()) && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(pathString.c_str()))
                return false;
            }
        unzCloseCurrentFile(uf);

        if ((i + 1) < unzGlobalInfo.number_entry)
            {
            if (unzGoToNextFile(uf) != UNZ_OK)
                return false;
            }
        }

    return (unzClose(uf) == 0);
    }

void upload(BeFileName zippedFile, Utf8String fileName, Utf8String resolution, Utf8String footprint, Utf8String extension)
    {
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    properties.Insert(RealityDataField::Name, fileName);
    if(resolution.length() > 2)
        properties.Insert(RealityDataField::ResolutionInMeters, resolution);
    properties.Insert(RealityDataField::Streamed, "false");
    Utf8String rootDocument = Utf8String(zippedFile.GetFileNameAndExtension().c_str());
    rootDocument.ReplaceAll("\\","/");
    properties.Insert(RealityDataField::RootDocument, rootDocument);
    properties.Insert(RealityDataField::Classification, "Terrain");
    properties.Insert(RealityDataField::Type, extension);
    properties.Insert(RealityDataField::Description, description);
    properties.Insert(RealityDataField::Visibility, "PUBLIC");
    properties.Insert(RealityDataField::Listable, "true");
    properties.Insert(RealityDataField::Copyright, "Commonwealth of Australia (Geoscience Australia) 2010");
    properties.Insert(RealityDataField::TermsOfUse, "Creative Commons Attribution 4.0 International Licence");
    properties.Insert(RealityDataField::Dataset, dataset);
    properties.Insert(RealityDataField::MetadataUrl, metadata);
    if(footprint.length() > 0)
        properties.Insert(RealityDataField::Footprint, footprint);

    Utf8String propertyString = RealityDataServiceUpload::PackageProperties(properties);

    BeFileName TempPath;
    Desktop::FileSystem::BeGetTempPath(TempPath);

    int retryTimer = 1000;
    while (retryTimer < 256000)
        {
        RealityDataServiceUpload* upload = new RealityDataServiceUpload(zippedFile, "", propertyString, false, true, statusFunc);
        if (upload->IsValidTransfer())
            {
            std::cout << Utf8PrintfString("Upload file : %s \n", zippedFile.GetNameUtf8().c_str());

            upload->SetProgressCallBack(progressFunc);
            upload->SetProgressStep(0.5);
            upload->OnlyReportErrors(true);
            const TransferReport& ur = upload->Perform();
            delete upload; //close filestream
            Utf8String report;
            ur.ToXml(report);
            if(report.ContainsI("error"))
                std::cout << zippedFile.GetNameUtf8() << " failed to upload\n" << std::endl;
            else if (memorySaverMode)
                BeFileName::BeDeleteFile(zippedFile.GetName());
            std::cout << report << std::endl;
            break;
            }
        else
            {
            std::cout << zippedFile.GetNameUtf8() << " failed to create\n" << std::endl;
            retryTimer *= 2;
            if(retryTimer < 512001) //ims gets grumpy sometimes and we need to retry
                {
                std::cout << "retrying in "<< retryTimer << "ms\n" << std::endl;
                Sleep(retryTimer);
                }
            else
                std::cout << "moving on\n" << std::endl;
            }
        }
    }

void extractData(BeFileName file)
    {
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    
    // Data extraction.
    // &&AR Not all traversed files are raster ... we must try out other types or introduce a generic creater.
    RealityDataExtractPtr pData = RasterData::Create(file.GetNameUtf8().c_str());

    // Name.
    Utf8String name = file.GetNameUtf8();
    name.erase(0, file.GetNameUtf8().find_last_of("\\") + 1);
    name = name.erase(name.find_last_of('.')).c_str();
    
    // Date.
    /*time_t lastModifiedTime;
    file.GetFileTime(NULL, NULL, &lastModifiedTime);
    DateTime date = DateTime();
    if (NULL != lastModifiedTime)
        DateTime::FromUnixMilliseconds(date, lastModifiedTime * 1000);

    Utf8String dateString = date.ToString();*/

    // Resolution and geocoding.
    RasterDataPtr pRasterData = dynamic_cast<RasterDataP>(pData.get());
    Utf8String resolution = "";
    if (pRasterData != NULL)
        {
        resolution = pRasterData->ComputeResolutionInMeters();
        }

    // Footprint.
    bvector<GeoPoint2d> shape = bvector<GeoPoint2d>();
    DRange2d extents = DRange2d();
    Utf8String footprint;
    if (SUCCESS == pData->GetFootprint(&shape, &extents))
        {
        if(shape.size() == 2)
            shape = Extents2Footprint(shape);
        footprint = RealityDataBase::FootprintToRDSString(shape, "WGS84");
        }
    
    if (resolution.length() == 0 || footprint.length() == 0)
        {
        std::cout << name << " broken\n" <<std::endl;
        return;
        }

    Utf8String extension = Utf8String(file.GetExtension().c_str());

    ZipFile(file);
    upload(file, name, resolution, footprint, extension);
    }

bool validatePath(Utf8StringR path)
    {
    std::string str;
    std::getline(std::cin, str);
    path = Utf8String(str.c_str());
    BeFileName fileName = BeFileName(path);
    return fileName.DoesPathExist();
    }

void walk(BeFileName root)
    {
    bvector<BeFileName> fileList;
    BeDirectoryIterator::WalkDirsAndMatch(fileList, root, L"*.tif", true);
    BeDirectoryIterator::WalkDirsAndMatch(fileList, root, L"*.adf", true);

    // Consider files that are equal or less than 1kb garbage.
    for (size_t i = 0; i < fileList.size(); ++i)
        {
        uint64_t size;
        fileList[i].GetFileSize(size);
        size /= 1024; // bytes to kylobites.
        if (size <= 1)
            {
            fileList.erase(fileList.begin() + i);
            i -= 1;
            }
        }

    for (BeFileName subFile : fileList)
        {
        extractData(subFile);
        }
    }

int main(int argc, char *argv[])
    {
    //RealityDataService::SetServerComponents("connect-realitydataservices.bentley.com", "2.5", "S3MXECPlugin--Server", "S3MX");
    //RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.5", "S3MXECPlugin--Server", "S3MX");
    std::cout << "If you want to specifically contact dev or qa, enter dev or qa. For a custom server url, type \"custom\".\n"
        "Otherwise, enter blank and we will connect you to the proper server for you ConnectionClient configuration\n" << std::endl;
    std::string str;
    std::getline(std::cin, str);
    Utf8String serverChoice = Utf8String(str.c_str());
    Utf8String server;
    if (serverChoice.EqualsI("dev"))
        server = MakeBuddiCall(103);
    else if (serverChoice.EqualsI("qa"))
        server = MakeBuddiCall(102);
    else if (serverChoice.EqualsI("custom"))
        {
        std::cout << "Enter server url\n" << std::endl;
        std::getline(std::cin, str);
        server = Utf8String(str.c_str()).Trim();
        }
    else
        server = MakeBuddiCall(0);
    
    RealityDataService::SetServerComponents(server, "2.5", "S3MXECPlugin--Server", "S3MX");

    std::cout << "Please enter the source folder on the local machine (must be existing folder)" << std::endl;
    Utf8String uploadPathUtf8;
    while (!validatePath(uploadPathUtf8))
        std::cout << "folder could not be found, try again\n" << std::endl;

    //BeFileName uploadPath = BeFileName("D:/Australia/SRTM1/");
    BeFileName uploadPath = BeFileName(uploadPathUtf8);

    std::cout << "do we need to unzip the contained files? (\"y\", \"n\")\n" << std::endl;
    std::getline(std::cin, str);
    bool doUnzip = str == "y";
    //BeFileName tempPath = BeFileName("D:/Australia2/");
    BeFileName tempPath;
    if(doUnzip)
        {
        std::cout << "please specify output folder for unzipped files\n" << std::endl;
        std::getline(std::cin, str);
        tempPath = BeFileName(str.c_str());
        }
    else
        tempPath = uploadPath;

    std::cout << "minimize memory use (delete zips, after upload)? (\"y\", \"n\")\n" << std::endl;
    std::getline(std::cin, str);
    memorySaverMode = str == "y";

    std::cout << "input metadata" << std::endl;
    std::getline(std::cin, str);
    metadata = Utf8String(str.c_str());

    std::cout << "input dataset" << std::endl;
    std::getline(std::cin, str);
    dataset = Utf8String(str.c_str());

    std::cout << "input description" << std::endl;
    std::getline(std::cin, str);
    description = Utf8String(str.c_str());

    if (uploadPath.DoesPathExist() && uploadPath.IsDirectory()) //path is directory, find all documents
        {
        if(doUnzip)
            {
            bvector<BeFileName> zipFileList;
            BeDirectoryIterator::WalkDirsAndMatch(zipFileList, uploadPath, L"*.zip", true);
            
            for (BeFileName subFile : zipFileList)
                {
                if (!uploadPath.Equals(tempPath))
                    {
                    BeFileName::EmptyAndRemoveDirectory(tempPath.GetName());
                    BeFileName::CreateNewDirectory(tempPath.GetName());
                    UnZipFile(subFile, tempPath); //move files from zip to tempPath
                    walk(tempPath);
                    }
                else
                    UnZipFile(subFile, tempPath); //move files from zip to tempPath
                }

            if (uploadPath.Equals(tempPath))
                walk(tempPath);
            else
                walk(uploadPath);
            }
        else
            walk(uploadPath);
        }
    }

/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/BeFileName.h>

#include <RealityPlatformTools/RealityDataDownload.h>

#include <zlib/zip/unzip.h>


//#define TRACE_DEBUG 1

#define MAX_NB_CONNECTIONS          10
#define DEFAULT_STEP_PROGRESSCALL   (64*1024)      // default step if filesize is absent.
#define MAX_FILENAME                512
#define READ_SIZE                   8192

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=======================================================================================
//                              RealityDataDownload
//=======================================================================================

/*static bvector<downloadCap>::itterator findInCap(const Utf8String& capId, const downloadCap& caps)
    {
    return std::find_if(caps.begin(), caps.end(), [&capId](const downloadCap& obj) {return obj.sourceId == capId; })
    }*/

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const UrlLink_UrlFile& pi_Link_FileName)
    {
    if (pi_Link_FileName.size() > 0)
        return new RealityDataDownload(pi_Link_FileName);
    else
        return NULL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Spencer.Mason      9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const Link_File_wMirrors& pi_Link_File_wMirrors)
    {
    if (pi_Link_File_wMirrors.size() > 0)
        return new RealityDataDownload(pi_Link_File_wMirrors);
    else
        return NULL;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Spencer.Mason      9/2015
//----------------------------------------------------------------------------------------
RealityDataDownloadPtr RealityDataDownload::Create(const Link_File_wMirrors_wSisters& pi_Link_File_wMirrors_wSisters)
{
    if (pi_Link_File_wMirrors_wSisters.size() > 0)
        return new RealityDataDownload(pi_Link_File_wMirrors_wSisters);
    else
        return NULL;
}

RealityDataDownload::RealityDataDownload() : m_pProgressFunc(nullptr), m_pHeartbeatFunc(nullptr), m_pStatusFunc(nullptr),
    m_pProxyFunc(nullptr), m_pTokenFunc(nullptr), m_certPath(WString()), m_curEntry(0)
    {
    m_pToolHandle = NULL;
    }

void RealityDataDownload::InitEntry(size_t i)
    {
    m_pEntries[i].iAppend = 0;
        m_pEntries[i].nbRetry = 0;
        m_pEntries[i].index = i;

        m_pEntries[i].downloadedSizeStep = 0; //DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
        m_pEntries[i].filesize = 0;
        m_pEntries[i].fromCache = true;                                 // from cache if possible by default
        m_pEntries[i].progressStep = 0.01f;
    }

void RealityDataDownload::AddSisterFiles(FileTransfer* ft, bvector<url_file_pair> sisters, size_t index, size_t sisterCount, size_t sisterIndex)
    {
    FileTransfer* sisFT = new FileTransfer(ft);
    Mirror_struct ms;
    ms.url = sisters[index].m_url;
    sisFT->filename = ms.filename = sisters[index].m_filePath;
    ms.tokenType = sisters[index].m_tokenType;
    ms.nextSister = nullptr;
    ms.totalSisters = sisterCount;
    ms.sisterIndex = sisterIndex;
    ms.cap = sisters[index].m_cap;
    sisFT->mirrors.clear();
    sisFT->mirrors.push_back(ms);

    ft->mirrors.back().nextSister = sisFT;
    index++;
    if (index >= sisters.size())
        return;
    else
        AddSisterFiles(sisFT, sisters, index, sisterCount, sisterIndex + 1);
    }

bool RealityDataDownload::SetupMirror(size_t index, int errorCode)
    {
    if(m_pEntries[index].mirrors.size() <= 1)
        return false;

    char errorMsg[512];
    sprintf(errorMsg, "could not download %ls, error code %d. Attempting to retrieve mirror file %ls",
        m_pEntries[index].mirrors[0].filename.c_str(),
        errorCode,
        m_pEntries[index].mirrors[1].filename.c_str());
    ReportStatus((int)m_pEntries[index].index, &(m_pEntries[index]), REALITYDATADOWNLOAD_MIRROR_CHANGE, errorMsg);
        
    m_pEntries[index].mirrors.erase(m_pEntries[index].mirrors.begin());
    m_pEntries[index].filename = m_pEntries[index].mirrors.front().filename;
    m_pEntries[index].iAppend = 0;
    m_pEntries[index].nbRetry = 0;

    m_pEntries[index].downloadedSizeStep = 0; //DEFAULT_STEP_PROGRESSCALL;   // default step if filesize is absent.
    m_pEntries[index].filesize = 0;
    m_pEntries[index].fromCache = true;                                 // from cache if possible by default
    m_pEntries[index].progressStep = 0.01f;
    SetupRequestandFile(&m_pEntries[index]);
    return true;
    }   

bool RealityDataDownload::SetupNextEntry()
    {
    SetupRequestStatus status;
    do 
        {
        while(m_omittedEntries.count(m_curEntry) > 0)
            ++m_curEntry;
        if (m_curEntry < m_nbEntry)
            {
            status = SetupRequestandFile(&m_pEntries[m_curEntry]);
            ++m_curEntry;
            }
        else
            return false;

        } while (SetupRequestStatus::FromCache == status);

    return true;
    }   


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
void RealityDataDownload::ExtractFileName(WString& pio_rFileName, const AString& pi_Url)
    {
    // Extract filename form URL, the last part of the URL until a '/'or '\' or '='
    WString urlW(pi_Url.c_str(), BentleyCharEncoding::Utf8);
    urlW.ReplaceAll(WCSALT_DIR_SEPARATOR, WCSDIR_SEPARATOR);
    bvector<WString> pathComponents;
    WString delim = WCSDIR_SEPARATOR;
    delim += L"=";
    BeStringUtilities::Split(urlW.c_str(), delim.c_str(), NULL, pathComponents);

    BeFileName::BuildName(pio_rFileName, NULL, pio_rFileName.c_str(), pathComponents[pathComponents.size() - 1].c_str(), NULL);

    }

bool RealityDataDownload::UnZipFile(WString& pi_strSrc, WString& pi_strDest)
    {
    char src[MAX_FILENAME];
    char dest[MAX_FILENAME];

    wcstombs (src, pi_strSrc.c_str(), pi_strSrc.size());
    wcstombs (dest, pi_strDest.c_str(), pi_strDest.size());

    src[pi_strSrc.size()] = 0;
    dest[pi_strDest.size()] = 0;

    return UnZipFile(src, dest);
    }

bool RealityDataDownload::UnZipFile(const char* pi_strSrc, const char* pi_strDest)
    {
    if(!strstr(pi_strSrc, ".zip"))
        return false;
    
    unzFile uf = unzOpen(pi_strSrc);
    if(nullptr == uf)
        return false;

    unz_global_info unzGlobalInfo;
    if(unzGetGlobalInfo (uf, &unzGlobalInfo) != 0)
        return false;

    uLong i;
    char read_buffer[ READ_SIZE ];
    for( i = 0; i < unzGlobalInfo.number_entry; ++i )
        {
        unz_file_info file_info;
        char filename[MAX_FILENAME];
        if ( unzGetCurrentFileInfo(
            uf,
            &file_info,
            filename,
            MAX_FILENAME,
            NULL, 0, NULL, 0 ) != UNZ_OK )
            return false;
        
        char fullpath[MAX_FILENAME];
        sprintf(fullpath, "%s%s", pi_strDest,filename);

        WString pathString(fullpath, BentleyCharEncoding::Utf8);
        if(!pathString.EndsWithI(L"/")) //a file
            {
            if(unzOpenCurrentFile( uf ) != UNZ_OK)
                return false;

            FILE *out = fopen( fullpath, "wb" );
            if ( out == NULL)
                return false;

            int status = UNZ_OK;
            do
                {
                status = unzReadCurrentFile( uf, read_buffer, READ_SIZE );
                if(status < 0)
                    return false;
                if(status > 0)
                    fwrite( read_buffer, status, 1, out );
                } while (status > 0 );
            fclose( out );
            // Set the date to original file
            DateTime fileTime(DateTime::Kind::Local, (uint16_t)file_info.tmu_date.tm_year, (uint8_t)(file_info.tmu_date.tm_mon + 1), (uint8_t)file_info.tmu_date.tm_mday, (uint8_t)file_info.tmu_date.tm_hour, (uint8_t)file_info.tmu_date.tm_min, (uint8_t)file_info.tmu_date.tm_sec);
            time_t fileModifTime = (time_t)(file_info.dosDate);
            int64_t fileTimeUnix;
            fileTime.ToUnixMilliseconds(fileTimeUnix);
            fileModifTime = fileTimeUnix / 1000;

            BeFileName::SetFileTime(WString(fullpath, true).c_str(), &fileModifTime, &fileModifTime);
            }
        else // a folder
            {
            if(!BeFileName::DoesPathExist(pathString.c_str()) && BeFileNameStatus::Success != BeFileName::CreateNewDirectory(pathString.c_str()))
                return false;
            }
        unzCloseCurrentFile( uf );
    
        if( ( i+1 ) < unzGlobalInfo.number_entry )
            {
            if ( unzGoToNextFile( uf ) != UNZ_OK)
                return false;
            }
        }
    
    return (unzClose (uf) == 0);
    }

void RealityDataDownload::ReportStatus(int index, void *pClient, int ErrorCode, const char* pMsg)
    {
    if(m_pStatusFunc)
        m_pStatusFunc(index, pClient, ErrorCode, pMsg);

    RealityDataDownload::FileTransfer* pEntry = (RealityDataDownload::FileTransfer*)pClient;
        
    bmap<WString, TransferReport*>::iterator it = m_dlReport->results.find(pEntry->filename);
    if(it == m_dlReport->results.end())
        return;//something went wrong

    TransferReport* tr = it->second;
    tr->filesize = pEntry->filesize;

    DownloadResult dr = DownloadResult();
    dr.errorCode = ErrorCode;
    dr.downloadProgress = pEntry->downloadedSizeStep;
    if(pEntry->filesize != 0)
        dr.downloadProgress /= pEntry->filesize;

    tr->retries.push_back(dr);

    if (ErrorCode != REALITYDATADOWNLOAD_RETRY_TENTATIVE)
        {
        tr->timeSpent = time(nullptr) - pEntry->mirrors[0].DownloadStart;
        }
    }

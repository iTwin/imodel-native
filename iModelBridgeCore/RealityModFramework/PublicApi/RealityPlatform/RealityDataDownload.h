/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityDataDownload.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "RealityPlatformAPI.h"
#include <Bentley/BeFile.h>

// 
// index --> Url index set at the create
// index == -1 --> General error, pClient will be NULL.
// ErrorCode --> Curl error code.
// If RealityDataDownload_ProgressCallBack returns # 0 --> cancel the transfer
typedef int(__cdecl *RealityDataDownload_ProgressCallBack)(int index, void *pClient, size_t ByteCurrent, size_t ByteTotal);
typedef void(__cdecl *RealityDataDownload_StatusCallBack) (int index, void *pClient, int ErrorCode, const char* pMsg);


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
//! RealityDataDownload
//! @bsiclass
//=======================================================================================
struct RealityDataDownload : public RefCountedBase
{
public:
    struct FileTransfer 
        {
        AString  url;
        WString  filename;
        int      index;


        // internal used only
        BeFile                  fileStream;
        size_t                  iAppend;
        RealityDataDownload_ProgressCallBack pProgressFunc;
        };

    
    REALITYDATAPLATFORM_EXPORT static RealityDataDownloadPtr Create(const std::pair<AString, WString>* pi_pUrl_OutFileName, int pi_nbEntry);

    REALITYDATAPLATFORM_EXPORT void SetProgressCallBack(RealityDataDownload_ProgressCallBack pi_func) {m_pProgressFunc = pi_func;};
    REALITYDATAPLATFORM_EXPORT void SetStatusCallBack(RealityDataDownload_StatusCallBack pi_func) { m_pStatusFunc = pi_func; };

    REALITYDATAPLATFORM_EXPORT bool Perform();

private:
    RealityDataDownload() { m_pCurlHandle=NULL;};
    RealityDataDownload(const std::pair<AString, WString>* pi_pUrl_OutFileName, int pi_nbEntry);
    ~RealityDataDownload();

    bool SetupCurlandFile(int pi_index);

    void*                       m_pCurlHandle;
    int                         m_nbEntry;
    int                         m_curEntry;
//    HArrayAutoPtr<FileTransfer> m_pEntries;
    FileTransfer                *m_pEntries;

    RealityDataDownload_ProgressCallBack    m_pProgressFunc;
    RealityDataDownload_StatusCallBack      m_pStatusFunc;

};

END_BENTLEY_REALITYPLATFORM_NAMESPACE
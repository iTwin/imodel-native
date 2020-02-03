/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../RealityPlatformTools/RealityDataService.cpp"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
const TransferReport& RealityDataServiceTransfer::Perform()
    {
    m_currentTransferedAmount = 0;
    m_progress = 0.0;

    m_report = TransferReport();

    if (!IsValidTransfer())
        {
        ReportStatus(0, nullptr, -1, "No files to transfer, please verify that the previous steps completed without failure");
        return m_report;
        }

    if(m_multiRequestCallback)
        m_multiRequestCallback(m_filesToTransfer, m_pProgressFunc);

    return m_report;
    }

//=====================================================================================
//! @bsimethod                                   Spencer.Mason              02/2017
//=====================================================================================
void RealityDataServiceTransfer::SetupRequestforFile(RealityDataUrl* request, bool verifyPeer)
    {
    // If cancel requested, don't queue new files
    if (NULL != m_pHeartbeatFunc && m_pHeartbeatFunc() != 0)
        return;

    RealityDataFileTransfer* fileTransfer = dynamic_cast<RealityDataFileTransfer*>(request);
    if (fileTransfer != nullptr)
    {
        fileTransfer->SetAzureToken(GetAzureToken());
        fileTransfer->UpdateTransferedSize();
    }
    else
        return; //unexpected request

    RawServerResponse& rawResponse = fileTransfer->GetResponse();
    rawResponse.clear();
    rawResponse.toolCode = ServerType::Azure;

    if(m_setupCallback)
        m_setupCallback(request, verifyPeer);
    }

void RealityDataServiceTransfer::InitTool()
    {}
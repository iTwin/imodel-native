//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSHIBPHandler.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> Class : HFSHIBPHandler
//:>---------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCInternetConnection.h>
#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCVersion.h>


/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class is use to handling the HIBP protocol for HFHHIBP.

    @see HFSHIBP
    -----------------------------------------------------------------------------
*/
class HFSHIBPHandler : public HFCShareableObject<HFSHIBPHandler>
    {
public:

    HDECLARE_BASECLASS_ID(5023);

    //:> Primary method
    HFSHIBPHandler(HFCPtr<HFCInternetConnection>&  pi_rpConnection,
                   uint32_t                        pi_ServerTimeOut);
    virtual                     ~HFSHIBPHandler();

    virtual uint32_t            CountEntries(const WString& pi_rPath);

    virtual void                GetEntries(const WString&   pi_rPath,
                                           uint32_t         pi_FirstEntryIndex = 0,
                                           uint32_t         pi_MaxCount = ULONG_MAX);

    virtual bool               ReadEntry(uint32_t*    po_pEntryIndex,
                                          WString*   po_pItemName,
                                          bool*     po_pFolder);

    virtual const HFCPtr<HFCInternetConnection>&
    GetConnection() const;


private:

    mutable HFCPtr<HFCInternetConnection>
    m_pConnection;
    WString             m_Version;
    HAutoPtr<HFCBuffer> m_pBuffer;
    wostringstream      m_CurrentRequest;
    bool               m_UseUTF8Protocol;
    uint32_t            m_ServerTimeOut;

    bool               ConnectToServer(const HFCVersion& pi_rVersion);


    bool               ReadEntry(HFCBuffer*    pio_pBuffer,
                                  WString*      po_pItemName,
                                  bool*        po_pFolder) const;

    void                ReadHIBPPathRespondHeader(HFCBuffer* pio_pBuffer,
                                                  WString*   po_pVersion,
                                                  uint32_t*    po_pDataSize) const;

    void                ReadHIBPPathDataHeader(HFCBuffer*  pio_pBuffer,
                                               uint32_t*     po_pCompression,
                                               uint32_t*     po_pCompressionInfo,
                                               uint32_t*     po_pNbEntries,
                                               uint32_t*     po_pCurrentPass) const;

    void                HandleHIBPError(HFCBuffer* pi_pBuffer) const;


    //:> Disabled methods
    HFSHIBPHandler(const HFSHIBPHandler&);
    HFSHIBPHandler& operator=(const HFSHIBPHandler&);
    };


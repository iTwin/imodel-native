//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPWEditor.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

class HRFPWRasterFile;
class HRFPWHandler;
class IHRFPWFileHandler;

class HRFPWEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)

    friend class HRFPWRasterFile;

    virtual         ~HRFPWEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock (uint32_t                 pi_PosBlockX,
                               uint32_t                 pi_PosBlockY,
                               Byte*                   po_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS          ReadBlock     (uint32_t                 pi_PosBlockX,
                                            uint32_t                 pi_PosBlockY,
                                            HFCPtr<HCDPacket>&       po_rpPacket,
                                            HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint32_t                 pi_PosBlockX,
                               uint32_t                 pi_PosBlockY,
                               const Byte*             pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

    virtual HSTATUS WriteBlock(uint32_t                 pi_PosBlockX,
                               uint32_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0);

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFPWEditor
    (HFCPtr<HRFRasterFile>     pi_rpRasterFile,
     uint32_t                  pi_Page,
     unsigned short           pi_Resolution,
     HFCAccessMode             pi_AccessMode);


private:

    GUID                m_DocumentID;
    time_t              m_DocumentTimestamp;

    IHRFPWFileHandler*  m_pHandler;

    // Methods Disabled
    HRFPWEditor(const HRFPWEditor& pi_rObj);
    HRFPWEditor& operator=(const HRFPWEditor& pi_rObj);

    };


#if 0
#include "IHRFPWFileHandler.h"

class HRFPWHandler : public IHRFPWFileHandler
    {
public:
    HRFPWHandler(const HFCURL& pi_rPWFile);
    ~HRFPWHandler();

    virtual GETTILE_STATUS GetTile(uint32_t pi_Page,
                                   unsigned short pi_Res,
                                   uint32_t pi_PosX,
                                   uint32_t pi_PosY,
                                   Byte*  po_pData);


private:
    HFCPtr<HRFRasterFile> m_pRasterFile;

    };
#endif
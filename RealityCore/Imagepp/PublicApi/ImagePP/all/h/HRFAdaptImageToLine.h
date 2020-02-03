//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFAdaptImageToLine
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

#include "HRFBlockAdapter.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRFRasterFile;

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
class HRFAdaptImageToLineCapabilities : public HRFBlockAdapterCapabilities
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptImageToLineCapabilities)

public:
    HRFAdaptImageToLineCapabilities();
    };

//-----------------------------------------------------------------------------
// This is a utility class to create a specific Implementation object.
// There will be an object that derives from this one for each Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------
class HRFAdaptImageToLineCreator : public HRFBlockAdapterCreator
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFAdaptImageToLineCreator)

public:
    virtual ~HRFAdaptImageToLineCreator();
        
    // Obtain the capabilities of stretcher
    virtual HRFBlockAdapterCapabilities* GetCapabilities() const;

    // Creation of implementator
    virtual HRFBlockAdapter*             Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                uint32_t              pi_Page,
                                                uint16_t       pi_Resolution,
                                                HFCAccessMode         pi_AccessMode) const;
    };

//-----------------------------------------------------------------------------
// This specific implementation adapt the storage type to another
//-----------------------------------------------------------------------------
class HRFAdaptImageToLine : public HRFBlockAdapter
    {
public:
    DEFINE_T_SUPER(HRFBlockAdapter)

    // friend class HRFRasterFile;
    HRFAdaptImageToLine(
        HRFBlockAdapterCapabilities*   pi_pCapabilities,
        HFCPtr<HRFRasterFile>          pi_rpRasterFile,
        uint32_t                      pi_Page,
        uint16_t                 pi_Resolution,
        HFCAccessMode                  pi_AccessMode);

    virtual                ~HRFAdaptImageToLine();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              Byte*                    po_pData) override;

    virtual HSTATUS ReadBlock(uint64_t                 pi_PosBlockX,
                              uint64_t                 pi_PosBlockY,
                              HFCPtr<HCDPacket>&       po_rpPacket) override
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket) override
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket);
        }



protected:

    // tile Information
    uint32_t                m_ExactBytesPerLine;
    uint32_t                m_RasterHeight;
    uint32_t                m_NextLineToWrite;

    HArrayAutoPtr<Byte>    m_pImage;

private:

    void    Alloc_m_pImage      (void);
    void    Delete_m_pImage     (void);

    // Methods Disabled
    HRFAdaptImageToLine(const HRFAdaptImageToLine& pi_rObj);
    HRFAdaptImageToLine& operator=(const HRFAdaptImageToLine& pi_rObj);
    };
END_IMAGEPP_NAMESPACE




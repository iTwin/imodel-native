//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGdalSupportedFileEditor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// This class describes the resolution editor interface
//-----------------------------------------------------------------------------

#pragma once

#include "HRFResolutionEditor.h"

// From GdalLib
class GDALRasterBand;

//RGB
#define RED_BAND       0
#define GREEN_BAND     1
#define BLUE_BAND      2
#define ALPHA_EXT_BAND 3
//Mono
#define GRAY_BAND      0
//Palette
#define PALETTE_BAND   0
//YCbCr
#define Y_BAND         0
#define CB_BAND        1
#define CR_BAND        2

BEGIN_IMAGEPP_NAMESPACE

class HRFGdalSupportedFile;

class HRFGdalSupportedFileEditor : public HRFResolutionEditor
    {
public:
    DEFINE_T_SUPER(HRFResolutionEditor)
    friend class HRFGdalSupportedFile;

    virtual ~HRFGdalSupportedFileEditor  ();

    // Edition by Block
    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              Byte*                   po_pData,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0) override;

    virtual HSTATUS ReadBlock(uint64_t                pi_PosBlockX,
                              uint64_t                pi_PosBlockY,
                              HFCPtr<HCDPacket>&      po_rpPacket,
                              HFCLockMonitor const*   pi_pSisterFileLock = 0)
        {
        return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
        }

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const Byte*              pi_pData,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0) override;

    virtual HSTATUS WriteBlock(uint64_t                 pi_PosBlockX,
                               uint64_t                 pi_PosBlockY,
                               const HFCPtr<HCDPacket>& pi_rpPacket,
                               HFCLockMonitor const*    pi_pSisterFileLock = 0)
        {
        return T_Super::WriteBlock(pi_PosBlockX,pi_PosBlockY,pi_rpPacket,pi_pSisterFileLock);
        }


    void                            SetRGBABandLinearScaling(double  pi_MinRedValue,
                                                             double  pi_MaxRedValue,
                                                             double  pi_MinGreenValue,
                                                             double  pi_MaxGreenValue,
                                                             double  pi_MinBlueValue,
                                                             double  pi_MaxBlueValue,
                                                             double* pi_pMinAlphaOrExtendValue = 0,
                                                             double* pi_pMaxAlphaOrExtendValue = 0);

    void                            SetSingleBandLinearScaling(double pi_MinGrayValue,
                                                               double pi_MaxGrayValue);

protected:
    // See the parent for Pointer to the raster file, to the resolution descriptor
    // and to the capabilities

    // Constructor
    HRFGdalSupportedFileEditor                 (HFCPtr<HRFRasterFile>       pi_rpRasterFile,
                                                uint32_t                   pi_Page,
                                                unsigned short              pi_Resolution,
                                                HFCAccessMode               pi_AccessMode);

    HSTATUS        ReadIntegerBlock            (uint64_t                    pi_PosBlockX,
                                                uint64_t                    pi_PosBlockY,
                                                Byte*                       po_pData,
                                                HFCLockMonitor const*       pi_pSisterFileLock = 0);

    HSTATUS        ReadRealBlock               (uint64_t                    pi_PosBlockX,
                                                uint64_t                    pi_PosBlockY,
                                                Byte*                       po_pData,
                                                HFCLockMonitor const*       pi_pSisterFileLock = 0);


    HSTATUS         WriteBandBlock             (const Byte*                 pi_pInBuffer,
                                                const uint64_t              pi_BandIndex,
                                                const uint64_t              pi_PosBlockX,
                                                const uint64_t              pi_PosBlockY);

    HSTATUS         ReadBandBlock              (Byte*                       po_pOutBuffer,
                                                const uint64_t              pi_BandIndex,
                                                const uint64_t              pi_PosBlockX,
                                                const uint64_t              pi_PosBlockY);

    void FindRealPVminMax(double*       po_pMin,
                          double*       po_pMax);

    typedef HSTATUS (HRFGdalSupportedFileEditor::*ReadBlockFncPtr)(uint64_t                 pi_PosBlockX,
                                                                   uint64_t                 pi_PosBlockY,
                                                                   Byte*                    pi_pData,
                                                                   HFCLockMonitor const*    pi_pSisterFileLock);

    typedef void (HRFGdalSupportedFileEditor::*ScalePixelFncPtr)(Byte* pi_pData);

    void    DetermineGdalDataType();
    void    DetermineScalingMethod();

    void    ComputeIOblockSize(uint32_t pi_PosBlockX,
                               uint32_t pi_PosBlockY);

    void    Scaling8BitsBlock(Byte* po_pData);

    void    Scaling16BitsBlock(Byte* po_pData);

    void    RevertScaling16BitsBlock(Byte* po_pData);

    void    Scaling32BitsBlock(Byte* po_pData);

    void    RevertScaling32BitsBlock(Byte* po_pData);

    void    ScalingFloatBlock(Byte* po_pData);

    void    ScalingDoubleBlock(Byte* po_pData);

    void    WriteColorTable();

    //Function Pointer
    ReadBlockFncPtr                 m_pReadBlockFnc;
    ScalePixelFncPtr                m_pScalePixelFnc;

    unsigned short                  m_GdalDataType;
    HArrayAutoPtr<GDALRasterBand*>  m_pRasterBands;
    HRFBlockType                    m_pBlockType;
    uint32_t                       m_NbBands;
    HArrayAutoPtr<double>           m_pBandMinimum;
    HArrayAutoPtr<double>           m_pBandScaling;
    int                             m_BlockWidth;
    int                             m_BlockHeight;
    int                             m_WidthToRead;
    int                             m_HeightToRead;

    //Optimization
    uint32_t                       m_PixelPerLineBand;
    uint32_t                       m_BytesPerLineBand;
    uint32_t                       m_NbBytePerBandPerPixel;
    uint32_t                       m_NbBitsPerBandPerPixel;
    int32_t                         m_PixelSpaceInBytes;
    int32_t                         m_LineSpaceInBytes;

    bool                            m_UseLinearBandScaling;

    Byte                            m_ShiftSize;
    uint64_t                        m_NbPixelsPerBlock;

    HArrayAutoPtr<Byte>             m_pTempRealBuffer;
    HAutoPtr<Byte>                  m_pTempSignedBuffer;

private:

    // Methods Disabled
    HRFGdalSupportedFileEditor(const HRFGdalSupportedFileEditor& pi_rObj);
    HRFGdalSupportedFileEditor& operator=(const HRFGdalSupportedFileEditor& pi_rObj);

    //Utilities
    void    SetPalette();

    };
END_IMAGEPP_NAMESPACE

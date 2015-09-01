//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFResolutionDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFResolutionDescriptor
//-----------------------------------------------------------------------------
// This class describes a resolution of an image.
//-----------------------------------------------------------------------------
#pragma once

//#include "HRFTypes.h"
//#include "HFCAccessMode.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class  HCDCodec;
class  HRFResolutionEditor;
class  HRPPixelType;

class HRFResolutionDescriptor : public HFCShareableObject<HRFResolutionDescriptor>
    {
    HDECLARE_SEALEDCLASS_ID(HRFResolutionDescriptorId_Base)

public:
    // Resolution Creation and Destruction
    IMAGEPP_EXPORT HRFResolutionDescriptor(HFCAccessMode                               pi_AccessMode,
                                   const HFCPtr<HRFRasterFileCapabilities>&    pi_rpResolutionCapabilities,
                                   double                                     pi_ResolutionXRatio,
                                   double                                     pi_ResolutionYRatio,
                                   const HFCPtr<HRPPixelType>&                 pi_rpPixelType,
                                   const HFCPtr<HCDCodec>&                     pi_rpCodec,
                                   HRFBlockAccess                              pi_ReaderBlockAccess,
                                   HRFBlockAccess                              pi_WriterBlockAccess,
                                   HRFScanlineOrientation                      pi_ScanlineOrientation,
                                   HRFInterleaveType                           pi_InterleaveType,
                                   bool                                       pi_IsInterlace,
                                   uint64_t                                   pi_Width,
                                   uint64_t                                   pi_Height,
                                   uint32_t                                    pi_BlockWidth,
                                   uint32_t                                    pi_BlockHeight,
                                   const HRFDataFlag*                          pi_pBlocksDataFlag = 0,
                                   HRFBlockType                                pi_BlockType = HRFBlockType::AUTO_DETECT,
                                   Byte                                       pi_NumberOfPass = 1,
                                   unsigned short                             pi_PaddingBits = 8, // for now always to 8 bit
                                   HRFDownSamplingMethod                       pi_DownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR);


    IMAGEPP_EXPORT HRFResolutionDescriptor(const HRFResolutionDescriptor& pi_rObj);
    IMAGEPP_EXPORT HRFResolutionDescriptor& operator=(const HRFResolutionDescriptor& pi_rObj);
    IMAGEPP_EXPORT ~HRFResolutionDescriptor();

    IMAGEPP_EXPORT bool CanCreateWith(HFCAccessMode                              pi_AccessMode,
                               const HFCPtr<HRFRasterFileCapabilities>&   pi_rpResolutionCapabilities) const;




    HFCAccessMode                           GetAccessMode           () const;

    // Resolution information
    double                                 GetResolutionXRatio     () const;
    double                                 GetResolutionYRatio     () const;

    // Get number of pass to complete a resolution data - prossive format        = n pass
    //                                                  - non-progressive format = 1 pass
    Byte                                   GetNumberOfPass         () const;
    unsigned short                         GetPaddingBits          () const;

    // Storage information
    const HFCPtr<HCDCodec>&                GetCodec                () const;
    HRFBlockType                           GetBlockType            () const;

    bool                                   DownSamplingMethodHasChanged() const;
    HRFDownSamplingMethod                  GetDownSamplingMethod   () const;
    void                                   SetDownSamplingMethod   (HRFDownSamplingMethod pi_DownSamplingMethod);

    HRFBlockAccess                         GetReaderBlockAccess    () const;
    HRFBlockAccess                         GetWriterBlockAccess    () const;

    HRFScanlineOrientation                 GetScanlineOrientation  () const;
    HRFInterleaveType                      GetInterleaveType       () const;
    bool                                   IsInterlace             () const;

    // Pixel information
    const HFCPtr<HRPPixelType>&            GetPixelType            () const;
    uint32_t                              GetBitsPerPixel         () const;

    // Resolution information
    uint64_t                               GetWidth                () const;
    uint64_t                               GetHeight               () const;

    uint64_t                               GetBitsPerWidth         () const;
    uint32_t                              GetPaddingBitsPerWidth  () const;
    uint64_t                               GetBytesPerWidth        () const;

    uint64_t                               GetSizeInBytes          () const;

    // Block information
    uint32_t                              GetBlockWidth           () const;
    uint32_t                              GetBlockHeight          () const;

    uint32_t                              GetBitsPerBlockWidth    () const;
    uint32_t                              GetBytesPerBlockWidth   () const;

    uint32_t                              GetPaddingBitsPerBlockWidth () const;
    uint32_t                              GetBlockSizeInBytes     () const;

    uint64_t                               GetBlocksPerWidth       () const;
    uint64_t                               GetBlocksPerHeight      () const;
    uint64_t                               CountBlocks             () const;
    uint64_t                               ComputeBlockIndex       (uint64_t  pi_TilePosX,
                                                                     uint64_t  pi_TilePosY) const;
    void                                    ComputeBlockPosition    (uint64_t  pi_Index,
                                                                     uint64_t*   po_pTilePosX,
                                                                     uint64_t*   po_pTilePosY) const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ bool                HasBlocksDataFlag () const;
    IMAGEPP_EXPORT /*IppImaging_Needs*/ const HRFDataFlag*  GetBlocksDataFlag () const;
    IMAGEPP_EXPORT /*IppImaging_Needs*/ bool                SetBlocksDataFlag (const HRFDataFlag* pi_pBlocksDataFlag);

    IMAGEPP_EXPORT HRFDataFlag                      GetBlockDataFlag       (uint64_t   pi_Index) const;
    IMAGEPP_EXPORT void                             SetBlockDataFlag       (uint64_t   pi_Index,
                                                                    HRFDataFlag pi_DataFlag);
    void                                    ClearBlockDataFlag     (uint64_t   pi_Index,
                                                                    HRFDataFlag pi_DataFlag);

    // Flag to know if the specified data has changed
    bool                                   PaletteHasChanged      () const;
    bool                                   BlockDataFlagHasChanged() const;


    IMAGEPP_EXPORT static double                   RoundResolutionRatio(uint32_t pi_MainSize, uint32_t pi_SubResSize);

    void                                    Saved();
    void                                    BlockDataFlagSaved();

protected:
    friend class HRFResolutionEditor;
    void                                    PaletteChanged();

private:
    void                                   SetPixelType            (HFCPtr<HRPPixelType>& pi_rpPixelType);
    HRFResolutionDescriptor();
    // Capabilities
    HFCPtr<HRFRasterFileCapabilities>   m_pResolutionCapabilities;

    // Access Mode
    HFCAccessMode                       m_AccessMode;

    // Resolution information
    double                             m_ResolutionXRatio;
    double                             m_ResolutionYRatio;
    Byte                               m_NumberOfPass;
    unsigned short                     m_PaddingBits;

    // Storage information
    HFCPtr<HRPPixelType>                m_pPixelType;
    uint32_t                            m_BitsPerPixel;
    HFCPtr<HCDCodec>                    m_pCodec;
    HRFBlockType                        m_BlockType;
    HRFDownSamplingMethod               m_DownSamplingMethod;
    HRFBlockAccess                      m_ReaderBlockAccess;
    HRFBlockAccess                      m_WriterBlockAccess;
    HRFScanlineOrientation              m_ScanlineOrientation;
    HRFInterleaveType                   m_InterleaveType;
    bool                               m_IsInterlace;

    // The resolution information
    uint64_t                           m_Width;
    uint64_t                           m_Height;

    uint64_t                           m_BitsPerWidth;
    uint32_t                            m_PaddingBitsPerWidth;
    uint64_t                           m_BytesPerWidth;

    uint64_t                           m_SizeInBytes;

    // The block information
    uint32_t                            m_BlockWidth;
    uint32_t                            m_BlockHeight;

    uint32_t                            m_BitsPerBlockWidth;
    uint32_t                            m_PaddingBitsPerBlockWidth;
    uint32_t                            m_BytesPerBlockWidth;

    uint32_t                            m_BlockSizeInBytes;

    uint64_t                           m_BlocksPerWidth;
    uint64_t                           m_BlocksPerHeight;


    HArrayAutoPtr<HRFDataFlag>          m_pBlocksDataFlag;
    size_t                              m_BlocksDataFlagSize;

    // Flag to know if the specified data has changed
    bool                               m_PaletteHasChanged;
    bool                               m_BlockDataFlagHasChanged;
    bool                               m_DownSamplingMethodHasChanged;
    };
END_IMAGEPP_NAMESPACE

#include "HRFResolutionDescriptor.hpp"


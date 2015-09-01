//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFResolutionDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFCachedFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

//-----------------------------------------------------------------------------
// Public
// Default constructor.
//-----------------------------------------------------------------------------
HRFResolutionDescriptor::HRFResolutionDescriptor()
    {
    }

/** ----------------------------------------------------------------------------------------------
 *  Construct a new Resolution Descriptor using the source file format capabalities
 *  (pi_rpResolutionCapabilities) to validate all other parameter.
 *  <P>
 *
 *  @param pi_AccessMode                 The resolution descriptor access mode.
 *
 *  @param pi_rpResolutionCapabilities   The capabilities of the resolution. These capabilities
 *                                       should be the same as the capabilities of the file format
 *                                       where the resolution has been loaded from or will be stored to.
 *  @param pi_ResolutionXRatio           The X(pixel wise along a scanline) resolution factor.
 *                                       The best resolution is 1 and sub-resolution factor increase
 *                                       with worst resolutions. This number must comply with the
 *                                       restrictions imposed by the capabilities.
 *  @param pi_ResolutionYRatio           The Y(From first to last scanline) resolution factor.
 *                                       The best resolution is 1 and sub-resolution factor increase
 *                                       with worst resolutions. This number must comply with the
 *                                       restrictions imposed by the capabilities. The relation with
 *                                       the X resolution specified mut also comply with the restrictions
 *                                       imposed by the capabilities.
 *  @param pi_rpPixelType                The pixel type for raster data in the resolution
 *  @param pi_rpCodecsList               The list of codecs that can be applied to the resolution data.
 *  @param pi_ReaderBlockAccess          ????
 *  @param pi_WriterBlockAccess          ????
 *  @param pi_ScanlineOrientation        The scanline orientation for all blocks in the resolution.
 *                                       This orientation must also fit with orientation of all resolutions
 *                                       in the page (for now)
 *  @param pi_InterleaveType             The interleave type for all raster data blocks in the resolution
 *  @param pi_IsInterlace                true if raster data is or will be interlaced in the blocks;
 *                                       false if there is no interlace.
 *  @param pi_Width                      The width of the resolution in pixels excluding padding if
 *                                       applicable.
 *  @param pi_Height                     The height of the resolution in pixels excluding padding if
 *                                       applicable.
 *  @param pi_BlockWidth                 The normal block width in pixels. The last block may have less
 *                                       than this width or be padded to fit the width.
 *  @param pi_BlockHeight                The normal block height in pixels. The last block may have less
 *                                       than this height or be padded to fit the width.
 *  @param pi_pBlocksDataFlag            The blocks data flag ... do not ask me, I just document here!
 *  @param pi_BlockType                  The block type for the resolution.
 *  @param pi_NumberOfPass               The number of passes for representing data. The number of passes is
 *                                       1 except for progressive formats.
 *  @param pi_PaddingBits                Must be 8 .. <font color="#FF0000"> <B> do not ask me why! </B></font>
 *  @param pi_DownSamplingMethod         The down-sampling method ???
 *  @param pi_rObj                       Reference to a resolution to copy data from.
 *  ----------------------------------------------------------------------------------------------
 */
HRFResolutionDescriptor::HRFResolutionDescriptor(
    HFCAccessMode                            pi_AccessMode,
    const HFCPtr<HRFRasterFileCapabilities>& pi_rpResolutionCapabilities,
    double                                  pi_ResolutionXRatio,
    double                                  pi_ResolutionYRatio,
    const HFCPtr<HRPPixelType>&              pi_rpPixelType,
    const HFCPtr<HCDCodec>&                  pi_rpCodec,
    HRFBlockAccess                           pi_ReaderBlockAccess,
    HRFBlockAccess                           pi_WriterBlockAccess,
    HRFScanlineOrientation                   pi_ScanlineOrientation,
    HRFInterleaveType                        pi_InterleaveType,
    bool                                    pi_IsInterlace,
    uint64_t                                pi_Width,
    uint64_t                                pi_Height,
    uint32_t                                 pi_BlockWidth,
    uint32_t                                 pi_BlockHeight,
    const HRFDataFlag*                       pi_pBlocksDataFlag,
    HRFBlockType                             pi_BlockType,
    Byte                                    pi_NumberOfPass,
    unsigned short                          pi_PaddingBits,
    HRFDownSamplingMethod                    pi_DownSamplingMethod)
    {
    // We don't accept a null codec
    HPRECONDITION(pi_rpCodec != 0);

    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpResolutionCapabilities != 0);
    m_pResolutionCapabilities = pi_rpResolutionCapabilities;

    // Validation with the capabilities if it's possible to create a resolution
    HFCPtr<HRFRasterFileCapabilities> pPixelTypeCapabilities = pi_rpResolutionCapabilities->
                                                               GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                                                                       pi_AccessMode);

    HASSERT(pPixelTypeCapabilities != 0 || pPixelTypeCapabilities->CountCapabilities() > 0);

    HCLASS_ID CodecID = (pi_rpCodec == NULL) ? HCDCodecIdentity::CLASS_ID : pi_rpCodec->GetClassID();

    HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
    for (uint32_t i = 0; i < pPixelTypeCapabilities->CountCapabilities() && pPixelTypeCapability == 0; i++)
        {
        pPixelTypeCapability = (const HFCPtr<HRFPixelTypeCapability>&)pPixelTypeCapabilities->GetCapability(i);

        if (!(pPixelTypeCapability->GetPixelTypeClassID() == pi_rpPixelType->GetClassID() && pPixelTypeCapability->SupportsCodec(CodecID)))
            pPixelTypeCapability = 0;
        } 

    // Assert that we have found a pixel type capability, other wise it
    // means that this pixel type is not supported by the pi_rpResolutionCapabilities.
    HASSERT(pPixelTypeCapability != 0 && pPixelTypeCapability->CountCodecs() > 0);

    // Find the codec capability associate to the source pixel type and codec
    HFCPtr<HRFCodecCapability> pCodecCapability = pPixelTypeCapability->GetCodecCapability(CodecID);

    // Assert that we have found a codec capability, other wise it
    // means that this codec is not supported by the pi_rpResolutionCapabilities.
    HASSERT(pCodecCapability != 0);
    HASSERT(pCodecCapability->CountBlockType() > 0);

    // Get all the valid block type for the source pixel type and codec.
    HFCPtr<HRFRasterFileCapabilities> pBlockTypeCapabilities;
    pBlockTypeCapabilities = pCodecCapability->GetBlockTypeCapabilities();

    // Auto-detect the storage type if needed
    m_BlockType = pi_BlockType;
    if (pi_BlockType == HRFBlockType::AUTO_DETECT)
        {
        if ((pi_BlockWidth == pi_Width) && (pi_BlockHeight == pi_Height))
            {
            if (pBlockTypeCapabilities->GetCapabilityOfType(HRFImageCapability::CLASS_ID, pi_AccessMode) != 0)
                m_BlockType = HRFBlockType::IMAGE;
            else
                // TR #148512
                // if a format has only the TILE capability and the image has 256x256 pixels the method HASSERT.
                // This is caused by pi_ResolutionXRatio != 1.0. The condition is there since 1998 and I don't know why.
                // If the file has no STRIP and LINE capability, we don't have the choice to use TILE
                if (pBlockTypeCapabilities->GetCapabilityOfType(HRFTileCapability::CLASS_ID, pi_AccessMode) != 0 &&
                    (pi_ResolutionXRatio != 1.0 ||
                     (pBlockTypeCapabilities->GetCapabilityOfType(HRFStripCapability::CLASS_ID, pi_AccessMode) == 0 &&
                      pBlockTypeCapabilities->GetCapabilityOfType(HRFLineCapability::CLASS_ID, pi_AccessMode) == 0)))
                    m_BlockType = HRFBlockType::TILE;
                else if (pBlockTypeCapabilities->GetCapabilityOfType(HRFStripCapability::CLASS_ID, pi_AccessMode) != 0)
                    m_BlockType = HRFBlockType::STRIP;
                else
                    m_BlockType = HRFBlockType::LINE;
            }
        else if (pi_BlockWidth == pi_BlockHeight)
            m_BlockType = HRFBlockType::TILE;
        else if (pi_BlockWidth == pi_Width)
            {
            if (pi_BlockHeight == 1)
                {
                if (pBlockTypeCapabilities->GetCapabilityOfType(HRFLineCapability::CLASS_ID, pi_AccessMode) != 0)
                    m_BlockType = HRFBlockType::LINE;
                else
                    m_BlockType = HRFBlockType::STRIP;
                }
            else
                m_BlockType = HRFBlockType::STRIP;
            }
        else
            m_BlockType = HRFBlockType::TILE;
        }
    HASSERT(m_BlockType != HRFBlockType::AUTO_DETECT);


    // Validate with the specified ScanlineOrientation
    HFCPtr<HRFCapability> pScanlineCapability = new HRFScanlineOrientationCapability(pi_AccessMode, pi_ScanlineOrientation);
    HASSERT(m_pResolutionCapabilities->Supports(pScanlineCapability));

    // Validate with the specified InterleaveType
    HFCPtr<HRFCapability> pInterleaveCapability = new HRFInterleaveCapability(pi_AccessMode, pi_InterleaveType);
    HASSERT(m_pResolutionCapabilities->Supports(pInterleaveCapability));

    // AccessMode
    m_AccessMode            = pi_AccessMode;

    // Resolution information
    m_ResolutionXRatio      = pi_ResolutionXRatio;
    m_ResolutionYRatio      = pi_ResolutionYRatio;

    m_NumberOfPass          = pi_NumberOfPass;
    m_PaddingBits           = pi_PaddingBits;

    // Storage information
    m_pPixelType            = pi_rpPixelType;
    m_BitsPerPixel          = m_pPixelType->CountPixelRawDataBits();
    m_DownSamplingMethod    = pi_DownSamplingMethod;

    m_pCodec                = pi_rpCodec;
    m_ReaderBlockAccess     = pi_ReaderBlockAccess;
    m_WriterBlockAccess     = pi_WriterBlockAccess;
    m_ScanlineOrientation   = pi_ScanlineOrientation;
    m_InterleaveType        = pi_InterleaveType;
    m_IsInterlace           = pi_IsInterlace;

    // The resolution size
    m_Width                 = pi_Width;
    m_Height                = pi_Height;

    // The Block size
    m_BlockWidth            = MAX(1, pi_BlockWidth);
    m_BlockHeight           = MAX(1, pi_BlockHeight);

    // Calc the number of blocks per width and height
    m_BlocksPerWidth  = (m_Width + m_BlockWidth - 1) / m_BlockWidth;
    m_BlocksPerHeight = (m_Height + m_BlockHeight - 1) / m_BlockHeight;

    // Calc the block size in Bits and bytes
    m_BitsPerBlockWidth         = m_BlockWidth * m_BitsPerPixel;
    m_PaddingBitsPerBlockWidth  = (m_PaddingBits - (m_BitsPerBlockWidth % m_PaddingBits)) % m_PaddingBits;
    m_BytesPerBlockWidth        = (m_BitsPerBlockWidth + m_PaddingBitsPerBlockWidth) / 8;
    m_BlockSizeInBytes          = m_BytesPerBlockWidth * m_BlockHeight;

    // Calc the resolution size in Bits and bytes
    m_BitsPerWidth          = m_Width * m_BitsPerPixel;
    m_PaddingBitsPerWidth   = (m_PaddingBits - (uint32_t)(m_BitsPerWidth % m_PaddingBits)) % m_PaddingBits;
    m_BytesPerWidth         = (m_BitsPerWidth + m_PaddingBitsPerWidth) / 8;
    m_SizeInBytes           = m_BlockSizeInBytes * m_BlocksPerWidth * m_BlocksPerHeight;

    // Validate the block data flag.
    if ((m_pResolutionCapabilities->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, pi_AccessMode) != 0) &&
        (pi_pBlocksDataFlag))
        {
        HASSERT((m_BlocksPerWidth * m_BlocksPerHeight) <= ULONG_MAX);
        m_BlocksDataFlagSize = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag    = new HRFDataFlag[m_BlocksDataFlagSize];
        memcpy(m_pBlocksDataFlag, pi_pBlocksDataFlag, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
        }

    // Validation with the capabilities if it's possible to create a resolution
    // with the specified BlockWidth & BlockHeight
    if (m_BlockType == HRFBlockType::TILE)
        {
        HFCPtr<HRFRasterFileCapabilities> pTileCapabilities;
        pTileCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFTileCapability::CLASS_ID,
                                                                          pi_AccessMode);

        HASSERT(pTileCapabilities != 0 && pTileCapabilities->CountCapabilities() != 0);

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pTileCapabilities && (Index < pTileCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFTileCapability> p_CurrentTileCapability;

            p_CurrentTileCapability = (HFCPtr<HRFTileCapability>&)pTileCapabilities->GetCapability(Index);

            if (!((pi_BlockWidth >= p_CurrentTileCapability->GetMinWidth()) &&
                  (pi_BlockWidth <= p_CurrentTileCapability->GetMaxWidth())))
                IsValid = false;

            if (!(((p_CurrentTileCapability->GetWidthIncrement() > 0) &&
                   !(pi_BlockWidth % p_CurrentTileCapability->GetWidthIncrement())) ||
                  (p_CurrentTileCapability->GetWidthIncrement() == 0)))
                IsValid = false;

            if (!((pi_BlockHeight >= p_CurrentTileCapability->GetMinHeight()) &&
                  (pi_BlockHeight <= p_CurrentTileCapability->GetMaxHeight())))
                IsValid = false;

            if (!(((p_CurrentTileCapability->GetHeightIncrement() > 0) &&
                   !(pi_BlockHeight % p_CurrentTileCapability->GetHeightIncrement())) ||
                  (p_CurrentTileCapability->GetHeightIncrement() == 0)))
                IsValid = false;

            if (!(m_BlockSizeInBytes <= p_CurrentTileCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }
        HASSERT(FoundValidCap);
        }
    // with the specified StripHeight
    else if (m_BlockType == HRFBlockType::STRIP)
        {
        HFCPtr<HRFRasterFileCapabilities> pStripCapabilities;
        pStripCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFStripCapability::CLASS_ID,
                                                                           pi_AccessMode);

        HASSERT(pStripCapabilities != 0 && pStripCapabilities->CountCapabilities() != 0);

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pBlockTypeCapabilities && (Index < pStripCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFStripCapability> p_CurrentStripCapability;

            p_CurrentStripCapability = (HFCPtr<HRFStripCapability>&)pStripCapabilities->GetCapability(Index);

            // If more than one block
            if (m_Height > m_BlockHeight)
                {
                if (!((pi_BlockHeight >= p_CurrentStripCapability->GetMinHeight()) &&
                      (pi_BlockHeight <= p_CurrentStripCapability->GetMaxHeight())))
                    IsValid = false;

                // Test if the strip height is a multiple of the increment.
                if (!(((p_CurrentStripCapability->GetHeightIncrement() > 0) &&
                       !(pi_BlockHeight % p_CurrentStripCapability->GetHeightIncrement())) ||
                      (p_CurrentStripCapability->GetHeightIncrement() == 0)))
                    IsValid = false;
                }

            if (!(m_BlockSizeInBytes <= p_CurrentStripCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }
        HASSERT(FoundValidCap);

        }
    else if (m_BlockType == HRFBlockType::IMAGE)
        {
        HFCPtr<HRFRasterFileCapabilities> pImageCapabilities;
        pImageCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFImageCapability::CLASS_ID,
                                                                           pi_AccessMode);

        HASSERT(pImageCapabilities != 0 && pImageCapabilities->CountCapabilities() != 0);

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pImageCapabilities && (Index < pImageCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFImageCapability> p_CurrentImageCapability;

            p_CurrentImageCapability = (HFCPtr<HRFImageCapability>&)pImageCapabilities->GetCapability(Index);

            if (!((pi_BlockWidth >= p_CurrentImageCapability->GetMinWidth()) &&
                  (pi_BlockWidth <= p_CurrentImageCapability->GetMaxWidth())))
                IsValid = false;

            if (!((pi_BlockHeight >= p_CurrentImageCapability->GetMinHeight()) &&
                  (pi_BlockHeight <= p_CurrentImageCapability->GetMaxHeight())))
                IsValid = false;

            if (!(m_BlockSizeInBytes <= p_CurrentImageCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }
        HASSERT(FoundValidCap);
        }
    else if (m_BlockType == HRFBlockType::LINE)
        {
        HFCPtr<HRFRasterFileCapabilities> pLineCapabilities;
        pLineCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFLineCapability::CLASS_ID,
                                                                          pi_AccessMode);

        HASSERT(pLineCapabilities != 0 && pLineCapabilities->CountCapabilities() != 0);

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pLineCapabilities && (Index < pLineCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFLineCapability> p_CurrentLineCapability;

            p_CurrentLineCapability = (HFCPtr<HRFLineCapability>&)pLineCapabilities->GetCapability(Index);

            if (!(m_BlockSizeInBytes <= p_CurrentLineCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }
        }

    m_PaletteHasChanged             = false;
    m_BlockDataFlagHasChanged       = false;
    m_DownSamplingMethodHasChanged  = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
// Resolution Creation
//-----------------------------------------------------------------------------
HRFResolutionDescriptor::HRFResolutionDescriptor(const HRFResolutionDescriptor& pi_rObj)
    {
    m_pResolutionCapabilities = pi_rObj.m_pResolutionCapabilities;

    // Resolution information
    m_ResolutionXRatio      = pi_rObj.m_ResolutionXRatio;
    m_ResolutionYRatio      = pi_rObj.m_ResolutionYRatio;
    m_PaddingBits           = pi_rObj.m_PaddingBits;
    m_NumberOfPass          = pi_rObj.m_NumberOfPass;

    // Storage information
    m_pPixelType            = pi_rObj.m_pPixelType;
    m_BitsPerPixel          = pi_rObj.m_BitsPerPixel;
    m_pCodec                = pi_rObj.m_pCodec;
    m_BlockType             = pi_rObj.m_BlockType;
    m_DownSamplingMethod    = pi_rObj.m_DownSamplingMethod;
    m_ReaderBlockAccess     = pi_rObj.m_ReaderBlockAccess;
    m_WriterBlockAccess     = pi_rObj.m_WriterBlockAccess;
    m_ReaderBlockAccess     = pi_rObj.m_ReaderBlockAccess;
    m_WriterBlockAccess     = pi_rObj.m_WriterBlockAccess;

    m_ScanlineOrientation   = pi_rObj.m_ScanlineOrientation;
    m_InterleaveType        = pi_rObj.m_InterleaveType;
    m_IsInterlace           = pi_rObj.m_IsInterlace;

    // The resolution size
    m_Width                 = pi_rObj.m_Width;
    m_Height                = pi_rObj.m_Height;

    // Calc the resolution size in Bits and bytes
    m_BitsPerWidth              = pi_rObj.m_BitsPerWidth;
    m_PaddingBitsPerWidth       = pi_rObj.m_PaddingBitsPerWidth;
    m_BytesPerWidth             = pi_rObj.m_BytesPerWidth;
    m_SizeInBytes               = pi_rObj.m_SizeInBytes;

    // The Block size
    m_BlockWidth             = pi_rObj.m_BlockWidth;
    m_BlockHeight            = pi_rObj.m_BlockHeight;

    // Calc the block size in Bits and bytes
    m_BitsPerBlockWidth         = pi_rObj.m_BitsPerBlockWidth;
    m_PaddingBitsPerBlockWidth  = pi_rObj.m_PaddingBitsPerBlockWidth;
    m_BytesPerBlockWidth        = pi_rObj.m_BytesPerBlockWidth;
    m_BlockSizeInBytes          = pi_rObj.m_BlockSizeInBytes;

    // The Blocks count
    m_BlocksPerWidth         = pi_rObj.m_BlocksPerWidth;
    m_BlocksPerHeight        = pi_rObj.m_BlocksPerHeight;

    if (pi_rObj.m_pBlocksDataFlag)
        {
        HASSERT((m_BlocksPerWidth * m_BlocksPerHeight) <= ULONG_MAX);
        m_BlocksDataFlagSize  = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag     = new HRFDataFlag[m_BlocksDataFlagSize];
        memcpy(m_pBlocksDataFlag, pi_rObj.m_pBlocksDataFlag, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
        }

    m_PaletteHasChanged             = pi_rObj.m_PaletteHasChanged;
    m_BlockDataFlagHasChanged       = pi_rObj.m_BlockDataFlagHasChanged;
    m_DownSamplingMethodHasChanged  = pi_rObj.m_DownSamplingMethodHasChanged;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
// Resolution Destruction
//-----------------------------------------------------------------------------
HRFResolutionDescriptor::~HRFResolutionDescriptor()
    {
    }


//-----------------------------------------------------------------------------
// public
// Copy operator equal
// Resolution Creation
//-----------------------------------------------------------------------------
HRFResolutionDescriptor& HRFResolutionDescriptor::operator=(const HRFResolutionDescriptor& pi_rObj)
    {
    m_pResolutionCapabilities = pi_rObj.m_pResolutionCapabilities;

    // Resolution information
    m_ResolutionXRatio      = pi_rObj.m_ResolutionXRatio;
    m_ResolutionYRatio      = pi_rObj.m_ResolutionYRatio;
    m_PaddingBits           = pi_rObj.m_PaddingBits;
    m_NumberOfPass          = pi_rObj.m_NumberOfPass;

    // Storage information
    m_pPixelType            = pi_rObj.m_pPixelType;
    m_BitsPerPixel          = pi_rObj.m_BitsPerPixel;
    m_pCodec                = pi_rObj.m_pCodec;
    m_BlockType             = pi_rObj.m_BlockType;
    m_DownSamplingMethod    = pi_rObj.m_DownSamplingMethod;
    m_ReaderBlockAccess     = pi_rObj.m_ReaderBlockAccess;
    m_WriterBlockAccess     = pi_rObj.m_WriterBlockAccess;
    m_ScanlineOrientation   = pi_rObj.m_ScanlineOrientation;
    m_InterleaveType        = pi_rObj.m_InterleaveType;
    m_IsInterlace           = pi_rObj.m_IsInterlace;

    // The resolution size
    m_Width                 = pi_rObj.m_Width;
    m_Height                = pi_rObj.m_Height;

    // Calc the resolution size in Bits and bytes
    m_BitsPerWidth              = pi_rObj.m_BitsPerWidth;
    m_PaddingBitsPerWidth       = pi_rObj.m_PaddingBitsPerWidth;
    m_BytesPerWidth             = pi_rObj.m_BytesPerWidth;
    m_SizeInBytes               = pi_rObj.m_SizeInBytes;

    // The Block size
    m_BlockWidth             = pi_rObj.m_BlockWidth;
    m_BlockHeight            = pi_rObj.m_BlockHeight;

    // Calc the block size in Bits and bytes
    m_BitsPerBlockWidth         = pi_rObj.m_BitsPerBlockWidth;
    m_PaddingBitsPerBlockWidth  = pi_rObj.m_PaddingBitsPerBlockWidth;
    m_BytesPerBlockWidth        = pi_rObj.m_BytesPerBlockWidth;
    m_BlockSizeInBytes          = pi_rObj.m_BlockSizeInBytes;

    // The Blocks count
    m_BlocksPerWidth         = pi_rObj.m_BlocksPerWidth;
    m_BlocksPerHeight        = pi_rObj.m_BlocksPerHeight;

    if (pi_rObj.m_pBlocksDataFlag)
        {
        HASSERT((m_BlocksPerWidth * m_BlocksPerHeight) <= ULONG_MAX);
        m_BlocksDataFlagSize  = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag     = new HRFDataFlag[m_BlocksDataFlagSize];
        memcpy(m_pBlocksDataFlag, pi_rObj.m_pBlocksDataFlag, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
        }

    m_PaletteHasChanged             = pi_rObj.m_PaletteHasChanged;
    m_BlockDataFlagHasChanged       = pi_rObj.m_BlockDataFlagHasChanged;
    m_DownSamplingMethodHasChanged  = pi_rObj.m_DownSamplingMethodHasChanged;

    return *this;
    }


bool HRFResolutionDescriptor::CanCreateWith(HFCAccessMode                            pi_AccessMode,
                                             const HFCPtr<HRFRasterFileCapabilities>& pi_rpResolutionCapabilities) const
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpResolutionCapabilities != 0);

    // Validation with the capabilities if it's possible to create a resolution
    HFCPtr<HRFRasterFileCapabilities> pPixelTypeCapabilities = pi_rpResolutionCapabilities->
                                                               GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                                                                       pi_AccessMode);

    if (pPixelTypeCapabilities == 0 || pPixelTypeCapabilities->CountCapabilities() == 0)
        return false;

    HCLASS_ID CodecID = (m_pCodec == NULL) ? HCDCodecIdentity::CLASS_ID : m_pCodec->GetClassID();

    HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
    for (uint32_t i = 0; i < pPixelTypeCapabilities->CountCapabilities() && pPixelTypeCapability == 0; i++)
        {
        pPixelTypeCapability = (const HFCPtr<HRFPixelTypeCapability>&)pPixelTypeCapabilities->GetCapability(i);

        if (!(pPixelTypeCapability->GetPixelTypeClassID() == m_pPixelType->GetClassID() && pPixelTypeCapability->SupportsCodec(CodecID)))
            pPixelTypeCapability = 0;
        }

    // Assert that we have found a pixel type capability, other wise it
    // means that this pixel type is not supported by the pi_rpResolutionCapabilities.
    if (pPixelTypeCapability == 0 || pPixelTypeCapability->CountCodecs() == 0)
        return false;

    // Find the codec capability associate to the source pixel type and codec
    HFCPtr<HRFCodecCapability> pCodecCapability = pPixelTypeCapability->GetCodecCapability(CodecID);

    // Assert that we have found a codec capability, other wise it
    // means that this codec is not supported by the pi_rpResolutionCapabilities.
    if (pCodecCapability == 0 || pCodecCapability->CountBlockType() == 0)
        return false;

    // Get all the valid block type for the source pixel type and codec.
    HFCPtr<HRFRasterFileCapabilities> pBlockTypeCapabilities;
    pBlockTypeCapabilities = pCodecCapability->GetBlockTypeCapabilities();


    // Validate with the specified ScanlineOrientation
    HFCPtr<HRFCapability> pScanlineCapability = new HRFScanlineOrientationCapability(pi_AccessMode, m_ScanlineOrientation);
    if (!pi_rpResolutionCapabilities->Supports(pScanlineCapability))
        return false;

    // Validate with the specified InterleaveType
    HFCPtr<HRFCapability> pInterleaveCapability = new HRFInterleaveCapability(pi_AccessMode, m_InterleaveType);

    if (!pi_rpResolutionCapabilities->Supports(pInterleaveCapability))
        return false;

    // Validate the block data flag.
    if ((pi_rpResolutionCapabilities->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID, pi_AccessMode) == 0) &&
        (m_pBlocksDataFlag != 0))
        return false;

    // Validation with the capabilities if it's possible to create a resolution
    // with the specified BlockWidth & BlockHeight
    if (m_BlockType == HRFBlockType::TILE)
        {
        HFCPtr<HRFRasterFileCapabilities> pTileCapabilities;
        pTileCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFTileCapability::CLASS_ID,
                                                                          pi_AccessMode);

        if (pTileCapabilities == 0 || pTileCapabilities->CountCapabilities() == 0)
            return false;

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pTileCapabilities && (Index < pTileCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFTileCapability> p_CurrentTileCapability;

            p_CurrentTileCapability = (HFCPtr<HRFTileCapability>&)pTileCapabilities->GetCapability(Index);

            if (!((m_BlockWidth >= p_CurrentTileCapability->GetMinWidth()) &&
                  (m_BlockWidth <= p_CurrentTileCapability->GetMaxWidth())))
                IsValid = false;

            if (!(((p_CurrentTileCapability->GetWidthIncrement() > 0) &&
                   !(m_BlockWidth % p_CurrentTileCapability->GetWidthIncrement())) ||
                  (p_CurrentTileCapability->GetWidthIncrement() == 0)))
                IsValid = false;

            if (!((m_BlockHeight >= p_CurrentTileCapability->GetMinHeight()) &&
                  (m_BlockHeight <= p_CurrentTileCapability->GetMaxHeight())))
                IsValid = false;

            if (!(((p_CurrentTileCapability->GetHeightIncrement() > 0) &&
                   !(m_BlockHeight % p_CurrentTileCapability->GetHeightIncrement())) ||
                  (p_CurrentTileCapability->GetHeightIncrement() == 0)))
                IsValid = false;

            if (!(m_BlockSizeInBytes <= p_CurrentTileCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }

        if (!FoundValidCap)
            return false;
        }
    // with the specified StripHeight
    else if (m_BlockType == HRFBlockType::STRIP)
        {
        HFCPtr<HRFRasterFileCapabilities> pStripCapabilities;
        pStripCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFStripCapability::CLASS_ID,
                                                                           pi_AccessMode);

        if (pStripCapabilities == 0 || pStripCapabilities->CountCapabilities() == 0)
            return false;

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pBlockTypeCapabilities && (Index < pStripCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFStripCapability> p_CurrentStripCapability;

            p_CurrentStripCapability = (HFCPtr<HRFStripCapability>&)pStripCapabilities->GetCapability(Index);

            if (!((m_BlockHeight >= p_CurrentStripCapability->GetMinHeight()) &&
                  (m_BlockHeight <= p_CurrentStripCapability->GetMaxHeight())))
                IsValid = false;

            if (!(((p_CurrentStripCapability->GetHeightIncrement() > 0) &&
                   !(m_BlockHeight % p_CurrentStripCapability->GetHeightIncrement())) ||
                  (p_CurrentStripCapability->GetHeightIncrement() == 0)))
                IsValid = false;

            if (!(m_BlockSizeInBytes <= p_CurrentStripCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }
        if (!FoundValidCap)
            return false;

        }
    else if (m_BlockType == HRFBlockType::IMAGE)
        {
        HFCPtr<HRFRasterFileCapabilities> pImageCapabilities;
        pImageCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFImageCapability::CLASS_ID,
                                                                           pi_AccessMode);

        if (pImageCapabilities == 0 || pImageCapabilities->CountCapabilities() == 0)
            return false;

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pImageCapabilities && (Index < pImageCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFImageCapability> p_CurrentImageCapability;

            p_CurrentImageCapability = (HFCPtr<HRFImageCapability>&)pImageCapabilities->GetCapability(Index);

            if (!((m_BlockWidth >= p_CurrentImageCapability->GetMinWidth()) &&
                  (m_BlockWidth <= p_CurrentImageCapability->GetMaxWidth())))
                IsValid = false;

            if (!((m_BlockHeight >= p_CurrentImageCapability->GetMinHeight()) &&
                  (m_BlockHeight <= p_CurrentImageCapability->GetMaxHeight())))
                IsValid = false;

            if (!(m_BlockSizeInBytes <= p_CurrentImageCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }

        if (!FoundValidCap)
            return false;
        }
    else if (m_BlockType == HRFBlockType::LINE)
        {
        HFCPtr<HRFRasterFileCapabilities> pLineCapabilities;
        pLineCapabilities = pBlockTypeCapabilities->GetCapabilitiesOfType(HRFLineCapability::CLASS_ID,
                                                                          pi_AccessMode);

        if (pLineCapabilities == 0 || pLineCapabilities->CountCapabilities() == 0)
            return false;

        bool FoundValidCap = false;

        for (uint32_t Index = 0; pLineCapabilities && (Index < pLineCapabilities->CountCapabilities()) && !FoundValidCap; Index++)
            {
            bool IsValid = true;
            HFCPtr<HRFLineCapability> p_CurrentLineCapability;

            p_CurrentLineCapability = (HFCPtr<HRFLineCapability>&)pLineCapabilities->GetCapability(Index);

            if (!(m_BlockSizeInBytes <= p_CurrentLineCapability->GetMaxSizeInBytes()))
                IsValid = false;

            if (IsValid)
                FoundValidCap = true;
            }

        if (!FoundValidCap)
            return false;
        }
    return true;
    }

//-----------------------------------------------------------------------------
// public
// SetPixelType
// Pixel information
//-----------------------------------------------------------------------------
void HRFResolutionDescriptor::SetPixelType(HFCPtr<HRPPixelType>& pi_rpPixelType)
    {
    HPRECONDITION(pi_rpPixelType != 0);

    // Validation with the capabilities if it's possible to create a resolution
    HFCPtr<HRFRasterFileCapabilities> pPixelTypeCapabilities = m_pResolutionCapabilities->
                                                               GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                                                                       GetAccessMode());

    HASSERT(pPixelTypeCapabilities != 0 || pPixelTypeCapabilities->CountCapabilities() > 0);

    HCLASS_ID CodecID = (m_pCodec == NULL) ? HCDCodecIdentity::CLASS_ID : m_pCodec->GetClassID();
    
    HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
    for (uint32_t i = 0; i < pPixelTypeCapabilities->CountCapabilities() && pPixelTypeCapability == 0; i++)
        {
        pPixelTypeCapability = (const HFCPtr<HRFPixelTypeCapability>&)pPixelTypeCapabilities->GetCapability(i);

        if (!(pPixelTypeCapability->GetPixelTypeClassID() == pi_rpPixelType->GetClassID() && pPixelTypeCapability->SupportsCodec(CodecID)))
            pPixelTypeCapability = 0;
        }

    // Assert that we have found a pixel type capability, other wise it
    // means that this pixel type is not supported by the pi_rpResolutionCapabilities.
    HASSERT(pPixelTypeCapability != 0 && pPixelTypeCapability->CountCodecs() > 0);

    m_pPixelType   = pi_rpPixelType;
    m_BitsPerPixel = m_pPixelType->CountPixelRawDataBits();
    }


//-----------------------------------------------------------------------------
// Public
// HasBlocksDataFlag
// Blocks information
//-----------------------------------------------------------------------------
bool HRFResolutionDescriptor::HasBlocksDataFlag() const
    {
    bool Result = false;

    // Look if we have a BlocksDataFlag
    if (m_pBlocksDataFlag.get() != 0)
        Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// public
// GetBlocksDataFlag
// Blocks information
//-----------------------------------------------------------------------------
const HRFDataFlag* HRFResolutionDescriptor::GetBlocksDataFlag() const
    {
    HPRECONDITION(HasBlocksDataFlag());
    return m_pBlocksDataFlag;
    }

//-----------------------------------------------------------------------------
// Public
// SetBlocksDataFlag
// Blocks information
//-----------------------------------------------------------------------------
bool HRFResolutionDescriptor::SetBlocksDataFlag(const HRFDataFlag* pi_pBlocksDataFlag)
    {
    // Validation with the capabilities if it's possible to set a BlocksDataFlag
    HPRECONDITION(m_pResolutionCapabilities->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID,HFC_WRITE_ONLY) != 0);

    // If we have no flag we create the flags buffer
    if (!HasBlocksDataFlag())
        {
        HASSERT(m_BlocksPerWidth* m_BlocksPerHeight <= ULONG_MAX);
        m_BlocksDataFlagSize = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag    = new HRFDataFlag[m_BlocksDataFlagSize];
        }

    // Replace the BlocksDataFlag by the specified
    memcpy(m_pBlocksDataFlag, pi_pBlocksDataFlag, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
    m_BlockDataFlagHasChanged = true;

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetBlockDataFlag
// Blocks information
//-----------------------------------------------------------------------------
HRFDataFlag HRFResolutionDescriptor::GetBlockDataFlag(uint64_t pi_Index) const
    {
    HPRECONDITION(HasBlocksDataFlag());
    HPRECONDITION(pi_Index < (m_BlocksPerWidth * m_BlocksPerHeight));
    return m_pBlocksDataFlag[pi_Index];
    }

//-----------------------------------------------------------------------------
// Public
// SetBlockDataFlag
// Blocks information
//-----------------------------------------------------------------------------
void HRFResolutionDescriptor::SetBlockDataFlag(uint64_t pi_Index, HRFDataFlag pi_DataFlag)
    {
    // Validation with the capabilities if it's possible to set a BlocksDataFlag
    HPRECONDITION(m_pResolutionCapabilities->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID,HFC_WRITE_ONLY) != 0);
    HPRECONDITION(pi_Index < (m_BlocksPerWidth * m_BlocksPerHeight));

    // If we have no flag we create the flags buffer
    if (!HasBlocksDataFlag())
        {
        HASSERT(m_BlocksPerWidth* m_BlocksPerHeight <= ULONG_MAX);
        m_BlocksDataFlagSize = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag    = new HRFDataFlag[m_BlocksDataFlagSize];
        memset(m_pBlocksDataFlag, HRFDATAFLAG_EMPTY, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
        }

    // Remove these flags, if sets one of them, because they are exlusif
    if (pi_DataFlag & (HRFDATAFLAG_EMPTY | HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN))
        m_pBlocksDataFlag[pi_Index] &= (0xff - (HRFDATAFLAG_EMPTY | HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN));

    m_pBlocksDataFlag[pi_Index] |= pi_DataFlag;

    m_BlockDataFlagHasChanged = true;
    }

//-----------------------------------------------------------------------------
// Public
// CLearBlockDataFlag
// Blocks information
//-----------------------------------------------------------------------------
void HRFResolutionDescriptor::ClearBlockDataFlag(uint64_t pi_Index, HRFDataFlag pi_DataFlag)
    {
    // Validation with the capabilities if it's possible to set a BlocksDataFlag
    HPRECONDITION(m_pResolutionCapabilities->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID,HFC_WRITE_ONLY) != 0);
    HPRECONDITION(pi_Index < (m_BlocksPerWidth * m_BlocksPerHeight));
    // If we have no flag we create the flags buffer
    if (!HasBlocksDataFlag())
        {
        HASSERT(m_BlocksPerWidth* m_BlocksPerHeight <= ULONG_MAX);
        m_BlocksDataFlagSize = (uint32_t)(m_BlocksPerWidth * m_BlocksPerHeight);
        m_pBlocksDataFlag    = new HRFDataFlag[m_BlocksDataFlagSize];
        memset(m_pBlocksDataFlag, HRFDATAFLAG_EMPTY, m_BlocksDataFlagSize * sizeof(HRFDataFlag));
        }
    m_pBlocksDataFlag[pi_Index] &= (0xff - pi_DataFlag);
    }
//-----------------------------------------------------------------------------
// Public
// BlockDataFlagHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
double HRFResolutionDescriptor::RoundResolutionRatio(uint32_t pi_MainSize, uint32_t pi_SubResSize)
    {
    double ResCurrent = (double)pi_SubResSize / (double)pi_MainSize;

    int32_t Exposent = round(log(1.0 / ResCurrent) / log(2.0));
    double Val2Exposent = pow(2.0, Exposent);
    double ResSize = pi_MainSize / Val2Exposent;

    if (fabs(ResSize - (double)pi_SubResSize) < 1.0 && (double)pi_SubResSize > ResSize)
        ResCurrent = 1.0 / Val2Exposent;

    return ResCurrent;
    }
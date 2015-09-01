//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFResolutionDescriptor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRFResolutionDescriptor
//-----------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
/**
 * Public <br>
 * GetAccessMode <br><br>
 *
 * This methode is used to get the access mode set in the resolution descriptor.
 * This access mode is used for validation on the raster file capabilities.
 *
 * @return The access mode for the current ResolutionDescriptor.
 *
 */
inline HFCAccessMode HRFResolutionDescriptor::GetAccessMode() const
    {
    return m_AccessMode;
    }

/**
 * Public
 * GetResolutionXRatio
 *
 * This methods return the X resolution factors. A resolution factor is 1.0 for
 * the best resolution of a page. Worst resolutions have a higher resolution factor.
 * Most multi-resolution pages have resolution factors equal to the power of
 * 2(2, 4, 8, 16 ...). Most multi-resolution pages have X and Y resolution factors
 * equal depending on the capacities of the file format.
 *
 * @return The X resolution ratio.
 *
 */
inline double HRFResolutionDescriptor::GetResolutionXRatio() const
    {
    return m_ResolutionXRatio;
    }

/**
 * Public
 * GetResolutionYRatio
 *
 * This methods return the Y resolution factors. A resolution factor is 1.0 for
 * the best resolution of a page. Worst resolutions have a higher resolution factor.
 * Most multi-resolution pages have resolution factors equal to the power of
 * 2(2, 4, 8, 16 ...). Most multi-resolution pages have X and Y resolution factors
 * equal depending on the capacities of the file format.
 *
 * @return The Y resolution ratio.
 *
 */
inline double HRFResolutionDescriptor::GetResolutionYRatio() const
    {
    return m_ResolutionYRatio;
    }

//-----------------------------------------------------------------------------
// public
// Get number of pass to complete a resolution data - prossive format        = n pass
//                                                  - non-progressive format = 1 pass
//-----------------------------------------------------------------------------
inline Byte HRFResolutionDescriptor::GetNumberOfPass () const
    {
    return m_NumberOfPass;
    }

//-----------------------------------------------------------------------------
// public
// Get padding bits
//-----------------------------------------------------------------------------
inline unsigned short HRFResolutionDescriptor::GetPaddingBits () const
    {
    return m_PaddingBits;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
// Pixel information
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelType>& HRFResolutionDescriptor::GetPixelType() const
    {
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// Public
// GetBitsPerPixel
// Pixel information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBitsPerPixel() const
    {
    return m_BitsPerPixel;
    }

//-----------------------------------------------------------------------------
// public
// GetCodecs
// Storage information
//-----------------------------------------------------------------------------
inline const HFCPtr<HCDCodec>& HRFResolutionDescriptor::GetCodec() const
    {
    return m_pCodec;
    }

//-----------------------------------------------------------------------------
// public
// GetBlockType
// Storage information
//-----------------------------------------------------------------------------
inline HRFBlockType HRFResolutionDescriptor::GetBlockType() const
    {
    return m_BlockType;
    }

//-----------------------------------------------------------------------------
// public
// GetDownSamplingMethod
// Storage information
//-----------------------------------------------------------------------------
inline HRFDownSamplingMethod HRFResolutionDescriptor::GetDownSamplingMethod() const
    {
    return m_DownSamplingMethod;
    }

//-----------------------------------------------------------------------------
// public
// SetDownSamplingMethod
// Storage information
//
// Note : When we call this method, we must sure that the resolution data was
//        generated with this resampling method. Normally, this method must not
//        be called directly...
//-----------------------------------------------------------------------------
inline void HRFResolutionDescriptor::SetDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {

    m_DownSamplingMethod = pi_DownSamplingMethod;
    m_DownSamplingMethodHasChanged = true;
    }

//-----------------------------------------------------------------------------
// public
// DownSamplingMethodHasChanged
// Storage information
//-----------------------------------------------------------------------------
inline bool HRFResolutionDescriptor::DownSamplingMethodHasChanged() const
    {
    return m_DownSamplingMethodHasChanged;
    }

//-----------------------------------------------------------------------------
// public
// GetReaderBlockAccess
// Storage information
//-----------------------------------------------------------------------------
inline HRFBlockAccess HRFResolutionDescriptor::GetReaderBlockAccess() const
    {
    return m_ReaderBlockAccess;
    }

//-----------------------------------------------------------------------------
// public
// GetWriterBlockAccess
// Storage information
//-----------------------------------------------------------------------------
inline HRFBlockAccess HRFResolutionDescriptor::GetWriterBlockAccess() const
    {
    return m_WriterBlockAccess;
    }

//-----------------------------------------------------------------------------
// public
// GetScanlineOrientation
// Storage information
//-----------------------------------------------------------------------------
inline  HRFScanlineOrientation HRFResolutionDescriptor::GetScanlineOrientation() const
    {
    return m_ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// public
// GetInterleave
// Storage information
//-----------------------------------------------------------------------------
inline HRFInterleaveType HRFResolutionDescriptor::GetInterleaveType() const
    {
    return m_InterleaveType;
    }

//-----------------------------------------------------------------------------
// public
// IsInterlace
// Storage information
//-----------------------------------------------------------------------------
inline bool HRFResolutionDescriptor::IsInterlace() const
    {
    return m_IsInterlace;
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
// Resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetWidth() const
    {
    return m_Width;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
// Resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetHeight() const
    {
    return m_Height;
    }

//-----------------------------------------------------------------------------
// public
// GetBitsPerWidth - The padding is exclude
// Resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetBitsPerWidth() const
    {
    return m_BitsPerWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetPaddingBitsPerWidth
// Resolution information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetPaddingBitsPerWidth() const
    {
    return m_PaddingBitsPerWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetBytesPerWidth - The padding is include
// Resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetBytesPerWidth() const
    {
    return m_BytesPerWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetSizeInBytes - The padding is include
// Resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetSizeInBytes() const
    {
    return m_SizeInBytes;
    }

//-----------------------------------------------------------------------------
// public
// GetBlockWidth
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBlockWidth() const
    {
    return m_BlockWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetBlockHeight
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBlockHeight() const
    {
    return m_BlockHeight;
    }

//-----------------------------------------------------------------------------
// public
// GetBitsPerBlockWidth - The padding is exclude
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBitsPerBlockWidth() const
    {
    return m_BitsPerBlockWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetPaddingBitsPerBlockWidth
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetPaddingBitsPerBlockWidth() const
    {
    return m_PaddingBitsPerBlockWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetBytesPerBlockWidth - The padding is include
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBytesPerBlockWidth() const
    {
    return m_BytesPerBlockWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetBlockSizeInBytes - The padding is include
// Block information
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionDescriptor::GetBlockSizeInBytes() const
    {
    return m_BlockSizeInBytes;
    }

//-----------------------------------------------------------------------------
// public
// GetBlocksPerWidth
// Block count
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetBlocksPerWidth() const
    {
    return m_BlocksPerWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetBlocksPerHeight
// Block count
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::GetBlocksPerHeight() const
    {
    return m_BlocksPerHeight;
    }

//-----------------------------------------------------------------------------
// public
// CountBlocks
// count the number of Blocks
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::CountBlocks() const
    {
    return m_BlocksPerWidth * m_BlocksPerHeight;
    }

//-----------------------------------------------------------------------------
// public
// ComputeBlockIndex
// Compute Block Index from position
//-----------------------------------------------------------------------------
inline uint64_t HRFResolutionDescriptor::ComputeBlockIndex(uint64_t pi_TilePosX,
                                                          uint64_t pi_TilePosY) const
    {
    return ((GetBlocksPerWidth() * (pi_TilePosY / GetBlockHeight())) +
            (pi_TilePosX / GetBlockWidth()));
    }

//-----------------------------------------------------------------------------
// public
// ComputeBlockPosition
// Compute Block Position from index
//-----------------------------------------------------------------------------
inline void HRFResolutionDescriptor::ComputeBlockPosition(uint64_t pi_Index,
                                                          uint64_t*  po_pTilePosX,
                                                          uint64_t*  po_pTilePosY) const
    {
    *po_pTilePosX = (uint32_t)(pi_Index % GetBlocksPerWidth()) * GetBlockWidth();
    *po_pTilePosY = (uint32_t)(pi_Index / GetBlocksPerWidth()) * GetBlockHeight();
    }

//-----------------------------------------------------------------------------
// Public
// PaletteHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFResolutionDescriptor::PaletteHasChanged      () const
    {
    return m_PaletteHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// BlockDataFlagHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFResolutionDescriptor::BlockDataFlagHasChanged() const
    {
    return m_BlockDataFlagHasChanged;
    }

//-----------------------------------------------------------------------------
// Private Access only by the Resolution Editor ancestor
// SetBlocksDataFlag
// Blocks information
//-----------------------------------------------------------------------------
inline void HRFResolutionDescriptor::PaletteChanged()
    {
    m_PaletteHasChanged = true;
    }

//-----------------------------------------------------------------------------
// Public
// Saved
// Resets flags when ResolutionDescriptor was saved
//-----------------------------------------------------------------------------
inline void HRFResolutionDescriptor::Saved()
    {
    m_PaletteHasChanged = false;
    m_BlockDataFlagHasChanged = false;
    }


//-----------------------------------------------------------------------------
// Public
// Saved
// Resets flags when DataFlag was saved
//-----------------------------------------------------------------------------
inline void HRFResolutionDescriptor::BlockDataFlagSaved()
    {
    m_BlockDataFlagHasChanged = false;
    }
END_IMAGEPP_NAMESPACE

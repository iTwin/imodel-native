//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelPalette.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRPPixelPalette
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Return the number of entries that can be stored in the palette.
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelPalette::GetMaxEntries() const
    {
    return m_MaxPaletteSize;
    }

//-----------------------------------------------------------------------------
// Return the number of entries present in the palette.
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelPalette::CountUsedEntries() const
    {
    return m_CountBufPalette;
    }

//-----------------------------------------------------------------------------
// Return the number of byte to save an entry.
//-----------------------------------------------------------------------------
inline uint32_t HRPPixelPalette::GetPixelEntrySize() const
    {
    return m_PixelEntrySize;
    }


//-----------------------------------------------------------------------------
// Returns a constant reference to the channel organisation own by this pixel
// palette
//-----------------------------------------------------------------------------
inline const HRPChannelOrg& HRPPixelPalette::GetChannelOrg() const
    {
    return(m_ChannelOrg);
    }

//-----------------------------------------------------------------------------
// Returns a pointer to an object containing a composite value stored at
// specified entry in the palette.
//-----------------------------------------------------------------------------
inline const void* HRPPixelPalette::GetCompositeValue(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_CountBufPalette);

    return &(m_BufPalette.pData[pi_Index*m_PixelEntrySize]);
    }

//-----------------------------------------------------------------------------
// LockEntry
//-----------------------------------------------------------------------------
inline void HRPPixelPalette::LockEntry(int32_t pi_Index)
    {
    HPRECONDITION(pi_Index < (int32_t)m_MaxPaletteSize);

    m_LockedEntryIndex = pi_Index;
    }

//-----------------------------------------------------------------------------
// GetLockedEntryIndex
//-----------------------------------------------------------------------------
inline int32_t HRPPixelPalette::GetLockedEntryIndex() const
    {
    return m_LockedEntryIndex;
    }

//-----------------------------------------------------------------------------
// GetLockedEntryIndex
//-----------------------------------------------------------------------------
inline void HRPPixelPalette::SetReadOnly(bool pi_ReadOnly)
    {
    m_ReadOnly = pi_ReadOnly;
    }

//-----------------------------------------------------------------------------
// GetLockedEntryIndex
//-----------------------------------------------------------------------------
inline bool HRPPixelPalette::IsReadOnly() const
    {
    return m_ReadOnly;
    }

//-----------------------------------------------------------------------------
// GetPalette
//-----------------------------------------------------------------------------
inline void HRPPixelPalette::GetPalette(Byte* po_pPalette) const
    {
    HPRECONDITION(po_pPalette != 0);

    memcpy(po_pPalette, m_BufPalette.pData, m_BufPalette.BufSize);
    }

END_IMAGEPP_NAMESPACE

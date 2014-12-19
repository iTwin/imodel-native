//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelPalette.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPPixelPalette
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelOrg.h"

class HRPPixelPalette : public HFCShareableObject<HRPPixelPalette>
    {
    HDECLARE_BASECLASS_ID(1005)

public:

    // Primary methods
    _HDLLg                 HRPPixelPalette ();
    _HDLLg                 HRPPixelPalette(uint32_t pi_MaxEntries,
                                           const HRPChannelOrg& pi_rChannelOrg);
    _HDLLg                 HRPPixelPalette(const HRPPixelPalette& pi_rObj);
    _HDLLg virtual         ~HRPPixelPalette();
    HRPPixelPalette&   operator=(const HRPPixelPalette& pi_rObj);

    // Comparison methods
    bool           operator==(const HRPPixelPalette& pi_rObj) const;
    bool           operator!=(const HRPPixelPalette& pi_rObj) const;

    // Palette management methods
    void            SetReadOnly(bool pi_ReadOnly);
    bool           IsReadOnly() const;
    uint32_t        GetMaxEntries() const;
    uint32_t        CountUsedEntries() const;
    _HDLLg uint32_t AddEntry(const void* pi_pValue);

    const HRPChannelOrg&
    GetChannelOrg() const;

    const void*     GetCompositeValue(uint32_t pi_EntryIndex) const;
    _HDLLg bool    SetCompositeValue(uint32_t pi_EntryIndex,
                                      const void* pi_pValue);

    // Utilities
    int32_t        FindCompositeValue(const void* pi_pCompositeValue,
                                       uint32_t ChannelsNotUsedMask = 0x00000000) const;

    void            LockEntry(int32_t pi_Index);
    int32_t        GetLockedEntryIndex() const;

    uint32_t        GetPixelEntrySize() const;

    void            GetPalette(Byte* po_ppPalette) const;
    void            SetPalette(const Byte* pi_pPalette, uint32_t pi_EntriesUsed);

protected:

private:


    // Attributes
    
    // The palette of pixel values.  This palette can exist and be
    // used even if the pixels are values but its main purpose is to
    // store values when pixels are indices.
    struct PaletteBuffer
        {
        PaletteBuffer() : pData(0), BufSize(0) {}
        Byte*   pData;
        size_t  BufSize;
        } m_BufPalette;
    uint32_t                    m_MaxPaletteBufferSize;
    uint32_t                    m_CountBufPalette;      // Number if entrie used.


    // Maximum size of the palette.
    // If pixels are indices (m_PixelAreIndices==true), this number
    // cannot exceed the maximum value allowed by the number of
    // bits stored in m_PixelDataBits.
    uint32_t                    m_MaxPaletteSize;

    // Channel organisation
    // This gives the size of the composite value as well as
    // the mean to access individual channel values
    HRPChannelOrg               m_ChannelOrg;


    // a default entry index (to be use for background color or other)
    int32_t                    m_LockedEntryIndex;

    // Info, not persistent
    // Number of byte to save an entry.
    uint32_t                    m_PixelEntrySize;

    bool                       m_ReadOnly;

    // Methods
    void           DeepDelete();
    void           DeepCopy(const HRPPixelPalette& pi_rObj);
    };

#include "HRPPixelPalette.hpp"

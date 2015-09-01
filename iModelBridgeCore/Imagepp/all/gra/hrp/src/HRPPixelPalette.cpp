//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelPalette.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelPalette
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelPalette.h>


// Mask used to store the last byte of a palette entry
static const Byte s_Mask[8] = {0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};

//-----------------------------------------------------------------------------
// Default Constructor.
//-----------------------------------------------------------------------------
HRPPixelPalette::HRPPixelPalette ()
    : m_ChannelOrg()
    {
    m_MaxPaletteSize = 0;
    m_CountBufPalette= 0;

    // no default entry index by default
    m_LockedEntryIndex = -1;

    m_PixelEntrySize = 0;
    m_ReadOnly = false;
    m_MaxPaletteBufferSize = 0;
    }


//-----------------------------------------------------------------------------
// Constructor.  It takes no argument, object properties must be set with
// channel and palette setup methods.
//-----------------------------------------------------------------------------
HRPPixelPalette::HRPPixelPalette(uint32_t             pi_MaxEntries,
                                 const HRPChannelOrg& pi_rChannelOrg)
    : m_ChannelOrg(pi_rChannelOrg)
    {
    // If the maximum number of entries is 256 or more, we initially create
    // the palette with 256 entries.  It will be resized appropriatly on each
    // AddEntry following the 256th AddEntry.

    m_MaxPaletteSize = pi_MaxEntries;

    // no default entry index by default
    m_LockedEntryIndex = -1;

    // Entry size
    m_PixelEntrySize = m_ChannelOrg.CountPixelCompositeValueBits() / 8 +
                       ((m_ChannelOrg.CountPixelCompositeValueBits() % 8) ? 1 : 0);

    m_MaxPaletteBufferSize = m_PixelEntrySize * m_MaxPaletteSize;

    // No entry
    m_CountBufPalette   = 0;
    m_ReadOnly = false;
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRPPixelPalette::HRPPixelPalette(const HRPPixelPalette& pi_rObj)
    : m_ChannelOrg(pi_rObj.m_ChannelOrg)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRPPixelPalette::~HRPPixelPalette()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// Deletes everything owned by the object
//-----------------------------------------------------------------------------
void HRPPixelPalette::DeepDelete()
    {
    m_CountBufPalette    = 0;
    m_BufPalette.BufSize = 0;
    delete[] m_BufPalette.pData;
    m_BufPalette.pData   = 0;
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another pixel type, doing a deep copy.
//-----------------------------------------------------------------------------
HRPPixelPalette& HRPPixelPalette::operator=(const HRPPixelPalette& pi_rObj)
    {
    HPRECONDITION(!m_ReadOnly);

    if (this != &pi_rObj)
        {

        DeepDelete();

        m_ChannelOrg    = pi_rObj.m_ChannelOrg;

        DeepCopy(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Copies everything owned by the object
//-----------------------------------------------------------------------------
void HRPPixelPalette::DeepCopy(const HRPPixelPalette& pi_rObj)
    {
    m_MaxPaletteSize        = pi_rObj.m_MaxPaletteSize;
    m_LockedEntryIndex      = pi_rObj.m_LockedEntryIndex;
    m_PixelEntrySize        = pi_rObj.m_PixelEntrySize;
    m_MaxPaletteBufferSize  = pi_rObj.m_MaxPaletteBufferSize;

    m_CountBufPalette       = pi_rObj.m_CountBufPalette;
    m_ReadOnly              = pi_rObj.m_ReadOnly;

    // Copy the content of the palette
    // Doing this instead of operator= on the list ensures that
    // each palette entry is copied (not only the pointers
    // to the palette entries)

    // Allocated Buf if not already done.
    if (pi_rObj.m_BufPalette.pData != 0)
        {
        // we assume that the buffer has been deleted previously if there was one
        m_BufPalette.BufSize = pi_rObj.m_BufPalette.BufSize;
        m_BufPalette.pData   = new Byte[m_BufPalette.BufSize];
        memcpy(m_BufPalette.pData, pi_rObj.m_BufPalette.pData, m_BufPalette.BufSize);
        }
    else
        {
        m_BufPalette.BufSize    = 0;
        m_BufPalette.pData      = 0;
        }
    }

//-----------------------------------------------------------------------------
// Equal To operator
//-----------------------------------------------------------------------------
bool HRPPixelPalette::operator==(const HRPPixelPalette& pi_rObj) const
    {
    bool State = true;

    // Return false if not same channel organisation
    if(GetChannelOrg() != pi_rObj.GetChannelOrg())
        State = false;

    // Return false if not same number of used entries or maximum entries
    if(CountUsedEntries() != pi_rObj.CountUsedEntries() ||
       GetMaxEntries() != pi_rObj.GetMaxEntries())
        State = false;

    // Return false if one of the palette entry is different
    for(uint32_t i=0; State && i < CountUsedEntries(); i++)
        {
        if(0 != memcmp(GetCompositeValue(i),pi_rObj.GetCompositeValue(i), m_PixelEntrySize))
            State = false;
        }

    if(m_LockedEntryIndex != pi_rObj.m_LockedEntryIndex)
        State = false;

    // Can't find any differences
    return State;
    }

//-----------------------------------------------------------------------------
// Not Equal To operator
//-----------------------------------------------------------------------------
bool HRPPixelPalette::operator!=(const HRPPixelPalette& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }

//-----------------------------------------------------------------------------
// Appends a new entry into the palette embedded in this pixel type.
//-----------------------------------------------------------------------------
uint32_t HRPPixelPalette::AddEntry(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(!m_ReadOnly);
    HPRECONDITION(m_CountBufPalette < m_MaxPaletteSize);

    if (m_CountBufPalette >= m_MaxPaletteSize)
        return m_CountBufPalette;

    // Allocated Buf if not already done.
    if (m_BufPalette.pData == 0)
        {
        m_BufPalette.BufSize = m_PixelEntrySize*m_MaxPaletteSize;
        m_BufPalette.pData   = new Byte[m_BufPalette.BufSize];
        }

    memcpy(&(m_BufPalette.pData[m_CountBufPalette*m_PixelEntrySize]), pi_pValue, m_PixelEntrySize);

    // Set the unused padding bits to 0 for comparison porpose
    (&(m_BufPalette.pData[m_CountBufPalette*m_PixelEntrySize]))[m_PixelEntrySize-1] &= s_Mask[m_ChannelOrg.CountPixelCompositeValueBits() % 8];

    ++m_CountBufPalette;

    return m_CountBufPalette-1;
    }

//-----------------------------------------------------------------------------
// Changes a specific entry in the palette.
//-----------------------------------------------------------------------------
bool HRPPixelPalette::SetCompositeValue(uint32_t    pi_Index,
                                         const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);
    HPRECONDITION(!m_ReadOnly);
    bool Ret = true;

    if (pi_Index == m_LockedEntryIndex)
        Ret = false;
    else
        {
        HASSERT(pi_Index < m_CountBufPalette);

        if (pi_Index < m_CountBufPalette)
            {
            memcpy(&(m_BufPalette.pData[pi_Index*m_PixelEntrySize]), pi_pValue, m_PixelEntrySize);

            // Set the unused padding bits to 0 for comparison purpose
            (&(m_BufPalette.pData[pi_Index*m_PixelEntrySize]))[m_PixelEntrySize-1] &= s_Mask[m_ChannelOrg.CountPixelCompositeValueBits() % 8];
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// FindCompositeValue
//-----------------------------------------------------------------------------
int32_t HRPPixelPalette::FindCompositeValue(const void*  pi_pCompositeValue,
                                           uint32_t     pi_ChannelsNotUsedMask) const
    {
    HPRECONDITION(pi_pCompositeValue != 0);

    int32_t Value;

    Byte* pEntryRawData = m_BufPalette.pData;

    // test if we must compare all entries
    if(pi_ChannelsNotUsedMask == 0x00000000)
        {
        // fast compare

        uint32_t EntryIndex;
        // compare the raw data with each entry of the palette
        for(EntryIndex = 0; EntryIndex < m_CountBufPalette; ++EntryIndex)
            {
            // compare and break if equal
            if(memcmp(pEntryRawData, pi_pCompositeValue, m_PixelEntrySize) == 0)
                break;
            else
                // increment the pointer in the palette
                pEntryRawData += m_PixelEntrySize;
            }

        if(EntryIndex == m_CountBufPalette)
            Value = -1;
        else
            Value = EntryIndex;
        }
    else
        {
        uint32_t Channels = GetChannelOrg().CountChannels();
        uint32_t ChannelIndex;

        // test if all the channels are 8 bits. otherwise, return - 1
        // HLXXX  We are not able to compute this kind of histogram on
        // channels not having 8 bits
        for(ChannelIndex = 0; ChannelIndex < Channels && GetChannelOrg()[ChannelIndex]->GetSize() == 8; ChannelIndex++)
            ;

        if(ChannelIndex == Channels)
            {
            uint32_t ChannelMask;
            uint32_t EntryIndex;

            // comparethe raw data with each entry of the palette
            for(EntryIndex = 0; EntryIndex < m_CountBufPalette; EntryIndex++)
                {
                // test each channel in the channels use mask
                ChannelMask = 0x00000001;

                bool Exit = false;
                ChannelIndex = 0;
                while(ChannelIndex < Channels && !Exit)
                    {
                    if(!(pi_ChannelsNotUsedMask & ChannelMask) &&
                       ((Byte*)pi_pCompositeValue)[ChannelIndex] != pEntryRawData[ChannelIndex])
                        {
                        Exit = true;
                        }
                    else
                        {
                        ChannelMask <<= 1;
                        ChannelIndex++;
                        }
                    }

                // if the entry is equal, break
                if(ChannelIndex == Channels)
                    break;
                else
                    // increment the pointer in the palette
                    pEntryRawData += m_PixelEntrySize;
                }

            if(EntryIndex == m_CountBufPalette)
                Value = -1;
            else
                Value = EntryIndex;
            }
        else
            {
            Value = -1;
            }
        }

    return Value;
    }


//-----------------------------------------------------------------------------
// SetPalette
//-----------------------------------------------------------------------------
void HRPPixelPalette::SetPalette(const Byte* pi_pPalette, uint32_t pi_EntriesUsed)
    {
    HPRECONDITION(pi_pPalette != 0);
    HPRECONDITION(pi_EntriesUsed <= GetMaxEntries());
    HPRECONDITION(!m_ReadOnly);

    if (m_BufPalette.pData == 0)
        {
        m_BufPalette.BufSize = m_MaxPaletteBufferSize;
        m_BufPalette.pData   = new Byte[m_BufPalette.BufSize];
        }

    memcpy(m_BufPalette.pData, pi_pPalette, pi_EntriesUsed * m_PixelEntrySize);
    m_CountBufPalette = pi_EntriesUsed;
    (&(m_BufPalette.pData[m_CountBufPalette*m_PixelEntrySize]))[m_PixelEntrySize-1] &= s_Mask[m_ChannelOrg.CountPixelCompositeValueBits() % 8];
    }

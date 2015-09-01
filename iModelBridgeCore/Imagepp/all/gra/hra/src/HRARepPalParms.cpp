//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRARepPalParms.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRARepPalParms
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRPQuantizedPalette.h>

//-----------------------------------------------------------------------------
// Copy constructor for HRARepPalParms
//-----------------------------------------------------------------------------
HRARepPalParms::HRARepPalParms(const HRARepPalParms& pi_rRepPalParms)
    {
    DeepCopy(pi_rRepPalParms);
    }

//-----------------------------------------------------------------------------
// Constructor for HRARepPalParms
//-----------------------------------------------------------------------------
HRARepPalParms::HRARepPalParms(const HFCPtr<HRPPixelType>&  pi_pPixelType,
                               bool                        pi_ComputeHistogram,
                               uint32_t                     pi_MaxEntries)
    {
    HPRECONDITION(pi_pPixelType != 0);
    HPRECONDITION(pi_pPixelType->CountIndexBits());

    InitObject(pi_pPixelType, pi_ComputeHistogram, pi_MaxEntries);
    }

//-----------------------------------------------------------------------------
// Constructor for HRARepPalParms
//-----------------------------------------------------------------------------
HRARepPalParms::HRARepPalParms(const HFCPtr<HRPPixelType>&  pi_pPixelType,
                               const HRASamplingOptions&    pi_rOptions,
                               bool                        pi_ComputeHistogram,
                               uint32_t                     pi_MaxEntries)
    : m_SamplingOptions(pi_rOptions)
    {
    HPRECONDITION(pi_pPixelType != 0);
    HPRECONDITION(pi_pPixelType->CountIndexBits());

    InitObject(pi_pPixelType, pi_ComputeHistogram, pi_MaxEntries);
    }

//-----------------------------------------------------------------------------
// Destructor for HRARepPalParms
//-----------------------------------------------------------------------------
HRARepPalParms::~HRARepPalParms()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// CreateQuantizedPalette - Create a new instance of the quantized palette
//
// Warning! The user of the method must delete the object.
//-----------------------------------------------------------------------------
HRPQuantizedPalette* HRARepPalParms::CreateQuantizedPalette() const
    {
    HRPQuantizedPalette* pQuantizedPalette = 0;

    uint32_t MaxEntries;

    if (m_MaxEntries > 0)
        MaxEntries = MIN(m_pPixelType->GetPalette().GetMaxEntries(), m_MaxEntries);
    else
        MaxEntries = m_pPixelType->GetPalette().GetMaxEntries();

    if (m_pPixelType->GetPalette().GetLockedEntryIndex() >= 0)
        {
        // We have an locked entry, decrement the Maximun color and set the color
        // ignored in the Qunatizepalette
        pQuantizedPalette = m_pPixelType->CreateQuantizedPalette(MaxEntries - 1);
        HASSERT(pQuantizedPalette != 0);
        pQuantizedPalette->AddIgnoreValue(m_pPixelType->GetPalette().
                                          GetCompositeValue(m_pPixelType->GetPalette().GetLockedEntryIndex()));
        }
    else
        pQuantizedPalette = m_pPixelType->CreateQuantizedPalette(MaxEntries);

    return pQuantizedPalette;
    }

//-----------------------------------------------------------------------------
// public
// SetPixelType
//-----------------------------------------------------------------------------
void HRARepPalParms::SetPixelType(const HFCPtr<HRPPixelType>& pi_pPixelType)
    {
    HPRECONDITION(pi_pPixelType != 0);
    HPRECONDITION(pi_pPixelType->CountIndexBits());

    m_pPixelType = pi_pPixelType;
    }

//-----------------------------------------------------------------------------
// public
// DeepCopy
//-----------------------------------------------------------------------------
void HRARepPalParms::DeepCopy(const HRARepPalParms& pi_rObj)
    {
    m_pPixelType        = pi_rObj.m_pPixelType;
    m_SamplingOptions   = pi_rObj.m_SamplingOptions;
    m_UseCache          = pi_rObj.m_UseCache;
    m_pHistogram        = pi_rObj.m_pHistogram;
    m_MaxEntries        = pi_rObj.m_MaxEntries;
    }

//-----------------------------------------------------------------------------
// public
// DeepDelete
//-----------------------------------------------------------------------------
void HRARepPalParms::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void HRARepPalParms::InitObject(const HFCPtr<HRPPixelType>& pi_pPixelType,
                                bool                       pi_ComputeHistogram,
                                uint32_t                    pi_MaxEntries)
    {

    // if the palette has not enough entries, we create them here
    uint32_t MaxEntries;
    if (pi_MaxEntries > 0)
        MaxEntries = MIN((pi_pPixelType->GetPalette()).GetMaxEntries(), pi_MaxEntries);
    else
        MaxEntries = pi_pPixelType->GetPalette().GetMaxEntries();

    uint32_t UsedEntries = (pi_pPixelType->GetPalette()).CountUsedEntries();
    uint32_t DummyValue = 0;
    HRPPixelPalette& rPalette = pi_pPixelType->LockPalette();
    if(MaxEntries != UsedEntries)
        {
        for(uint32_t Index = UsedEntries; Index < MaxEntries; Index++)
            rPalette.AddEntry(&DummyValue);
        }

    // clear palette
    uint32_t PaletteSize = pi_pPixelType->GetPalette().CountUsedEntries();
    for(uint32_t Index = 0; Index < PaletteSize; Index++)
        rPalette.SetCompositeValue(Index, &DummyValue);

    pi_pPixelType->UnlockPalette();

    m_pPixelType = pi_pPixelType;
    m_MaxEntries = pi_MaxEntries;
    m_UseCache = true;

    // create an  histogram if necessary
    if(pi_ComputeHistogram)
        m_pHistogram = new HRPHistogram(pi_pPixelType->GetPalette());
    }

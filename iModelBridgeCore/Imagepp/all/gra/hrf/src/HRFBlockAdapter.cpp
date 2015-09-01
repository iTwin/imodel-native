//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBlockAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFBlockAdapter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>

//-----------------------------------------------------------------------------
// This ancestor class define the standard
// interface to query about the supported thing.
// There will be an object that derives from
// this one for each specific Implementation object.
// Each specific implementation of this object add
// only their supported thing to the list.
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities::HRFBlockAdapterCapabilities()
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities::~HRFBlockAdapterCapabilities()
    {
    }

//-----------------------------------------------------------------------------
// Query about the supported thing of that specific implementation
//-----------------------------------------------------------------------------
bool HRFBlockAdapterCapabilities::Supports(HRFBlockAdapterCapabilities::Capability pi_Capability) const
    {
    bool Supported = false;

    // Iterator is used to loop through the vector.
    HRFBlockAdapterCapabilities::ListOfCapabilities::const_iterator theCapabilitiesIterator;

    theCapabilitiesIterator = m_ListOfCapabilities.begin();

    // Try to find the same Capabilities
    while (theCapabilitiesIterator != m_ListOfCapabilities.end())
        {
        if (*theCapabilitiesIterator == pi_Capability)
            {
            Supported = true;
            // Stop the research
            theCapabilitiesIterator =  m_ListOfCapabilities.end();
            }
        else
            theCapabilitiesIterator++;
        }

    return Supported;
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFBlockAdapter::HRFBlockAdapter(HRFBlockAdapterCapabilities*   pi_pCapabilities,
                                 HFCPtr<HRFRasterFile>          pi_rpRasterFile,
                                 uint32_t                       pi_Page,
                                 unsigned short                pi_Resolution,
                                 HFCAccessMode                  pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pCapabilities = pi_pCapabilities;
    m_pAdaptedResolutionEditor = static_cast<HRFRasterFileBlockAdapter*>(GetRasterFile().GetPtr())->GetExtendedFile()->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);

    // Retreive a ptr on the true Orignal file
    if (pi_rpRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        m_pTheTrueOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)pi_rpRasterFile)->GetOriginalFile();
    else
        m_pTheTrueOriginalFile = pi_rpRasterFile;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFBlockAdapter::~HRFBlockAdapter()
    {
    SaveDataFlag();
    }

//-----------------------------------------------------------------------------
// public
// SaveDataFlag
// Synchronize information between the Adapted resolution editor
// Used by HRSObjectStore just before the SaveDataFlag.
//-----------------------------------------------------------------------------
void HRFBlockAdapter::SaveDataFlag()
    {
    // Check if
    if ((m_pAdaptedResolutionEditor->GetResolutionCapabilities()->GetCapabilityOfType(HRFBlocksDataFlagCapability::CLASS_ID,HFC_WRITE_ONLY) != 0) &&
        m_pAdaptedResolutionEditor->GetResolutionDescriptor()->HasBlocksDataFlag())
        {
        uint64_t CurrentBlockIndex;
        uint64_t CurrentOriginalBlockIndex;
        HRFDataFlag CurrentFlag;

        // Convert the flags to the original
        for (uint32_t BlockY=0; BlockY<m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetHeight(); BlockY += m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight())
            {
            for (uint32_t BlockX=0; BlockX<m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetWidth(); BlockX += m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth())
                {
                // Keep the current flag
                CurrentBlockIndex = GetResolutionDescriptor()->ComputeBlockIndex(BlockX, BlockY);
                CurrentFlag = GetResolutionDescriptor()->GetBlockDataFlag(CurrentBlockIndex);

                // Apply the flag to the original
                CurrentOriginalBlockIndex = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->ComputeBlockIndex(BlockX, BlockY);
                m_pAdaptedResolutionEditor->GetResolutionDescriptor()->SetBlockDataFlag(CurrentOriginalBlockIndex, CurrentFlag);

                if (!(CurrentFlag & HRFDATAFLAG_DIRTYFORSUBRES))
                    {
                    // Remove the DIRTY FOR SUB RES flag from the HRF
                    m_pAdaptedResolutionEditor->GetResolutionDescriptor()->ClearBlockDataFlag(CurrentOriginalBlockIndex, HRFDATAFLAG_DIRTYFORSUBRES);
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// SetPalette
// Palette information
//-----------------------------------------------------------------------------
void HRFBlockAdapter::SetPalette(const HRPPixelPalette& pi_rPalette)
    {
    HRFResolutionEditor::SetPalette(pi_rPalette);

    if (GetResolutionDescriptor()->PaletteHasChanged())
        m_pAdaptedResolutionEditor->SetPalette(GetResolutionDescriptor()->GetPixelType()->GetPalette());
    }

//-----------------------------------------------------------------------------
// This ancestor implement the Wrapper on the Capabilities
// Query about the supported thing of that specific implementation
//-----------------------------------------------------------------------------
bool HRFBlockAdapter::Supports(HRFBlockAdapterCapabilities::Capability pi_Capability) const
    {
    return m_pCapabilities->Supports(pi_Capability);
    }

//-----------------------------------------------------------------------------
// public
// GetAdaptedResolutionEditor
//-----------------------------------------------------------------------------
const HRFResolutionEditor* HRFBlockAdapter::GetAdaptedResolutionEditor() const
    {
    return m_pAdaptedResolutionEditor;
    }
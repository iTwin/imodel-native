//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheFileCreator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the CacheFile implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFCacheFileCreator::HRFCacheFileCreator()
    {
    }

//-----------------------------------------------------------------------------
// PostConstructor
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::PostConstructor()
    {
    // Get the supported pixel types for the selected cache format
    m_ListOfPixelType = GetCapabilities()->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID, HFC_READ_WRITE_CREATE);

    // Assign the default codec and compression quality for all pixel type
    m_SelectedCodecs.clear();
    m_SelectedCodecsQuality.clear();

    m_SelectedSubResCodecs.clear();
    m_SelectedSubResCodecsQuality.clear();

    for (uint32_t Index=0; Index < m_ListOfPixelType->CountCapabilities(); Index++)
        {
        HFCPtr<HRFPixelTypeCapability> pPixelType;
        pPixelType = ((HFCPtr<HRFPixelTypeCapability>&)m_ListOfPixelType->GetCapability(Index));

        if (pPixelType->CountCodecs() > 0)
            {
            m_SelectedCodecs.push_back(pPixelType->GetCodecCapabilityByIndex(0)->GetCodecClassID());
            m_SelectedSubResCodecs.push_back(pPixelType->GetCodecCapabilityByIndex(0)->GetCodecClassID());

            if ((pPixelType->GetCodecCapabilityByIndex(0)->GetCodecClassID() == HCDCodecIJG::CLASS_ID) ||
                (pPixelType->GetCodecCapabilityByIndex(0)->GetCodecClassID() == HCDCodecFlashpix::CLASS_ID))
                {
                m_SelectedCodecsQuality.push_back(80);
                m_SelectedSubResCodecsQuality.push_back(80);
                }
            else
                {
                m_SelectedCodecsQuality.push_back(0);
                m_SelectedSubResCodecsQuality.push_back(0);
                }
            }
        else
            {
            m_SelectedCodecs.push_back(HCDCodecIdentity::CLASS_ID);
            m_SelectedSubResCodecs.push_back(HCDCodecIdentity::CLASS_ID);
            m_SelectedCodecsQuality.push_back(0);
            m_SelectedSubResCodecsQuality.push_back(0);
            }
        }

    m_DownSamplingMethodForValuesPixelType  = HRFDownSamplingMethod::AVERAGE;
    m_DownSamplingMethodForIndexedPixelType = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
    m_DownSamplingMethodFor1BitPixelType    = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFCacheFileCreator::~HRFCacheFileCreator()
    {
    }

//-----------------------------------------------------------------------------
// CountPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
uint32_t HRFCacheFileCreator::CountPixelType() const
    {
    return m_ListOfPixelType->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// GetPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFCacheFileCreator::GetPixelType(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountPixelType());
    return ((HFCPtr<HRFPixelTypeCapability>&)m_ListOfPixelType->GetCapability(pi_Index))->GetPixelTypeClassID();
    }

//-----------------------------------------------------------------------------
// CountCodecsFor
// Codec interface
// Get the codec count for the PixelType
//-----------------------------------------------------------------------------
uint32_t HRFCacheFileCreator::CountCodecsFor(HCLASS_ID  pi_PixelType) const
    {
    // Get the PixelType capability associete with pi_rpPixelType
    HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
    pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      pi_PixelType,
                                                      new HRFRasterFileCapabilities());

    pPixelTypeCapability = static_cast<HRFPixelTypeCapability*>(m_ListOfPixelType->
                           GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixelTypeCapability)).GetPtr());


    HASSERT(pPixelTypeCapability != 0);
    return pPixelTypeCapability->CountCodecs();
    }

//-----------------------------------------------------------------------------
// GetCodecFor
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFCacheFileCreator::GetCodecFor(HCLASS_ID  pi_PixelType,
                                           uint32_t     pi_Index) const
    {
    // Get the PixelType capability associete with pi_rpPixelType
    HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
    pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                      pi_PixelType,
                                                      new HRFRasterFileCapabilities());

    pPixelTypeCapability = static_cast<HRFPixelTypeCapability*>(m_ListOfPixelType->
                           GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixelTypeCapability)).GetPtr());

    HASSERT(pPixelTypeCapability != 0);
    HASSERT(pi_Index < pPixelTypeCapability->CountCodecs());

    return pPixelTypeCapability->GetCodecCapabilityByIndex(pi_Index)->GetCodecClassID();
    }


//-----------------------------------------------------------------------------
// SelectCodecFor
// Codec interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SelectCodecFor(HCLASS_ID  pi_PixelType,
                                         HCLASS_ID  pi_Codec)

    {
    if (CountCodecsFor(pi_PixelType) > 0)
        {
        // Validate if the specified codec is supported
        bool CodecSupported = false;
        uint32_t CodecCount = CountCodecsFor(pi_PixelType);
        for (uint32_t Index=0; (Index < CodecCount) && (!CodecSupported); Index++)
            {
            if (pi_Codec == GetCodecFor(pi_PixelType, Index))
                {
                // Assign the specified codec because is supported
                CodecSupported = true;

                // Find the index for the specified pixel type
                uint32_t IndexFound = CountPixelType();
                for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
                    if (pi_PixelType == GetPixelType(Index))
                        IndexFound = Index;

                // Validate the pixel type index
                HASSERT(IndexFound < m_SelectedCodecs.size());
                m_SelectedCodecs[IndexFound] = pi_Codec;
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// SelectSubResCodecFor
// Codec interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SelectSubResCodecFor(HCLASS_ID  pi_PixelType,
                                               HCLASS_ID  pi_Codec)

    {
    if (CountCodecsFor(pi_PixelType) > 0)
        {
        // Validate if the specified codec is supported
        bool CodecSupported = false;
        uint32_t CodecCount = CountCodecsFor(pi_PixelType);
        for (uint32_t Index=0; (Index < CodecCount) && (!CodecSupported); Index++)
            {
            if (pi_Codec == GetCodecFor(pi_PixelType, Index))
                {
                // Assign the specified codec because is supported
                CodecSupported = true;

                // Find the index for the specified pixel type
                uint32_t IndexFound = CountPixelType();
                for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
                    if (pi_PixelType == GetPixelType(Index))
                        IndexFound = Index;

                // Validate the pixel type index
                HASSERT(IndexFound < m_SelectedSubResCodecs.size());
                m_SelectedSubResCodecs[IndexFound] = pi_Codec;
                }
            }
        }
    }



//-----------------------------------------------------------------------------
// GetSelectedCodecFor
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFCacheFileCreator::GetSelectedCodecFor(HCLASS_ID  pi_PixelType) const
    {
    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedCodecs.size());
    return m_SelectedCodecs[IndexFound];
    }

//-----------------------------------------------------------------------------
// GetSelectedCodecFor
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFCacheFileCreator::GetSelectedSubResCodecFor(HCLASS_ID  pi_PixelType) const
    {
    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedSubResCodecs.size());
    return m_SelectedSubResCodecs[IndexFound];
    }


//-----------------------------------------------------------------------------
// CountCompressionStepFor
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFCacheFileCreator::CountCompressionStepFor(HCLASS_ID  pi_PixelType) const
    {
    uint32_t CountCompressionStep = 0;

    if ((GetSelectedCodecFor(pi_PixelType) == HCDCodecIJG::CLASS_ID) ||
        (GetSelectedCodecFor(pi_PixelType) == HCDCodecFlashpix::CLASS_ID))
        CountCompressionStep = 100;

    return CountCompressionStep;
    }

//-----------------------------------------------------------------------------
// SelectCompressionQualityFor
// Codec interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SelectCompressionQualityFor(HCLASS_ID  pi_PixelType,
                                                      uint32_t     pi_Quality)
    {
    HPRECONDITION(CountCompressionStepFor(pi_PixelType) > 0);
    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedCodecsQuality.size());
    m_SelectedCodecsQuality[IndexFound] = pi_Quality;
    }

//-----------------------------------------------------------------------------
// SelectSubResCompressionQualityFor
// Codec interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SelectSubResCompressionQualityFor(HCLASS_ID  pi_PixelType,
                                                            uint32_t     pi_Quality)
    {
    HPRECONDITION(CountCompressionStepFor(pi_PixelType) > 0);
    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedSubResCodecsQuality.size());
    m_SelectedSubResCodecsQuality[IndexFound] = pi_Quality;
    }



//-----------------------------------------------------------------------------
// GetSelectedCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFCacheFileCreator::GetSelectedCompressionQualityFor(HCLASS_ID  pi_PixelType) const
    {
    HPRECONDITION(CountCompressionStepFor(pi_PixelType) > 0);
    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedCodecsQuality.size());
    return m_SelectedCodecsQuality[IndexFound];
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFCacheFileCreator::GetSelectedSubResCompressionQualityFor(HCLASS_ID  pi_PixelType) const
    {
    HPRECONDITION(CountCompressionStepFor(pi_PixelType) > 0);

    // Find the index for the specified pixel type
    uint32_t IndexFound = CountPixelType();
    for (uint32_t Index=0; (Index < CountPixelType()) && (IndexFound == CountPixelType()); Index++)
        if (pi_PixelType == GetPixelType(Index))
            IndexFound = Index;

    HASSERT(IndexFound < m_SelectedSubResCodecsQuality.size());
    return m_SelectedSubResCodecsQuality[IndexFound];
    }


//-----------------------------------------------------------------------------
// SetDownSamplingMethodForValuesPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SetDownSamplingMethodForValuesPixelType(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_DownSamplingMethodForValuesPixelType = pi_DownSamplingMethod;
    }


//-----------------------------------------------------------------------------
// GetDownSamplingMethodForValuesPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFCacheFileCreator::GetDownSamplingMethodForValuesPixelType() const
    {
    return m_DownSamplingMethodForValuesPixelType;
    }

//-----------------------------------------------------------------------------
// SetDownSamplingMethodForIndexedPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SetDownSamplingMethodForIndexedPixelType(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_DownSamplingMethodForIndexedPixelType = pi_DownSamplingMethod;
    }


//-----------------------------------------------------------------------------
// GetDownSamplingMethodForIndexedPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFCacheFileCreator::GetDownSamplingMethodForIndexedPixelType() const
    {
    return m_DownSamplingMethodForIndexedPixelType;
    }

//-----------------------------------------------------------------------------
// SetDownSamplingMethodFor1BitPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
void HRFCacheFileCreator::SetDownSamplingMethodFor1BitPixelType(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_DownSamplingMethodFor1BitPixelType = pi_DownSamplingMethod;
    }


//-----------------------------------------------------------------------------
// GetDownSamplingMethodFor1BitPixelType
// Down sampling interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFCacheFileCreator::GetDownSamplingMethodFor1BitPixelType() const
    {
    return m_DownSamplingMethodFor1BitPixelType;
    }

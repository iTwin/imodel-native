//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFImportExport.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRFImportExport
// ----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecECW.h>
#include <Imagepp/all/h/HCDCodecJPEG2000.h>
#include <Imagepp/all/h/HCDCodecFactory.h>

#include <Imagepp/all/h/HRFHGRPageFile.h>
#include <Imagepp/all/h/HRFTWFPageFile.h>
#include <Imagepp/all/h/HRFImportExport.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>

#include <Imagepp/all/h/HRFExportOptions.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>



//-----------------------------------------------------------------------------
// Constructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HRFImportExport::HRFImportExport (const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster)
    {
    // World Cluster
    m_pWorldCluster = pi_pWorldCluster;

    // Default Value
    m_ListOfFileFormat = HRFRasterFileFactory::GetInstance()->GetCreators(HFC_CREATE_ONLY);

    m_ResampleIsForce     = false;
    m_SourceHasTransfo    = false;
    m_ImageSizeIsLock     = false;
    m_ScaleFactorIsLock   = false;
    m_MaintainAspectRatio = false;

    // Default Values
    m_OriginalSize.SetX(0.0);
    m_OriginalSize.SetY(0.0);
    m_DefaultResampleSize.SetX(0.0);
    m_DefaultResampleSize.SetY(0.0);

    m_DefaultResampleScaleFactorX = 0.0;
    m_DefaultResampleScaleFactorY = 0.0;

    m_OverrideDownSamplingMethod = HRFDownSamplingMethod::NONE;
    m_OverrideSubResDownSamplingMethod = HRFDownSamplingMethod::NONE;

    // Default color initialization
    m_aRGBDefaultColor[0] = 0; // Black
    m_aRGBDefaultColor[1] = 0;
    m_aRGBDefaultColor[2] = 0;
    // Default color initialization
    m_aRGBADefaultColor[0] = 0; // Black
    m_aRGBADefaultColor[1] = 0;
    m_aRGBADefaultColor[2] = 0;
    m_aRGBADefaultColor[3] = 0;
    m_isRGBADefaultColorSet = false;

    m_UseDestinationPaletteIfIndexed = false;
    m_BuildedPixelTypeSetted = false;
    m_BuildedSubResPixelTypeSetted = false;
    }

//-----------------------------------------------------------------------------
// Destructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HRFImportExport::~HRFImportExport()
    {

    }


//-----------------------------------------------------------------------------
// SetExportOptions
// Select all export options
//-----------------------------------------------------------------------------
void HRFImportExport::SetExportOptions(const HFCPtr<HRFExportOptions>& pi_rpExportOptions)
    {
    SelectExportFileFormat(pi_rpExportOptions->GetFileFormat());
    SelectPixelType(pi_rpExportOptions->GetPixelType());

    if (CountDownSamplingMethod() > 0)
        SelectDownSamplingMethod(pi_rpExportOptions->GetDownSamplingMethod());

    HASSERT(CountCodecs() > 0);

    //TR 234102 If the codec chosen by the application (user) cannot
    //be selected
    if (SelectCodec(pi_rpExportOptions->GetCodec()) == false)
        {
        Utf8String FileName;

        //Depending on when the export file name is set in relation with
        //the call to this method, it is possible that the export file name
        //isn't set yet.
        if (GetSelectedExportFilename() != 0)
            {
            FileName = GetSelectedExportFilename()->GetURL();
            }

        throw HRFInvalidExportOptionException(FileName);
        }

    if (CountCompressionStep() > 1)
        {
        SelectCompressionQuality(pi_rpExportOptions->GetCompressionQuality());
        }

    if (CountCompressionRatioStep() > 1)
        {
        SelectCompressionRatio(pi_rpExportOptions->GetCompressionRatio());
        }


    SelectBlockType(pi_rpExportOptions->GetBlockType());

    if (CountSubResPixelType() > 0)
        {
        SelectSubResPixelType(pi_rpExportOptions->GetSubResPixelType());

        if (CountSubResDownSamplingMethod() > 0)
            SelectSubResDownSamplingMethod(pi_rpExportOptions->GetSubResDownSamplingMethod());

        HASSERT(CountSubResCodecs() > 0);

        SelectSubResCodec(pi_rpExportOptions->GetSubResCodec());

        if (CountSubResCompressionStep() > 1)
            {
            SelectSubResCompressionQuality(pi_rpExportOptions->GetSubResCompressionQuality());
            }

        if (CountSubResCompressionRatioStep() > 1)
            {
            SelectSubResCompressionRatio(pi_rpExportOptions->GetSubResCompressionRatio());
            }

        SelectSubResBlockType(pi_rpExportOptions->GetSubResBlockType());
        }

    SetResample(pi_rpExportOptions->GetResample());

    // Thing's to do only when resampling has been requested.
    if (pi_rpExportOptions->GetResample())
        {
        SetImageWidth(pi_rpExportOptions->GetImageWidth());
        SetImageHeight(pi_rpExportOptions->GetImageHeight());
        SetScaleFactorX(pi_rpExportOptions->GetScaleFactorX());
        SetScaleFactorY(pi_rpExportOptions->GetScaleFactorY());
        }
    SetBlockWidth(pi_rpExportOptions->GetBlockWidth());
    SetBlockHeight(pi_rpExportOptions->GetBlockHeight());
    SelectEncoding(pi_rpExportOptions->GetEncoding());

    
    SetGeocoding(pi_rpExportOptions->GetGeocodingCP());

    if (CountGeoreferenceFormats() > 0)
        SelectGeoreferenceFormat(pi_rpExportOptions->GetGeoreferenceFormat());
    }


//-----------------------------------------------------------------------------
// CountExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
HRFExportOptions HRFImportExport::GetSelectedExportOptions() const
    {
    return m_ExportOptions;
    }

//-----------------------------------------------------------------------------
// CountExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountExportFileFormat() const
    {
    return (uint32_t)m_ListOfFileFormat.size();
    }

//-----------------------------------------------------------------------------
// GetExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
HRFRasterFileCreator* HRFImportExport::GetExportFileFormat(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountExportFileFormat());
    return m_ListOfFileFormat[pi_Index];
    }

//-----------------------------------------------------------------------------
// SelectExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectExportFileFormat(const HRFRasterFileCreator* pi_pCreator)
    {
    if (pi_pCreator != m_ExportOptions.GetFileFormat())
        {
        m_ExportOptions.SetFileFormat(pi_pCreator);
        m_pSelectedFileFormatCapabilities = ((HRFRasterFileCreator*)pi_pCreator)->GetCapabilities();
        PrepareExportFileFormatOptions();
        }
    }

//-----------------------------------------------------------------------------
// SelectExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
void HRFImportExport::PrepareExportFileFormatOptions()
    {
    // Get the list of valid pixel type with this file format.
    m_pListOfValidPixelType = m_pSelectedFileFormatCapabilities->GetCapabilitiesOfType(HRFPixelTypeCapability::CLASS_ID,
                              HFC_CREATE_ONLY);

    // Update list of encoding
    m_ListOfValidEncodingType.clear();

    if (m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                                                               HFC_CREATE_ONLY))
        m_ListOfValidEncodingType.push_back(HRFEncodingType::MULTIRESOLUTION);

    if (m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFSingleResolutionCapability::CLASS_ID,
                                                               HFC_CREATE_ONLY))
        m_ListOfValidEncodingType.push_back(HRFEncodingType::STANDARD);

    if (m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFInterlaceCapability::CLASS_ID,
                                                               HFC_CREATE_ONLY))
        m_ListOfValidEncodingType.push_back(HRFEncodingType::PROGRESSIVE);

    // Select Default value
    HASSERT(CountPixelType() > 0);
    SelectPixelTypeByIndex(0);

    HASSERT(CountEncoding() > 0);
    SelectEncodingByIndex(0);

    // Tag section add to the list
    HFCPtr<HRFRasterFileCapabilities>  TagCapabilities;
    TagCapabilities = m_pSelectedFileFormatCapabilities->GetCapabilitiesOfType(HRFTagCapability::CLASS_ID,
                                                                               HFC_CREATE_ONLY);
    // Clear the list
    m_ListOfValidTag.clear();

    if (TagCapabilities)
        {
        for (uint32_t IndexTag=0; IndexTag < TagCapabilities->CountCapabilities(); IndexTag++)
            {
            HFCPtr<HRFTagCapability> pTagCapability;
            pTagCapability = ((HFCPtr<HRFTagCapability>&)TagCapabilities->GetCapability(IndexTag));
            m_ListOfValidTag.push_back(pTagCapability->GetTag());
            }
        }

    // Handle georeference formats
    m_ListOfValidGeoreferenceFormats.clear();
    m_ListOfValidGeoreferenceFormats.push_back(HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE);

    if (!m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID,
                                                                HFC_CREATE_ONLY))
        {
        m_ListOfValidGeoreferenceFormats.push_back(HRFGeoreferenceFormat::GEOREFERENCE_IN_HGR);
        m_ListOfValidGeoreferenceFormats.push_back(HRFGeoreferenceFormat::GEOREFERENCE_IN_WORLD_FILE);
        }

    BestMatchSelectedValues();
    }

//-----------------------------------------------------------------------------
// SelectExportFileFormatByIndex
// Export File Format interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectExportFileFormatByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountExportFileFormat());
    SelectExportFileFormat(GetExportFileFormat(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
const HRFRasterFileCreator* HRFImportExport::GetSelectedExportFileFormat() const
    {
    return m_ExportOptions.GetFileFormat();
    }

//-----------------------------------------------------------------------------
// GetSelectedExportFileFormat
// Export File Format interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedExportFileFormatIndex() const
    {
    uint32_t IndexFound = 0;
    bool  Found      = false;

    for (uint32_t Index=0; (Index < CountExportFileFormat()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetFileFormat() == GetExportFileFormat(Index))
            {
            Found      = true;
            IndexFound = Index;
            }
        }

    return IndexFound;
    }

//-----------------------------------------------------------------------------
// SelectExportFilename
// Export File name interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectExportFilename(const HFCPtr<HFCURL>& pi_rpURLPath)
    {
    // Extract the Path
    Utf8String Path(pi_rpURLPath->GetURL());

    // Find the file extension
    Utf8String::size_type DotPosInFilename = Path.rfind('.');

    // Extract the extension and the drive dir name
    if (DotPosInFilename != Utf8String::npos)
        m_pSelectedExportFilename = pi_rpURLPath;
    else
        {
        Utf8String Extensions(((HRFRasterFileCreator*)(m_ExportOptions.GetFileFormat()))->GetExtensions());

        // Find the file extension
        Utf8String::size_type DotPos  = Extensions.find('.');
        Utf8String::size_type DotVPos = Extensions.find(';', DotPos);

        // Extract the first extension in the list
        if (DotPos != Utf8String::npos)
            {
            Utf8String Extension;

            if (DotVPos != Utf8String::npos)
                Extension = Extensions.substr(DotPos+1, DotVPos - DotPos - 1);
            else
                Extension = Extensions.substr(DotPos+1, Extensions.length() - DotPos - 1);

            m_pSelectedExportFilename = HFCURL::Instanciate(pi_rpURLPath->GetURL() + Utf8String(".") + Extension);
            }
        else
            m_pSelectedExportFilename = pi_rpURLPath;
        }
    }

//-----------------------------------------------------------------------------
// GetSelectedExportFilename
// Export File name interface
//-----------------------------------------------------------------------------
const HFCPtr<HFCURL>& HRFImportExport::GetSelectedExportFilename() const
    {
    return m_pSelectedExportFilename;
    }

//-----------------------------------------------------------------------------
// CountPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountPixelType() const
    {
    return m_pListOfValidPixelType->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// GetPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetPixelType(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountPixelType());
    return ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->
            GetCapability(pi_Index))->GetPixelTypeClassID();
    }

//-----------------------------------------------------------------------------
// SelectPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectPixelType(HCLASS_ID pi_PixelType)
    {
    bool Found = false;

    HFCPtr<HRFPixelTypeCapability> pPixelType;

    // Find the pixel type in the file capabilities.
    for (uint32_t Index = 0; (Index < CountPixelType()) && (!Found); Index++)
        {
        // Get the pixel type capabilitity.
        pPixelType = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));

        if (pPixelType->GetPixelTypeClassID() == pi_PixelType)
            {
            m_ExportOptions.SetPixelType(pi_PixelType);

            // Used for acceleration
            m_pSelectedPixelTypeCapability = pPixelType;

            Found = true;
            }
        }
    HASSERT(pPixelType != 0);

    // Get the list of valid codec with this pixel type.
    m_pListOfValidCodec = m_pSelectedPixelTypeCapability->GetCodecCapabilities()->
                          GetCapabilitiesOfType(HRFCodecCapability::CLASS_ID,
                                                HFC_CREATE_ONLY);

    // Select the first valid codec associate with this pixel type
    HASSERT(CountCodecs() > 0);
    SelectCodecByIndex(0);

    // Select the first down sampling methods associate with this pixel type.
    if (CountDownSamplingMethod() > 0)
        SelectDownSamplingMethodByIndex(0);

    // Select sub res pixel type associate with this pixel type.
    if (CountSubResPixelType() > 0)
        {
        SelectSubResPixelType(m_ExportOptions.GetPixelType());

        if (CountSubResDownSamplingMethod() > 0)
            SelectSubResDownSamplingMethodByIndex(0);
        }

    // reset the builded PixelType
    m_pPixelType = 0;
    m_BuildedPixelTypeSetted = false;
    }

//-----------------------------------------------------------------------------
// SelectPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectPixelTypeByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountPixelType());
    SelectPixelType(GetPixelType(pi_Index));
    }

//-----------------------------------------------------------------------------
// SelectPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType)
    {
    HPRECONDITION(pi_rpPixelType != 0);

    if (GetSelectedPixelType() != pi_rpPixelType->GetClassID())
        SelectPixelType(pi_rpPixelType->GetClassID());

    // keep the builded PixelType
    m_pPixelType = pi_rpPixelType;
    m_BuildedPixelTypeSetted = true;
    }

//-----------------------------------------------------------------------------
// GetSelectedPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSelectedPixelType() const
    {
    return m_ExportOptions.GetPixelType();
    }

//-----------------------------------------------------------------------------
// GetPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
const HFCPtr<HRPPixelType>& HRFImportExport::GetPixelType() const
    {
    if (m_pPixelType == 0 || m_pPixelType->GetClassID() != GetSelectedPixelType())
        m_pPixelType = HRPPixelTypeFactory::GetInstance()->Create(GetSelectedPixelType());

    // return the builded PixelType
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// GetSelectedPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedPixelTypeIndex() const
    {
    bool  Found      = false;
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountPixelType()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetPixelType() == GetPixelType(Index))
            {
            IndexFound = Index;
            Found      = true;
            }
        }

    return IndexFound;
    }


//-----------------------------------------------------------------------------
// CountSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResPixelType() const
    {
    uint32_t Count = 0;

    HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
    pMultiResCapability = static_cast<HRFMultiResolutionCapability*>(m_pSelectedFileFormatCapabilities->
                           GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());
    if ((pMultiResCapability != 0) && (!pMultiResCapability->IsSinglePixelType()))
        Count = m_pListOfValidPixelType->CountCapabilities();

    return Count;
    }

//-----------------------------------------------------------------------------
// GetSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSubResPixelType(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountSubResPixelType());
    return ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->
            GetCapability(pi_Index))->GetPixelTypeClassID();
    }


//-----------------------------------------------------------------------------
// SelectSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResPixelType(HCLASS_ID pi_PixelType)
    {
    bool Found = false;

    HFCPtr<HRFPixelTypeCapability> pPixelType;

    // Find the pixel type in the file capabilities.
    for (uint32_t Index = 0; (Index < CountSubResPixelType()) && (!Found); Index++)
        {
        pPixelType = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));

        if (pPixelType->GetPixelTypeClassID() == pi_PixelType)
            {
            m_ExportOptions.SetSubResPixelType(pi_PixelType);

            // Used for acceleration
            m_pSelectedSubResPixelTypeCapability = pPixelType;

            Found = true;
            }
        }
    HASSERT(pPixelType != 0);

    // Get the list of valid codec with this pixel type. (in create access)
    m_pListOfValidSubResCodec = m_pSelectedSubResPixelTypeCapability->GetCodecCapabilities()->
                                GetCapabilitiesOfType(HRFCodecCapability::CLASS_ID,
                                                      HFC_CREATE_ONLY);

    // Select the first valid codec associate with this pixel type
    HASSERT(CountSubResCodecs() > 0);
    SelectSubResCodecByIndex(0);

    // Select the first down sampling methods associate with this pixel type.
    if (CountSubResDownSamplingMethod() > 0)
        SelectSubResDownSamplingMethodByIndex(0);

    // reset the builded sub res PixelType
    m_pSubResPixelType = 0;
    m_BuildedSubResPixelTypeSetted = false;
    }

//-----------------------------------------------------------------------------
// SelectSubResPixelTypeByIndex
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResPixelTypeByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountSubResPixelType());
    SelectSubResPixelType(GetSubResPixelType(pi_Index));
    }

//-----------------------------------------------------------------------------
// SelectSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType)
    {
    HPRECONDITION(pi_rpPixelType != 0);

    if (GetSelectedSubResPixelType() != pi_rpPixelType->GetClassID())
        SelectSubResPixelType(pi_rpPixelType->GetClassID());

    // keep the builded PixelType
    m_pSubResPixelType = pi_rpPixelType;
    m_BuildedSubResPixelTypeSetted = true;
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSelectedSubResPixelType() const
    {
    return m_ExportOptions.GetSubResPixelType();
    }

//-----------------------------------------------------------------------------
// GetSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
const HFCPtr<HRPPixelType>& HRFImportExport::GetSubResPixelType() const
    {
    if (m_pSubResPixelType == 0 || m_pSubResPixelType->GetClassID() != GetSelectedSubResPixelType())
        m_pSubResPixelType = HRPPixelTypeFactory::GetInstance()->Create(GetSelectedSubResPixelType());

    // return the builded PixelType
    return m_pSubResPixelType;
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResPixelType
// Pixel Type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResPixelTypeIndex() const
    {
    bool  Found      = false;
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountSubResPixelType()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetSubResPixelType() == GetSubResPixelType(Index))
            {
            IndexFound = Index;
            Found      = true;
            }
        }

    return IndexFound;
    }

//-----------------------------------------------------------------------------
// CountDownSamplingMethod
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountDownSamplingMethod() const
    {
    return m_pSelectedPixelTypeCapability->CountDownSamplingMethod();
    }

//-----------------------------------------------------------------------------
// GetDownSamplingMethod
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFImportExport::GetDownSamplingMethod(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountDownSamplingMethod());
    return m_pSelectedPixelTypeCapability->GetDownSamplingMethod(pi_Index);
    }

//-----------------------------------------------------------------------------
// SelectDownSamplingMethod
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_ExportOptions.SetDownSamplingMethod(pi_DownSamplingMethod);
    }

//-----------------------------------------------------------------------------
// SelectDownSamplingMethodByIndex
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectDownSamplingMethodByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountDownSamplingMethod());
    SelectDownSamplingMethod(GetDownSamplingMethod(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedDownSamplingMethod
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFImportExport::GetSelectedDownSamplingMethod() const
    {
    return m_ExportOptions.GetDownSamplingMethod();
    }

//-----------------------------------------------------------------------------
// GetSelectedDownSamplingMethod
// DownSamplingMethod interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedDownSamplingMethodIndex() const
    {
    bool  Found      = false;
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountDownSamplingMethod()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetDownSamplingMethod() == GetDownSamplingMethod(Index))
            {
            IndexFound = Index;
            Found      = true;
            }
        }

    return IndexFound;
    }

//-----------------------------------------------------------------------------
// CountSubResDownSamplingMethod
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResDownSamplingMethod() const
    {
    return m_pSelectedSubResPixelTypeCapability->CountDownSamplingMethod();
    }

//-----------------------------------------------------------------------------
// GetSubResDownSamplingMethod
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFImportExport::GetSubResDownSamplingMethod(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountSubResDownSamplingMethod());
    return m_pSelectedSubResPixelTypeCapability->GetDownSamplingMethod(pi_Index);
    }

//-----------------------------------------------------------------------------
// SelectSubResDownSamplingMethod
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResDownSamplingMethod(HRFDownSamplingMethod pi_SubResDownSamplingMethod)
    {
    m_ExportOptions.SetSubResDownSamplingMethod(pi_SubResDownSamplingMethod);
    }

//-----------------------------------------------------------------------------
// SelectSubResDownSamplingMethodByIndex
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResDownSamplingMethodByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountSubResDownSamplingMethod());
    SelectSubResDownSamplingMethod(GetSubResDownSamplingMethod(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResDownSamplingMethod
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
HRFDownSamplingMethod HRFImportExport::GetSelectedSubResDownSamplingMethod() const
    {
    return m_ExportOptions.GetSubResDownSamplingMethod();
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResDownSamplingMethod
// SubResDownSamplingMethod interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResDownSamplingMethodIndex() const
    {
    bool  Found      = 0;
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountSubResDownSamplingMethod()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetSubResDownSamplingMethod() == GetSubResDownSamplingMethod(Index))
            {
            IndexFound = Index;
            Found      = true;
            }
        }
    return IndexFound;
    }


/**----------------------------------------------------------------------------
 This method override all previous/after call to SelectDownSamplingMethod().

 The method don't check if the current file type support the capability of
 DownSamplingMethod.

 You normally don't use this method, because for exemple, if you set
 AVERAGE with a TIFF file, you can't retreive the DownSampling option, TIFF
 file don't save the option.

 @param pi_DownSamplingMethod Specify the downSampling method.

 @see HRFImportExport::SelectDownSamplingMethod
-----------------------------------------------------------------------------*/
void HRFImportExport::OverrideDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_OverrideDownSamplingMethod = pi_DownSamplingMethod;
    }

/**----------------------------------------------------------------------------
 This method override all previous/after call to SelectSubResDownSamplingMethod().

 The method don't check if the current file type support the capability of
 DownSamplingMethod.

 You normally don't use this method, because for exemple, if you set
 AVERAGE with a TIFF file, you can't retreive the DownSampling option, TIFF
 file don't save the option.

 @param pi_DownSamplingMethod Specify the downSampling method.

 @see HRFImportExport::SelectSubResDownSamplingMethod
-----------------------------------------------------------------------------*/
void HRFImportExport::OverrideSubResDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_OverrideSubResDownSamplingMethod = pi_DownSamplingMethod;
    }

//-----------------------------------------------------------------------------
// CountCodec
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountCodecs() const
    {
    HPRECONDITION(m_pListOfValidCodec != 0);
    return m_pListOfValidCodec->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// GetCodec
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetCodec(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountCodecs());
    return ((HFCPtr<HRFCodecCapability>&) m_pListOfValidCodec->
            GetCapability(pi_Index))->
           GetCodecClassID();
    }

//-----------------------------------------------------------------------------
// SelectCodec
// Codec interface
//-----------------------------------------------------------------------------
bool HRFImportExport::SelectCodec(HCLASS_ID pi_Codec)
    {
    return SelectCodecSample(HCDCodecFactory::GetInstance().Create(pi_Codec));
    }

//-----------------------------------------------------------------------------
// SelectCodecSample
// Codec interface
//-----------------------------------------------------------------------------
bool HRFImportExport::SelectCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec)
    {
    HPRECONDITION(CountCodecs() > 0);

    bool Found = false;
    HFCPtr<HRFCodecCapability> pCodecCapability;

    // Try to find the codec in the selected pixel type capability
    for (uint32_t Index = 0; (Index < CountCodecs()) && (!Found); Index++)
        {
        // Get the codec capability.
        pCodecCapability = ((HFCPtr<HRFCodecCapability>&) m_pListOfValidCodec->GetCapability(Index));

        if (pCodecCapability->GetCodecClassID() == pi_rpCodec->GetClassID())
            {
            m_ExportOptions.SetCodecSample(pi_rpCodec);

            // Used for acceleration.
            m_pSelectedCodecCapability = pCodecCapability;

            Found = true;
            }
        }
    HASSERT(m_pSelectedCodecCapability != 0);

    //If the selected codec has been found, extract the import/export options
    //related to the selected codec.
    if (Found == true)
        {
        if ((m_ExportOptions.GetCodecSample()->IsCompatibleWith(HCDCodecECW::CLASS_ID)))
            {
            m_ExportOptions.SetCompressionRatio(10);
            m_CountCompressionRatioStep = 25;
            }
        else if ((m_ExportOptions.GetCodecSample()->IsCompatibleWith(HCDCodecJPEG2000::CLASS_ID)))
            {
            m_ExportOptions.SetCompressionRatio(15);
            m_CountCompressionRatioStep = 50;
            }
        else
            {
            m_ExportOptions.SetCompressionRatio(0);
            m_CountCompressionRatioStep = 0;
            }

        if ((m_ExportOptions.GetCodecSample()->GetClassID() == HCDCodecIJG::CLASS_ID) ||
            (m_ExportOptions.GetCodecSample()->GetClassID() == HCDCodecFlashpix::CLASS_ID) )
            {
            m_ExportOptions.SetCompressionQuality(95);
            m_CountCompressionStep = 100;
            }
        else
            {
            m_ExportOptions.SetCompressionQuality(0);
            m_CountCompressionStep = 0;
            }

        // Reset the count
        m_ListOfValidBlockType.clear();

        // Update list of Block Type associate with this codec.
        m_pTileCapability  = static_cast<HRFTileCapability*>(m_pSelectedCodecCapability->
                              GetBlockTypeCapabilities()->
                              GetCapabilityOfType(HRFTileCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pStripCapability = static_cast<HRFStripCapability*>(m_pSelectedCodecCapability->
                              GetBlockTypeCapabilities()->
                              GetCapabilityOfType(HRFStripCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pLineCapability  = static_cast<HRFLineCapability*>(m_pSelectedCodecCapability->
                              GetBlockTypeCapabilities()->
                              GetCapabilityOfType(HRFLineCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pImageCapability = static_cast<HRFImageCapability*>(m_pSelectedCodecCapability->
                              GetBlockTypeCapabilities()->
                              GetCapabilityOfType(HRFImageCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        if (m_pTileCapability != 0)
            m_ListOfValidBlockType.push_back(HRFBlockType::TILE);
        if (m_pStripCapability != 0)
            m_ListOfValidBlockType.push_back(HRFBlockType::STRIP);
        if (m_pLineCapability != 0)
            m_ListOfValidBlockType.push_back(HRFBlockType::LINE);
        if (m_pImageCapability != 0)
            m_ListOfValidBlockType.push_back(HRFBlockType::IMAGE);

        // Change The block Type before Image Size
        HASSERT(CountBlockType() > 0);
        SelectBlockTypeByIndex(0);
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// SelectCodec
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectCodecByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountCodecs());
    SelectCodec(GetCodec(pi_Index));
    }


//-----------------------------------------------------------------------------
// GetSelectedCodec
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSelectedCodec() const
    {
    return m_ExportOptions.GetCodec();
    }

//-----------------------------------------------------------------------------
// GetSelectedCodecSample
// Codec interface
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HRFImportExport::GetSelectedCodecSample() const
    {
    return m_ExportOptions.GetCodecSample();
    }

//-----------------------------------------------------------------------------
// GetSelectedCodecIndex
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedCodecIndex() const
    {
    bool  Found      = false;
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountCodecs()) && (!Found); Index++)
        {
        if (m_ExportOptions.GetCodec() == GetCodec(Index))
            {
            IndexFound = Index;
            Found      = true;
            }
        }
    return IndexFound;
    }

//-----------------------------------------------------------------------------
// CountCompressionStep
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountCompressionStep() const
    {
    return m_CountCompressionStep;
    }

//-----------------------------------------------------------------------------
// SelectCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectCompressionQuality(uint32_t pi_Quality)
    {
    HPRECONDITION(pi_Quality <= CountCompressionStep());
    m_ExportOptions.SetCompressionQuality(pi_Quality);
    }

//-----------------------------------------------------------------------------
// GetSelectedCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedCompressionQuality() const
    {
    return m_ExportOptions.GetCompressionQuality();
    }

//-----------------------------------------------------------------------------
// CountCompressionRatioStep
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountCompressionRatioStep() const
    {
    return m_CountCompressionRatioStep;
    }

//-----------------------------------------------------------------------------
// SelectCompressionRatio
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectCompressionRatio(uint32_t pi_Ratio)
    {
    HPRECONDITION(pi_Ratio <= CountCompressionRatioStep());
    m_ExportOptions.SetCompressionRatio(pi_Ratio);
    }

//-----------------------------------------------------------------------------
// GetSelectedCompressionRatio
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedCompressionRatio() const
    {
    return m_ExportOptions.GetCompressionRatio();
    }

//-----------------------------------------------------------------------------
// CountSubResCodecs
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResCodecs() const
    {
    HPRECONDITION(m_pListOfValidSubResCodec != 0);
    return m_pListOfValidSubResCodec->CountCapabilities();
    }

//-----------------------------------------------------------------------------
// GetSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSubResCodec(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountSubResCodecs());
    return ((HFCPtr<HRFCodecCapability>&) m_pListOfValidSubResCodec->
            GetCapability(pi_Index))->
           GetCodecClassID();
    }

//-----------------------------------------------------------------------------
// SelectSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
bool HRFImportExport::SelectSubResCodec(HCLASS_ID pi_Codec)
    {
    return SelectSubResCodecSample(HCDCodecFactory::GetInstance().Create(pi_Codec));
    }

//-----------------------------------------------------------------------------
// SelectSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
bool HRFImportExport::SelectSubResCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec)
    {
    HPRECONDITION(CountSubResCodecs() > 0);

    bool Found = false;
    HFCPtr<HRFCodecCapability> pSubResCodecCapability;

    // Find the specifie codec
    for (uint32_t Index = 0; (Index < CountSubResCodecs()) && (!Found); Index++)
        {
        // Get the codec capability.
        pSubResCodecCapability = ((HFCPtr<HRFCodecCapability>&) m_pListOfValidSubResCodec->
                                  GetCapability(Index));

        if (pSubResCodecCapability->GetCodecClassID() == pi_rpCodec->GetClassID())
            {
            m_ExportOptions.SetSubResCodecSample(pi_rpCodec);

            // Used for acceleration.
            m_pSelectedSubResCodecCapability = pSubResCodecCapability;
            Found = true;
            }
        }

    if (Found == true)
        {
        if ((m_ExportOptions.GetSubResCodecSample()->IsCompatibleWith(HCDCodecECW::CLASS_ID)))
            {
            m_ExportOptions.SetSubResCompressionRatio(10);
            m_SubResCountCompressionRatioStep = 25;
            }
        else if ((m_ExportOptions.GetSubResCodecSample()->IsCompatibleWith(HCDCodecJPEG2000::CLASS_ID)))
            {
            m_ExportOptions.SetSubResCompressionRatio(15);
            m_SubResCountCompressionRatioStep = 50;
            }
        else
            {
            m_ExportOptions.SetSubResCompressionRatio(0);
            m_SubResCountCompressionRatioStep = 0;
            }

        if ((m_ExportOptions.GetSubResCodecSample()->GetClassID() == HCDCodecIJG::CLASS_ID) ||
            (m_ExportOptions.GetSubResCodecSample()->GetClassID() == HCDCodecFlashpix::CLASS_ID) )
            {
            m_ExportOptions.SetSubResCompressionQuality(95);
            m_SubResCountCompressionStep = 100;
            }
        else
            {
            m_ExportOptions.SetSubResCompressionQuality(0);
            m_SubResCountCompressionStep = 0;
            }

        m_ListOfValidSubResBlockType.clear();

        // Update list of sub res block type associate with this codec.
        m_pSubResTileCapability  = static_cast<HRFTileCapability*>(m_pSelectedSubResCodecCapability->
                                    GetBlockTypeCapabilities()->
                                    GetCapabilityOfType(HRFTileCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pSubResStripCapability = static_cast<HRFStripCapability*>(m_pSelectedSubResCodecCapability->
                                    GetBlockTypeCapabilities()->
                                    GetCapabilityOfType(HRFStripCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pSubResLineCapability  = static_cast<HRFLineCapability*>(m_pSelectedSubResCodecCapability->
                                    GetBlockTypeCapabilities()->
                                    GetCapabilityOfType(HRFLineCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        m_pSubResImageCapability = static_cast<HRFImageCapability*>(m_pSelectedSubResCodecCapability->
                                    GetBlockTypeCapabilities()->
                                    GetCapabilityOfType(HRFImageCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        if (m_pSubResTileCapability != 0)
            m_ListOfValidSubResBlockType.push_back(HRFBlockType::TILE);
        if (m_pSubResStripCapability != 0)
            m_ListOfValidSubResBlockType.push_back(HRFBlockType::STRIP);
        if (m_pSubResLineCapability != 0)
            m_ListOfValidSubResBlockType.push_back(HRFBlockType::LINE);
        if (m_pSubResImageCapability != 0)
            m_ListOfValidSubResBlockType.push_back(HRFBlockType::IMAGE);

        // Change The block Type before Image Size
        HASSERT(CountSubResBlockType() > 0);
        SelectSubResBlockTypeByIndex(0);
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// SelectSubResCodecByIndex
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResCodecByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountSubResCodecs());
    SelectSubResCodec(GetSubResCodec(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
HCLASS_ID HRFImportExport::GetSelectedSubResCodec() const
    {
    return m_ExportOptions.GetSubResCodec();
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodec>& HRFImportExport::GetSelectedSubResCodecSample() const
    {
    return m_ExportOptions.GetSubResCodecSample();
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCodec
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResCodecIndex() const
    {
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountSubResCodecs()) && (IndexFound == 0); Index++)
        {
        if (m_ExportOptions.GetSubResCodec() == GetSubResCodec(Index))
            {
            IndexFound = Index;
            }
        }

    return IndexFound;
    }

//-----------------------------------------------------------------------------
// CountSubResCompressionStep
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResCompressionStep() const
    {
    return m_SubResCountCompressionStep;
    }

//-----------------------------------------------------------------------------
// SelectSubResCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResCompressionQuality(uint32_t pi_Quality)
    {
    HPRECONDITION(pi_Quality <= CountSubResCompressionStep());
    m_ExportOptions.SetSubResCompressionQuality(pi_Quality);
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCompressionQuality
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResCompressionQuality() const
    {
    return m_ExportOptions.GetSubResCompressionQuality();
    }

//-----------------------------------------------------------------------------
// CountSubResCompressionStep
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResCompressionRatioStep() const
    {
    return m_SubResCountCompressionRatioStep;
    }

//-----------------------------------------------------------------------------
// SelectSubResCompressionRatio
// Codec interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResCompressionRatio(uint32_t pi_Ratio)
    {
    HPRECONDITION(pi_Ratio <= CountSubResCompressionRatioStep());
    m_ExportOptions.SetSubResCompressionRatio(pi_Ratio);
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResCompressionRatio
// Codec interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResCompressionRatio() const
    {
    return m_ExportOptions.GetSubResCompressionRatio();
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetImageWidth(uint32_t pi_Width)
    {
    HASSERT(!m_ImageSizeIsLock);

    if (pi_Width != m_ExportOptions.GetImageWidth())
        {
        // Update the image width.
        m_ExportOptions.SetImageWidth(pi_Width);

        // Validate the selected value
        if (m_pImageCapability)
            m_ExportOptions.SetImageWidth(m_pImageCapability->ValidateWidth(m_ExportOptions.GetImageWidth()));

        // Caculate the ratio between the new width and the source clip shape.
        // NB. We check if the ClipShape is empty only to support old stuff in RasterLib.
        //
        if (!m_ClipShape.IsEmpty())
            {
            HVEShape ClipShape(m_ClipShape);
            ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
            double Ratio = m_ExportOptions.GetImageWidth() / ClipShape.GetExtent().GetWidth();

            // Affect the change to scale factor.
            if (!m_ScaleFactorIsLock)
                {
                // Update the scale X.
                m_ExportOptions.SetScaleFactorX(Ratio);

                if (m_MaintainAspectRatio)
                    {
                    // Update the scale Y.
                    m_ExportOptions.SetScaleFactorY(Ratio);
                    }
                }
            if (m_MaintainAspectRatio)
                {
                // Update the image height.
                m_ExportOptions.SetImageHeight((uint32_t)(ClipShape.GetExtent().GetHeight() * Ratio));
                }
            }

        UpdateBlockValues();
        }
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetImageHeight(uint32_t pi_Height)
    {
    HASSERT(!m_ImageSizeIsLock);

    if (pi_Height != m_ExportOptions.GetImageHeight())
        {
        // Update the image height.
        m_ExportOptions.SetImageHeight(pi_Height);

        // Validate the selected value
        if (m_pImageCapability)
            m_ExportOptions.SetImageHeight(m_pImageCapability->ValidateHeight(m_ExportOptions.GetImageHeight()));

        // Caculate the ratio between the new height and the source clip shape.
        // NB. We check if the ClipShape is empty only to support old stuff in RasterLib.
        //
        if (!m_ClipShape.IsEmpty())
            {
            HVEShape ClipShape(m_ClipShape);
            ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
            double Ratio = m_ExportOptions.GetImageHeight() / ClipShape.GetExtent().GetHeight();

            // Affect the change to scale factor only if we resample
            if (!m_ScaleFactorIsLock)
                {
                // Update the scale Y.
                m_ExportOptions.SetScaleFactorY(Ratio);

                if (m_MaintainAspectRatio)
                    {
                    // Update the scale X.
                    m_ExportOptions.SetScaleFactorX(Ratio);
                    }
                }
            if (m_MaintainAspectRatio)
                {
                // Update the image width.
                m_ExportOptions.SetImageWidth((uint32_t)(ClipShape.GetExtent().GetWidth() * Ratio));
                }
            }

        UpdateBlockValues();
        }
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
uint32_t HRFImportExport::GetImageWidth() const
    {
    return m_ExportOptions.GetImageWidth();
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
uint32_t HRFImportExport::GetImageHeight() const
    {
    return m_ExportOptions.GetImageHeight();
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetScaleFactorX(double pi_ScaleX)
    {
    HASSERT(!m_ScaleFactorIsLock);
    HASSERT(!m_ClipShape.IsEmpty());

    // Update the scale X.
    m_ExportOptions.SetScaleFactorX(pi_ScaleX);

    // Caculate the ratio between the new width and the source clip shape.
    HVEShape ClipShape(m_ClipShape);
    ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));

    if (!m_ImageSizeIsLock)
        {
        // Update the image width.
        HFCGrid WidthGrid(0.0, 0.0, ClipShape.GetExtent().GetWidth() * pi_ScaleX, 1.0);
        HASSERT(WidthGrid.GetWidth() <= UINT32_MAX);
        m_ExportOptions.SetImageWidth((uint32_t)WidthGrid.GetWidth());

        if (m_MaintainAspectRatio)
            {
            // Update the image height.
            HFCGrid HeightGrid(0.0, 0.0, 1.0, ClipShape.GetExtent().GetHeight() * pi_ScaleX);
            HASSERT(WidthGrid.GetHeight() <= UINT32_MAX);
            m_ExportOptions.SetImageHeight((uint32_t)HeightGrid.GetHeight());
            }
        }
    if (m_MaintainAspectRatio)
        {
        // Update the scale Y.
        m_ExportOptions.SetScaleFactorY(pi_ScaleX);
        }
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetScaleFactorY(double pi_ScaleY)
    {
    HASSERT(!m_ScaleFactorIsLock);
    HASSERT(!m_ClipShape.IsEmpty());

    // Update the scale Y.
    m_ExportOptions.SetScaleFactorY(pi_ScaleY);

    HVEShape ClipShape(m_ClipShape);
    ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));

    if (!m_ImageSizeIsLock)
        {
        // Update the image height.
        HFCGrid HeightGrid(0.0, 0.0, 1.0, ClipShape.GetExtent().GetHeight() * pi_ScaleY);

        HASSERT(HeightGrid.GetHeight() <= UINT32_MAX);
        m_ExportOptions.SetImageHeight((uint32_t)HeightGrid.GetHeight());

        if (m_MaintainAspectRatio)
            {
            // Update the image width.
            HFCGrid WidthGrid(0.0, 0.0, ClipShape.GetExtent().GetWidth() * pi_ScaleY, 1.0);

            HASSERT(HeightGrid.GetWidth() <= UINT32_MAX);
            m_ExportOptions.SetImageWidth((uint32_t)WidthGrid.GetWidth());
            }
        }
    if (m_MaintainAspectRatio)
        {
        // Update the scale X.
        m_ExportOptions.SetScaleFactorX(pi_ScaleY);
        }
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
double HRFImportExport::GetScaleFactorX() const
    {
    return m_ExportOptions.GetScaleFactorX();
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
double HRFImportExport::GetScaleFactorY() const
    {
    return m_ExportOptions.GetScaleFactorY();
    }

//-----------------------------------------------------------------------------
// CountBlockType
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountBlockType() const
    {
    HPRECONDITION(m_pSelectedCodecCapability != 0);
    return (uint32_t)m_ListOfValidBlockType.size();
    }

//-----------------------------------------------------------------------------
// GetBlockType
// Block Type  interface
//-----------------------------------------------------------------------------
HRFBlockType HRFImportExport::GetBlockType(uint32_t pi_Index) const
    {
    HPRECONDITION((pi_Index < CountBlockType()) && (m_ListOfValidBlockType.size() > 0));

    return m_ListOfValidBlockType[pi_Index];
    }

//-----------------------------------------------------------------------------
// SelectBlockType
// Block Type  interfaceinterface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectBlockType(HRFBlockType pi_BlockType)
    {
    // Set the new block type only if we support it
    HPRECONDITION(m_pSelectedCodecCapability->SupportsBlockType(pi_BlockType));

    if (((pi_BlockType == HRFBlockType::TILE)  && (m_pTileCapability)) ||
        ((pi_BlockType == HRFBlockType::LINE)  && (m_pLineCapability)) ||
        ((pi_BlockType == HRFBlockType::STRIP) && (m_pStripCapability)) ||
        ((pi_BlockType == HRFBlockType::IMAGE) && (m_pImageCapability)))
        {
        m_ExportOptions.SetBlockType(pi_BlockType);

        UpdateBlockValues();

        SetBlockWidth(256);
        SetBlockHeight(256);
        }
    }

//-----------------------------------------------------------------------------
// UpdateBlockValues
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::UpdateBlockValues()
    {
    // By Default no increment step
    m_BlockWidthIncrementStep           = 0;
    m_BlockHeightIncrementStep          = 0;
    m_CountBlockWidthIncrementStep      = 0;
    m_CountBlockHeightIncrementStep     = 0;
    m_MinBlockWidth                     = 0;
    m_MinBlockHeight                    = 0;

    // Change the selected block type only when the format support the specified type
    if ((GetSelectedBlockType() == HRFBlockType::TILE) && (m_pTileCapability))
        {
        m_BlockWidthIncrementStep   = m_pTileCapability->GetWidthIncrement();
        m_BlockHeightIncrementStep  = m_pTileCapability->GetHeightIncrement();
        m_MinBlockWidth             = m_pTileCapability->GetMinWidth();
        m_MinBlockHeight            = m_pTileCapability->GetMinHeight();

        if (m_BlockWidthIncrementStep)
            m_CountBlockWidthIncrementStep = m_pTileCapability->GetMaxWidth() / m_BlockWidthIncrementStep;

        if (m_BlockHeightIncrementStep)
            m_CountBlockHeightIncrementStep = m_pTileCapability->GetMaxHeight() / m_BlockHeightIncrementStep;
        }
    else if ((GetSelectedBlockType() == HRFBlockType::STRIP) && (m_pStripCapability))
        {
        m_BlockHeightIncrementStep = m_pStripCapability->GetHeightIncrement();
        m_MinBlockHeight           = m_pStripCapability->GetMinHeight();

        if (m_BlockHeightIncrementStep)
            {
            if (MIN(m_pStripCapability->GetMaxHeight(),GetImageHeight()) > m_MinBlockHeight)
                {
                m_CountBlockHeightIncrementStep = ((MIN(m_pStripCapability->GetMaxHeight(),GetImageHeight() + 1) - m_MinBlockHeight)
                                                   / m_BlockHeightIncrementStep) + 1;
                }
            }

        m_MinBlockWidth = GetImageWidth();
        }
    else if ((GetSelectedBlockType() == HRFBlockType::LINE) && (m_pLineCapability))
        {
        m_MinBlockWidth  = GetImageWidth();
        m_MinBlockHeight = 1;
        }
    else if ((GetSelectedBlockType() == HRFBlockType::IMAGE) && (m_pImageCapability))
        {
        m_MinBlockWidth  = GetImageWidth();
        m_MinBlockHeight = GetImageHeight();
        }

    // Update Block Dimension
    SetBlockWidth(GetBlockWidth());
    SetBlockHeight(GetBlockHeight());
    }

//-----------------------------------------------------------------------------
// SelectBlockTypeByIndex
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectBlockTypeByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountBlockType());
    SelectBlockType(GetBlockType(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedBlockType
// Block Type  interface
//-----------------------------------------------------------------------------
HRFBlockType HRFImportExport::GetSelectedBlockType() const
    {
    return m_ExportOptions.GetBlockType();
    }

//-----------------------------------------------------------------------------
// GetSelectedBlockType
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedBlockTypeIndex() const
    {
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountBlockType()) && (IndexFound == 0); Index++)
        {
        if (m_ExportOptions.GetBlockType() == GetBlockType(Index))
            {
            IndexFound = Index;
            }
        }
    return IndexFound;
    }

//-----------------------------------------------------------------------------
// SetBlockWidth
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::SetBlockWidth(uint32_t pi_Width)
    {
    m_ExportOptions.SetBlockWidth(pi_Width);

    // Validate the selected value
    if (GetSelectedBlockType() == HRFBlockType::TILE)
        {
        m_ExportOptions.SetBlockWidth(m_pTileCapability->ValidateWidth(m_ExportOptions.GetBlockWidth()));

        if (m_pTileCapability->IsSquare())
            m_ExportOptions.SetBlockHeight(m_ExportOptions.GetBlockWidth());
        }
    else if (GetSelectedBlockType() == HRFBlockType::IMAGE)
        m_ExportOptions.SetBlockWidth(m_pImageCapability->ValidateWidth(m_ExportOptions.GetBlockWidth()));
    else if (GetSelectedBlockType() == HRFBlockType::STRIP)
        m_ExportOptions.SetBlockWidth(GetImageWidth());
    else if (GetSelectedBlockType() == HRFBlockType::LINE)
        m_ExportOptions.SetBlockWidth(GetImageWidth());
    else
        m_ExportOptions.SetBlockWidth(0);
    }

//-----------------------------------------------------------------------------
// SetBlockHeight
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::SetBlockHeight(uint32_t pi_Height)
    {
    m_ExportOptions.SetBlockHeight(pi_Height);

    // Validate the selected value
    if (GetSelectedBlockType() == HRFBlockType::TILE)
        {
        m_ExportOptions.SetBlockHeight(m_pTileCapability->ValidateHeight(m_ExportOptions.GetBlockHeight()));
        if (m_pTileCapability->IsSquare())
            m_ExportOptions.SetBlockWidth(m_ExportOptions.GetBlockHeight());
        }
    else if (GetSelectedBlockType() == HRFBlockType::STRIP)
        m_ExportOptions.SetBlockHeight(m_pStripCapability->ValidateHeight(m_ExportOptions.GetBlockHeight()));
    else if (GetSelectedBlockType() == HRFBlockType::IMAGE)
        m_ExportOptions.SetBlockHeight(GetImageHeight());
    else if (GetSelectedBlockType() == HRFBlockType::LINE)
        m_ExportOptions.SetBlockHeight(1);
    else
        m_ExportOptions.SetBlockHeight(0);
    }

//-----------------------------------------------------------------------------
// GetBlockWidth
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetBlockWidth() const
    {
    return m_ExportOptions.GetBlockWidth();
    }

//-----------------------------------------------------------------------------
// GetBlockHeight
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetBlockHeight() const
    {
    return m_ExportOptions.GetBlockHeight();
    }

//-----------------------------------------------------------------------------
// GetMinimumBlockWidth
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetMinimumBlockWidth() const
    {
    return m_MinBlockWidth;
    }

//-----------------------------------------------------------------------------
// GetMinimumBlockHeight
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetMinimumBlockHeight() const
    {
    return m_MinBlockHeight;
    }

//-----------------------------------------------------------------------------
// GetBlockWidthIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetBlockWidthIncrementStep() const
    {
    return m_BlockWidthIncrementStep;
    }

//-----------------------------------------------------------------------------
// GetBlockHeightIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetBlockHeightIncrementStep() const
    {
    return m_BlockHeightIncrementStep;
    }

//-----------------------------------------------------------------------------
// CountBlockWidthIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountBlockWidthIncrementStep() const
    {
    return m_CountBlockWidthIncrementStep;
    }

//-----------------------------------------------------------------------------
// CountBlockHeightIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountBlockHeightIncrementStep() const
    {
    return m_CountBlockHeightIncrementStep;
    }

//-----------------------------------------------------------------------------
// SelectBlockWidthIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectBlockWidthIncrementStep(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountBlockWidthIncrementStep());

    SetBlockWidth(m_MinBlockWidth + (pi_Index * GetBlockWidthIncrementStep()));
    }

//-----------------------------------------------------------------------------
// SelectBlockHeightIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectBlockHeightIncrementStep(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountBlockHeightIncrementStep());

    SetBlockHeight(m_MinBlockHeight + (pi_Index * GetBlockHeightIncrementStep()));
    }

//-----------------------------------------------------------------------------
// GetSelectedBlockWidthIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedBlockWidthIncrementStep() const
    {
    return ((GetBlockWidth() - m_MinBlockWidth) / GetBlockWidthIncrementStep());
    }

//-----------------------------------------------------------------------------
// GetSelectedBlockHeightIncrementStep
// Block Type  interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedBlockHeightIncrementStep() const
    {
    return ((GetBlockHeight() - m_MinBlockHeight) / GetBlockHeightIncrementStep());
    }

//-----------------------------------------------------------------------------
// CountSubResBlockType
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountSubResBlockType() const
    {
    HPRECONDITION(m_pSelectedSubResCodecCapability != 0);
    return (uint32_t)m_ListOfValidSubResBlockType.size();
    }

//-----------------------------------------------------------------------------
// GetSubResBlockType
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
HRFBlockType HRFImportExport::GetSubResBlockType(uint32_t pi_Index) const
    {
    HPRECONDITION((pi_Index < CountBlockType()) && (m_ListOfValidSubResBlockType.size() > 0));

    return m_ListOfValidSubResBlockType[pi_Index];
    }

//-----------------------------------------------------------------------------
// SelectSubResBlockType
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResBlockType(HRFBlockType pi_BlockType)
    {
    // Set the new block type only if we support it
    HPRECONDITION(m_pSelectedSubResCodecCapability->SupportsBlockType(pi_BlockType));

    if (((pi_BlockType == HRFBlockType::TILE)  && (m_pSubResTileCapability)) ||
        ((pi_BlockType == HRFBlockType::LINE)  && (m_pSubResLineCapability)) ||
        ((pi_BlockType == HRFBlockType::STRIP) && (m_pSubResStripCapability)) ||
        ((pi_BlockType == HRFBlockType::IMAGE) && (m_pSubResImageCapability)))
        {
        m_ExportOptions.SetSubResBlockType(pi_BlockType);

        // **** TO DO **** DG
        // UpdateSubResBlockValues();
        //
        // SetSubResBlockWidth(256);
        // SetSubResBlockHeight(256);
        }
    }

//-----------------------------------------------------------------------------
// SelectSubResBlockTypeByIndex
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectSubResBlockTypeByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountSubResBlockType());
    SelectSubResBlockType(GetSubResBlockType(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResBlockType
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
HRFBlockType HRFImportExport::GetSelectedSubResBlockType() const
    {
    return m_ExportOptions.GetSubResBlockType();
    }

//-----------------------------------------------------------------------------
// GetSelectedSubResBlockTypeIndex
// Sub resolution Block Interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedSubResBlockTypeIndex() const
    {
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountBlockType()) && (IndexFound == 0); Index++)
        {
        if (m_ExportOptions.GetSubResBlockType() == GetSubResBlockType(Index))
            {
            IndexFound = Index;
            }
        }
    return IndexFound;
    }

//-----------------------------------------------------------------------------
// CountEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountEncoding() const
    {
    return (uint32_t)m_ListOfValidEncodingType.size();
    }

//-----------------------------------------------------------------------------
// GetEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
HRFEncodingType HRFImportExport::GetEncoding(uint32_t pi_Index) const
    {
    return m_ListOfValidEncodingType[pi_Index];
    }

//-----------------------------------------------------------------------------
// SelectEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectEncoding(HRFEncodingType pi_EncodingType)
    {
    bool Found = false;

    for (uint32_t Index=0; (Index < CountEncoding()) && (!Found); Index++)
        {
        if (pi_EncodingType == GetEncoding(Index))
            {
            Found = true;
            m_ExportOptions.SetEncoding(pi_EncodingType);
            }
        }
    }

//-----------------------------------------------------------------------------
// SelectEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectEncodingByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountEncoding());

    SelectEncoding(GetEncoding(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
HRFEncodingType HRFImportExport::GetSelectedEncoding() const
    {
    return m_ExportOptions.GetEncoding();
    }

//-----------------------------------------------------------------------------
// GetSelectedEncoding
// Encoding type interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedEncodingIndex() const
    {
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountEncoding()) && (IndexFound == 0); Index++)
        {
        if (m_ExportOptions.GetEncoding() == GetEncoding(Index))
            {
            IndexFound = Index;
            }
        }
    return IndexFound;
    }

//-----------------------------------------------------------------------------
// Tag interface
// CountTag
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountTag() const
    {
    return (uint32_t)m_ListOfValidTag.size();
    }

//-----------------------------------------------------------------------------
// Tag interface
// FindTagCP
//-----------------------------------------------------------------------------
const HFCPtr<HPMGenericAttribute> HRFImportExport::GetTag(uint32_t pi_Index) const 
    {
    HPRECONDITION(pi_Index < CountTag());
    return m_ListOfValidTag[pi_Index];
    }

//-----------------------------------------------------------------------------
// Tag interface
// HasTag
//-----------------------------------------------------------------------------
bool HRFImportExport::HasTag(HPMGenericAttribute const& pi_tag) const 
    {
    for (uint32_t Index=0; (Index < CountTag()); Index++)
        {
        if (pi_tag.GetID() == (*m_ListOfValidTag[Index]).GetID())
            {
            return true;
            }
        }
    return false;
    }

//-----------------------------------------------------------------------------
// Tag interface
// SetTag
//-----------------------------------------------------------------------------
void HRFImportExport::SetTag(const HFCPtr<HPMGenericAttribute>& pi_rpTag)
    {
    for (uint32_t Index=0; (Index < CountTag()); Index++)
        {
        if (pi_rpTag->SameAttributeAs((*m_ListOfValidTag[Index])))
            {
            m_ExportOptions.SetTag((HFCPtr<HPMGenericAttribute>&) pi_rpTag);
            break;
            }
        }
    }

//-----------------------------------------------------------------------------
// Get the metadata container list
// GetMetaDataContainerList
//-----------------------------------------------------------------------------
const HMDMetaDataContainerList& HRFImportExport::GetMetaDataContainerList() const
    {
    return m_ExportOptions.GetMetaDataContainerList();
    }

//-----------------------------------------------------------------------------
// protected: Tag interface
// Clear the list of ExportOptions:Tag
//-----------------------------------------------------------------------------
void HRFImportExport::ClearTagList()
    {
    m_ExportOptions.m_TagList.Clear();
    }

//-----------------------------------------------------------------------------
// protected: SetMetaDataContainer
// Add the metadata container to the page descriptor
//-----------------------------------------------------------------------------
void HRFImportExport::SetMetaDataContainer(HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer)
    {
    m_ExportOptions.SetMetaDataContainer(pi_rpMDContainer);
    }

//-----------------------------------------------------------------------------
// CountGeoreferenceFormats
// GeoreferenceFormat interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::CountGeoreferenceFormats() const
    {
    return (uint32_t)m_ListOfValidGeoreferenceFormats.size();
    }

//-----------------------------------------------------------------------------
// GetGeoreferenceFormat
// GeoreferenceFormat interface
//-----------------------------------------------------------------------------
HRFGeoreferenceFormat HRFImportExport::GetGeoreferenceFormat(uint32_t pi_Index) const
    {
    HPRECONDITION(pi_Index < CountGeoreferenceFormats());

    return m_ListOfValidGeoreferenceFormats[pi_Index];
    }

/** ------------------------------------------------------------------------------------------
    This function is used to select the georeference format to aply for this export. This
    format can be either in a hgr file, in a world file or directly in the file format (this is
    in image). If georeference is set to in image and the destination file format do not support
    transformation model, no georeference will be used for this export.
    @end

    @h3{Note:}
    If georeference format is set to in image and source has a tranformation model and the selected
    format do not support transfo model, we have to force resample option.
    As example, if you try to export from a raster with transformation model to a JPEG with no HGR or
    World file, you have no way to store the source's model so you need to resample.
    @end

    @param pi_Format  Georeference format to used for this export.
    @end

    @see HRFImportExport#SetResample(bool)
    @see HRFImportExport#SetResampleIsForce(bool)
    @end
    ------------------------------------------------------------------------------------------
*/
void HRFImportExport::SelectGeoreferenceFormat(HRFGeoreferenceFormat pi_Format)
    {
    HASSERT ((pi_Format == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE) ||
             (pi_Format == HRFGeoreferenceFormat::GEOREFERENCE_IN_HGR)   ||
             (pi_Format == HRFGeoreferenceFormat::GEOREFERENCE_IN_WORLD_FILE));

    m_ExportOptions.SetGeoreferenceFormat(pi_Format);

    // If source has a tranfo model and georeference format is in image and the selected format do not support
    // transfo model, we have to force resample.
    if ((m_SourceHasTransfo) &&
        (pi_Format == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE) &&
        (!m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID,
                                                                 HFC_CREATE_ONLY)))
        {
        SetResample(true);
        SetResampleIsForce(true);
        }
    else
        SetResampleIsForce(false);
    }

//-----------------------------------------------------------------------------
// SelectGeoreferenceFormatByIndex
// GeoreferenceFormat interface
//-----------------------------------------------------------------------------
void HRFImportExport::SelectGeoreferenceFormatByIndex(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < CountGeoreferenceFormats());

    SelectGeoreferenceFormat(GetGeoreferenceFormat(pi_Index));
    }

//-----------------------------------------------------------------------------
// GetSelectedGeoreferenceFormat
// GeoreferenceFormat interface
//-----------------------------------------------------------------------------
HRFGeoreferenceFormat HRFImportExport::GetSelectedGeoreferenceFormat() const
    {
    return m_ExportOptions.GetGeoreferenceFormat();
    }

//-----------------------------------------------------------------------------
// GetSelectedGeoreferenceFormatIndex
// GeoreferenceFormat interface
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::GetSelectedGeoreferenceFormatIndex() const
    {
    uint32_t IndexFound = 0;

    for (uint32_t Index=0; (Index < CountGeoreferenceFormats()) && (IndexFound == 0); Index++)
        {
        if (m_ExportOptions.GetGeoreferenceFormat() == GetGeoreferenceFormat(Index))
            {
            IndexFound = Index;
            }
        }

    return IndexFound;
    }


/** ------------------------------------------------------------------
    SetGeocoding
    ------------------------------------------------------------------
*/
void HRFImportExport::SetGeocoding(GeoCoordinates::BaseGCSCP pi_pGeocoding)
    {
    m_ExportOptions.SetGeocoding(pi_pGeocoding);
    }

/** ------------------------------------------------------------------
    GetGeocodingCP
    ------------------------------------------------------------------
*/
GeoCoordinates::BaseGCSCP HRFImportExport::GetGeocodingCP() const
    {
    return m_ExportOptions.GetGeocodingCP();
    }

	

//-----------------------------------------------------------------------------
// GetRGBDefaultColor
//-----------------------------------------------------------------------------
void const* HRFImportExport::GetRGBDefaultColor() const
    {
    return m_aRGBDefaultColor;
    }


//-----------------------------------------------------------------------------
// SetRGBDefaultColor
//-----------------------------------------------------------------------------
void HRFImportExport::SetRGBDefaultColor(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);

    memcpy(m_aRGBDefaultColor, pi_pValue, 3);
    }

//-----------------------------------------------------------------------------
// SetRGBADefaultColor
//-----------------------------------------------------------------------------
void HRFImportExport::SetRGBADefaultColor(const void* pi_pValue)
    {
    HPRECONDITION(pi_pValue != 0);

    memcpy(m_aRGBADefaultColor, pi_pValue, 4);
    m_isRGBADefaultColorSet=true;
    }


//-----------------------------------------------------------------------------
// CreatePageFromSelectedValues
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFImportExport::CreatePageFromSelectedValues()
    {
    HFCPtr<HRFPageDescriptor>                      pPageDesc;
    HRFPageDescriptor::ListOfResolutionDescriptor  ResDescList;
    HAutoPtr<HGFResolutionDescriptor>              pPyramidDesc;
    bool                                           IsSinglePixelType = true;

    if (GetSelectedEncoding() == HRFEncodingType::MULTIRESOLUTION)
        {
        HFCPtr<HRFMultiResolutionCapability> pMultiResCapability;
        pMultiResCapability = static_cast<HRFMultiResolutionCapability*>(m_pSelectedFileFormatCapabilities->
                               GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

        IsSinglePixelType = pMultiResCapability->IsSinglePixelType();

        pPyramidDesc = new HGFResolutionDescriptor(m_ExportOptions.GetImageWidth(),
                                                   m_ExportOptions.GetImageHeight(),
                                                   pMultiResCapability->GetSmallestResWidth(),
                                                   pMultiResCapability->GetSmallestResHeight(),
                                                   (GetSelectedBlockType() == HRFBlockType::TILE));
        }
    // Othewise Encoding is HRFEncodingType::ENCODING_STANDARD
    else
        pPyramidDesc = new HGFResolutionDescriptor(m_ExportOptions.GetImageWidth(),
                                                   m_ExportOptions.GetImageHeight());

    // Get the default interleave type from the export format
    HFCPtr<HRFInterleaveCapability> pInterleaveCapability;
    pInterleaveCapability = static_cast<HRFInterleaveCapability*>(m_pSelectedFileFormatCapabilities->
                             GetCapabilityOfType(HRFInterleaveCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

    // Get the default interlace from the export format
    bool IsInterlace = false;
    if (GetSelectedEncoding() == HRFEncodingType::PROGRESSIVE)
        IsInterlace = (m_pSelectedFileFormatCapabilities->GetCapabilityOfType(HRFInterlaceCapability::CLASS_ID, HFC_WRITE_ONLY) != 0);

    // Get the default ScanlineOrientation type from the export format
    HFCPtr<HRFScanlineOrientationCapability> pScanlineOrientationCapability;
    pScanlineOrientationCapability = static_cast<HRFScanlineOrientationCapability*>(m_pSelectedFileFormatCapabilities->
                                      GetCapabilityOfType(HRFScanlineOrientationCapability::CLASS_ID, HFC_CREATE_ONLY).GetPtr());

    // Create the selected pixel type
    HFCPtr<HRPPixelType> pPixelType = GetPixelType();

    Byte defaultPixelValue[HRPPixelType::MAX_PIXEL_BYTES];

    // We need a converter to transform RGB <<default>> value to one compatible with selected pixel type
    HFCPtr<HRPPixelConverter> pConverter;

    if (m_isRGBADefaultColorSet)
        {
        // Get this converter (V32R8G8B8A8 -> selected pixel type)
        pConverter = HRPPixelTypeV32R8G8B8A8().GetConverterTo(pPixelType);

        // Convert color
        pConverter->Convert(m_aRGBADefaultColor, defaultPixelValue);
        }
    else
        {
        // Get this converter (V24R8G8B8 -> selected pixel type)
        pConverter = HRPPixelTypeV24R8G8B8().GetConverterTo(pPixelType);

        // Convert color
        pConverter->Convert(m_aRGBDefaultColor, defaultPixelValue);
        }

    // Set pixel type default value
    pPixelType->SetDefaultRawData(defaultPixelValue);

    // Create the selected Sub pixel type
    HFCPtr<HRPPixelType> pSubPixelType;

    if (!IsSinglePixelType)
        {
        if (m_BuildedSubResPixelTypeSetted ||
            GetSelectedPixelType() != GetSelectedSubResPixelType()) // compare the HCLASS_ID
            pSubPixelType = GetSubResPixelType();
        else
            pSubPixelType = pPixelType;

        if (m_isRGBADefaultColorSet)
            {
            // Get converter (V32R8G8B8A8 -> selected pixel type)
            pConverter = HRPPixelTypeV32R8G8B8A8().GetConverterTo(pSubPixelType);

            // Convert color
            pConverter->Convert(m_aRGBADefaultColor,defaultPixelValue);

            // Set pixel type default value
            pSubPixelType->SetDefaultRawData(defaultPixelValue);
            }
        else
            {
            // Get converter (V24R8G8B8 -> selected pixel type)
            pConverter = HRPPixelTypeV24R8G8B8().GetConverterTo(pSubPixelType);

            // Convert color
            pConverter->Convert(m_aRGBDefaultColor,defaultPixelValue);

            // Set pixel type default value
            pSubPixelType->SetDefaultRawData(defaultPixelValue);
            }
        }

    HFCPtr<HCDCodec> pMainCodec;

    if (CountCodecs() > 0)
        {
        // Add the selected codec to the list
        // If the format is block oriented compression or not we add the selected codecs to the list
        pMainCodec = GetSelectedCodecSample();

        if (pMainCodec->IsCompatibleWith(HCDCodecErMapperSupported::CLASS_ID))
            ((HFCPtr<HCDCodecErMapperSupported>&)pMainCodec)->SetCompressionRatio((uint16_t)m_ExportOptions.GetCompressionRatio());
        if (pMainCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
            ((HFCPtr<HCDCodecIJG>&)pMainCodec)->SetQuality((Byte)m_ExportOptions.GetCompressionQuality());
        else if (pMainCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
            ((HFCPtr<HCDCodecFlashpix>&)pMainCodec)->SetQuality((Byte)m_ExportOptions.GetCompressionQuality());
        }
    else
        {
        pMainCodec = new HCDCodecIdentity();
        }

    // Create the codec list for the selected PixelType
    HFCPtr<HCDCodec> pSubResCodec;

    if ((!IsSinglePixelType) && (CountSubResCodecs() > 0))
        {
        // Add the selected codec to the list
        // If the format is block oriented compression or not we add the selected codecs to the list
        pSubResCodec = GetSelectedSubResCodecSample();

        if (pSubResCodec->IsCompatibleWith(HCDCodecErMapperSupported::CLASS_ID))
            ((HFCPtr<HCDCodecErMapperSupported>&)pSubResCodec)->SetCompressionRatio((uint16_t)m_ExportOptions.GetSubResCompressionRatio());
        else if (pSubResCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
            ((HFCPtr<HCDCodecIJG>&)pSubResCodec)->SetQuality((Byte)m_ExportOptions.GetSubResCompressionQuality());
        else if (pSubResCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
            ((HFCPtr<HCDCodecFlashpix>&)pSubResCodec)->SetQuality((Byte)m_ExportOptions.GetSubResCompressionQuality());
        }
    else
        {
        pSubResCodec = new HCDCodecIdentity();
        }

    // Create the resolution list.
    for (uint16_t ResCount = 0 ; ResCount < pPyramidDesc->CountResolutions() ; ResCount++)
        {
        HFCPtr<HRFResolutionDescriptor> pResDesc;

        // Set the block size in function of the Block type.
        uint32_t           BlockWidth=0;
        uint32_t           BlockHeight=0;
        HRFBlockAccess      BlockAccess;
        HFCPtr<HCDCodec>    pThisResCodec = pMainCodec;

        if (GetSelectedBlockType() ==  HRFBlockType::LINE)
            {
            BlockWidth  = pPyramidDesc->GetWidth(ResCount);
            BlockHeight = 1;
            BlockAccess = m_pLineCapability->GetBlockAccess();
            }
        else if (GetSelectedBlockType() ==  HRFBlockType::TILE)
            {
            BlockWidth  = GetBlockWidth();
            BlockHeight = GetBlockHeight();
            BlockAccess = m_pTileCapability->GetBlockAccess();
            }
        else if (GetSelectedBlockType() ==  HRFBlockType::STRIP)
            {
            BlockWidth  = pPyramidDesc->GetWidth(ResCount);
            BlockHeight = GetBlockHeight();
            BlockAccess = m_pStripCapability->GetBlockAccess();
            }
        else if (GetSelectedBlockType() ==  HRFBlockType::IMAGE)
            {
            BlockWidth  = pPyramidDesc->GetWidth(ResCount);
            BlockHeight = pPyramidDesc->GetHeight(ResCount);
            BlockAccess = m_pImageCapability->GetBlockAccess();
            }
        else
            HASSERT(0);

        HRFDownSamplingMethod DownSamplingMethod;

        if (m_OverrideDownSamplingMethod == HRFDownSamplingMethod::NONE)
            {
            if (CountDownSamplingMethod() > 0)
                DownSamplingMethod = GetSelectedDownSamplingMethod();
            else
                DownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
            }
        else
            DownSamplingMethod = m_OverrideDownSamplingMethod;

        if ((!IsSinglePixelType) && (ResCount > 0))
            {
            pPixelType = pSubPixelType;
            pThisResCodec = pSubResCodec;

            if (m_OverrideSubResDownSamplingMethod == HRFDownSamplingMethod::NONE)
                {
                if (CountSubResDownSamplingMethod() > 0)
                    DownSamplingMethod = GetSelectedSubResDownSamplingMethod();
                else
                    DownSamplingMethod = HRFDownSamplingMethod::NEAREST_NEIGHBOUR;
                }
            else
                DownSamplingMethod = m_OverrideSubResDownSamplingMethod;
            }

        // Create a resolution descriptor.
        pResDesc = new HRFResolutionDescriptor(HFC_CREATE_ONLY,
                                               m_pSelectedFileFormatCapabilities,
                                               pPyramidDesc->GetResolution(ResCount),
                                               pPyramidDesc->GetResolution(ResCount),
                                               pPixelType,
                                               pThisResCodec,
                                               BlockAccess,
                                               BlockAccess,
                                               pScanlineOrientationCapability->GetScanlineOrientation(),
                                               pInterleaveCapability->GetInterleaveType(),
                                               IsInterlace,
                                               pPyramidDesc->GetWidth(ResCount),
                                               pPyramidDesc->GetHeight(ResCount),
                                               BlockWidth,
                                               BlockHeight,
                                               0,
                                               GetSelectedBlockType(),
                                               1,
                                               8,
                                               DownSamplingMethod);

        // Add the resolution descriptor to the resolution descriptor list.
        ResDescList.push_back(pResDesc);
        }

    // Create the page descriptor.
    HPMAttributeSet TagList(m_ExportOptions.GetTagList());
    pPageDesc = new HRFPageDescriptor (HFC_CREATE_ONLY,
                                       m_pSelectedFileFormatCapabilities,
                                       ResDescList,
                                       0,                             // RepresentativePalette
                                       0,                             // Histogram
                                       0,                             // Thumbnail
                                       0,                             // ClipShape
                                       0,                             // TransfoModel
                                       0,                             // Filters
                                       &TagList,                      // Tag list
                                       0);                            // Duration

    //  pPageDesc
    if (GetMetaDataContainerList().GetNbContainers() > 0)
        {
        //All metadata containers could be seen as modified before they are
        //saved to the new raster file.
        HFCPtr<HMDMetaDataContainerList> pMDContainers(new HMDMetaDataContainerList(GetMetaDataContainerList()));

        pMDContainers->SetModificationStatus(true);
        pPageDesc->SetListOfMetaDataContainer(pMDContainers, true);
        }

    //Set geocoding in page descriptor
    pPageDesc->SetGeocoding(GetGeocodingCP());

    return pPageDesc;
    }


//-----------------------------------------------------------------------------
// ComposeFilenameWithOptions
//-----------------------------------------------------------------------------
Utf8String HRFImportExport::ComposeFilenameWithOptions() const
    {
    Utf8String ComposedFilename;

    Utf8Char  TempValue[1024];

    // Pixel Type
    BeStringUtilities::Snprintf(TempValue, "%lu", GetSelectedPixelType());
    ComposedFilename += Utf8String("PixelType=") + Utf8String(TempValue);

    // Codec
    BeStringUtilities::Snprintf(TempValue, "%lu", GetSelectedCodec());
    ComposedFilename += Utf8String("_Codec=") + Utf8String(TempValue);

    BeStringUtilities::Snprintf(TempValue, "%lu", GetSelectedCompressionQuality());
    ComposedFilename += Utf8String("_Quality=") + Utf8String(TempValue);

    // Image size
    if (GetResample() == true)
        ComposedFilename += Utf8String("Resample=true");
    else
        ComposedFilename += Utf8String("Resample=false");

    BeStringUtilities::Snprintf(TempValue, "%lu", GetImageWidth());
    ComposedFilename += Utf8String("_ImageSize(") + Utf8String(TempValue) + Utf8String(", ");

    BeStringUtilities::Snprintf(TempValue, "%lu", GetImageHeight());
    ComposedFilename += Utf8String(TempValue) + Utf8String(")");


    // Block size
    BeStringUtilities::Snprintf(TempValue, "%lu", GetBlockWidth());
    if (GetSelectedBlockType() == HRFBlockType::TILE)
        ComposedFilename += Utf8String("_TileSize(") + Utf8String(TempValue) + Utf8String(", ");
    else if (GetSelectedBlockType() == HRFBlockType::STRIP)
        ComposedFilename += Utf8String("_StripSize(") + Utf8String(TempValue) + Utf8String(", ");
    else if (GetSelectedBlockType() == HRFBlockType::LINE)
        ComposedFilename += Utf8String("_LineSize(") + Utf8String(TempValue) + Utf8String(", ");
    else if (GetSelectedBlockType() == HRFBlockType::IMAGE)
        ComposedFilename += Utf8String("_ImagekSize(") + Utf8String(TempValue) + Utf8String(", ");

    BeStringUtilities::Snprintf(TempValue, "%lu", GetBlockHeight());
    ComposedFilename += Utf8String(TempValue) + Utf8String(")");

    // Encoding
    if (GetSelectedEncoding() == HRFEncodingType::MULTIRESOLUTION)
        ComposedFilename += Utf8String("_Encoding=MultiResolution");
    else if (GetSelectedEncoding() == HRFEncodingType::STANDARD)
        ComposedFilename += Utf8String("_Encoding=Standard");
    else if (GetSelectedEncoding() == HRFEncodingType::PROGRESSIVE)
        ComposedFilename += Utf8String("_Encoding=Progressive");

// Do we want to try all formats for each pixel type, compression, etc...
#if 0
    // Georeference format
    if (GetSelectedGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE)
        ComposedFilename += Utf8String("_Georeference=Image");
    else if (GetSelectedGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_HGR)
        ComposedFilename += Utf8String("_Georeference=HGR");
    else if (GetSelectedGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_WORLD_FILE)
        ComposedFilename += Utf8String("_Georeference=WorldFile");
#endif

    return ComposedFilename;
    }

//-----------------------------------------------------------------------------
// ExportToAllOptions
// use by the tester
//-----------------------------------------------------------------------------
uint32_t HRFImportExport::ExportToAllOptions(const HFCPtr<HFCURL>& pi_rpURLPath)
    {
    uint32_t Result = 0;

    // Do we want to try all georeference formats for each pixel type, compression, etc...
    // If we do, add a for loop on the georeference formats list.

    for (uint32_t PixelIndex=0; PixelIndex < CountPixelType(); PixelIndex++)
        {
        SelectPixelTypeByIndex(PixelIndex);

        for (uint32_t CodecIndex=0; CodecIndex < CountCodecs(); CodecIndex++)
            {
            SelectCodecByIndex(CodecIndex);

            for (uint32_t BlockIndex=0; BlockIndex < CountBlockType(); BlockIndex++)
                {
                SelectBlockTypeByIndex(BlockIndex);

                if (CountCompressionStep() > 1)
                    {
                    // By default we test 4 quality of compression
                    int32_t NumberOfLevels = MIN(3, CountCompressionStep() - 1);
                    for (int32_t Level = 1; Level <= NumberOfLevels; Level ++)
                        {
                        SelectCompressionQuality(MIN(CountCompressionStep() - 1, (CountCompressionStep() / NumberOfLevels) * Level));

                        // Add the path to the Booster url
                        Utf8String ExportPath(pi_rpURLPath->GetURL());

                        // Add the / if necessary
                        if ((ExportPath[ExportPath.size() - 1] != '/') && (ExportPath[ExportPath.size() - 1] != '\\'))
                            ExportPath += "\\";

                        ExportPath += ComposeFilenameWithOptions();

                        SelectExportFilename(HFCURL::Instanciate(ExportPath));
                        try
                            {
                            StartExport();
                            }
                        catch(...)
                            {
                            Result++;
                            }
                        }
                    }
                else if (CountCompressionRatioStep() > 1)
                    {
                    // By default we test 4 quality of compression
                    int32_t NumberOfLevels = MIN(3, CountCompressionRatioStep() - 1);
                    for (int32_t Level = 1; Level <= NumberOfLevels; Level ++)
                        {
                        SelectCompressionRatio(MIN(CountCompressionRatioStep() - 1, (CountCompressionRatioStep() / NumberOfLevels) * Level));

                        // Add the path to the Booster url
                        Utf8String ExportPath(pi_rpURLPath->GetURL());

                        // Add the / if necessary
                        if ((ExportPath[ExportPath.size() - 1] != '/') && (ExportPath[ExportPath.size() - 1] != '\\'))
                            ExportPath += "\\";

                        ExportPath += ComposeFilenameWithOptions();

                        SelectExportFilename(HFCURL::Instanciate(ExportPath));
                        try
                            {
                            StartExport();
                            }
                        catch(...)
                            {
                            Result++;
                            }
                        }
                    }
                else
                    {
                    // Add the path to the Booster url
                    Utf8String ExportPath(pi_rpURLPath->GetURL());

                    // Add the / if necessary
                    if ((ExportPath[ExportPath.size() - 1] != '/') && (ExportPath[ExportPath.size() - 1] != '\\'))
                        ExportPath += "\\";

                    ExportPath += ComposeFilenameWithOptions();

                    SelectExportFilename(HFCURL::Instanciate(ExportPath));
                    try
                        {
                        StartExport();
                        }
                    catch(...)
                        {
                        Result++;
                        }
                    }
                }
            }
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// CreateFileFromSelectedValues
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFImportExport::CreateFileFromSelectedValues()
    {
    HFCPtr<HRFRasterFile> pOutputFile;

    const HRFRasterFileCreator* pRasterFileCreator(GetSelectedExportFileFormat());

    // Create output directory if it doesn't exits.  Otherwise file creation will fail.
    if (GetSelectedExportFilename()->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        BeFileName outName(static_cast<HFCURLFile const*>(GetSelectedExportFilename().GetPtr())->GetAbsoluteFileName());
        BeFileName outDir = outName.GetDirectoryName();
        if (!outDir.DoesPathExist())
            BeFileName::CreateNewDirectory(outDir.c_str());
        }

    // Create the basic destination file
    // Is the destination is the COM ECW(ERMapper) (1429) OR Is the destination is COM Jpeg2000File (1477)
    if (pRasterFileCreator->GetRasterFileClassID() == HRFFileId_Ecw/*HRFEcwFile::CLASS_ID*/ || 
        pRasterFileCreator->GetRasterFileClassID() == HRFFileId_Jpeg2000/*HRFJpeg2000File::CLASS_ID*/)
        {
        pOutputFile = pRasterFileCreator->Create(GetSelectedExportFilename(), HFC_CREATE_ONLY);
        }
    else
        // TR 185890: Restore code of HRFImportExport.cpp version 1.14 that was lost in version 1.15.
        //            We need write access for format like HMR so we can compute the histogram after the export is completed.
        //            Adding write access will also give us read access. See HRSObjectStore::Constructor()
        pOutputFile = pRasterFileCreator->Create(GetSelectedExportFilename(), HFC_WRITE_AND_CREATE);

    if (pOutputFile != 0)
        {
        // If source file has a transfo model and the output file doesn't support TransfoModel and georef format is
        // set to GEOREFERENCE_IN_IMAGE. Resampling must be force.
        HASSERT (((m_SourceHasTransfo) &&
                  (!pOutputFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_CREATE_ONLY)) &&
                  ((m_ExportOptions.GetGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE)) &&
                  GetResampleIsForce()) ||
                 ((m_SourceHasTransfo) ||
                  (!pOutputFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_CREATE_ONLY)) ||
                  (m_ExportOptions.GetGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE)));


        // Add the page before any adapter.
        if (! pOutputFile->AddPage(CreatePageFromSelectedValues()))
            throw HFCFileNotCreatedException(GetSelectedExportFilename()->GetURL());

        if (m_ExportOptions.GetGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_HGR)
            {
            HFCPtr<HFCURL> pPageFileName(HRFHGRPageFileCreator::GetInstance()->ComposeURLFor(GetSelectedExportFilename()));
            HFCPtr<HRFHGRPageFile> pPageFile(new HRFHGRPageFile(pPageFileName,
                                                                GetImageWidth(),
                                                                GetImageHeight(),
                                                                HFC_CREATE_ONLY));

            if (pPageFile != 0)
                {
                pOutputFile = new HRFRasterFilePageDecorator(pOutputFile, (HFCPtr<HRFPageFile>&) pPageFile);
                }
            else
                {
                throw HFCSisterFileNotCreatedException(pPageFileName->GetURL());
                }
            }
        else if (m_ExportOptions.GetGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_WORLD_FILE)
            {
            HFCPtr<HFCURL> pPageFileName(HRFTWFPageFileCreator::GetInstance()->ComposeURLFor(GetSelectedExportFilename()));
            HFCPtr<HRFTWFPageFile> pPageFile(new HRFTWFPageFile(pPageFileName,
                                                                HFC_CREATE_ONLY));
            if (pPageFile != 0)
                {
                pOutputFile = new HRFRasterFilePageDecorator(pOutputFile, (HFCPtr<HRFPageFile>&) pPageFile);
                }
            else
                {
                throw HFCSisterFileNotCreatedException(pPageFileName->GetURL());
                }
            }
        }
    else
        {
        throw HFCFileNotCreatedException(GetSelectedExportFilename()->GetURL());
        }

    return pOutputFile;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetResample(bool pi_Resampling)
    {
    m_ExportOptions.SetResample(pi_Resampling);

    // If resample is set then reset the width and height to
    // resample size.
    if (pi_Resampling)
        {
        // Image size section (Default resample size).
        SetMaintainAspectRatio(false);

        SetImageSizeIsLock(false);
        SetImageWidth((uint32_t)(GetDefaultResampleSize().GetX()));
        SetImageHeight((uint32_t)(GetDefaultResampleSize().GetY()));

        // Scaling Section (Default resample scaling).
        SetScaleFactorIsLock(false);
        SetScaleFactorX(GetDefaultResampleScaleFactorX());
        SetScaleFactorY(GetDefaultResampleScaleFactorY());

        SetMaintainAspectRatio(true);
        }
    // Else then reset the width and height to
    // original size.
    else
        {
        // Image size section (Default original size).
        SetMaintainAspectRatio(false);

        SetImageSizeIsLock(false);
        SetImageWidth((uint32_t)(GetOriginalSize().GetX()));
        SetImageHeight((uint32_t)(GetOriginalSize().GetY()));
        SetImageSizeIsLock(true);

        SetScaleFactorIsLock(false);
        SetScaleFactorX(1.0);
        SetScaleFactorY(1.0);
        SetScaleFactorIsLock(true);
        }
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @return Description of the return value
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::GetResample() const
    {
    return m_ExportOptions.GetResample();
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    If Original size is set to zero, force resample is force.

    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param pi_Resampling ....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetResampleIsForce(bool pi_Resampling)
    {
    if (m_OriginalSize.GetX() != 0.0 && m_OriginalSize.GetY() != 0.0)
        m_ResampleIsForce = pi_Resampling;
    else
        m_ResampleIsForce = true;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @return true if resample is force, this means that we have no other
            that resample the destination.
    @end

    @see HRFExportOption
    @see ThisRelatedClass#ThisReleatedFunction()
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::GetResampleIsForce() const
    {
    return m_ResampleIsForce;
    }

/** ------------------------------------------------------------------

    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @return Description of the return value
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
double HRFImportExport::GetDefaultResampleScaleFactorX() const
    {
    return m_DefaultResampleScaleFactorX;
    }


/** ------------------------------------------------------------------

    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @return Description of the return value
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
double HRFImportExport::GetDefaultResampleScaleFactorY() const
    {
    return m_DefaultResampleScaleFactorY;
    }

/** ------------------------------------------------------------------
    This function return the default original size for this export. This
    size is computed at the begining of the export process using ....
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @return Description of the return value
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
HGF2DPosition HRFImportExport::GetOriginalSize() const
    {
    return m_OriginalSize;
    }


/** ------------------------------------------------------------------
    This function return the default resample size for this export. This
    size is computed using  ...

    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @return Description of the return value
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
HGF2DPosition HRFImportExport::GetDefaultResampleSize() const
    {
    return m_DefaultResampleSize;
    }

//-----------------------------------------------------------------------------
// SetResamplingMethod
//-----------------------------------------------------------------------------
void HRFImportExport::SetResamplingMethod(const HFCPtr<HRPFilter>& pi_rFilter)
    {
    HPRECONDITION(pi_rFilter != 0);

    m_pResamplingFilter = pi_rFilter;
    }

//-----------------------------------------------------------------------------
// GetResamplingMethod
//-----------------------------------------------------------------------------
const HFCPtr<HRPFilter>& HRFImportExport::GetResamplingMethod() const
    {
    return m_pResamplingFilter;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::ImageSizeIsLock()
    {
    return m_ImageSizeIsLock;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetImageSizeIsLock(bool pi_Check)
    {
    m_ImageSizeIsLock = pi_Check;
    }


/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::ScaleFactorIsLock()
    {
    return m_ScaleFactorIsLock;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetScaleFactorIsLock(bool pi_Check)
    {
    m_ScaleFactorIsLock = pi_Check;
    }


/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::MaintainAspectRatioIsCheck()
    {
    return m_MaintainAspectRatio;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetMaintainAspectRatio(bool pi_Check)
    {
    m_MaintainAspectRatio = pi_Check;
    }

/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
void HRFImportExport::SetUseDestinationPaletteIfIndexed(bool pi_UseDestinationPalette)
    {
    m_UseDestinationPaletteIfIndexed = pi_UseDestinationPalette;
    }


/** ------------------------------------------------------------------
    My Function description
    @end

    @h3{Note:}
    Any notes or remarks on my function used
    @end

    @h3{Inheritance notes:}
    Any note for programmers who want to overwrite my function.
    @end

    @param .....
    @end

    @see MyRelatedClass
    @see ThisRelatedClass#ThisReleatedFunction()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyFunction.doc">
          This link description. </a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\MyLibDoc.doc">
          This link description. </a>}
    @end
    ------------------------------------------------------------------
*/
bool HRFImportExport::UseDestinationPaletteIfIndexed() const
    {
    return m_UseDestinationPaletteIfIndexed;
    }


//-----------------------------------------------------------------------------
// ValidateExportSize
// Check if it is an uncompressed export, and if so,
// validate that the size of the export isn't greater than the maximum
// file's size supported by the selected format.
//-----------------------------------------------------------------------------
void HRFImportExport::ValidateUncompressedExportSize(HFCPtr<HRFRasterFile>& pi_prDstRasterFile,
                                                     bool*                    po_pIsCompressedImg) const
    {
    HPRECONDITION(pi_prDstRasterFile != 0);

    uint64_t                UncompressedDataSize = 0;
    bool                  HasCompressedRes = false;

    static const uint64_t sMaximumHeaderSize = (uint64_t)pow(2.0, 22); //Tolerance value used as an estimate
    //of the header size (4194304 bytes).

    uint64_t MaxImgDataSize = pi_prDstRasterFile->GetMaxFileSizeInBytes() - sMaximumHeaderSize;

    for (uint16_t ResNumber=0; ResNumber < pi_prDstRasterFile->GetPageDescriptor(0)->CountResolutions(); ++ResNumber)
        {
        const HFCPtr<HCDCodec>& pCodec = pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec();

        if (pCodec != 0 && pCodec->GetClassID() != HCDCodecIdentity::CLASS_ID)
            {
            HasCompressedRes = true;
            break;
            }

        UncompressedDataSize += pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(ResNumber)->GetSizeInBytes();

        if (UncompressedDataSize > MaxImgDataSize)
            {
            throw HFCFileOutOfRangeException(pi_prDstRasterFile->GetURL()->GetURL());
            }
        }

#ifdef _HMR_DEBUG
    if (HasCompressedRes == true)
        {
        uint64_t FirstResSize = pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetResolutionSize();
        HFCPtr<HCDCodec>        pFirstResCodec      = pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec();
        uint64_t                SubResTotalSize     = 0;

        for (uint16_t ResNumber=1; (ResNumber < pi_prDstRasterFile->GetPageDescriptor(0)->CountResolutions()) && IsOnlyOneCodecUsed; ResNumber++)
            {
            if (pFirstResCodec->GetClassID() == pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(ResNumber)->GetCodec().GetClassID())
                {
                SubResTotalSize += pi_prDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(ResNumber)->GetResolutionSize();
                }
            else
                {
                HASSERT(!"Not Supported : Export size estimatation for resolutions with different compressions"); //
                }
            }
        }
#endif

    if (po_pIsCompressedImg != 0)
        {
        *po_pIsCompressedImg = HasCompressedRes;
        }
    }
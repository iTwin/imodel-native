//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFxChFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFxChFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRFxChFile.h>
#include <Imagepp/all/h/HRFxChEditor.h>

#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFHMRFile.h>

#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRPPixelType.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>

#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>


#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <BeXml/BeXml.h>



/** ---------------------------------------------------------------------------
    URL composition utility.

      @return HFCPtr<HFCURLFile>
    ---------------------------------------------------------------------------
 */
static HFCPtr<HFCURL> ComposeChannelURL(const HFCPtr<HFCURLFile>& pi_rpXMLFileURL,
                                        const Utf8String             pi_ChannelStr)
    {
    HPRECONDITION(pi_rpXMLFileURL != 0);
    HPRECONDITION(!pi_ChannelStr.empty());

    HFCPtr<HFCURL> pChURL = 0;
    Utf8String NewChPathNameStr;

    // Compose URL string
    if ((pi_ChannelStr.find(":\\\\") != Utf8String::npos) || (pi_ChannelStr.find("://") != Utf8String::npos))
        {
        // Complete URL nothing to do
        NewChPathNameStr = pi_ChannelStr;
        }
    else if (  (pi_ChannelStr.find("\\\\") != Utf8String::npos) || (pi_ChannelStr.find("//") != Utf8String::npos)
               || (pi_ChannelStr.find(":\\") != Utf8String::npos) || (pi_ChannelStr.find(":/") != Utf8String::npos) )
        {
        // Path is complete but we must add "file://" prefix
        NewChPathNameStr = Utf8String(HFCURLFile::s_SchemeName() + "://") + pi_ChannelStr;
        }
    else
        {
        // Path is relative to xml file location
        Utf8String Path     = pi_rpXMLFileURL->GetPath();
        Utf8String FileName = pi_rpXMLFileURL->GetFilename();

        Utf8String::size_type FileNamePos = Path.rfind(FileName);

        if (FileNamePos != Utf8String::npos)
            Path = Path.substr(0, FileNamePos);

        NewChPathNameStr = Utf8String(HFCURLFile::s_SchemeName() + "://")
                           + pi_rpXMLFileURL->GetHost() + "\\"
                           + Path
                           + pi_ChannelStr;
        }

    // Compose real URL
    try
        {
        pChURL = HFCURL::Instanciate(NewChPathNameStr);
        return pChURL;
        }
    catch(...)
        {
        return 0; // not a valid URL
        }
    }


/** ---------------------------------------------------------------------------
    Block capabilities of the xCh file format.
    Any access is posssible, it depends of the channels file format.
    ---------------------------------------------------------------------------
 */
class HRFxChBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    //:> Constructor
    HRFxChBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add (new HRFLineCapability(HFC_READ_ONLY,            // AccessMode
                                   INT32_MAX,                 // MaxWidth
                                   HRFBlockAccess::RANDOM)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY, // AccessMode
                                  INT32_MAX,      // MaxSizeInBytes
                                  1,             // MinWidth
                                  INT32_MAX,      // MaxWidth
                                  1,             // WidthIncrement
                                  1,             // MinHeight
                                  INT32_MAX,      // MaxHeight
                                  1,             // HeightIncrement
                                  false));       // Not Square

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_ONLY,  // AccessMode
                                   INT32_MAX,       // MaxSizeInBytes
                                   1,              // MinHeight
                                   INT32_MAX,       // MaxHeight
                                   1));            // HeightIncrement
        }
    };


/** ---------------------------------------------------------------------------
    Codec capability of the xCh file format.
    xCh file format is always seen as it does not support any compression.
    But channels can! (Handled by their respective HRF)
    ---------------------------------------------------------------------------
 */
class HRFxChCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFxChCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFxChBlockCapabilities()));
        }
    };


/** ---------------------------------------------------------------------------
    HRFxChCapabilities
    ---------------------------------------------------------------------------
 */
HRFxChCapabilities::HRFxChCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV24R8G8B8
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFxChCodecCapabilities()));

    // PixelTypeV32R8G8B8A8
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFxChCodecCapabilities()));

    // PixelTypeV48R16G16B16
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFxChCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_ONLY,  // AccessMode,
        true,           // SinglePixelType,
        true,           // SingleBlockType,
        true,           // ArbitaryXRatio,
        true);          // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Transfo model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DProjective::CLASS_ID));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DAffine::CLASS_ID));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DSimilitude::CLASS_ID));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DStretch::CLASS_ID));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DTranslation::CLASS_ID));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY,
                                      HGF2DIdentity::CLASS_ID));

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_ONLY));

    // Shape capabilities
    Add(new HRFClipShapeCapability(HFC_READ_ONLY, HRFCoordinateType::PHYSICAL));
    Add(new HRFClipShapeCapability(HFC_READ_ONLY, HRFCoordinateType::LOGICAL));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_ONLY));

    // BlockDataFlag capability
    Add(new HRFBlocksDataFlagCapability(HFC_READ_ONLY));

    // All tags
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeTitle));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeNotes));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeKeyWord));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeVersion));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDirector));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCompany));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeVendor));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSecurityLevel));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeLegalDisclaimer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeContentWarning));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeForeground(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDontSupportPersistentColor(true)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageSlo(4)));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogEllipsoid);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogInvFlattening);
    pGeocodingCapability->AddSupportedKey(GeogAzimuthUnits);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridianLong);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel1);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel2);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseNorthing);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginNorthing);
    pGeocodingCapability->AddSupportedKey(ProjCenterLong);
    pGeocodingCapability->AddSupportedKey(ProjCenterLat);
    pGeocodingCapability->AddSupportedKey(ProjCenterEasting);
    pGeocodingCapability->AddSupportedKey(ProjCenterNorthing);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtNatOrigin);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtCenter);
    pGeocodingCapability->AddSupportedKey(ProjAzimuthAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }


HFC_IMPLEMENT_SINGLETON(HRFxChCreator)


/** ---------------------------------------------------------------------------
    Constructor.
    Creator.
    ---------------------------------------------------------------------------
 */
HRFxChCreator::HRFxChCreator()
    : HRFRasterFileCreator(HRFxChFile::CLASS_ID)
    {
    // xCh capabilities instance member initialization
    m_pCapabilities = 0;
    }


/** ---------------------------------------------------------------------------
    Return file format label.

    @return string xCh file format label.
    ---------------------------------------------------------------------------
 */
Utf8String HRFxChCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_MultiChannel()); // Multi Channel Image File Format
    }


/** ---------------------------------------------------------------------------
    Return file format scheme.

    @return string scheme of URL.
    ---------------------------------------------------------------------------
 */
Utf8String HRFxChCreator::GetSchemes() const
    {
    return Utf8String(HFCURLFile::s_SchemeName());
    }


/** ---------------------------------------------------------------------------
    Return file format extension.

    @return string xCh extension.
    ---------------------------------------------------------------------------
 */
Utf8String HRFxChCreator::GetExtensions() const
    {
    return Utf8String("*.xch");
    }


/** ---------------------------------------------------------------------------
    Open/Create xCh raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.

    @return HFCPtr<HRFRasterFile> Address of the created HRFRasterFile instance.
    ---------------------------------------------------------------------------
 */
HFCPtr<HRFRasterFile> HRFxChCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFxChFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


/** ---------------------------------------------------------------------------
    Verify file validity.

    @param pi_rpURL  File's URL.
    @param pi_Offset Starting point in the file (not supported).

    @return true if the file is a valid xCh file, false otherwise.
    ---------------------------------------------------------------------------
 */
bool HRFxChCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    bool   bResult = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)pi_rpURL);
        // at least, the file must be have the right extension...
        if (rpURL->GetExtension().EqualsI("xch"))
            {
            Utf8String XMLFileName;

            // Extract the standard file name from the main URL
            XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
            XMLFileName += "\\";
            XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            // Open XML file
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, XMLFileName.c_str ());

            //Validate pDom
            if (pDom.IsValid() && (BEXML_Success == xmlStatus))
                {
                // Validate main node presence       
                BeXmlNodeP pMainNode = pDom->GetRootElement();
                if (NULL != pMainNode && BeStringUtilities::Stricmp(pMainNode->GetName(), "MultiChannelImageFileFormat") == 0)
                {
                    bResult = true;

                    // Validate VERSION node presence (for now only validate node presence)          
                    BeXmlNodeP pVersionNode = pMainNode->SelectSingleNode("VERSION");
                    if (!pVersionNode)
                        bResult = false;

                    // Validate CHANNELS node presence
                    BeXmlNodeP pChannelNode = pMainNode->SelectSingleNode("CHANNELS");
                    if (!bResult || !pChannelNode)
                        bResult = false;
                    else
                    {
                        // Validate COUNT node presence
                        BeXmlNodeP pCountNode = pChannelNode->SelectSingleNode("COUNT");
                        if (!pCountNode)
                            bResult = false;
                        else
                            {
                            // Validate channel count, must be 3 or 4 (type = UInt32)
                            uint32_t ChannelCount = 0;
                            if (BEXML_Success != pCountNode->GetContentUInt32Value(ChannelCount) || !(ChannelCount == 3 || ChannelCount == 4))
                                bResult = false;

                            // Validate RED node presence
                            BeXmlNodeP pRedNode = pChannelNode->SelectSingleNode("RED");
                            if (!pRedNode)
                                bResult = false;

                            // Validate GREEN node presence
                            BeXmlNodeP pGreenNode = pChannelNode->SelectSingleNode("GREEN");
                            if (!pGreenNode)
                                bResult = false;

                            // Validate BLUE node presence
                            BeXmlNodeP pBlueNode = pChannelNode->SelectSingleNode("BLUE");
                            if (!pBlueNode)
                                bResult = false;

                            if (ChannelCount == 4)
                                {
                                // Validate Alpha or Panchromatic node presence    
                                BeXmlNodeP pAlphaNode = pChannelNode->SelectSingleNode("ALPHA");
                                BeXmlNodeP pPanchromNode = pChannelNode->SelectSingleNode("PANCHROMATIC");
                                if (!(pAlphaNode || pPanchromNode) || (pAlphaNode && pPanchromNode))
                                    bResult = false;
                                }
                            }
                        }
                    }
                }
            }
        }

    return bResult;
    }

/** ---------------------------------------------------------------------------
    Get list of related files from a given URL.

    @param pi_rpURL          Main file's URL.
    @param pio_rRelatedURLs  List of related URLs.

    @return true (current file format is multi-file).
    ---------------------------------------------------------------------------
 */
bool HRFxChCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                    ListOfRelatedURLs&    pio_rRelatedURLs) const
    {
    HASSERT (pio_rRelatedURLs.size() == 0);

    Utf8String XMLFileName;
    Utf8String RedFileNameStr;
    Utf8String GreenFileNameStr;
    Utf8String BlueFileNameStr;
    Utf8String AlphaFileNameStr;
    Utf8String PanChromaticFileNameStr;

    HFCPtr<HFCURL> pRedFileURL   = 0;
    HFCPtr<HFCURL> pGreenFileURL = 0;
    HFCPtr<HFCURL> pBlueFileURL  = 0;
    HFCPtr<HFCURL> pAlphaFileURL = 0;
    HFCPtr<HFCURL> pPanChromaticFileURL = 0;

    // Extract the standard file name from the main URL
    XMLFileName = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost();
    XMLFileName += "\\";
    XMLFileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    // Open XML file
    BeXmlStatus xmlStatus;
    BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, XMLFileName.c_str ());

    //Validate pDom
    if (pDom.IsNull() || (BEXML_Success != xmlStatus))
        return false;

    // Validate main node presence       
    BeXmlNodeP pMainNode = pDom->GetRootElement ();
    if (NULL == pMainNode)
        return false;

    // Validate RED node presence
    BeXmlNodeP pRedNode = pMainNode->SelectSingleNode ("CHANNELS/RED");
    pRedNode->GetContent (RedFileNameStr);

    // Validate GREEN node presence
    BeXmlNodeP pGreenNode = pMainNode->SelectSingleNode ("CHANNELS/GREEN");
    pGreenNode->GetContent (GreenFileNameStr);

    // Validate BLUE node presence
    BeXmlNodeP pBlueNode = pMainNode->SelectSingleNode ("CHANNELS/BLUE");
    pBlueNode->GetContent (BlueFileNameStr);

    // Empty fields ?
    if ((RedFileNameStr == "") || (GreenFileNameStr == "") || (BlueFileNameStr == ""))
        return false;

    // Compose URLs
    if (!(pRedFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)pi_rpURL, RedFileNameStr)))
        return false;
    if (!(pGreenFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)pi_rpURL, GreenFileNameStr)))
        return false;
    if (!(pBlueFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)pi_rpURL, BlueFileNameStr)))
        return false;

    // Construct related URLs list
    pio_rRelatedURLs.push_back(pRedFileURL);
    pio_rRelatedURLs.push_back(pGreenFileURL);
    pio_rRelatedURLs.push_back(pBlueFileURL);

    // Alpha and panchromatic files
    BeXmlNodeP pAlphaNode = pMainNode->SelectSingleNode("CHANNELS/ALPHA");
    pAlphaNode->GetContent(AlphaFileNameStr);
    if (AlphaFileNameStr != "")
        {
        if (!(pAlphaFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)pi_rpURL, AlphaFileNameStr)))
            return false;

        pio_rRelatedURLs.push_back(pAlphaFileURL);
        }

    // Validate panchromatic node presence
    BeXmlNodeP pPanchromNode = pMainNode->SelectSingleNode("CHANNELS/PANCHROMATIC");
    pPanchromNode->GetContent(PanChromaticFileNameStr);
    if (PanChromaticFileNameStr != "")
    {
        if (!(pPanChromaticFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)pi_rpURL, PanChromaticFileNameStr)))
            return false;

        pio_rRelatedURLs.push_back(pPanChromaticFileURL);
    }

    return true;
    }

/** ---------------------------------------------------------------------------
    Get capabilities of the xCh file format.

    @return xCh format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFxChCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFxChCapabilities();

    return m_pCapabilities;
    }


/** ---------------------------------------------------------------------------
    Constructor.
    Open xCh raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.
    ---------------------------------------------------------------------------
 */
HRFxChFile::HRFxChFile(const HFCPtr<HFCURL>& pi_rpURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {

    // The ancestor stores the access mode
    m_IsOpen = false;
    m_ChannelCount = 0;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpURL->GetURL());
        }


    Open();
    HASSERT(m_IsOpen == true);
    CreateDescriptors();
    CreateChannelResolutionEditors();
    }


/** ---------------------------------------------------------------------------
    Special constructor.
    Open xCh raster file.

    @param pi_rpRedFileURL   URL of red channel file.
    @param pi_rpGreenFileURL URL of green channel file.
    @param pi_rpBlueFileURL  URL of blue channel file.
    @param pi_rpAlphaFileURL URL of alpha channel file.
    ---------------------------------------------------------------------------
 */
HRFxChFile::HRFxChFile(const HFCPtr<HFCURL>& pi_rpRedFileURL,
                       const HFCPtr<HFCURL>& pi_rpGreenFileURL,
                       const HFCPtr<HFCURL>& pi_rpBlueFileURL,
                       const HFCPtr<HFCURL>& pi_rpAlphaFileURL)
    : HRFRasterFile(pi_rpRedFileURL/*will be modified*/, HFC_READ_ONLY, 0)
    {
    HPRECONDITION(pi_rpRedFileURL != 0);
    HPRECONDITION(pi_rpGreenFileURL != 0);
    HPRECONDITION(pi_rpBlueFileURL != 0);

    m_IsOpen = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpRedFileURL->GetURL());
        }

    // Compose a pseudo, unique, main URL (lightly modified red file URL)
    Utf8String newURLHostStr;
    Utf8String newURLPathStr;

    newURLHostStr = ((HFCPtr<HFCURLFile>&)GetURL())->GetHost();
    newURLPathStr = ((HFCPtr<HFCURLFile>&)GetURL())->GetPath() + "_xCh";

    m_pURL = new HFCURLFile(newURLHostStr, newURLPathStr);

    // Validate each file existence
    HFCStat RedFileStat(pi_rpRedFileURL);
    HFCStat GreenFileStat(pi_rpGreenFileURL);
    HFCStat BlueFileStat(pi_rpBlueFileURL);

    if (!RedFileStat.IsExistent())
        throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                    pi_rpRedFileURL->GetURL());
    if (!GreenFileStat.IsExistent())
        throw HRFCannotOpenChildFileException( GetURL()->GetURL(),
                                    pi_rpGreenFileURL->GetURL());
    if (!BlueFileStat.IsExistent())
        throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                    pi_rpBlueFileURL->GetURL());

    // Construct related URLs list
    m_ListOfRelatedURLs.push_back(pi_rpRedFileURL);
    m_ListOfRelatedURLs.push_back(pi_rpGreenFileURL);
    m_ListOfRelatedURLs.push_back(pi_rpBlueFileURL);

    if (!pi_rpAlphaFileURL)
        m_ChannelCount = 3;
    else
        {
        m_ChannelCount = 4;

        HFCStat AlphaFileStat(pi_rpAlphaFileURL);

        if (!AlphaFileStat.IsExistent())
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        pi_rpAlphaFileURL->GetURL());

        m_ListOfRelatedURLs.push_back(pi_rpAlphaFileURL);
        }

    // Open files read-only
    m_pRedFile   = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpRedFileURL, true);
    m_pGreenFile = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpGreenFileURL, true);
    m_pBlueFile  = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpBlueFileURL, true);

    if (!m_pRedFile)
        throw HRFCannotOpenChildFileException( GetURL()->GetURL(),
                                    pi_rpRedFileURL->GetURL());

    if (!m_pGreenFile)
        throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                    pi_rpGreenFileURL->GetURL());

    if (!m_pBlueFile)
        throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                    pi_rpBlueFileURL->GetURL());

    if (m_ChannelCount == 4) // 32 or 64 bits - there is an alpha channel!
        {
        m_pAlphaFile = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpAlphaFileURL, true);

        if (!m_pAlphaFile)
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        pi_rpAlphaFileURL->GetURL());
        }

    // Validate channel files
    ValidateChannelFilesRGBA(m_pRedFile, m_pGreenFile, m_pBlueFile, m_pAlphaFile);

    m_IsOpen = true;

    CreateDescriptors();
    CreateChannelResolutionEditors();
    }


/** ---------------------------------------------------------------------------
    Destructor.
    Destroy xCh file object.
    ---------------------------------------------------------------------------
 */
HRFxChFile::~HRFxChFile()
    {
    // Close the file
    Close();
    }


/** ---------------------------------------------------------------------------
    Create wrapper editor for all channel data manipulation (read).

    @param pi_Page        Page to create an editor for.
    @param pi_Resolution  Resolution.
    @param pi_AccessMode  Access and sharing mode.

    @return appropriate created resolution editor wrapper.
    ---------------------------------------------------------------------------
 */
HRFResolutionEditor* HRFxChFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        uint16_t pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (m_pPanchromaticFile)
        pEditor = new HRFxChEditorPanchromatic(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFxChEditorRGBA(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }


/** ---------------------------------------------------------------------------
    Get capabilities of xCh file format.

    @return xCh file format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFxChFile::GetCapabilities () const
    {
    return HRFxChCreator::GetInstance()->GetCapabilities();
    }


/** ---------------------------------------------------------------------------
    Get a grayscale palette from the pixel type color palette.

    @return a new grayscale palette if original pixel type palette entries were
    all grayscale, 0 otherwise.
    ---------------------------------------------------------------------------
 */
Byte* HRFxChFile::GetGrayscalePalette (HFCPtr<HRPPixelType> pi_pPixelType)
    {
    HArrayAutoPtr<Byte> pGrayscalePal;
    pGrayscalePal = new Byte[256];

    const HRPPixelPalette& rPalette = pi_pPixelType->GetPalette();

    uint32_t PaletteSize = rPalette.GetMaxEntries();

    if (PaletteSize < 256)
        return 0;

    uint32_t Index;
    for(Index = 0; Index < 256; Index++)
        {
        Byte* pValue = (Byte*)rPalette.GetCompositeValue(Index);

        if((pValue[0] != pValue[1]) || (pValue[0]  != pValue[2]))
            break;

        pGrayscalePal[Index] = pValue[0];
        }

    if (Index < 256)
        return 0;
    else
        return pGrayscalePal.release();
    }


/** ---------------------------------------------------------------------------
    Channel files validation
    All channels' pixel type must be HRPPixelTypeV8Gray8, HRPPixelTypeV16Gray16
    or HRPPixelTypeI1R8G8B8 (with grayscale palette). Also, all channel files must be of the same type,
    same compression, same block organization, same resolution type (multi or single).
    ---------------------------------------------------------------------------
 */
void HRFxChFile::ValidateChannelFilesRGB (const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                          const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                          const HFCPtr<HRFRasterFile>& pi_rpBlueFile)
    {
    // Rasters must be of the same format
    if (!((pi_rpRedFile->GetClassID() == pi_rpGreenFile->GetClassID()) &&
          (pi_rpRedFile->GetClassID() == pi_rpBlueFile->GetClassID())))
        throw HRFXCHChannelsAreNotOfTheSameFormatException(GetURL()->GetURL());

    // ...and same compression type
    HCLASS_ID RedFileCodecClsid   = pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();
    HCLASS_ID GreenFileCodecClsid = pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();
    HCLASS_ID BlueFileCodecClsid  = pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();

    if (!((RedFileCodecClsid == GreenFileCodecClsid) &&
          (RedFileCodecClsid == BlueFileCodecClsid)))
        throw HRFXCHChannelsDoNotHaveSameCompressionException(GetURL()->GetURL());

    // All files must be grayscale
    HFCPtr<HRPPixelType> pRedChPixelType   = pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
    HFCPtr<HRPPixelType> pGreenChPixelType = pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
    HFCPtr<HRPPixelType> pBlueChPixelType  = pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();

    if (!( pRedChPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) ||
           pRedChPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID) ) &&
        (pi_rpRedFile->GetClassID() != HRFHMRFile::CLASS_ID) )
        {
        // not a valid pixel type!
        throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpRedFile->GetURL()->GetURL());
        }
    else if (pi_rpRedFile->GetClassID() == HRFHMRFile::CLASS_ID)
        {
        // OK but only for a grayscale palette
        if (!((pRedChPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID)) &&
              (m_pRedMap = GetGrayscalePalette(pRedChPixelType))))
            throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpRedFile->GetURL()->GetURL());
        }

    if (!(pGreenChPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) ||
          pGreenChPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID)  ) &&
        (pi_rpGreenFile->GetClassID() != HRFHMRFile::CLASS_ID) )
        {
        // not a valid pixel type!
        throw HRFXCHChannelsIsNotAValidGrayscaleException( pi_rpGreenFile->GetURL()->GetURL());
        }
    else if (pi_rpGreenFile->GetClassID() == HRFHMRFile::CLASS_ID)
        {
        // OK but only for a grayscale palette
        if (!((pGreenChPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID)) &&
              (m_pGreenMap = GetGrayscalePalette(pGreenChPixelType))))
            throw HRFXCHChannelsIsNotAValidGrayscaleException( pi_rpGreenFile->GetURL()->GetURL());
        }

    if (!(pBlueChPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) ||
          pBlueChPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID)  ) &&
        (pi_rpBlueFile->GetClassID() != HRFHMRFile::CLASS_ID) )
        {
        // not a valid pixel type!
        throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpBlueFile->GetURL()->GetURL());
        }
    else if (pi_rpBlueFile->GetClassID() == HRFHMRFile::CLASS_ID)
        {
        // OK but only for a grayscale palette
        if (!((pBlueChPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID)) &&
              (m_pBlueMap = GetGrayscalePalette(pBlueChPixelType))))
            throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpBlueFile->GetURL()->GetURL());
        }

    // All files must have same dimensions
    if (! (  (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth()
              == pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth()
                 == pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight()
                 == pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight()
                 == pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())))
        throw HRFFXHChannelsDoNotHaveTheSameDimensionsException(GetURL()->GetURL());

    // All files must have same resolution number
    if (! (  (pi_rpRedFile->GetPageDescriptor(0)->CountResolutions()
              == pi_rpGreenFile->GetPageDescriptor(0)->CountResolutions())
             && (pi_rpRedFile->GetPageDescriptor(0)->CountResolutions()
                 == pi_rpBlueFile->GetPageDescriptor(0)->CountResolutions())))
        throw HRFFXHChannelsDoNotHaveTheSameResCountException( GetURL()->GetURL());

    // All files must have same block dimensions
    if (! (  (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth()
              == pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth()
                 == pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight()
                 == pi_rpGreenFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight())
             && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight()
                 == pi_rpBlueFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight())))
        throw HRFXCHChannelsBlockDimensionsDifferException(GetURL()->GetURL());
    }

    void HRFxChFile::ValidateChannelFilesRGBA(const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                              const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                              const HFCPtr<HRFRasterFile>& pi_rpBlueFile,
                                              const HFCPtr<HRFRasterFile>& pi_rpAlphaFile)
    {
        ValidateChannelFilesRGB(pi_rpRedFile, pi_rpGreenFile, pi_rpBlueFile);

        // Verify alpha channel
        // Rasters must be of the same format
        if (!(pi_rpRedFile->GetClassID() == pi_rpAlphaFile->GetClassID()))
            throw HRFXCHChannelsAreNotOfTheSameFormatException(GetURL()->GetURL());

        // ...and same compression type
        HCLASS_ID AlphaFileCodecClsid = pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();
        HCLASS_ID RedFileCodecClsid = pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();

        if (!(RedFileCodecClsid == AlphaFileCodecClsid))
            throw HRFXCHChannelsDoNotHaveSameCompressionException(GetURL()->GetURL());

        // All files must be grayscale
        HFCPtr<HRPPixelType> pAlphaChPixelType = pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();

        if (!(pAlphaChPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) ||
            pAlphaChPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID)) &&
            (pi_rpAlphaFile->GetClassID() != HRFHMRFile::CLASS_ID))
        {
            // not a valid pixel type!
            throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpAlphaFile->GetURL()->GetURL());
        }
        else if (pi_rpAlphaFile->GetClassID() == HRFHMRFile::CLASS_ID)
        {
            // OK but only for a grayscale palette
            if (!((pAlphaChPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID)) &&
                (m_pAlphaMap = GetGrayscalePalette(pAlphaChPixelType))))
                throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpAlphaFile->GetURL()->GetURL());
        }

        // All files must have same dimensions
        if (!((pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth()
            == pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
            && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight()
                == pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())))
            throw HRFFXHChannelsDoNotHaveTheSameDimensionsException(GetURL()->GetURL());

        // All files must have same resolution number
        if (!(pi_rpRedFile->GetPageDescriptor(0)->CountResolutions()
            == pi_rpAlphaFile->GetPageDescriptor(0)->CountResolutions()))
            throw HRFFXHChannelsDoNotHaveTheSameResCountException(GetURL()->GetURL());

        // All files must have same block dimensions
        if (!((pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth()
            == pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth())
            && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight()
                == pi_rpAlphaFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight())))
            throw HRFXCHChannelsBlockDimensionsDifferException(GetURL()->GetURL());
    }

void HRFxChFile::ValidateChannelFilesPanchromatic(const HFCPtr<HRFRasterFile>& pi_rpPanchromaticFile,
                                                  const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                                  const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                                  const HFCPtr<HRFRasterFile>& pi_rpBlueFile)
    {
        ValidateChannelFilesRGB(pi_rpRedFile, pi_rpGreenFile, pi_rpBlueFile);

        // Verify panchromatic channel
        // Rasters must be of the same format
        if (!(pi_rpRedFile->GetClassID() == pi_rpPanchromaticFile->GetClassID()))
            throw HRFXCHChannelsAreNotOfTheSameFormatException(GetURL()->GetURL());

        // ...and same compression type
        HCLASS_ID FileCodecClsid = pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();
        HCLASS_ID RedFileCodecClsid = pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->GetClassID();
        if (!(RedFileCodecClsid == FileCodecClsid))
            throw HRFXCHChannelsDoNotHaveSameCompressionException(GetURL()->GetURL());

        // All files must be grayscale
        HFCPtr<HRPPixelType> pPixelType = pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
        if (!(pPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID) ||
            pPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID)))
        {
            // not a valid pixel type!
            throw HRFXCHChannelsIsNotAValidGrayscaleException(pi_rpPanchromaticFile->GetURL()->GetURL());
        }

        // Panchromatic resolution == 2x RGB files
        if (!((((pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth()*2)-1)
               == pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
             && (((pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight()*2)-1)
                == pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())))
            throw HRFFXHChannelsDoNotHaveTheSameDimensionsException(GetURL()->GetURL());

        // All files must have same resolution number
        if (!(pi_rpRedFile->GetPageDescriptor(0)->CountResolutions()
            == pi_rpPanchromaticFile->GetPageDescriptor(0)->CountResolutions()))
            throw HRFFXHChannelsDoNotHaveTheSameResCountException(GetURL()->GetURL());

        // All files must have same block dimensions
        //if (!((pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth()
        //    == pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockWidth())
        //    && (pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight()
        //        == pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight())))
        //    throw HRFXCHChannelsBlockDimensionsDifferException(GetURL()->GetURL());
        
        // Same block type
        if (pi_rpPanchromaticFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() !=
            pi_rpRedFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType())
            throw HRFXCHChannelsBlockDimensionsDifferException(GetURL()->GetURL());
    }



/** ---------------------------------------------------------------------------
    Open all 3 or 4 channel raster files.
    Creator's IsKindOfFile() method should have been called first to
    validate files.

    @return true, raster files have been created.
    ---------------------------------------------------------------------------
 */
bool HRFxChFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        Utf8String XMLFileName;
        Utf8String ChannelCountStr;
        Utf8String RedFileNameStr;
        Utf8String GreenFileNameStr;
        Utf8String BlueFileNameStr;
        Utf8String AlphaFileNameStr;
        Utf8String PanchromaticFileNameStr;

        HFCPtr<HFCURL> pRedFileURL   = 0;
        HFCPtr<HFCURL> pGreenFileURL = 0;
        HFCPtr<HFCURL> pBlueFileURL  = 0;
        HFCPtr<HFCURL> pAlphaFileURL = 0;
        HFCPtr<HFCURL> pPanchromaticFileURL = 0;

        // Extract the standard file name from the main URL
        XMLFileName = ((HFCPtr<HFCURLFile>&)GetURL())->GetHost();
        XMLFileName += "\\";
        XMLFileName += ((HFCPtr<HFCURLFile>&)GetURL())->GetPath();

        // Open XML file
        BeXmlStatus xmlStatus;
        BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, XMLFileName.c_str ());

        // Validate pDom
        if (pDom.IsNull() || (BEXML_Success != xmlStatus))
            return false;

        // Validate main node presence       
        BeXmlNodeP pMainNode = pDom->GetRootElement ();
        if (NULL == pMainNode)
            return false;

        // Get count node
        BeXmlNodeP pCountNode = pMainNode->SelectSingleNode ("CHANNELS/COUNT");
        pCountNode->GetContentUInt32Value (m_ChannelCount);

        // Validate RED node presence
        BeXmlNodeP pRedNode = pMainNode->SelectSingleNode ("CHANNELS/RED");
        pRedNode->GetContent (RedFileNameStr);

        // Validate GREEN node presence
        BeXmlNodeP pGreenNode = pMainNode->SelectSingleNode ("CHANNELS/GREEN");
        pGreenNode->GetContent (GreenFileNameStr);

        // Validate BLUE node presence
        BeXmlNodeP pBlueNode = pMainNode->SelectSingleNode ("CHANNELS/BLUE");
        pBlueNode->GetContent (BlueFileNameStr);

        // Empty fields ?
        if (RedFileNameStr == "")
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "RED");
        if (GreenFileNameStr == "")
            throw HRFInvalidParamValueException( GetURL()->GetURL(),
                                            "GREEN");
        if (BlueFileNameStr == "")
            throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                            "BLUE");

        // Compose URLs
        if (!(pRedFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)GetURL(), RedFileNameStr)))
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        RedFileNameStr);
        if (!(pGreenFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)GetURL(), GreenFileNameStr)))
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        GreenFileNameStr);
        if (!(pBlueFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)GetURL(), BlueFileNameStr)))
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        BlueFileNameStr);

        // Validate each file existence
        HFCStat RedFileStat(pRedFileURL);
        HFCStat GreenFileStat(pGreenFileURL);
        HFCStat BlueFileStat(pBlueFileURL);

        if (!RedFileStat.IsExistent())
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        RedFileNameStr);
        if (!GreenFileStat.IsExistent())
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        GreenFileNameStr);
        if (!BlueFileStat.IsExistent())
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                        BlueFileNameStr);

        // Construct related URLs list
        m_ListOfRelatedURLs.push_back(pRedFileURL);
        m_ListOfRelatedURLs.push_back(pGreenFileURL);
        m_ListOfRelatedURLs.push_back(pBlueFileURL);

        // Open files read-only
        m_pRedFile   = HRFRasterFileFactory::GetInstance()->OpenFile(pRedFileURL, true);
        m_pGreenFile = HRFRasterFileFactory::GetInstance()->OpenFile(pGreenFileURL, true);
        m_pBlueFile  = HRFRasterFileFactory::GetInstance()->OpenFile(pBlueFileURL, true);

        if (!m_pRedFile)
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),pRedFileURL->GetURL());

        if (!m_pGreenFile)
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),pGreenFileURL->GetURL());

        if (!m_pBlueFile)
            throw HRFCannotOpenChildFileException(GetURL()->GetURL(),pBlueFileURL->GetURL());

        // there is an alpha channel or panchromatic file ?
        if (m_ChannelCount == 4) 
            {
            // Validate Alpha node presence
            BeXmlNodeP pAlphaNode = pMainNode->SelectSingleNode ("CHANNELS/ALPHA");
            pAlphaNode->GetContent (AlphaFileNameStr);

            // Validate panchromatic node presence
            BeXmlNodeP pPanchromNode = pMainNode->SelectSingleNode("CHANNELS/PANCHROMATIC");
            pPanchromNode->GetContent(PanchromaticFileNameStr);

            // Empty field ?
            if (AlphaFileNameStr != "")
                {
                // Compose URL
                if (!(pAlphaFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)GetURL(), AlphaFileNameStr)))
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                                                AlphaFileNameStr);

                HFCStat AlphaFileStat(pAlphaFileURL);

                if (!AlphaFileStat.IsExistent())
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                                AlphaFileNameStr);

                m_ListOfRelatedURLs.push_back(pAlphaFileURL);

                m_pAlphaFile = HRFRasterFileFactory::GetInstance()->OpenFile(pAlphaFileURL, true);

                if (!m_pAlphaFile)
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                                pAlphaFileURL->GetURL());

                ValidateChannelFilesRGBA(m_pRedFile, m_pGreenFile, m_pBlueFile, m_pAlphaFile);
                }
            else if(PanchromaticFileNameStr != "")
                {
                // Compose URL
                if (!(pPanchromaticFileURL = ComposeChannelURL((HFCPtr<HFCURLFile>&)GetURL(), PanchromaticFileNameStr)))
                    throw HRFInvalidParamValueException(GetURL()->GetURL(),
                        PanchromaticFileNameStr);

                HFCStat AlphaFileStat(pPanchromaticFileURL);

                if (!AlphaFileStat.IsExistent())
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                        PanchromaticFileNameStr);

                m_ListOfRelatedURLs.push_back(pPanchromaticFileURL);

                m_pPanchromaticFile = HRFRasterFileFactory::GetInstance()->OpenFile(pPanchromaticFileURL, true);

                if (!m_pPanchromaticFile)
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(), pPanchromaticFileURL->GetURL());

                ValidateChannelFilesPanchromatic(m_pPanchromaticFile, m_pRedFile, m_pGreenFile, m_pBlueFile);
                }
            else
                throw HRFInvalidParamValueException(GetURL()->GetURL(), "Invalid channel 4");
            }

        m_IsOpen = true;
        }

    return true;
    }


/** ---------------------------------------------------------------------------
    Create resolutions and page descriptors based on red channel raster file.
    ---------------------------------------------------------------------------
 */
void HRFxChFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    HFCPtr<HRFPageDescriptor> pChannelPageDescriptor;
    HFCPtr<HRPPixelType> pChannelPixelType;
    HFCPtr<HRPPixelType> pPixelType;

    if (m_pPanchromaticFile)
        {
        // use Panchromatic page descriptor, the resolution is 2x better.
        pChannelPageDescriptor = m_pPanchromaticFile->GetPageDescriptor(0);
        pChannelPixelType = pChannelPageDescriptor->GetResolutionDescriptor(0)->GetPixelType();
        if (pChannelPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
            pPixelType = new HRPPixelTypeV48R16G16B16();
        else
            pPixelType = new HRPPixelTypeV24R8G8B8();
        HASSERT(m_ChannelCount == 4);
        }
    else
        {
        pChannelPageDescriptor = m_pRedFile->GetPageDescriptor(0);
        pChannelPixelType = pChannelPageDescriptor->GetResolutionDescriptor(0)->GetPixelType();

        if (m_ChannelCount == 3)
            if (pChannelPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
                pPixelType = new HRPPixelTypeV48R16G16B16();
            else
                pPixelType = new HRPPixelTypeV24R8G8B8();
        else
            if (pChannelPixelType->IsCompatibleWith(HRPPixelTypeV16Gray16::CLASS_ID))
                pPixelType = new HRPPixelTypeV64R16G16B16A16();
            else
                pPixelType = new HRPPixelTypeV32R8G8B8A8();
        }

    // Compose resolution descriptors
    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
    for (uint16_t Resolution=0; Resolution < pChannelPageDescriptor->CountResolutions(); Resolution++)
        {
        HFCPtr<HRFResolutionDescriptor> pChannelResolutionDescriptor = pChannelPageDescriptor->GetResolutionDescriptor(Resolution);

        const HRFDataFlag* pDataFlag = 0;
        if (pChannelResolutionDescriptor->HasBlocksDataFlag())
            pDataFlag = pChannelResolutionDescriptor->GetBlocksDataFlag();

        HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
            HFC_READ_ONLY,                                          // AccessMode,
            GetCapabilities(),                                      // Capabilities,
            pChannelResolutionDescriptor->GetResolutionXRatio(),    // XResolutionRatio,
            pChannelResolutionDescriptor->GetResolutionYRatio(),    // YResolutionRatio,
            pPixelType,                                             // PixelType (24 bits RGB or 32 bits RGBA),
            new HCDCodecIdentity(),                                 // Codec,
            pChannelResolutionDescriptor->GetReaderBlockAccess(),   // RBlockAccess,
            pChannelResolutionDescriptor->GetWriterBlockAccess(),   // WBlockAccess,
            pChannelResolutionDescriptor->GetScanlineOrientation(), // ScanLineOrientation,
            pChannelResolutionDescriptor->GetInterleaveType(),      // InterleaveType
            pChannelResolutionDescriptor->IsInterlace(),            // IsInterlace,
            pChannelResolutionDescriptor->GetWidth(),               // Width,
            pChannelResolutionDescriptor->GetHeight(),              // Height,
            pChannelResolutionDescriptor->GetBlockWidth(),          // BlockWidth,
            pChannelResolutionDescriptor->GetBlockHeight(),         // BlockHeight,
            pDataFlag,                                              // BlocksDataFlag,
            pChannelResolutionDescriptor->GetBlockType());             // BlockType

        ListOfResolutionDescriptor.push_back(pResolution);
        }

    HFCPtr<HRFClipShape> pClipShape         = 0;
    HFCPtr<HGF2DTransfoModel> pTransfoModel = 0;
    HRPFilter*                      pFilter = 0;

    if (pChannelPageDescriptor->HasClipShape())
        pClipShape = pChannelPageDescriptor->GetClipShape();

    if (pChannelPageDescriptor->HasTransfoModel())
        pTransfoModel = pChannelPageDescriptor->GetTransfoModel();

    if (pChannelPageDescriptor->HasFilter())
        pFilter = (HRPFilter*)&(pChannelPageDescriptor->GetFilter());


    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),                      // Capabilities,
                                   ListOfResolutionDescriptor,              // ResolutionDescriptor,
                                   0,                                      // RepresentativePalette,
                                   0,                                      // Histogram,
                                   0,                                      // Thumbnail,
                                   pClipShape,                              // ClipShape,
                                   pTransfoModel,                         // TransfoModel,
                                   pFilter,                                  // Filters
                                   pChannelPageDescriptor->GetTagsPtr()); // Defined Tag

    const HFCPtr<HMDMetaDataContainerList>& prMDContainerList(pChannelPageDescriptor->GetListOfMetaDataContainer());

    if (prMDContainerList != 0)
        {
        HFCPtr<HMDMetaDataContainerList> pClonedMDContainerList(new HMDMetaDataContainerList(*prMDContainerList));

        pPage->SetListOfMetaDataContainer(pClonedMDContainerList);
        }

    // Geocoding
    if (NULL != pChannelPageDescriptor->GetGeocodingCP())
        pPage->SetGeocoding(pChannelPageDescriptor->GetGeocodingCP());

    m_ListOfPageDescriptor.push_back(pPage);
    }


/** ---------------------------------------------------------------------------
    Create resolution editors for each channel raster file.
    ---------------------------------------------------------------------------
 */
void HRFxChFile::CreateChannelResolutionEditors()
    {
    // Create editors for each channel
    for (uint16_t i=0; i<m_pRedFile->GetPageDescriptor(0)->CountResolutions(); i++)
        {
        m_RedFileResolutionEditor.push_back(m_pRedFile->CreateResolutionEditor(0, i, HFC_READ_ONLY));
        HASSERT(m_RedFileResolutionEditor[i] != 0);
        m_GreenFileResolutionEditor.push_back(m_pGreenFile->CreateResolutionEditor(0, i, HFC_READ_ONLY));
        HASSERT(m_GreenFileResolutionEditor[i] != 0);
        m_BlueFileResolutionEditor.push_back(m_pBlueFile->CreateResolutionEditor(0, i, HFC_READ_ONLY));
        HASSERT(m_BlueFileResolutionEditor[i] != 0);
        }

    if (m_ChannelCount == 4 && m_pAlphaFile)
        for (uint16_t i=0; i<m_pRedFile->GetPageDescriptor(0)->CountResolutions(); i++)
            {
            m_AlphaFileResolutionEditor.push_back(m_pAlphaFile->CreateResolutionEditor(0, i, HFC_READ_ONLY));
            HASSERT(m_AlphaFileResolutionEditor[i] != 0);
            }

    if (m_ChannelCount == 4 && m_pPanchromaticFile)
        for (uint16_t i = 0; i < m_pRedFile->GetPageDescriptor(0)->CountResolutions(); i++)
        {
            m_PanchromaticFileResolutionEditor.push_back(m_pPanchromaticFile->CreateResolutionEditor(0, i, HFC_READ_ONLY));
            HASSERT(m_PanchromaticFileResolutionEditor[i] != 0);
        }

    }


/** ---------------------------------------------------------------------------
    Close all channel raster files.
    ---------------------------------------------------------------------------
 */
void HRFxChFile::Close()
    {
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        // Clean up the ResolutionEditor lists
        for (size_t i=0; i<m_RedFileResolutionEditor.size(); i++)
            {
            delete m_RedFileResolutionEditor[i];
            delete m_GreenFileResolutionEditor[i];
            delete m_BlueFileResolutionEditor[i];
            }

        m_RedFileResolutionEditor.clear();
        m_GreenFileResolutionEditor.clear();
        m_BlueFileResolutionEditor.clear();

        if (m_ChannelCount == 4)
            {
            for (size_t i=0; i<m_AlphaFileResolutionEditor.size(); i++)
                delete m_AlphaFileResolutionEditor[i];
            m_AlphaFileResolutionEditor.clear();

            for (size_t i = 0; i < m_PanchromaticFileResolutionEditor.size(); i++)
                delete m_PanchromaticFileResolutionEditor[i];
            m_PanchromaticFileResolutionEditor.clear();
            }

        m_IsOpen = false;
        }
    }

/** ---------------------------------------------------------------------------
Save files
---------------------------------------------------------------------------
*/
void HRFxChFile::Save()
    {

    HASSERT(!"HRFxChFile::Save():xCh format is read only");
    }

/** ---------------------------------------------------------------------------
    Get the world identificator of the xCh file format.
    (Based on red channel raster file)
    ---------------------------------------------------------------------------
 */
const HGF2DWorldIdentificator HRFxChFile::GetWorldIdentificator () const
    {
    HASSERT(m_pRedFile != 0);

    return m_pRedFile->GetWorldIdentificator();
    }

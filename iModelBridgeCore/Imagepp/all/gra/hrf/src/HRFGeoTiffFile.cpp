//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFGeoTiffFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/ImageppLib.h>

#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HTIFFDirectory.h>
#include <ImagePP/all/h/HTIFFGeoKey.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFTiffTileEditor.h>
#include <Imagepp/all/h/HRFTiffStripEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>

#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include <Imagepp/all/h/HCDCodec.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DProjective.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>




//-----------------------------------------------------------------------------
// HRFGeoTiffCapabilities
//-----------------------------------------------------------------------------
HRFGeoTiffCapabilities::HRFGeoTiffCapabilities()
    : HRFTiffCapabilities()
    {
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability;

    pGeocodingCapability = new HRFGeocodingCapability(HFC_READ_WRITE_CREATE);

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
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
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DProjective::CLASS_ID));
    }

HFC_IMPLEMENT_SINGLETON(HRFGeoTiffCreator)

//-----------------------------------------------------------------------------
// Creator TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFGeoTiffCreator::HRFGeoTiffCreator()
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;

    // the TIFF creator, from which this class descends, has set the class ID to
    // the TIFF ID.  Override it wich the GEOTiff ID
    m_ClassID = HRFGeoTiffFile::CLASS_ID;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFGeoTiffCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_GeoTiff()); //Geo TIFF Tagged Image File Format
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFGeoTiffCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                HFCAccessMode         pi_AccessMode,
                                                uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFGeoTiffFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFGeoTiffCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    HAutoPtr<HTIFFFile>  pTiff;
    uint32_t    DirOffset;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);

    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFGeoTiffCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if ((pTiff->IsValid(&pErr) || ((pErr != 0) && !pErr->IsFatal())))
        {
        // the tiff was opened successfully, verify if it is
        // a real tiff, or a HMR file.
        bResult = true;

        // To detect if it is a HMR file, verify if the private tag is present
        // if so, set the result to false
        if ((pTiff->GetField (HMR_IMAGEINFORMATION, &DirOffset)) ||
            (pTiff->GetField (HMR2_IMAGEINFORMATION, &DirOffset)))
            bResult = false;
        else
            {
            // Check for matrix and/or GeoKeys
            double*    pMat4by4;
            uint32_t    Count;
            unsigned short*    pKeyDirectory;
            unsigned short*    pTagRagBag;
            uint32_t    KeyCount;

//             bool IsTiffIntergraphTags = pTiff->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4) ||
//                                          pTiff->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag);

            if ((!(pTiff->GetField(GEOTRANSMATRIX, &Count, &pMat4by4) ||
                   pTiff->GetField(GEOTIEPOINTS, &Count, &pMat4by4)   ||
                   pTiff->GetField(GEOPIXELSCALE, &Count, &pMat4by4)  ||
                   pTiff->GetField(GEOKEYDIRECTORY, &KeyCount, &pKeyDirectory))) ||
                //TIFF Intergraph tags are found and have priority over GeoTIFF tags
                ((pTiff->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4) ||
                  pTiff->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag)) &&
                  (ImageppLib::GetHost().GetImageppLibAdmin()._IsIgnoreGeotiffIntergraphTags() == false)))
                {
                bResult = false;
                }
            else
                {
                // Declare the current pixel type and codec
                HFCPtr<HRPPixelType> CurrentPixelType;
                HFCPtr<HCDCodec>     CurrentCodec;
                try
                    {
                    // Create the current pixel type and codec
                    CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pTiff);
                    CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pTiff);
                    }

                catch (...)
                    {
                    bResult = false;
                    }

                if (bResult)
                    {
                    // Create the codec list to be attach to the PixelType Capability.
                    HFCPtr<HRFRasterFileCapabilities> pCurrentCodecCapability = new HRFRasterFileCapabilities();
                    pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                        CurrentCodec->GetClassID(),
                                                                        new HRFTiffBlockCapabilities()));

                    // Create the capability for the current pixel type and codec
                    HFCPtr<HRFCapability> pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                                                            CurrentPixelType->GetClassID(),
                                                                                            pCurrentCodecCapability);
                    // Check if we support these pixel type and codec
                    bResult = ((HRFRasterFileCreator*)this)->GetCapabilities()->Supports(pPixelTypeCapability);
                    }

                if (bResult)
                    {
                    // Validate Geo reference.
                    bool IsValidModel = false;

                    double*  pTiePoints;
                    double*  pPixelScale;
                    double*  pMatrix;      // 4 x 4

                    uint32_t  NbTiePoints  = 0;
                    uint32_t  NbPixelScale = 0;
                    uint32_t  MatSize      = 0;

                    // Get all the geo reference tag.
                    // Matrix
                    pTiff->GetField(GEOTRANSMATRIX, &MatSize, &pMatrix);
                    // TiePoint Pixel(x,y,z) System(x,y,z) ...
                    pTiff->GetField(GEOTIEPOINTS, &NbTiePoints, &pTiePoints);
                    // Scale x,y,z
                    pTiff->GetField(GEOPIXELSCALE, &NbPixelScale, &pPixelScale);

                    // Case 0 : No positionning tag in the file.
                    if (NbTiePoints == 0 && NbPixelScale == 0 && MatSize == 0)
                        IsValidModel = true;
                    // Case 1: One pair of tie point present. (And Case 6)
                    else if (NbTiePoints == 6 && NbPixelScale == 0)
                        IsValidModel = true;
                    // Case 3: One pair of tie point present and pixelscale. (And Case 6)
                    else if (NbTiePoints == 6 && NbPixelScale == 3)
                        IsValidModel = true;
                    // Case 4: Model Transformation tag present. (And Case 7)
                    else if (NbTiePoints == 0 && MatSize == 16)
                        IsValidModel = true;
                    // Case 5: Many pairs of tie points defined. (And Case 9, Case 6)
                    else if (NbTiePoints > 6 && NbTiePoints <= 24)
                        IsValidModel = true;
                    // Support more than 24 TiePoints in readOnly.
                    else if (NbTiePoints > 24)
                        IsValidModel = true;

                    // Assert that the matrix is valid.
                    if (MatSize == 16)
                        {
                        HFCMatrix<3, 3> TheMatrix;

                        TheMatrix[0][0] = pMatrix[0];
                        TheMatrix[0][1] = pMatrix[1];
                        TheMatrix[0][2] = pMatrix[3];
                        TheMatrix[1][0] = pMatrix[4];
                        TheMatrix[1][1] = pMatrix[5];
                        TheMatrix[1][2] = pMatrix[7];
                        TheMatrix[2][0] = pMatrix[12];
                        TheMatrix[2][1] = pMatrix[13];

                        // Try to not invalidate the model just because the
                        // global scale is not set. TR# 138746
                        if (!HDOUBLE_EQUAL_EPSILON(pMatrix[15], 0.0) )
                            TheMatrix[2][2] = pMatrix[15];
                        else
                            TheMatrix[2][2] = 1.0;

                        IsValidModel = IsValidMatrix(TheMatrix);
                        }

                    bResult = IsValidModel;
                    unsigned short GeoShortValue;

                    if (pTiff->GetGeoKeyInterpretation().GetValue(GTModelType, &GeoShortValue))
                        {
                        if(GeoShortValue == TIFFGeo_ModelTypeGeocentric)
                            bResult = false;
                        }
                    }
                }
            }
        }
    else
        bResult = false;


    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFGeoTiffCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFGeoTiffCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of GEotiff file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFGeoTiffCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFGeoTiffCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFGeoTiffFile::HRFGeoTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
    m_StoreUsingMatrix    = false;
    m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation = false;
    m_DefaultCoordSysIsIntergraphIfUnitNotResolved = false;
    m_RatioToMeter = 1.0;


    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file without open
//-----------------------------------------------------------------------------
HRFGeoTiffFile::HRFGeoTiffFile(const HFCPtr<HFCURL>& pi_rURL,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset,
                                      bool                 pi_DontOpenFile)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    m_StoreUsingMatrix = false;
    m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation = false;
    m_DefaultCoordSysIsIntergraphIfUnitNotResolved = false;
    m_RatioToMeter = 1.0;

    }

//-----------------------------------------------------------------------------
// public Destructor
//-----------------------------------------------------------------------------

HRFGeoTiffFile::~HRFGeoTiffFile()
    {
    try
        {
        SaveGeoTiffFile();
        // The tiff ancestor close the file and update palette, Transfo model and tags
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

/** -----------------------------------------------------------------------------
    Application must call this method to get :
    - The default unit.
    - The current InterpretUnit
    <h3>see HRFGeoTiffFile::SetDefaultInterpretationGeoRef</h3>
    -----------------------------------------------------------------------------
 */
void HRFGeoTiffFile::GetDefaultInterpretationGeoRef(double* po_RatioToMeter,
                                                           bool*   po_InterpretUnit,
                                                           bool*   po_InterpretUnitINTGR)
    {
    if (po_RatioToMeter)
        *po_RatioToMeter = m_RatioToMeter;
    if (po_InterpretUnit)
        *po_InterpretUnit = m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation;
    if (po_InterpretUnitINTGR)
        *po_InterpretUnitINTGR = m_DefaultCoordSysIsIntergraphIfUnitNotResolved;
    }

//-----------------------------------------------------------------------------
// Public CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFGeoTiffFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                            unsigned short pi_Resolution,
                                                            HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType() == HRFBlockType::TILE)
        pEditor = new HRFTiffTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFTiffStripEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public  AddPage
// File manipulation
//-----------------------------------------------------------------------------

bool HRFGeoTiffFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage->CountResolutions() > 0);

    // Validate the page and all resolutions with the GeoTiff capabilities
    // Create other resolution if necessary

    // always we have a transfo model with GeoTiff
    if (!pi_pPage->HasTransfoModel())
        {
        // Make model between Physical and Logical CoordSys.
        // Flip the Y Axe because the origin in a ModelSpace is at Bottom-Left
        HFCPtr<HGF2DTransfoModel> pModel = (HFCPtr<HGF2DTransfoModel>)new HGF2DStretch ();
        ((HFCPtr<HGF2DStretch>&)pModel)->SetXScaling(1.0);
        ((HFCPtr<HGF2DStretch>&)pModel)->SetYScaling(-1.0);
        ((HFCPtr<HGF2DStretch>&)pModel)->SetTranslation(HGF2DDisplacement (0.0, (uint32_t)pi_pPage->GetResolutionDescriptor(0)->GetHeight()));
        pi_pPage->SetTransfoModel(*pModel);
        }

    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys; 

    IRasterBaseGcsCP pBaseGCS = pi_pPage->GetGeocodingCP();
    if ((pBaseGCS != 0) && pBaseGCS->IsValid())
        {
        pGeoTiffKeys           = new HCPGeoTiffKeys();
        pBaseGCS->GetGeoTiffKeys(pGeoTiffKeys);
        }

    if (pGeoTiffKeys == NULL)
        {
        pGeoTiffKeys = new HCPGeoTiffKeys();
        }

    uint32_t LongValue;

    // Set up mantatory tag
    if (!(pGeoTiffKeys->HasKey(GTModelType)))
        {
        pGeoTiffKeys->AddKey(GTModelType, (uint32_t)TIFFGeo_ModelTypeProjected);
        }

    pGeoTiffKeys->GetValue(GTModelType, &LongValue);

    // Set up mantatory tags for ModelTypeProjected if needed
    if (LongValue == TIFFGeo_ModelTypeProjected)
        {
        if (!(pGeoTiffKeys->HasKey(ProjectedCSType)))
            {
            pGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)TIFFGeo_UserDefined);
            }

        pGeoTiffKeys->GetValue(ProjectedCSType, &LongValue);

        if (LongValue == TIFFGeo_UserDefined)
            {
            // Look if PCSCitation is present if not the a default value is set.
            if (!(pGeoTiffKeys->HasKey(PCSCitation)))
                {
                pGeoTiffKeys->AddKey(PCSCitation, "User defined (Default value)");
                }

            // Look if GeographicType is present if not the a default value is set.
            if (!(pGeoTiffKeys->HasKey(GeographicType)))
                {
                pGeoTiffKeys->AddKey(GeographicType, (uint32_t)TIFFGeo_UserDefined);
                }

            // Look if Projection is present if not the a default value is set.
            if (!(pGeoTiffKeys->HasKey(Projection)))
                {
                pGeoTiffKeys->AddKey(Projection, (uint32_t)TIFFGeo_UserDefined);
                }

            pGeoTiffKeys->GetValue(Projection, &LongValue);

            // Look if Projection is set to used defined.
            if (LongValue == TIFFGeo_UserDefined)
                {
                // Look if ProjCoorTrans is present if not the a default value is set to the file.
                if (!(pGeoTiffKeys->HasKey(ProjCoordTrans)))
                    {
                    pGeoTiffKeys->AddKey(ProjCoordTrans, (uint32_t)TIFFGeo_UserDefined);
                    }

                // Look if ProjLinearUnits is present if not the a default value is set to the file.
                if ((pGeoTiffKeys->HasKey(ProjLinearUnits)))
                    {
                    // Look if ProjLinearUnits is set to used defined.
                    pGeoTiffKeys->GetValue(ProjLinearUnits, &LongValue);

                    if (LongValue == TIFFGeo_UserDefined)
                        {
                        // Look if ProjLinearUnitSize is present if not the a default value is set to the file.
                        if (!(pGeoTiffKeys->HasKey(ProjLinearUnitSize)))
                            {
                            pGeoTiffKeys->AddKey(ProjLinearUnitSize, (double)1.0);
                            }
                        }
                    }
                }
            }
        }

    pi_pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys),true);


    // Add the page descriptor to the list
    return HRFTiffFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGeoTiffFile::GetCapabilities () const
    {
    return HRFGeoTiffCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFGeoTiffFile::Save()
    {
    SaveGeoTiffFile();
    HRFTiffFile::Save();
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFGeoTiffFile::GetFileCurrentSize() const
    {
    return HRFTiffFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the Geo Tiff file
//-----------------------------------------------------------------------------
void HRFGeoTiffFile::SaveGeoTiffFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        // Write Information
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            {
            // Update the modification to the file
            for (uint32_t Page=0; Page < CountPages(); Page++)
                {
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

                // Select the page
                SetImageInSubImage (GetIndexOfPage(Page));

                RasterFileGeocoding const& fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();
                HCPGeoTiffKeys const& inputGeoTiffKeys = fileGeocoding.GetGeoTiffKeys();

                HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys(inputGeoTiffKeys.Clone());

                uint32_t GeoKeyLongValue;
                bool isGeoTiffKeysModified(false);

                // If PROJECTED_CSTYPE is present, add all default tag related
                pGeoTiffKeys->GetValue(ProjectedCSType, &GeoKeyLongValue);

                if (pGeoTiffKeys->HasKey(ProjectedCSType) &&
                    (GeoKeyLongValue == TIFFGeo_UserDefined))
                    {
                    // Look if PCSCitation is present if not the a default value is set to the file.
                    if (!(pGeoTiffKeys->HasKey(PCSCitation)))
                        {
                        string Text = "User defined (Default value)";
                        GetFilePtr()->GetGeoKeyInterpretation().SetValues(PCSCitation, Text.c_str());

                        // add the tag into the page descriptor
                        pGeoTiffKeys->AddKey(PCSCitation, Text);
                        isGeoTiffKeysModified=true;
                        }

                    // Look if GeographicType is present if not the a default value is set to the file.
                    if (!(pGeoTiffKeys->HasKey(GeographicType)))
                        {
                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(GeographicType, (unsigned short) TIFFGeo_UserDefined);

                        // add the tag into the page descriptor
                        pGeoTiffKeys->AddKey(GeographicType, (uint32_t)TIFFGeo_UserDefined);
                        isGeoTiffKeysModified=true;
                        }

                    // Look if Projection is present if not the a default value is set to the file.
                    if (!(pGeoTiffKeys->HasKey(Projection)))
                        {
                        GetFilePtr()->GetGeoKeyInterpretation().SetValue(Projection, (unsigned short) TIFFGeo_UserDefined);

                        // add the tag into the page descriptor
                        pGeoTiffKeys->AddKey(Projection, (uint32_t)TIFFGeo_UserDefined);
                        isGeoTiffKeysModified=true;
                        }
                    // Look if Projection is set to used defined.
                    pGeoTiffKeys->GetValue(Projection, &GeoKeyLongValue);

                    if (GeoKeyLongValue >= TIFFGeo_UserDefined)
                        {
                        // Look if ProjCoorTrans is present if not the a default value is set to the file.
                        if (!(pGeoTiffKeys->HasKey(ProjCoordTrans)))
                            {
                            GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjCoordTrans, (unsigned short) TIFFGeo_UserDefined);

                            // add the tag into the page descriptor
                            pGeoTiffKeys->AddKey(ProjCoordTrans, (uint32_t)TIFFGeo_UserDefined);
                            isGeoTiffKeysModified=true;
                            }

                        // Look if ProjLinearUnits is present if not the a default value is set to the file.
                        if (!(pGeoTiffKeys->HasKey(ProjLinearUnits)))
                            {
                            //For now we never write Geotiff file in UOR, if we want to do so, enable line below (See TR 97326)
                            //if (!(m_sDefaultCoordSysIsIntergraphIfUnitNotResolved && !m_DefaultUnitWasFound))
                                {
                                GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjLinearUnits, (unsigned short) TIFFGeo_Linear_Meter);

                                // add the tag into the page descriptor
                                pGeoTiffKeys->AddKey(ProjLinearUnits, (uint32_t)TIFFGeo_Linear_Meter);
                                isGeoTiffKeysModified=true;
                                }
                            }
                        else // Projection is present look if it is used defined
                            {
                            // Look if ProjLinearUnits is set to used defined.
                            pGeoTiffKeys->GetValue(ProjLinearUnits, &GeoKeyLongValue);

                            if (GeoKeyLongValue >= TIFFGeo_UserDefined)
                                {
                                // Look if ProjLinearUnitSize is present if not the a default value is set to the file.
                                if (!(pGeoTiffKeys->HasKey(ProjLinearUnitSize)))
                                    {
                                    GetFilePtr()->GetGeoKeyInterpretation().SetValue(ProjLinearUnitSize, (double) 1,0);

                                    // add the tag into the page descriptor
                                    pGeoTiffKeys->AddKey(ProjLinearUnitSize, (double)1.0);
                                    isGeoTiffKeysModified=true;
                                    }
                                }
                            }
                        }
                    }
                if (isGeoTiffKeysModified)
                    pPageDescriptor->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys.GetPtr()));

                // Update the TransfoModel
                if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                    WriteTransfoModel(pGeoTiffKeys,
                                      pPageDescriptor->GetTransfoModel(),
                                      Page);
                }
            }
        }

    }
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFGeoTiffFile::Open(bool pi_CreateBigTifFormat)
    {
    bool Ret;

    // Call the parent
    if ((Ret = HRFTiffFile::Open(pi_CreateBigTifFormat)))
        {
        // if this is not a new file then check for GeoTiff validation
        if (!GetAccessMode().m_HasCreateAccess)
            {
            // Valid GeoTiff File
            // Check if the GeoTiff Directory is present
            double*    pMat4by4;
            uint32_t    Count;
            unsigned short* pKeyDirectory;
            uint32_t KeyCount;
            if (!(GetFilePtr()->GetField(GEOTRANSMATRIX, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOTIEPOINTS, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOPIXELSCALE, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOKEYDIRECTORY, &KeyCount, &pKeyDirectory)))
                  throw HFCFileNotSupportedException(GetURL()->GetURL());
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFGeoTiffFile::Open(const HFCPtr<HFCURL>& pi_rpURL)
    {
    bool Ret;

    // Call the parent
    if ((Ret = HRFTiffFile::Open(pi_rpURL)))
        {
        // if this is not a new file then check for GeoTiff validation
        if (!GetAccessMode().m_HasCreateAccess)
            {
            // Valid GeoTiff File
            // Check if the GeoTiff Directory is present
            double*    pMat4by4;
            uint32_t    Count;
            unsigned short* pKeyDirectory;
            uint32_t KeyCount;
            if (!(GetFilePtr()->GetField(GEOTRANSMATRIX, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOTIEPOINTS, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOPIXELSCALE, &Count, &pMat4by4) ||
                  GetFilePtr()->GetField(GEOKEYDIRECTORY, &KeyCount, &pKeyDirectory)))
                  throw HFCFileNotSupportedException(pi_rpURL->GetURL());
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Protected createDescriptors
//-----------------------------------------------------------------------------

void HRFGeoTiffFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    HFCAccessMode AccessMode = GetAccessMode();
    uint32_t TESTJPEGISOCompression;
    GetFilePtr()->GetField(COMPRESSION, &TESTJPEGISOCompression);

    // Cannot allow to create and or edit file with jpeg compression and data source has been stored as RGB
    // (should'nt but happen..)
    if (TESTJPEGISOCompression == COMPRESSION_JPEG)
        {
        unsigned short SamplePerPixel = 0;
        unsigned short Photometric    = 0;

        GetFilePtr()->GetField(SAMPLESPERPIXEL, &SamplePerPixel);
        GetFilePtr()->GetField(PHOTOMETRIC    , &Photometric);

        if ( Photometric == PHOTOMETRIC_RGB && SamplePerPixel == 3)
            {
            if(AccessMode.m_HasCreateAccess || AccessMode.m_HasWriteAccess)
                {
                throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());
                }
            }
        }

    // Create the descriptor for each resolution of each page
    for (uint32_t Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Select the page
        SetImageInSubImage (GetIndexOfPage(Page));

        // Compression the GetCodecsList
        HFCPtr<HCDCodec> pCodec = CreateCodecFromFile(GetFilePtr(), Page);

        // Pixel Type
        HFCPtr<HRPPixelType> PixelType = CreatePixelTypeFromFile(GetFilePtr(), Page);

        // the main 1:1 width
        uint32_t MainWidth = 0;
        // MainWidth to calc the resolution ratio
        GetFilePtr()->GetField(IMAGEWIDTH, &MainWidth);

        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < CalcNumberOfSubResolution(GetIndexOfPage(Page))+1; Resolution++)
            {
            // Obtain Resolution Information

            // Select the page and resolution
            SetImageInSubImage (GetIndexOfPage(Page)+Resolution);
            // resolution dimension
            uint32_t Width;
            uint32_t Height;
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
            GetFilePtr()->GetField(IMAGELENGTH, &Height);

            uint32_t BlockWidth (0);
            uint32_t BlockHeight (0);

            HRFBlockType BlockType(HRFBlockType::TILE);
            if (GetFilePtr()->IsTiled())
                {
                // Tile dimension
                GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);
                GetFilePtr()->GetField(TILELENGTH, &BlockHeight);
                }
            else
                {
                // Strip dimension
                BlockWidth = Width;
                GetFilePtr()->GetField(ROWSPERSTRIP, &BlockHeight);
                BlockType = HRFBlockType::STRIP;
                }


            double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MainWidth, Width);

            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                AccessMode,                                     // AccessMode,
                GetCapabilities(),                              // Capabilities,
                Ratio,                                          // XResolutionRatio,
                Ratio,                                          // YResolutionRatio,
                PixelType,                                      // PixelType,
                pCodec,                                         // Codec,
                HRFBlockAccess::RANDOM,                         // RStorageAccess,
                HRFBlockAccess::RANDOM,                         // WStorageAccess,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                HRFInterleaveType::PIXEL,                       // InterleaveType
                false,                                          // IsInterlace,
                Width,                                          // Width,
                Height,                                         // Height,
                BlockWidth,                                     // BlockWidth,
                BlockHeight,                                    // BlockHeight,
                0,                                              // BlocksDataFlag
                BlockType);                                     // BlockType

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tag information
        char*  pSystem;
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;
        SetImageInSubImage (GetIndexOfPage(Page));

        // Get common TIFF family tags
        HRFTiffFile::GetBaselineTags(&TagList, *PixelType);


        // INKNAMES Tag
        if (GetFilePtr()->GetField(INKNAMES, &pSystem))
            {
            pTag = new HRFAttributeInkNames(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // SLO
        pTag = new HRFAttributeImageSlo((unsigned short)GetScanLineOrientation().m_ScanlineOrientation);
        TagList.Set(pTag);

        // GeoRef Tag
        HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys;

        GetGeoTiffKeys(pGeoTiffKeys);

        // TranfoModel
        HFCPtr<HGF2DTransfoModel> pTransfoModel = CreateTransfoModelFromGeoTiff(pGeoTiffKeys.GetPtr(), Page);

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (AccessMode,
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       0,                           // Histogram,
                                       0,                           // Thumbnail,
                                       0,                           // ClipShape,
                                       pTransfoModel,               // TransfoModel,
                                       0,                           // Filters
                                       &TagList);                    // Defined Tag

        // Set geocoding
        pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys));

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }
/** -----------------------------------------------------------------------------
    This method create a HGF2DTransfoModel using the infomation found in the
    geo tiff file tag.

    @return true if the capabilities are the same and false otherwise. Note that access
            mode do not need to be identical but the access mode of self must be included in
            the access mode of given.

    <h3>see </h3>
    <LI><a href = "../../../doc/HRFGeoTiffFile.doc"> HRFGeoTiffFile.doc </a></LI>
    -----------------------------------------------------------------------------
 */
    HFCPtr<HGF2DTransfoModel> HRFGeoTiffFile::CreateTransfoModelFromGeoTiff(HCPGeoTiffKeys const*          pi_rpGeoTiffKeys,
                                                                            uint32_t                       pi_PageNb)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    double*  pTiePoints;
    double*  pPixelScale;
    double*  pMatrix;    // 4 x 4

    uint32_t  NbTiePoints = 0;
    uint32_t  NbPixelScale = 0;
    uint32_t  MatSize = 0;                  

    // Get all the geo reference tag.
    // Matrix
    GetFilePtr()->GetField(GEOTRANSMATRIX, &MatSize, &pMatrix);

    // TiePoint Pixel(x,y,z) System(x,y,z) ...
    GetFilePtr()->GetField(GEOTIEPOINTS, &NbTiePoints, &pTiePoints);
    // We support more than 24 tie points only in readOnly mode.
    if (NbTiePoints > 24 && ((GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)))
            throw HRFMoreThan24TiePointReadOnlyException(GetURL()->GetURL());

    // Scale x,y,z
    GetFilePtr()->GetField(GEOPIXELSCALE, &NbPixelScale, &pPixelScale);

    double FactorModelToMeter;
    bool   DefaultUnitWasFound;
    GetDefaultInterpretationGeoRef(&FactorModelToMeter);

    pTransfoModel = HCPGeoTiffKeys::CreateTransfoModelFromGeoTiff(pi_rpGeoTiffKeys, FactorModelToMeter,
                                                                    pMatrix, MatSize,
                                                                    pPixelScale, NbPixelScale,
                                                                    pTiePoints, NbTiePoints,
                                m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                &DefaultUnitWasFound);

    SetUnitFoundInFile(DefaultUnitWasFound, pi_PageNb);

    return pTransfoModel;
    }


//-----------------------------------------------------------------------------
// Protected WriteTransfoModel
// Used to call different fonction depending of the file type.
//-----------------------------------------------------------------------------

bool HRFGeoTiffFile::WriteTransfoModel(HCPGeoTiffKeys const*                    pi_rpGeoTiffKeys,
                                        const HFCPtr<HGF2DTransfoModel>&        pi_rpTransfoModel,
                                        uint32_t                               pi_Page)
    {
    WriteTransfoModelFromGeoTiff(pi_rpGeoTiffKeys,
                                 pi_rpTransfoModel,
                                 pi_Page);
    return true;
    }

//-----------------------------------------------------------------------------
// Private WriteTransfoModelFromGeoTiff - Write the GeoTiff Tag part of the model.
//-----------------------------------------------------------------------------

void HRFGeoTiffFile::WriteTransfoModelFromGeoTiff(HCPGeoTiffKeys const*                 pi_rpGeoTiffKeys,
                                                  const HFCPtr<HGF2DTransfoModel>&      pi_pModel,
                                                  uint32_t                             pi_Page)
    {
    // Translate the model units in geotiff units.
    bool    DefaultUnitWasFound = false;

    uint32_t NbTiePoints = 0;
    uint32_t NbPixelScale = 0;
    uint32_t MatSize = 0;
    uint32_t ImageWidth = 0;
    uint32_t ImageHeight = 0;

    // Get all the geo reference tag if all ready defined.
    double* pTiePoints;
    double* pPixelScale;
    double* pMatrix;  
    // Matrix
    GetFilePtr()->GetField(GEOTRANSMATRIX, &MatSize, &pMatrix);
    // TiePoint Pixel(x,y,z) System(x,y,z) ...
    GetFilePtr()->GetField(GEOTIEPOINTS, &NbTiePoints, &pTiePoints);
    // Scale x,y,z
    GetFilePtr()->GetField(GEOPIXELSCALE, &NbPixelScale, &pPixelScale);

    GetFilePtr()->GetField(IMAGEWIDTH, &ImageWidth);
    GetFilePtr()->GetField(IMAGELENGTH, &ImageHeight);

    double aMatrix[16];
    double aPixelScale[3];
    double aTiePoints[24];

    HCPGeoTiffKeys::WriteTransfoModelFromGeoTiff(pi_rpGeoTiffKeys, pi_pModel, ImageWidth, ImageHeight, m_StoreUsingMatrix,
                                                 aMatrix, MatSize,
                                                 aPixelScale, NbPixelScale,
                                                 aTiePoints, NbTiePoints,
                                                 m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
                                                 &DefaultUnitWasFound);

    if (MatSize == 16)
        GetFilePtr()->SetField(GEOTRANSMATRIX, 16, aMatrix);
    else
        GetFilePtr()->RemoveTag(GEOTRANSMATRIX);

    if (NbPixelScale == 3)
        GetFilePtr()->SetField(GEOPIXELSCALE, 3, aPixelScale);
    else
        GetFilePtr()->RemoveTag(GEOPIXELSCALE);

    if (NbTiePoints != 0)
        GetFilePtr()->SetField(GEOTIEPOINTS, NbTiePoints, aTiePoints);
    else
        GetFilePtr()->RemoveTag(GEOTIEPOINTS);

    SetUnitFoundInFile(DefaultUnitWasFound, pi_Page);
    }


/** -----------------------------------------------------------------------------
    This method is used to get the geokey tag form a geotiff file.

    @param pio_rTagList  A reference on the list of tag to be set on the
                         descriptor.

    -----------------------------------------------------------------------------
 */
void HRFGeoTiffFile::GetGeoTiffKeys(HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys)
    {
    HASSERT(po_rpGeoTiffKeys == 0);

    HFCPtr<HPMGenericAttribute> pTag;
    char* pString;
    unsigned short GeoShortValue;
    unsigned short GTModelTypeValue;
    double GeoDoubleValue;

    po_rpGeoTiffKeys = new HCPGeoTiffKeys();

    // GTModelType
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GTModelType, &GTModelTypeValue))
        {
        po_rpGeoTiffKeys->AddKey(GTModelType, (uint32_t)GTModelTypeValue);
        }
    else
        {
        po_rpGeoTiffKeys->AddKey(GTModelType, (uint32_t)TIFFGeo_ModelTypeProjected);
        }

    // GTRasterType
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GTRasterType, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GTRasterType, (uint32_t)GeoShortValue);
        }
    else
        {
        po_rpGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);
        }

    // PCSCitation
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(PCSCitation, &pString))
        {
        po_rpGeoTiffKeys->AddKey(PCSCitation, pString);
        }

    // ProjectedCSType
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjectedCSType, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)GeoShortValue);
        }
    else
        {
        if (GTModelTypeValue == TIFFGeo_ModelTypeProjected)
            {
            po_rpGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)TIFFGeo_UserDefined);
            }
        }

    // GTCitation
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GTCitation, &pString))
        {
        po_rpGeoTiffKeys->AddKey(GTCitation, pString);
        }

    // Projection
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(Projection, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(Projection, (uint32_t)GeoShortValue);
        }

    // ProjCoordTrans
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjCoordTrans, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjCoordTrans, (uint32_t)GeoShortValue);
        }

    // ProjLinearUnits
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(ProjLinearUnits, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjLinearUnits, (uint32_t)GeoShortValue);
        }

    // ProjLinearUnitSize
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjLinearUnitSize, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjLinearUnitSize, GeoDoubleValue);
        }

    // GeographicType
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeographicType, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeographicType, (uint32_t)GeoShortValue);
        }

    // GeogCitation
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogCitation, &pString))
        {
        po_rpGeoTiffKeys->AddKey(GeogCitation, pString);
        }

    // GeogGeodeticDatum
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogGeodeticDatum, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogGeodeticDatum, (uint32_t)GeoShortValue);
        }

    // GeogPrimeMeridian
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogPrimeMeridian, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogPrimeMeridian, (uint32_t)GeoShortValue);
        }

    // GeogLinearUnits
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogLinearUnits, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogLinearUnits, (uint32_t)GeoShortValue);
        }

    // GeogLinearUnitSize
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogLinearUnitSize, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogLinearUnitSize, GeoDoubleValue);
        }

    // GeogAngularUnits
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogAngularUnits, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogAngularUnits, (uint32_t)GeoShortValue);
        }

    // GeogAngularUnitSize
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogAngularUnitSize, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogAngularUnitSize, GeoDoubleValue);
        }

    // GeogEllipsoid
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogEllipsoid, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogEllipsoid, (uint32_t)GeoShortValue);
        }

    // GeogSemiMajorAxis
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogSemiMajorAxis, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogSemiMajorAxis, GeoDoubleValue);
        }

    // GeogSemiMinorAxis
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogSemiMinorAxis, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogSemiMinorAxis, GeoDoubleValue);
        }

    // GeogInvFlattening
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogInvFlattening, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogInvFlattening, GeoDoubleValue);
        }

    // GeogAzimuthUnits
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GeogAzimuthUnits, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogAzimuthUnits, (uint32_t)GeoShortValue);
        }

    // GeogPrimeMeridianLong
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(GeogPrimeMeridianLong, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(GeogPrimeMeridianLong, GeoDoubleValue);
        }

    // ProjStdParallel1
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStdParallel1, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjStdParallel1, GeoDoubleValue);
        }

    // ProjStdParallel2
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStdParallel2, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjStdParallel2, GeoDoubleValue);
        }

    // ProjNatOriginLong
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjNatOriginLong, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjNatOriginLong, GeoDoubleValue);
        }

    // ProjNatOriginLat
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjNatOriginLat, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjNatOriginLat, GeoDoubleValue);
        }

    // ProjFalseEasting
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseEasting, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseEasting, GeoDoubleValue);
        }

    // ProjFalseNorthing
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseNorthing, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseNorthing, GeoDoubleValue);
        }

    // ProjFalseOriginLong
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginLong, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseOriginLong, GeoDoubleValue);
        }

    // ProjFalseOriginLat
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginLat, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseOriginLat, GeoDoubleValue);
        }

    // ProjFalseOriginEasting
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginEasting, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseOriginEasting, GeoDoubleValue);
        }

    // ProjFalseOriginNorthing
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjFalseOriginNorthing, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjFalseOriginNorthing, GeoDoubleValue);
        }

    // ProjCenterLong
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterLong, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjCenterLong, GeoDoubleValue);
        }

    // ProjCenterLat
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterLat, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjCenterLat, GeoDoubleValue);
        }

    // ProjCenterEasting
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterEasting, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjCenterEasting, GeoDoubleValue);
        }

    // ProjCenterNorthing
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjCenterNorthing, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjCenterEasting, GeoDoubleValue);
        }

    // ProjScaleAtNatOrigin
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjScaleAtNatOrigin, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjScaleAtNatOrigin, GeoDoubleValue);
        }

    // ProjScaleAtCenter
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjScaleAtCenter, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjScaleAtCenter, GeoDoubleValue);
        }

    // ProjAzimuthAngle
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjAzimuthAngle, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjAzimuthAngle, GeoDoubleValue);
        }

    // ProjStraightVertPoleLong
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjStraightVertPoleLong, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjStraightVertPoleLong, GeoDoubleValue);
        }

    // ProjRectifiedGridAngle
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(ProjRectifiedGridAngle, &GeoDoubleValue))
        {
        po_rpGeoTiffKeys->AddKey(ProjRectifiedGridAngle, GeoDoubleValue);
        }

    // VerticalCSType
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalCSType, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(VerticalCSType, (uint32_t)GeoShortValue);
        }

    // VerticalCitation
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValues(VerticalCitation, &pString))
        {
        po_rpGeoTiffKeys->AddKey(VerticalCitation, pString);
        }

    // VerticalDatum
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalDatum, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(VerticalDatum, (uint32_t)GeoShortValue);
        }

    // VerticalUnits
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(VerticalUnits, &GeoShortValue))
        {
        po_rpGeoTiffKeys->AddKey(VerticalUnits, (uint32_t)GeoShortValue);
        }
    }

//-----------------------------------------------------------------------------
// Public GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------

const HGF2DWorldIdentificator HRFGeoTiffFile::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() > 0);
    HPRECONDITION(GetFilePtr() != 0);

    HGF2DWorldIdentificator World = HGF2DWorld_GEOTIFFUNKNOWN;

    // Change world id if GTModelType is ModelTypeGeographic.
    unsigned short GeoShortValue;
    if (GetFilePtr()->GetGeoKeyInterpretation().GetValue(GTModelType, &GeoShortValue))
        {
        switch (GeoShortValue)
            {
            case TIFFGeo_ModelTypeProjected:
                World = HGF2DWorld_HMRWORLD;
                break;

            case TIFFGeo_ModelTypeGeographic:
                World = HGF2DWorld_GEOGRAPHIC;
                break;

            case TIFFGeo_ModelTypeSpecialMSJ:
            case TIFFGeo_ModelTypeSpecialMSSE:
                World = HGF2DWorld_INTERGRAPHWORLD;
                break;
            }
        }

    if (m_DefaultCoordSysIsIntergraphIfUnitNotResolved && !m_DefaultUnitWasFound[0])
        {
        World = HGF2DWorld_INTERGRAPHWORLD;
        }

    return World;
    }

/** -----------------------------------------------------------------------------
    This method must be called before all other calls since result is then unpredictable.
    Application must call this method to set :
    - A default unit. (By default, default unit is meter.)

    - To impose an interpretation of ProjectedCSType and ProjLinearUnits.
    Refer to the section concerning interpretation of geo-reference for details.
    Our implementation exclude ProjLinearUnits if the ProjectedCSType != UserDefined.

    - To impose an interpretation on the unit.
    Refer to the section concerning interpretation of geo-reference for details.
    Our implementation will exclude ProjLinearUnits and use INTERGRAPH coordsys only if unit
    cannot be resolve from other settings and this flag is true.(see TR 97326)

    This method doesn't set the status flag Changed on the TransfoModel in the PageDescriptor.

    @param pi_RatioToMeter      Ratio used to translate the model to meter.
    @param pi_InterpretUnit     true : The files will be opened using ProjLinearUnits, can
                                   redefine the unit of ProjectedCSType.
                                false: Our standard.(default)
    @param pi_InterpretUnitINTGR true : We consider the file to be in the INTERGRAPH coord sys
                                   is unit cannot be resolved (see TR 97326).
                                 false: Our standard.(default)
    -----------------------------------------------------------------------------
 */
void HRFGeoTiffFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                            uint32_t pi_Page,
                                            bool   pi_CheckSpecificUnitSpec,
                                            bool   pi_InterpretUnitINTGR)
    {
    m_RatioToMeter = pi_RatioToMeter;
    m_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation = pi_CheckSpecificUnitSpec ;
    m_DefaultCoordSysIsIntergraphIfUnitNotResolved            = pi_InterpretUnitINTGR;

    // Update the model in each page
    uint32_t NbPage = CountPages();
    for (uint32_t Page=0; Page < NbPage; Page++)
        {
        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

        // Select the page
        SetImageInSubImage (GetIndexOfPage(Page));

        // TranfoModel
        HCPGeoTiffKeys const& geoTiffKeys = pPageDescriptor->GetRasterFileGeocoding().GetGeoTiffKeys();

        HFCPtr<HGF2DTransfoModel> pTransfoModel = CreateTransfoModelFromGeoTiff(&geoTiffKeys,
                                                                                Page);
        pPageDescriptor->SetTransfoModel(*pTransfoModel);
        pPageDescriptor->SetTransfoModelUnchanged();
        }
    }

/** -----------------------------------------------------------------------------
    This method is used to indicate that the geo-reference must be stored using the
    ModelTransformationTag in the GeoTIFF file. This setting cannot be changed back
    to its original value. This setting will not result in the storage of the file.
    Only if geo-reference has been modified will this setting apply.

    <h3>see </h3>
    <LI><a href = "../../../doc/HRFGeoTiffFile.doc"> HRFGeoTiffFile.doc </a></LI>
    -----------------------------------------------------------------------------
*/
void HRFGeoTiffFile::StoreUsingModelTransformationTag ()
    {
    m_StoreUsingMatrix = true;
    }

//----------------------------------------------------------------- Static methods





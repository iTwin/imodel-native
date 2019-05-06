//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFPDFFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFPDFFile.h>

#if defined(IPP_HAVE_PDF_SUPPORT) 

#include <ImagePP/all/h/HRFMacros.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>

#include <ImagePP/all/h/HFCCallbacks.h>
#include <ImagePP/all/h/HFCCallbackRegistry.h>
#include <ImagePP/all/h/HFCURLFile.h>

#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DTranslation.h>

#include <ImagePP/all/h/HMDAnnotations.h>
#include <ImagePP/all/h/HMDLayers.h>
#include <ImagePP/all/h/HMDLayerInfoPDF.h>
#include <ImagePP/all/h/HMDVolatileLayers.h>
#include <ImagePP/all/h/HMDContext.h>

#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HRFPDFException.h>
#include <ImagePP/all/h/HRFPDFEditor.h>

#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFUtility.h>

#include <ImagePP/all/h/HRPPixelTypeV24B8G8R8.h>

#include <ImagePP/all/h/HVETileIDIterator.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>

double HRFPDFFile::s_dpiConvertScaleFactor = (96.0 / 72.0);



//-----------------------------------------------------------------------------
// PDFInterfaceWrapper for MicroStation
//-----------------------------------------------------------------------------\

#include "HRFPDFLibInterface.h"

ExtensionID gExtensionID = 0;       // For PDF!!!

typedef struct
    {
    Utf8String                          FileName;
    const HFCAuthenticationCallback* pPasswordCallback;
    uint16_t                  RetryCount;
    bool                            Canceled;
    bool                            HasPassword;
    } OpenPasswordCallbackInfo;


//-----------------------------------------------------------------------------
// class PDFAuthenticationError
//
// Authentication error type specific to PDF files.
//-----------------------------------------------------------------------------
class PDFAuthenticationError : public HFCAuthenticationError
    {
public:
    explicit PDFAuthenticationError(const HRFPDFException& pi_rException)
        :   m_RelatedException(pi_rException),
            HFCAuthenticationError()
        {}

private:
    virtual Utf8String _ToString() const
        {
        return m_RelatedException.GetExceptionMessage();
        }

    virtual void _Throw() const
        {
        throw m_RelatedException;
        }

    const HRFPDFException    m_RelatedException;
    }; 


ASBool ClientAuthUserProc(PDDoc pdDoc, void* clientData)
    {


    if (clientData == 0)
        return false;

    // call callback

    // TR #159510
    // for now, RasterManager don't support PDF file with password. RasterManager must be deal
    // with Sub message ID 12. In this case, HasPassword must be different that S_OK
    struct HFCPDFAuthentication PDFAuthentication(((OpenPasswordCallbackInfo*)clientData)->FileName,
                                                  HFCPDFAuthentication::OPEN,
                                                  ((OpenPasswordCallbackInfo*)clientData)->RetryCount);


    ((OpenPasswordCallbackInfo*)clientData)->HasPassword =
        ((OpenPasswordCallbackInfo*)clientData)->pPasswordCallback->GetAuthentication(&PDFAuthentication);

    ((OpenPasswordCallbackInfo*)clientData)->Canceled = ((OpenPasswordCallbackInfo*)clientData)->pPasswordCallback->IsCancelled();

    if (((OpenPasswordCallbackInfo*)clientData)->HasPassword && !((OpenPasswordCallbackInfo*)clientData)->Canceled)
        {
        // supply the user password
        // request Open permission
        PDPerms permsRequested = pdPermOpen;
        PDPerms permsAllowed;
        DURING
        /* Requesting Open permission requires a user password */
        permsAllowed = PDDocAuthorize(pdDoc, permsRequested, const_cast<char*>(PDFAuthentication.GetPassword().c_str()));
        HANDLER
        #if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
            Exception; // avoid C4101
        #endif
        permsAllowed = 0;
        END_HANDLER

        return (permsRequested & permsAllowed) != 0;
        }
    else
        return false;
    };

/**************************************************************************

Directory List Handler

**************************************************************************/

class IppPDFWrapper : public PDFWrapper
    {
public:

    IppPDFWrapper(const Utf8String& pi_rFileName)
        {
        HPRECONDITION(!pi_rFileName.empty());

        volatile ASPathName PathName    = 0;
        ASFileSys           FileSys     = 0;

        m_Document = 0;
        m_FileName = pi_rFileName;

        HRFPDFFile::InitializePDFLibraryInThread();

        size_t  destinationBuffSize = pi_rFileName.GetMaxLocaleCharBytes();
        char*  FileSpecMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(FileSpecMBS,pi_rFileName.c_str(),destinationBuffSize);

        PathName = ASFileSysCreatePathName(FileSys,
                                           ASAtomFromString("Cstring"),
                                           FileSpecMBS,
                                           0);

        OpenPasswordCallbackInfo CallbackInfo;

        const HFCAuthenticationCallback* pCallback(HFCAuthenticationCallback::
                                                   GetCallbackFromRegistry(HFCPDFAuthentication::CLASS_ID));

        if (pCallback != 0)
            {
            CallbackInfo.FileName = pi_rFileName;
            CallbackInfo.pPasswordCallback = pCallback;
            CallbackInfo.RetryCount = 0;
            }

        HFCPDFAuthentication PDFAuthentication(pi_rFileName, HFCPDFAuthentication::OPEN);

        try
            {
            bool TryAgain;
            do
                {
                TryAgain = false;
                DURING
                /* Open pdDoc from asPathName.*/
                m_Document = PDDocOpenEx(PathName,
                                         FileSys,
                                         (pCallback != 0 ? ASCallbackCreateProto(PDAuthProcEx, &ClientAuthUserProc) : 0),
                                         (pCallback != 0 ? &CallbackInfo : 0),
                                         false);

                HANDLER
                    #if USE_CPLUSPLUS_EXCEPTIONS_FOR_ASEXCEPTIONS
                        Exception; // avoid C4101
                    #endif
                if (ErrGetCode(ERRORCODE) != pdErrNeedPassword)
                    {
                    throw HRFPDFException(pi_rFileName, ERRORCODE);
                    }

                if (pCallback)
                    {
                    // increment count in callback
                    PDFAuthentication.IncrementRetryCount();
                    ++CallbackInfo.RetryCount;

                    if (CallbackInfo.Canceled)
                        {
                        throw HRFAuthenticationCancelledException( pi_rFileName);
                        }
                    else if (!CallbackInfo.HasPassword)
                        {
                        throw HRFNeedOpenPasswordException(pi_rFileName);
                        }
                    else if (CallbackInfo.RetryCount >= pCallback->RetryCount(PDFAuthentication.GetPasswordType()))
                        {
                        throw HRFAuthenticationMaxRetryCountReachedException(pi_rFileName);
                        }
                    else
                        {
                        HFCPtr<HFCAuthenticationError> pLastError(new PDFAuthenticationError(HRFPDFException(pi_rFileName,
                                                                                             ERRORCODE)));
                        PDFAuthentication.PushLastError(pLastError);
                        TryAgain = true;
                        }
                    }
                else
                    throw HRFNeedOpenPasswordException(pi_rFileName);

                END_HANDLER
                }
            while(TryAgain);

            bool HasRestrictedOp = HRFPDFLibInterface::IsOpRestrictedPDF(m_Document);

            if (HasRestrictedOp == true)
                {
                throw HRFOperationRestrictedPDFNotSupportedException(pi_rFileName);
                }

            m_MainThreadId  = GetCurrentThreadId();

            PDDocGetSecurityData(m_Document);
            }
        catch (...)
            {
            //Ensure that Terminate is called so we don't have a mismatch between
            //the number of initializations and terminations of the PDF library.
            HRFPDFFile::TerminatePDFLibraryInThread();
            throw;
            }
        /*
                // copy password
                void *pSecData = PDDocGetSecurityData(m_Document);

                if (PDDocPermRequest(m_Document, PDPermReqObjDoc, PDPermReqOprCopy, NULL) != PDPermReqGranted)
                {
                    if (pCallback != 0)
                    {
                        Byte* pPassword;
                        size_t PasswordSize;
                        HRESULT HasPassword;
                        UShort Retry = 0;

                        bool TryAgain;
                        do
                        {
                            TryAgain = false;
        #if 0
                            HasPassword = pCallback->GetPassword(pi_rFileName,
                                                                 RestrictionPassword,
                                                                 1,   // message ID
                                                                 11,  // Sub message ID
                                                                 "File protected. Enter a Permission Password",
                                                                 Retry,
                                                                 &pPassword,
                                                                 &PasswordSize);
        #endif
                            // TR #159510
                            // for now, RasterManager don't support PDF file with password. RasterManager must be deal
                            // with Sub message ID 12. In this case, HasPassword must be different that S_OK
                            HasPassword = pCallback->GetPassword(pi_rFileName,
                                                                 HRFPasswordCallback::RestrictionPassword,
                                                                 1,   // message ID
                                                                 12,  // Sub message ID
                                                                 "This PDF document is protected using Password Secutity.",
                                                                 Retry,
                                                                 &pPassword,
                                                                 &PasswordSize);

                            if (HasPassword == S_OK)
                            {

                                HAutoPtr<char> pmbPassword;
                                pmbPassword =  HFCUnicodeConverter::FromWideCharToMultiByte((Utf8Char*)pPassword);

                                if (PDDocAuthorize(m_Document, pdPermCopy, pmbPassword) != pdPermCopy)
                                {
                                    TryAgain = true;
                                    ++Retry;
                                }
                                else
                                {
                                    TryAgain = false;
                                }
                             }
                             else
                                throw HRFNeedOpenPasswordException(pi_rFileName);

                        } while(TryAgain && Retry < 3);
                    }
                    else
                        throw HRFNeedRestrictionPasswordException(pi_rFileName);
                }*/
        };

    virtual ~IppPDFWrapper()
        {
        if (m_Document != 0)
            PDDocClose(m_Document);

        HRFPDFFile::TerminatePDFLibraryInThread();
        };

    void* CloneDocument()
        {
        PDDoc               pDocument;

        volatile ASPathName PathName    = 0;
        ASFileSys           FileSys     = 0;

        PDDocGetSecurityData(m_Document);

        size_t  destinationBuffSize = m_FileName.GetMaxLocaleCharBytes();
        char*  FileNameMBS= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(FileNameMBS,m_FileName.c_str(),destinationBuffSize);

        PathName = ASFileSysCreatePathName(FileSys,
                                           ASAtomFromString("Cstring"),
                                           FileNameMBS,
                                           0);

        OpenPasswordCallbackInfo CallbackInfo;
        HFCAuthenticationCallback* pCallback = (HFCAuthenticationCallback*)HFCCallbackRegistry::GetInstance()->GetCallback(HFCAuthenticationCallback::CLASS_ID);

        if (pCallback != 0)
            {
            CallbackInfo.FileName = m_FileName;
            CallbackInfo.pPasswordCallback = pCallback;
            CallbackInfo.RetryCount = 0;
            }

        HFCPDFAuthentication PDFAuthentication(m_FileName, HFCPDFAuthentication::OPEN);

        /* Open pdDoc from asPathName.*/
        pDocument = PDDocOpenEx(PathName,
                                FileSys,
                                (pCallback != 0 ? ASCallbackCreateProto(PDAuthProcEx, &ClientAuthUserProc) : 0),
                                (pCallback != 0 ? &CallbackInfo : 0),
                                false);

        return pDocument;
        }

    void FreeClonedDocument(void* pi_pDoc)
        {
        PDDocClose((PDDoc)pi_pDoc);
        }

    void* GetDocument()
        {
        uint32_t ThreadId = GetCurrentThreadId();
        void*  pDocument = 0;

        if (ThreadId == m_MainThreadId)
            {
            pDocument = m_Document;
            }
        else
            {
            pDocument = CloneDocument();
            }

        return pDocument;
        }

    uint32_t CountPages() const
        {
        HPRECONDITION(m_Document != 0);
        return HRFPDFLibInterface::CountPages(m_Document);
        };

    void PageSize(uint32_t   pi_Page,
                  uint32_t*    po_pWidth,
                  uint32_t*    po_pHeight,
                  double*   po_pDPI)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());
        HPRECONDITION(po_pWidth != 0);
        HPRECONDITION(po_pHeight != 0);
        HPRECONDITION(po_pDPI != 0);

        HRFPDFLibInterface::PageSize(m_Document,
                                     pi_Page,
                                     *po_pWidth,
                                     *po_pHeight,
                                     *po_pDPI);
        };
    void GetMaxResolutionSize(uint32_t   pi_Page, double dpiConvertScaleFactor, uint32_t&     po_maxResSize)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());

        HRFPDFLibInterface::GetMaxResolutionSize(m_Document, pi_Page, dpiConvertScaleFactor, po_maxResSize);
        };

    void GetLayers(uint32_t pi_Page, HFCPtr<HMDLayers>& pi_rpLayers)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());

        HRFPDFLibInterface::GetLayers(m_Document,
                                      pi_Page,
                                      pi_rpLayers);
        }

    void SetLayerVisibility(uint32_t                   pi_Page,
                            HFCPtr<HMDVolatileLayers>& pi_rpVolatileLayers)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());
        HPRECONDITION(pi_rpVolatileLayers != 0);

        if (m_MainThreadId == GetCurrentThreadId())
            {
            HASSERT(pi_rpVolatileLayers != 0);

            HRFPDFLibInterface::SetLayerVisibility(m_Document,
                                                   pi_Page,
                                                   pi_rpVolatileLayers);
            }
        }

    virtual void GetAnnotations(uint32_t                pi_Page,
                                HFCPtr<HMDAnnotations>& po_rpAnnotations)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());

        HRFPDFLibInterface::GetAnnotations(m_Document,
                                           pi_Page,
                                           po_rpAnnotations);
        }

    virtual void GetGeocodingAndReferenceInfo(uint32_t                       pi_Page,
                                              uint32_t                       pi_RasterizePageWidth,
                                              uint32_t                       pi_RasterizePageHeight,
                                              GeoCoordinates::BaseGCSPtr&    po_rpGeocoding,
                                              HFCPtr<HGF2DTransfoModel>&     po_rpGeoreference)
        {
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());

        HRFPDFLibInterface::GetGeocodingAndReferenceInfo(m_Document,
                                                         pi_Page,
                                                         pi_RasterizePageWidth,
                                                         pi_RasterizePageHeight,
                                                         po_rpGeocoding,
                                                         po_rpGeoreference);
        }
           
    virtual void GetDimensionForDWGUnderlay(uint32_t pi_Page,                                                                                                          
                                            double& po_xDimension, 
                                            double& po_yDimension) const   
        {   
        HPRECONDITION(m_Document != 0);
        HPRECONDITION(pi_Page < CountPages());
          
        HRFPDFLibInterface::GetDimensionForDWGUnderlay(m_Document,
                                                       pi_Page,                                                         
                                                       po_xDimension, 
                                                       po_yDimension);       
        }     

    uint32_t GetMainThreadId() const
        {
        return m_MainThreadId;
        }


private:

    uint16_t            m_CurrentInd;
    PDDoc                     m_Document;

    uint32_t                 m_MainThreadId;
    Utf8String                   m_FileName;
    };


//-----------------------------------------------------------------------------
// HRFPDFBlockCapabilities
//-----------------------------------------------------------------------------
class HRFPDFBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPDFBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,                             // AccessMode
                                  PDF_TILE_SIZE_IN_BYTES,                   // MaxSizeInBytes
                                  PDF_TILE_WIDTH_HEIGHT,                         // MinWidth
                                  PDF_TILE_WIDTH_HEIGHT,                         // MaxWidth
                                  0,                                         // WidthIncrement
                                  PDF_TILE_WIDTH_HEIGHT,                         // MinHeight
                                  PDF_TILE_WIDTH_HEIGHT,                         // MaxHeight
                                  0));                                       // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFPDFCodecCapabilities
//-----------------------------------------------------------------------------
class HRFPDFCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFPDFCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFPDFBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFPDFCapabilities
//-----------------------------------------------------------------------------
HRFPDFCapabilities::HRFPDFCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV24B8G8R8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24B8G8R8::CLASS_ID,
                                   new HRFPDFCodecCapabilities()));


    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    // 16bit integer is the data type used by the PDF library API. This means that the maximum size
    // supported by the PDF is 32767. We need to adjust with the DPI scale factor to prevent the 16 bit overflow
    uint32_t MaxResolutionSize = (uint32_t)(32767.0 / HRFPDFFile::s_dpiConvertScaleFactor);
    Add(new HRFMultiResolutionCapability(HFC_READ_ONLY,
                                         true,              // SinglePixelType,
                                         true,              // SingleBlockType,
                                         false,             // ArbitaryXRatio,
                                         false,             // ArbitaryYRatio
                                         true,              // XYRatioLocked, default value
                                         16,                // smallest res width, default value
                                         16,                // smallest res height, default value
                                         MaxResolutionSize, // biggest res width
                                         MaxResolutionSize, // biggest res height
                                         true));            // UnlimitesResolution

    Add(new HRFMultiPageCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0)));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Geocoding capability - Put all tags since the geocoding in a PDF file is specfied by a WKT
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_WRITE_CREATE));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
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

HFC_IMPLEMENT_SINGLETON(HRFPDFCreator)

//-----------------------------------------------------------------------------
// HRFPDfCreator
// This is the creator to instantiate PDF format
//-----------------------------------------------------------------------------
HRFPDFCreator::HRFPDFCreator()
    : HRFRasterFileCreator(HRFPDFFile::CLASS_ID)
    {
    // PDF capabilities instance member initialization
    m_pCapabilities = 0;
    }

bool HRFPDFCreator::CanRegister() const
    {
    return HRFPDFFile::CanLoadPDFWrapper();
    }

// Identification information
Utf8String HRFPDFCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_PDF()); //Adobe File Format
    }

// Identification information
Utf8String HRFPDFCreator::GetSchemes() const
    {
    return Utf8String(HFCURLFile::s_SchemeName());
    }

// Identification information
Utf8String HRFPDFCreator::GetExtensions() const
    {
    return Utf8String("*.pdf");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFPDFCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFPDFFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

// Opens the file and verifies if it is the right type
bool HRFPDFCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    bool       bResult = false;

    HPRECONDITION(pi_rpURL != 0);

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        {
        if (HRFPDFFile::CanLoadPDFWrapper())
            {
            FILE* pFile;
            Utf8String FileName = ((const HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() +
                               "\\" +
                               ((const HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            if ((pFile = _wfopen(FileName.c_str(), "rb")) != 0)
                {
                char aBuffer[5] = {0, 0, 0, 0, 0};
                fread(aBuffer, 1, 5, pFile);
                fclose(pFile);
                if (strncmp(aBuffer, "%PDF-", 5) == 0)
                    bResult = true;
                }
            }
        }

    return bResult;
    }

// Create or get the singleton capabilities of PDF file.
const HFCPtr<HRFRasterFileCapabilities>& HRFPDFCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFPDFCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFPDFFile::~HRFPDFFile()
    {
    m_pPDFWrapper = 0;

    ContextPageTilePool::iterator PageTilePool(m_ContextPageTilePool.begin());

    while (PageTilePool != m_ContextPageTilePool.end())
        {
        PageTilePool::iterator TilePool(PageTilePool->second.begin());
        while (TilePool != PageTilePool->second.end())
            {
            TilePool::iterator TilePoolItr(TilePool->second.begin());
            while (TilePoolItr != TilePool->second.end())
                {
                delete [] TilePoolItr->second;
                TilePoolItr++;
                }
            TilePool->second.clear();
            TilePool++;
            }

        PageTilePool++;
        }

    m_ContextPageTilePool.clear();
    }

//-----------------------------------------------------------------------------
// Public
// Function for initializing the PDF library
//-----------------------------------------------------------------------------
int32_t HRFPDFFile::InitializePDFLibraryInThread()
    {
    //BEIJING_WIP_THREADS.
    if (!PdfLibInitializerManager::Initialize())
        throw HFCDllNotFoundException("Adobe PDF Dlls");

    PDFLDataRec* pPDFLData = PdfLibInitializerManager::GetPDFLDataInitInfo();
    ASInt32 Status = PDFLInitHFT(pPDFLData);
    HASSERT(Status == 0);           
        
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
// Function for terminating the PDF library
//-----------------------------------------------------------------------------
void HRFPDFFile::TerminatePDFLibraryInThread()
    {
    //BEIJING_WIP_THREADS.
    PDFLTermHFT();
    PdfLibInitializerManager::Terminate();
    }



//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFPDFFile::CreateResolutionEditor(uint32_t        pi_Page,
                                                        uint16_t pi_Resolution,
                                                        HFCAccessMode   pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution == 1);

    return new HRFPDFEditor(this,
                            pi_Page,
                            1.0,
                            pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFPDFFile::CreateUnlimitedResolutionEditor(uint32_t       pi_Page,
                                                                 double        pi_Resolution,
                                                                 HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);

    return new HRFPDFEditor(this,
                            pi_Page,
                            pi_Resolution,
                            pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFPDFFile::Save()
    {
    //Nothing do to here
    }

//-----------------------------------------------------------------------------
// Public
// SetContext
// Set the context
//-----------------------------------------------------------------------------
void HRFPDFFile::SetContext(uint32_t                 pi_PageIndex,
                            const HFCPtr<HMDContext>& pi_rContext)
    {
    HRFRasterFile::SetContext(pi_PageIndex, pi_rContext);

    HFCPtr<HMDMetaDataContainer> pVolatileLayers;

    pVolatileLayers = pi_rContext->GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO);

    if (pVolatileLayers != 0)
        {
        HASSERT(pVolatileLayers->IsCompatibleWith(HMDVolatileLayers::CLASS_ID));

        SetLayerVisibility(pi_PageIndex,
                           (HFCPtr<HMDVolatileLayers>&)pVolatileLayers);
        }
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFPDFFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HASSERT(0);

    return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetMainThreadId
// Get main thread ID
//-----------------------------------------------------------------------------
uint32_t HRFPDFFile::GetMainThreadId() const
    {
    return m_MainThreadId;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPDFFile::GetCapabilities () const
    {
    return HRFPDFCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
uint32_t HRFPDFFile::CountPages() const
    {
    return m_NumPages;
    }

//-----------------------------------------------------------------------------
// Public
// GetDimensionForDWGUnderlay
//-----------------------------------------------------------------------------
void HRFPDFFile::GetDimensionForDWGUnderlay(uint32_t pi_Page,                                                                                                          
                                            double& po_xDimension, 
                                            double& po_yDimension) const

    {            
    m_pPDFWrapper->GetDimensionForDWGUnderlay(pi_Page,
                                              po_xDimension, 
                                              po_yDimension);
    }

//-----------------------------------------------------------------------------
// Public
// GetPageDescriptor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFPDFFile::GetPageDescriptor(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < m_NumPages);

    HFCPtr<HRFPageDescriptor> pPage;

    Pages::const_iterator Itr(m_Pages.find(pi_Page));

    if (Itr == m_Pages.end())
        {
        uint32_t PageWidth;
        uint32_t PageHeight;
        double PageDPI;

        m_pPDFWrapper->PageSize(pi_Page, &PageWidth, &PageHeight, &PageDPI);
                                             
        HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor =
            new HRFResolutionDescriptor(GetAccessMode(),
                                        GetCapabilities(),
                                        1.0,
                                        1.0,
                                        new HRPPixelTypeV24B8G8R8(),
                                        new HCDCodecIdentity(),
                                        HRFBlockAccess::RANDOM,                         // RBlockAccess,
                                        HRFBlockAccess::RANDOM,                         // WBlockAccess,
                                        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                        HRFInterleaveType::PIXEL,                       // InterleaveType
                                        false,
                                        PageWidth,
                                        PageHeight,
                                        PDF_TILE_WIDTH_HEIGHT,
                                        PDF_TILE_WIDTH_HEIGHT);

        //TFS#8555
        uint32_t MaxResolutionSize;
        m_pPDFWrapper->GetMaxResolutionSize(pi_Page, HRFPDFFile::s_dpiConvertScaleFactor, MaxResolutionSize);

        HPMAttributeSet             TagList;
        HFCPtr<HPMGenericAttribute> pTag;

        //The resolution is always expressed in inches (DPI)
        uint16_t             ResolutionUnit = 2;

        pTag = new HRFAttributeResolutionUnit(ResolutionUnit);
        TagList.Set(pTag);

        pTag = new HRFAttributeXResolution(PageDPI);
        TagList.Set(pTag);

        pTag = new HRFAttributeYResolution(PageDPI);
        TagList.Set(pTag);

        //Search for some geo-coding and geo-reference information
        GeoCoordinates::BaseGCSPtr pBaseGCS;
        HFCPtr<HGF2DTransfoModel>     pGeoreference;

        if (GCSServices->_IsAvailable())
            {
            m_pPDFWrapper->GetGeocodingAndReferenceInfo(pi_Page,
                                                        PageWidth,
                                                        PageHeight,
                                                        pBaseGCS,
                                                        pGeoreference);
            }

        pPage = new HRFPageDescriptor(GetAccessMode(),
                                      GetCapabilities(),
                                      pResolutionDescriptor,
                                      0,
                                      0,
                                      0,
                                      0,
                                      pGeoreference,
                                      0,
                                      &TagList,
                                      0,
                                      false,        // resizable
                                      true,         // unlimited
                                      1,
                                      1,
                                      MaxResolutionSize,
                                      MaxResolutionSize);

        if (!pBaseGCS.IsNull() && pBaseGCS->IsValid())
            pPage->SetGeocoding(pBaseGCS.get());


        HFCPtr<HMDMetaDataContainerList> pMDContainers;
        HFCPtr<HMDLayers>                pLayers;

        pMDContainers = new HMDMetaDataContainerList();

        //Layers
        m_pPDFWrapper->GetLayers(pi_Page, pLayers);

        if ((pLayers != 0))
            {
            pMDContainers->SetMetaDataContainer((HFCPtr<HMDMetaDataContainer>&)pLayers);
            }

        //Annotation
        HFCPtr<HMDAnnotations> pAnnotations;

        m_pPDFWrapper->GetAnnotations(pi_Page, pAnnotations);

        if ((pAnnotations != 0))
            {
            pMDContainers->SetMetaDataContainer((HFCPtr<HMDMetaDataContainer>&)pAnnotations);
            }


        //If the metadata container list isn't empty, set it in the page descriptor
        if (pMDContainers->GetNbContainers() > 0)
            {
            pPage->SetListOfMetaDataContainer(pMDContainers);
            }

        m_Pages.insert(Pages::value_type(pi_Page, pPage));
        }
    else
        pPage = Itr->second;

    HPRECONDITION(pPage != 0);
    return pPage;
    }


bool HRFPDFFile::CanLoadPDFWrapper()
    {
    return PdfLibInitializerManager::IsPDFLibAvailable();
    }


//-----------------------------------------------------------------------------
// public
// CanPerformLookAhead
//
//-----------------------------------------------------------------------------
bool HRFPDFFile::CanPerformLookAhead (uint32_t pi_Page) const
    {
    //TR 271618 : Disable for now since this increase the time during the print
    //            with use of 1024 per 1024 tiles (the increase was caused by
    //            the fact in HRATiledRaster there is not check to determine if
    //            the region requested by the lookahead is already covered by HRA tiles.
    return false;
    }

//-----------------------------------------------------------------------------
// public
// SetLayerVisibility
//
// Set the visibility of the layers
//-----------------------------------------------------------------------------
void HRFPDFFile::SetLayerVisibility(uint32_t                   pi_Page,
                                    HFCPtr<HMDVolatileLayers>& pi_rpVolatileLayers)
    {
    m_pPDFWrapper->SetLayerVisibility(pi_Page, pi_rpVolatileLayers);
    }


//-----------------------------------------------------------------------------
// public
// SetLayerVisibility
//
// Set the visibility of the layers
//-----------------------------------------------------------------------------
void HRFPDFFile::SetLayerVisibility(uint32_t pi_Page)
    {
    HFCPtr<HMDVolatileLayers> pVolatileLayers;

    m_pPDFWrapper->SetLayerVisibility(pi_Page, pVolatileLayers);
    }

//-----------------------------------------------------------------------------
// public
// setLookAhead
//
// Sets the LookAhead for a list of blocks
//-----------------------------------------------------------------------------
void HRFPDFFile::SetLookAhead(uint32_t               pi_Page,
                              const HGFTileIDList&   pi_rBlocks,
                              uint32_t               pi_ConsumerID,
                              bool                  pi_Async)
    {
    HASSERT(0);
    }

//-----------------------------------------------------------------------------
// public
// SetLookAhead
//
// Sets the LookAhead for a shape
//-----------------------------------------------------------------------------
void HRFPDFFile::SetLookAhead(uint32_t               pi_Page,
                              uint16_t        pi_Resolution,
                              const HVEShape&        pi_rShape,
                              uint32_t               pi_ConsumerID,
                              bool                  pi_Async)
    {
    //Something with more bits than UInt64 should be used on architecture with pointer greater than 64 bits.
    HPRECONDITION(sizeof(GetContext(pi_Page).GetPtr()) <= 8);

    HFCMonitor Monitor(m_TilePoolKey);

    if (!pi_rShape.IsEmpty())
        {
        ContextPageTilePool::iterator PageTilePool(m_ContextPageTilePool.find((uint64_t)GetContext(pi_Page).GetPtr()));

        //Create the context page tile pool.
        if (PageTilePool == m_ContextPageTilePool.end())
            {
            pair<ContextPageTilePool::iterator, bool> InsertPair(m_ContextPageTilePool.insert(ContextPageTilePool::value_type((uint64_t)GetContext(pi_Page).GetPtr(), HRFPDFFile::PageTilePool())));
            HPOSTCONDITION(InsertPair.second);

            PageTilePool = InsertPair.first;
            }

        //Create the page tile pool.
        PageTilePool::iterator TilePool(PageTilePool->second.find(pi_Page));
        if (TilePool != PageTilePool->second.end())
            {
            HRFPDFFile::TilePool::iterator TilePoolItr(TilePool->second.begin());
            while (TilePoolItr != TilePool->second.end())
                {
                delete [] TilePoolItr->second;
                TilePoolItr++;
                }
            TilePool->second.clear();
            }
        else
            {
            pair<PageTilePool::iterator, bool> InsertPair(PageTilePool->second.insert(PageTilePool::value_type(pi_Page, HRFPDFFile::TilePool())));
            HPOSTCONDITION(InsertPair.second);

            TilePool = InsertPair.first;
            }

        // find the resolution editor into the ResolutionEditorRegistry
        ResolutionEditorRegistry::const_iterator ResItr(m_ResolutionEditorRegistry.begin());
        HRFPDFEditor* pResEditor = 0;
        while (pResEditor == 0 && ResItr != m_ResolutionEditorRegistry.end())
            {
            if ((*ResItr)->GetPage() == pi_Page && (*ResItr)->GetResolutionIndex() == pi_Resolution)
                pResEditor = (HRFPDFEditor*)*ResItr;
            else
                ResItr++;
            }
        HASSERT(pResEditor != 0);

        // Extract the needed blocks from the region

        HFCPtr<HRFResolutionDescriptor> pResDesc = pResEditor->GetResolutionDescriptor();

        uint32_t BlockWidth  = pResDesc->GetBlockWidth();
        uint32_t BlockHeight = pResDesc->GetBlockHeight();

        HGFTileIDDescriptor TileDesc(pResDesc->GetWidth(),
                                     pResDesc->GetHeight(),
                                     BlockWidth,
                                     BlockHeight);

        uint32_t MinX        = UINT32_MAX;
        uint32_t MinY        = UINT32_MAX;
        uint32_t MaxX        = 0;
        uint32_t MaxY        = 0;
        bool  IsTileFound = false;

        // Make sure the input shape will not be destroyed
        HVEShape* pShape = const_cast<HVEShape*>(&pi_rShape);
        pShape->IncrementRef();
            {

            HVETileIDIterator TileIterator(&TileDesc, HFCPtr<HVEShape>(pShape));

            uint64_t PosX;
            uint64_t PosY;
            uint64_t BlockIndex = TileIterator.GetFirstTileIndex();
            while (BlockIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                TileDesc.GetPositionFromIndex(BlockIndex, &PosX, &PosY);
                HASSERT(PosX <= UINT32_MAX);
                HASSERT(PosY <= UINT32_MAX);

                MinX = MIN(MinX, (uint32_t)PosX);
                MinY = MIN(MinY, (uint32_t)PosY);
                MaxX = MAX(MaxX, (uint32_t)PosX + BlockWidth);
                MaxY = MAX(MaxY, (uint32_t)PosY + BlockHeight);

                BlockIndex = TileIterator.GetNextTileIndex();

                IsTileFound = true;
                }
            }
        // Decrement after the block because the HVETileIDIterator keeps an
        // HFCPtr to the input shape, so we want the Iterator to be destroyed
        // before we decrement.
        pShape->DecrementRef();

        if (IsTileFound == true)
            {
            uint32_t Width = MaxX - MinX;
            uint32_t Height = MaxY - MinY;
            HArrayAutoPtr<Byte> pData(new Byte[Width * Height * 3]);   // 24 bits
            if (pData != 0)
                {

                if (pResEditor->ReadBlockPDF(MinX, MinY, MaxX, MaxY, pData.get()) == H_SUCCESS)
                    {
                    uint32_t NbXTile = Width / BlockWidth;
                    uint32_t NbYTile = Height / BlockHeight;
                    uint32_t BytesPerBlockWidth = pResDesc->GetBytesPerBlockWidth();
                    uint32_t SrcWidth = Width * 3;    // 24 bits
                    Byte* pSrc;
                    Byte* pDst;
                    uint32_t BlockX;
                    uint32_t PosX;
                    uint32_t BlockY;
                    uint32_t PosY;

                    for (BlockX = 0, PosX = MinX; BlockX < NbXTile; ++BlockX, PosX += BlockWidth)
                        {
                        for (BlockY = 0, PosY = MinY; BlockY < NbYTile; ++BlockY, PosY += BlockHeight)
                            {
                            HArrayAutoPtr<Byte> pBlockData(new Byte[pResDesc->GetBlockSizeInBytes()]);
                            pDst = pBlockData.get();
                            pSrc = pData.get() + BlockY * BlockHeight * SrcWidth;
                            pSrc += BlockX * BytesPerBlockWidth;

                            for (uint32_t Line = 0; Line < BlockHeight; ++Line)
                                {
                                memcpy(pDst, pSrc, BytesPerBlockWidth);
                                pDst += BytesPerBlockWidth;
                                pSrc += SrcWidth;
                                }

                            TilePool->second.insert(TilePool::value_type(TileDesc.ComputeID(PosX, PosY, pi_Resolution), pBlockData.release()));
                            }
                        }
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// StopLookAhead
//
// Stops LookAhead for a consumer
//-----------------------------------------------------------------------------
void HRFPDFFile::StopLookAhead(uint32_t pi_Page,
                               uint32_t pi_ConsumerID)
    {
    // the LookAhead is not threaded, do nothing
    }

//---------------------------------------------------------- Protected

//-----------------------------------------------------------------------------
// protected
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFPDFFile::HRFPDFFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    HPRECONDITION(pi_rURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

    m_MainThreadId  = GetCurrentThreadId();

    Utf8String FileName;

    FileName = ((const HFCPtr<HFCURLFile>&)pi_rURL)->GetHost() +
               "\\" +
               ((const HFCPtr<HFCURLFile>&)pi_rURL)->GetPath();

    //Read-Only format
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
 
    m_pPDFWrapper = new IppPDFWrapper(FileName);

    // Create Page and Res Descriptors.
    CreateDescriptors();


//    HRFPDFCreator* pCreator = (HRFPDFCreator*)HRFRasterFileFactory::GetInstance()->GetCreator(HRFPDFFile::CLASS_ID);
//    if (pCreator != 0)
//        CommandParameters.pi_pPasswordCallback = (HRFPasswordCallback*)pCreator->GetCallback(HRFPasswordCallback::CLASS_ID);

    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//
// Get only the number of pages. The page was created only when is necessary
//-----------------------------------------------------------------------------
void HRFPDFFile::CreateDescriptors()
    {
    m_NumPages = m_pPDFWrapper->CountPages();
    /*
        bool Success = GetAnnotations(&m_Annotations);

        HASSERT(Success == true);*/
    }

//-----------------------------------------------------------------------------
// Protected
// GetDocument
//
// Get the PDF document pointer (PDDoc)
//-----------------------------------------------------------------------------
void* HRFPDFFile::GetDocument(uint32_t pi_Page) const
    {
    void* pPDDoc = m_pPDFWrapper->GetDocument();

    if (m_MainThreadId != GetCurrentThreadId())
        {
        HRFRasterFile::ContextMap::const_iterator pContextIter = m_Contexts.find(pi_Page);

        if (pContextIter != m_Contexts.end())
            {
            HFCPtr<HMDMetaDataContainer> pVolatileLayers(pContextIter->
                                                         second->
                                                         GetMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO));

            if (pVolatileLayers != 0)
                {
                HRFPDFLibInterface::SetLayerVisibility((PDDoc)pPDDoc,
                                                       pi_Page,
                                                       (HFCPtr<HMDVolatileLayers>&)pVolatileLayers);
                }
            }
        }

    return pPDDoc;
    }

//-----------------------------------------------------------------------------
// private
// RemoveLookAhead
//
// Remove the LookAhead entries for this editor
//-----------------------------------------------------------------------------
void HRFPDFFile::RemoveLookAhead(uint32_t   pi_Page,
                                 uint16_t pi_Resolution)
    {
    //Something with more bits than UInt64 should be used on architecture with pointer greater than 64 bits.
    HPRECONDITION(sizeof(GetContext(pi_Page).GetPtr()) <= 8);

    HFCMonitor Monitor(m_TilePoolKey);

    ContextPageTilePool::iterator ContextPageTilePoolItr(m_ContextPageTilePool.find((uint64_t)GetContext(pi_Page).GetPtr()));

    if (ContextPageTilePoolItr != m_ContextPageTilePool.end())
        {
        PageTilePool::iterator PageTilePoolItr(ContextPageTilePoolItr->second.find(pi_Page));

        if (PageTilePoolItr != ContextPageTilePoolItr->second.end())
            {
            HGFTileIDDescriptor TileIDDescriptor;
            TilePool::iterator TilePoolItr(PageTilePoolItr->second.begin());
            while (TilePoolItr != PageTilePoolItr->second.end())
                {
                if (TileIDDescriptor.GetLevel(TilePoolItr->first) == pi_Resolution)
                    {
                    RemoveBlockFromConsumers(pi_Page, TilePoolItr->first);
                    if (TilePoolItr->second != 0)
                        delete[] TilePoolItr->second;

                    TilePoolItr = PageTilePoolItr->second.erase(TilePoolItr);
                    }
                else
                    TilePoolItr++;
                }

            if (PageTilePoolItr->second.empty())
                ContextPageTilePoolItr->second.erase(PageTilePoolItr);
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPDFFile::GetWorldIdentificator () const
    {
    return GetPageWorldIdentificator(0);
    }

//-----------------------------------------------------------------------------
// Public
// GetPageWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPDFFile::GetPageWorldIdentificator (uint32_t pi_Page) const
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);

    HGF2DWorldIdentificator WorldId = HGF2DWorld_UNKNOWNWORLD;

    if (GetPageDescriptor(pi_Page)->HasTransfoModel() == true)
        {
        WorldId = HGF2DWorld_GEOGRAPHIC;
        }

    return WorldId;
    }

//-----------------------------------------------------------------------------
// Public
// HasLookAheadByExtent
//-----------------------------------------------------------------------------
bool HRFPDFFile::HasLookAheadByExtent(uint32_t  pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    //TR 271618 : Disable for now since this increase the time during the print
    //            with use of 1024 per 1024 tiles (the increase was caused by
    //            the fact in HRATiledRaster there is not check to determine if
    //            the region requested by the lookahead is already covered by HRA tiles.
    return false;
    }

#endif // IPP_HAVE_PDF_SUPPORT

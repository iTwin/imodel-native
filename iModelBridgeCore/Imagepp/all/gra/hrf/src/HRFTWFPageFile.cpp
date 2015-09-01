//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTWFPageFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTWFPageFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>



#include <Imagepp/all/h/HRFTWFPageFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFUtility.h>


//-----------------------------------------------------------------------------
// Class HRFTWFCapabilities
//-----------------------------------------------------------------------------
HRFTWFCapabilities::HRFTWFCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }


// Singleton
HFC_IMPLEMENT_SINGLETON(HRFTWFPageFileCreator)

//-----------------------------------------------------------------------------
// Class HRFTWFPageFileCreator
//
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------
HRFTWFPageFileCreator::HRFTWFPageFileCreator()
    {
    }

/**----------------------------------------------------------------------------
 @return true if the file sister file TWF associate with the Raster file
         is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true : Try to find an associate the sister file TWF with
                     all types of files(ITiff, GeoTiff, etc)
                     false(default) : Try to find an associate sister file TWF
                     only with the files don't support Georeference.
-----------------------------------------------------------------------------*/
bool HRFTWFPageFileCreator::HasFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                    bool pi_ApplyonAllFiles) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);

    bool HasPageFile = false;

    // Don't forget to synchronize HRFHGRPageFile.cpp too.

    // TR 108925 HFC_READ_ONLY must be used because we don't want sister of georeferenced file like sid and ecw if pi_ApplyonAllFiles is OFF.
    //(pi_ApplyonAllFiles || !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_CREATE_ONLY))) )

    // We added pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) to fix a problem caused by the TR 162805
    //  This TR adds the capability of TransfoModel on CAL file only to support SLO.

    if ((pi_rpForRasterFile->GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID)) &&
        (pi_ApplyonAllFiles || pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) ||
         !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID, HFC_READ_ONLY))) )
        {
        HFCPtr<HFCURL>  pURLForPageFile = FoundFileFor(pi_rpForRasterFile->GetURL());
        if (pURLForPageFile != 0)
            HasPageFile = true;
        }

    return HasPageFile;
    }

//-----------------------------------------------------------------------------
// public
// CreateFor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageFile> HRFTWFPageFileCreator::CreateFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);

    HFCPtr<HRFPageFile> pPageFile;
    HFCPtr<HFCURL> pURL;
    if (pi_rpForRasterFile->GetAccessMode().m_HasCreateAccess)
        pURL = ComposeURLFor(pi_rpForRasterFile->GetURL());
    else
        {
        if ((pURL = FoundFileFor(pi_rpForRasterFile->GetURL())) == 0)
            pURL = ComposeURLFor(pi_rpForRasterFile->GetURL());
        }

    pPageFile = new HRFTWFPageFile(pURL, pi_rpForRasterFile->GetAccessMode());

    return pPageFile;
    }

//-----------------------------------------------------------------------------
// public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFTWFPageFileCreator::ComposeURLFor(const HFCPtr<HFCURL>&   pi_rpURLFileName) const
    {
    HFCPtr<HFCURL>  URLForPageFile;

    // Decompose the file name
    if (!pi_rpURLFileName->IsCompatibleWith(HFCURLFile::CLASS_ID))
        throw HFCInvalidUrlForSisterFileException(pi_rpURLFileName->GetURL());

    WString Path(((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetPath());

    // Find the file extension
    WString::size_type DotPos = Path.rfind(L'.');

    // Extract the extension
    if (DotPos != WString::npos)
        {
        WString Extension;
        WString DriveDirName;

        // Compose the decoration file name
        Extension = Path.substr(DotPos+1, Path.length() - DotPos - 1);
        if (Extension.size() > 2)
            {
            Extension[1] = Extension[Extension.size() - 1];
            Extension[2] = L'w';
            Extension.resize(3);
            }
        else
            {
            Extension += L"w";
            }

        DriveDirName = Path.substr(0, DotPos);
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") +
                                        ((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetHost() + L"\\" +
                                        Path.substr(0, DotPos) +
                                        L"." +
                                        Extension);
        }
    else
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") +
                                        ((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetHost() + L"\\" +
                                        Path +
                                        L".w");


    return URLForPageFile;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFTWFPageFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFTWFCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// FoundFileFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFTWFPageFileCreator::FoundFileFor(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // try with the first rules
    HFCPtr<HFCURL> pPageFileURL = ComposeURLFor(pi_rpURL);

    HAutoPtr<HFCStat> pPageFileStat;
    pPageFileStat = new HFCStat(pPageFileURL);

    // Check if the decoration file exist
    if (!pPageFileStat->IsExistent())
        {
        // try with the second rules
        // Check if we have an extension
        WString URL = pi_rpURL->GetURL();
        WString::size_type DotPos = URL.rfind(L'.');
        if (DotPos != WString::npos)
            {
            URL += L"w";
            pPageFileURL = HFCURL::Instanciate(URL);
            pPageFileStat = new HFCStat(pPageFileURL);
            if (!pPageFileStat->IsExistent())
                pPageFileURL = 0;
            }
        else
            pPageFileURL = 0;
        }
    return pPageFileURL;
    }

//-----------------------------------------------------------------------------
// Class HRFTWFPageFile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFTWFPageFile::HRFTWFPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                               HFCAccessMode           pi_AccessMode)
    : HRFPageFile(pi_rpURL, pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);

    if (pi_AccessMode == HFC_CREATE_ONLY)
        m_pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_CREATE_ONLY | HFC_WRITE_ONLY, 0, true);
    else
        m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode, 0, true);

    m_DefaultRatioToMeter = ImageppLib::GetHost().GetImageppLibAdmin()._GetDefaultRatioToMeter();

    if (!pi_AccessMode.m_HasCreateAccess)
        {
        ReadFile();
        if (!IsValidTWFFile())
            throw HRFInvalidSisterFileException(pi_rpURL->GetURL());
        }
    else
        {
        m_A00 = 1.0;
        m_A01 = 0.0;
        m_A10 = 0.0;
        m_A11 = -1.0;
        m_Tx  = 0.0;
        m_Ty  = 0.0;
        }

    CreateDescriptor();

    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFTWFPageFile::~HRFTWFPageFile()
    {
    // Obtain the Page descriptor
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

    bool UpdateFile = false;
    if (GetAccessMode().m_HasCreateAccess)
        UpdateFile = true;
    else if (pPageDescriptor->HasTransfoModel() && pPageDescriptor->TransfoModelHasChanged())
        UpdateFile = true;

    if (UpdateFile)
        {
        WriteToDisk();
        }
    }

//-----------------------------------------------------------------------------
// Public
// WriteToDisk
// Write current transformation to disk
//----------------------------------------------------------------------------
void HRFTWFPageFile::WriteToDisk()
    {
    try
        {
        WriteFile();
        m_pFile->Flush();

        //// Reopen R/W with share access for Projectwise
        //if (m_pFile->GetAccessMode().m_HasWriteAccess && !(m_pFile->GetAccessMode().m_HasReadShare))
        //    {
        //    m_pFile = 0;    // Close the file
        //    m_pFile = HFCBinStream::Instanciate(GetURL(), HFC_READ_WRITE_OPEN | HFC_SHARE_READ_WRITE, 0, true);
        //    }
        }
    catch(...)
        {
        // Errors can happen, but they surely can't propagate in a destructor!
        // Break anyway so that we can make sure it's not a fatal error.
#if defined (ANDROID) || defined (__APPLE__)
        HASSERT(!L"Write error TWF file");
#elif defined (_WIN32)
        HDEBUGCODE(DebugBreak(););
#endif
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFTWFPageFile::GetCapabilities () const
    {
    return HRFTWFPageFileCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFTWFPageFile::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() == 1);

    return HGF2DWorld_HMRWORLD;
    }

//-----------------------------------------------------------------------------
// Static section
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Private section
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Private
// IsValidTWFFile
// Multiple returns for simplicity
//-----------------------------------------------------------------------------
bool HRFTWFPageFile::IsValidTWFFile() const
    {
    // In this case we don't need to use HDOUBLE_EQUAL
    if(((m_A00 == 0.0) && (m_A10 == 0.0)) ||
       ((m_A01 == 0.0) && (m_A11 == 0.0)) ||
       ((m_A00 == 0.0) && (m_A01 == 0.0)) ||
       ((m_A10 == 0.0) && (m_A11 == 0.0)))
        return false;

    // Origin must be greater that -2e294
    if (m_Tx < -HMAX_EPSILON || m_Ty < -HMAX_EPSILON)
        return false;

    // Origin must be smaller than 2e294
    if (m_Tx >= HMAX_EPSILON || m_Ty >= HMAX_EPSILON)
        return false;

    // Limit the pixelsize (?) inside [-2e294, 2e294]
    if (m_A00 > HMAX_EPSILON || m_A00 < -HMAX_EPSILON ||
        m_A11 > HMAX_EPSILON || m_A11 < -HMAX_EPSILON)
        return false;

    HGF2DAffine Model;
    Model.SetByMatrixParameters(m_Tx - 0.5*(m_A00 + m_A10),
                                m_A00,
                                m_A01,
                                m_Ty - 0.5*(m_A01 + m_A11),
                                m_A10,
                                m_A11);
    if ( HDOUBLE_EQUAL_EPSILON(Model.GetXScaling(), 0.0) ||
         HDOUBLE_EQUAL_EPSILON(Model.GetYScaling(), 0.0) )
        return false;

    if (fabs(Model.GetAnorthogonality()) > (89*PI/180.0))
        return false;

    return true;
    }

//-----------------------------------------------------------------------------
// Private
// ReadFile
//-----------------------------------------------------------------------------
void HRFTWFPageFile::ReadFile()
    {
    HPRECONDITION(m_pFile != 0);

    string Line;

    // x-dimension of a pixel in map units (this value may not be 0.0)
    ReadLine(&Line);
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_PixelSizeX()));

    if (!ConvertStringToDouble(Line, &m_A00))
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_PixelSizeX()));

    if (Line.empty())
        ReadLine(&Line);

    // Rotation term
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_RotationAboutY()));

    if (!ConvertStringToDouble(Line, &m_A10))
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_RotationAboutY()));

    if (Line.empty())
        ReadLine(&Line);

    // Rotation term
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_RotationAboutX()));

    if (!ConvertStringToDouble(Line, &m_A01))
        throw HRFSisterFileMissingParamGroupException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_RotationAboutX()));

    if (Line.empty())
        ReadLine(&Line);

    // negative of y-dimension of a pixel in map units (this value may not be 0.0)
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_PixelSizeY()));

    if (!ConvertStringToDouble(Line, &m_A11))
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_PixelSizeY()));

    if (Line.empty())
        ReadLine(&Line);

    // x coordinate of center of upper left pixel in map units
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_TranslationX()));


    if  (!ConvertStringToDouble(Line, &m_Tx))
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_TranslationX()));

    if (Line.empty())
        ReadLine(&Line);

    // y coordinte of center of upper left corner in map units
    if (Line.empty())
        throw HRFSisterFileMissingParamException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_TranslationY()));

    if (!ConvertStringToDouble(Line, &m_Ty))
        throw HRFSisterFileInvalidParamValueException(m_pFile->GetURL()->GetURL(),
                                        ImagePPMessages::GetStringW(ImagePPMessages::TWF_TranslationY()));
    }

//-----------------------------------------------------------------------------
// Private
// CreateDescriptor
//-----------------------------------------------------------------------------
void HRFTWFPageFile::CreateDescriptor()
    {
    HPRECONDITION(IsValidTWFFile());

    HFCPtr<HGF2DTransfoModel> pTransfoModel = BuildTransfoModel();
    HRFScanlineOrientation TransfoModelSLO = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

    // Create the Page information and add it to the list of page descriptor
    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor(GetAccessMode(),      // Access mode
                                  GetCapabilities(),    // Capabilities,
                                  0,                    // RepresentativePalette,
                                  0,                    // Histogram,
                                  0,                    // Thumbnail,
                                  0,                    // ClipShape,
                                  pTransfoModel,        // TransfoModel,
                                  &TransfoModelSLO,     // TransfoModelOrientation
                                  0,                    // Filters
                                  0);                   // Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Private
// BuildTransfoModel
//
// The translation in the TWF file are defined from the center of the first pixel
// in the upper left corner.
//
//  x' = Ax + By + C
//  y' = Dx + Ey + F
//
//  where x'  = calculated x-coordinate of the pixel on the map
//        y'  = calculated y-coordinate of the pixel on the map
//        x   = column number of the pixel in the image
//        y   = row number of the pixel in the image
//        A   = x-scale
//        B,D = rotation term
//        C   = translation term; x-origin (x-coordinate of the center of the
//              upper left corner)
//        E   = y-scale
//        F   = translation term; y-origin (y-coordinate of the center of the
//              upper left corner)
//
// Change axes x,y by i,j
// where i = x - 0.5
//       j = y - 0.5
//
// Now, use the i and j axes instead of x and y
// x' = Ai + Bj + C  => x' = A(x - 0.5) + B(y - 0.5) + C  => x' = Ax + By + C - 0.5(A + B)
// y' = Di + Ej + F  => y' = D(x - 0.5) + E(y - 0.5) + F  => y' = Dx + Ey + F - 0.5(D + E)
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFTWFPageFile::BuildTransfoModel() const
    {

    // Build the transformation model.
    HFCPtr<HGF2DTransfoModel> pModel;
    HFCPtr<HGF2DTransfoModel> pFinalTransfo;

    double FactorModelToMeter = GetDefaultRatioToMeter();

    pModel = new HGF2DAffine();
    // TWF are centered at the center of first pixel.
    ((HFCPtr<HGF2DAffine>&)pModel)->SetByMatrixParameters(m_Tx - 0.5*(m_A00 + m_A10),
                                                          m_A00,
                                                          m_A01,
                                                          m_Ty - 0.5*(m_A01 + m_A11),
                                                          m_A10,
                                                          m_A11);

    // If we are in creationMode, create a new file, we don't apply the Factor, it will be
    // apply only at save time. Because if we apply the Factor here, and remove at the save time
    // the factor will be 1.0
    if ((FactorModelToMeter != 1.0) && !GetAccessMode().m_HasCreateAccess)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(FactorModelToMeter);
        pScaleModel->SetYScaling(FactorModelToMeter);

        pFinalTransfo = pModel->ComposeInverseWithDirectOf(*pScaleModel);
        }
    else
        pFinalTransfo = pModel;

    return pFinalTransfo;
    }

//-----------------------------------------------------------------------------
// Private
// Write the data into the TWF file.  To do this we need to rewrite the entire
// file.
//-----------------------------------------------------------------------------
void HRFTWFPageFile::WriteFile()
    {
    HPRECONDITION(m_pFile != 0);
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HGF2DTransfoModel> pModel = pPageDescriptor->GetTransfoModel();

    // reset the file
    m_pFile->SeekToBegin();

    string Data;

    // Check if the transformation can be represented by a matrix.
    if (pModel->CanBeRepresentedByAMatrix())
        {
        double FactorModelToMeter = GetDefaultRatioToMeter();

        // Apply inverse factor to Matrix
        if (FactorModelToMeter != 1.0)
            {
            HASSERT(FactorModelToMeter != 0);
            HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
            pScaleModel->SetXScaling(1/FactorModelToMeter);
            pScaleModel->SetYScaling(1/FactorModelToMeter);

            pModel = pModel->ComposeInverseWithDirectOf(*pScaleModel);
            }

        // Create a translation to remove the 0.5 x 0.5 pixel centering.

        // Extract the matrix parameters from the transformation.
        HFCMatrix<3, 3> Matrix = pModel->GetMatrix();
        m_A00 = Matrix[0][0];
        m_A01 = Matrix[0][1];
        m_A10 = Matrix[1][0];
        m_A11 = Matrix[1][1];
        m_Tx  = Matrix[0][2] + 0.5*(m_A00 + m_A10); // remove the 0.5 x 0.5 pixel centering.
        m_Ty  = Matrix[1][2] + 0.5*(m_A01 + m_A11); // remove the 0.5 x 0.5 pixel centering.
        }
    else
        {
        throw HRFInvalidTransfoForSisterFileException(GetURL()->GetURL());
        }

    char aBuffer[256];

    size_t NewSize = 0;

    if (fabs(m_A00) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_A00);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_A00);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (fabs(m_A10) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_A10);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_A10);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (fabs(m_A01) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_A01);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_A01);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (fabs(m_A11) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_A11);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_A11);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (fabs(m_Tx) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_Tx);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_Tx);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (fabs(m_Ty) < 1E-12)
        {
        sprintf(aBuffer, "%.12lg\r\n", m_Ty);
        }
    else
        {
        sprintf(aBuffer, "%.12lf\r\n", m_Ty);
        }
    NewSize += m_pFile->Write(aBuffer, strlen(aBuffer));

    if (m_pFile->GetSize() > NewSize)
        {
        NewSize = (size_t)(m_pFile->GetSize() - NewSize);
        memset(aBuffer, ' ', 256);

        // clear the end of the file
        while (NewSize > 256)
            {
            m_pFile->Write(aBuffer, 256);
            NewSize -= 256;
            }

        if (NewSize > 0)
            m_pFile->Write(aBuffer, NewSize);
        }

    m_pFile->Flush();
    }

//-----------------------------------------------------------------------------
// Private
// ConvertStringToDouble
//-----------------------------------------------------------------------------
bool HRFTWFPageFile::ConvertStringToDouble(string& pio_rString, double* po_pDouble) const
    {
    char* pCurrentPosInLine;
    *po_pDouble = strtod(pio_rString.c_str(), &pCurrentPosInLine);

    if ((*po_pDouble == 0.0 || *po_pDouble == HUGE_VAL || *po_pDouble == -HUGE_VAL) &&
        errno == ERANGE)
        return false;
    else
        {
        size_t PosFromBeginning = pCurrentPosInLine - pio_rString.c_str();
        if (PosFromBeginning == 0)
            return false;
        else
            {
            pio_rString = pio_rString.substr(PosFromBeginning);
            return true;
            }
        }
//    return (pStopPtr - pi_rString.c_str() == pi_rString.length());
    }

//-----------------------------------------------------------------------------
// Private
// ReadLine
//-----------------------------------------------------------------------------
void HRFTWFPageFile::ReadLine(string*   po_pString)
    {
    HPRECONDITION(m_pFile != 0);
    HPRECONDITION(po_pString != 0);

    const int BufferSize = 64;
    char      Buffer[BufferSize+1];
    string    CurrentLine;

    bool EndOfLine = false;
    po_pString->erase();
    while (!EndOfLine)
        {
        memset(Buffer, 0, BufferSize+1);
        for (unsigned short i = 0; i < BufferSize && !EndOfLine; i++)
            {
            m_pFile->Read(&Buffer[i], 1);
            EndOfLine = Buffer[i] == '\n' || m_pFile->EndOfFile();
            }

        *po_pString += Buffer;
        }

    CleanUpString(po_pString);
    }


//-----------------------------------------------------------------------------
// Private
// CleanUpString
//
// Remove SPACE/TAB/ENTER from the begin and end of the file.
//-----------------------------------------------------------------------------
void HRFTWFPageFile::CleanUpString(string* pio_pString) const
    {
    HPRECONDITION(pio_pString != 0);

    size_t Pos = 0;

    // Remove the SPACE/TAB at the begin of the string.
    while (Pos < pio_pString->size() && !IsValidChar((*pio_pString)[Pos]))
        Pos++;

    *pio_pString = pio_pString->substr(Pos);

    if(!pio_pString->empty())
        {
        // Remove the SPACE/TAB/ENTER at the end of the string.
        Pos = pio_pString->size() - 1;
        while(Pos >= 0 && !IsValidChar((*pio_pString)[Pos]))
            Pos--;

        *pio_pString = pio_pString->substr(0, Pos+1);
        }
    }

//-----------------------------------------------------------------------------
// Private
// IsValidChar
//
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER.
//-----------------------------------------------------------------------------
bool HRFTWFPageFile::IsValidChar(const char pi_Char) const
    {
    bool IsValid = true;

    switch (pi_Char)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            IsValid = false;
            break;
        }

    return IsValid;
    }

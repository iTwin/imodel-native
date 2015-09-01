//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphMonochromeFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphMonochromeFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFIntergraphMonochromeFile.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HCDCodecHMRCCITT.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFUtility.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphMonochromeFile::HRFIntergraphMonochromeFile(const HFCPtr<HFCURL>& pi_rURL,
                                                         HFCAccessMode         pi_AccessMode,
                                                         uint64_t             pi_Offset)
    : HRFIntergraphFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
//    SetBitPerPixel(1); ????

#if (0)
    if (GetAccessMode().m_HasCreateAccess)
        {
        // Create a new file
        Create();
        }
    else
        {
        // Open an existent file
        Open();
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
#endif
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFIntergraphMonochromeFile::HRFIntergraphMonochromeFile(const HFCPtr<HFCURL>& pi_rURL,
                                                         HFCAccessMode         pi_AccessMode,
                                                         uint64_t             pi_Offset,
                                                         bool                 pi_DontOpenFile)
    : HRFIntergraphFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
//    SetBitPerPixel(1);
    }

//-----------------------------------------------------------------------------
// Destructor
// Closes the Intergraph monochrome file
//-----------------------------------------------------------------------------
HRFIntergraphMonochromeFile::~HRFIntergraphMonochromeFile()
    {
    // This function must be called here before destruction of Intergraph file
    // since the close function uses overridden virtual functions
    HRFIntergraphFile::Close();
    }




//-----------------------------------------------------------------------------
// GetTransfoModel
// Prepares and sets the internal transformation model
//-----------------------------------------------------------------------------
void HRFIntergraphMonochromeFile::GetTransfoModel()
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    CheckNotNullTransfoModel();

    InitScanlineOrientation();

    // Instanciate pointer to the affine required
    HFCPtr<HGF2DAffine> pNewAffine;

    // Check if there is no matrix in the file or if Intergraph version is 2
    // Additionaly if matrix is present check that the content of the matrix is valid
    // Conditions of invalidity are as follows
    // | a  b  c  d |
    // | e  f  g  h |
    // | i  j  k  l |
    // | m  n  o  p |
    // If a and b are both 0.0 or
    // if a and e are both 0.0 or
    // if e anf f are both 0.0 or
    // if b anf f are both 0.0
    if ((m_ZeroMatrixFound) ||
        ((m_IntergraphHeader.IBlock1.trn[0] == 0.0 && m_IntergraphHeader.IBlock1.trn[1] == 0.0) ||
         (m_IntergraphHeader.IBlock1.trn[0] == 0.0 && m_IntergraphHeader.IBlock1.trn[4] == 0.0) ||
         (m_IntergraphHeader.IBlock1.trn[4] == 0.0 && m_IntergraphHeader.IBlock1.trn[5] == 0.0) ||
         (m_IntergraphHeader.IBlock1.trn[1] == 0.0 && m_IntergraphHeader.IBlock1.trn[5] == 0.0)))

        {
        // The matrix is either not present, invalid or the Intergraph version too small
        // We here build a simple model relevant of positionless files
        // Instanciate affine
        pNewAffine = new HGF2DAffine();

        // Obtain physical width and height (pixels per scan line and number of scan lines)
        // from base resolution
        int32_t NumPixelsPerScanline = GetWidth (0);
        int32_t NumScanlines         = GetHeight(0);


        // The following transformation models are transformations from file SLO to SLO 4
        // to which is added a flip for the fact SLO 4 has an Y inversion compared to
        // its logical coordinate system.
        // NOTE: By transformation from file SLO to SLO 4 we mean that the visual upper-left
        // corner of the image will be located at the origin of the logical coordinate system
        // We did not use the strange Intergraph convention where flips and inversions are
        // perfomed but where the origin of the physical coordinate system remains at the origin
        // of the logical coordinate system.
        // All files will be located at the same position if they are only different by their
        // SLO and byte order
        // SLO 0
        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(0.0, 0.0, 1.0, 0.0, -1.0, 0.0);
            }
        // SLO 1
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(NumScanlines, 0.0, -1.0, 0.0, -1.0, 0.0);
            }
        // SLO 2
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(0.0, 0.0, 1.0, -NumPixelsPerScanline, 1.0, 0.0);
            }
        // SLO 3
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(NumScanlines, 0.0, -1.0, -NumPixelsPerScanline, 1.0, 0.0);
            }
        // SLO 4
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(0.0, 1.0, 0.0, 0.0, 0.0, -1.0);
            }
        // SLO 5
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(NumPixelsPerScanline, -1.0, 0.0, 0.0, 0.0, -1.0);
            }
        // SLO 6
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(0.0, 1.0, 0.0, -NumScanlines, 0.0, 1.0);
            }
        // SLO 7
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(NumPixelsPerScanline, -1.0, 0.0, -NumScanlines, 0.0, 1.0);
            }
        }
    else
        {
#ifdef IRASB_STYLE
        // The Intergraph version is 2 or more and there is a valid matrix


        // The absolute scale is the maximum in absolute value of all 4 scaling factor
        double AbsoluteScale = MAX(fabs(m_IntergraphHeader.IBlock1.trn[0]),
                                    MAX(fabs(m_IntergraphHeader.IBlock1.trn[1]),
                                        MAX(fabs(m_IntergraphHeader.IBlock1.trn[4]),
                                            fabs(m_IntergraphHeader.IBlock1.trn[5]))));

        // Compute scale X
        // The X scale is the absolute scale for most cases
        double ScaleX = AbsoluteScale;

        // Compute Y Scale
        // The Y scale is the absolute scale for most cases
        double ScaleY = AbsoluteScale;

        // The model created related the Physical coordinate system to the logical coordinate system

        // Instanciate required affine
        pNewAffine = new HGF2DAffine();

        // SLO 0
        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            // For SLO 0 some adjustements are required
            // The X scale is the absolute scale where sign of mat[0][0] is applied
            ScaleX = (m_IntergraphHeader.IBlock1.trn[0] < 0.0 ? -AbsoluteScale :
                      AbsoluteScale);

            // The Y scale is the absolute scale where sign of mat[1][1] is applied
            ScaleY = (m_IntergraphHeader.IBlock1.trn[5] < 0.0 ? -AbsoluteScale :
                      AbsoluteScale);

            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              0.0,
                                              ScaleX,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -ScaleY,
                                              0.0);
            }
        // SLO 1
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              0.0,
                                              -ScaleX,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -ScaleY,
                                              0.0);
            }
        // SLO 2
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              0.0,
                                              ScaleX,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              ScaleY,
                                              0.0);
            }
        // SLO 3
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              0.0,
                                              -ScaleX,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              ScaleY,
                                              0.0);
            }
        // SLO 4
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            // For SLO 4 some adjustements are required
            // The X scale is the absolute scale where sign of mat[0][0] is applied
            ScaleX = (m_IntergraphHeader.IBlock1.trn[0] < 0.0 ? -AbsoluteScale :
                      AbsoluteScale);

            // The Y scale is the absolute scale where sign of mat[1][1] is applied
            ScaleY = (m_IntergraphHeader.IBlock1.trn[5] < 0.0 ? -AbsoluteScale :
                      AbsoluteScale);

            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              ScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              -ScaleY);
            }
        // SLO 5
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -ScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              -ScaleY);
            }
        // SLO 6
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              ScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              ScaleY);
            }
        // SLO 7
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -ScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              ScaleY);
            }
#else // DESCARTE STYLE
        // The Intergraph version is 3 or more and there is a valid matrix


        // Extract rotation parameters if any
        double Rotation = m_IntergraphHeader.IBlock1.rot;  // Radians);

        // Extract skew (affinity)
        double Skew = m_IntergraphHeader.IBlock1.skw;      // Radians);

        // The model created related the Physical coordinate system to the logical coordinate system

        // Instanciate required affine
        pNewAffine = new HGF2DAffine();

        // Calculate rotation, scalings and affinity (Descartes style)
        // The difference between I++ parameters and Descartes is that
        // scaling factors are always positive and mirroring is distributed
        // along rotation and affinity
        double DescartesStyleRotation;
        double DescartesStyleAffinity;
        double DescartesStyleScaleX;
        double DescartesStyleScaleY;

        // Extract Descartes rotation
        DescartesStyleRotation = atan2(m_IntergraphHeader.IBlock1.trn[4],
                                       m_IntergraphHeader.IBlock1.trn[0]);

        // Extract Descartes affinity
        DescartesStyleAffinity = atan2(-m_IntergraphHeader.IBlock1.trn[1],
                                       m_IntergraphHeader.IBlock1.trn[5]) - DescartesStyleRotation;

        // The most precise calculation to X scaling
        if (fabs(m_IntergraphHeader.IBlock1.trn[4]) > fabs(m_IntergraphHeader.IBlock1.trn[0]))
            DescartesStyleScaleX = fabs(m_IntergraphHeader.IBlock1.trn[4] / sin(DescartesStyleRotation));
        else
            DescartesStyleScaleX = fabs(m_IntergraphHeader.IBlock1.trn[0] / cos(DescartesStyleRotation));

        // The most precise calculation to Y scaling
        if (fabs(m_IntergraphHeader.IBlock1.trn[1]) > fabs(m_IntergraphHeader.IBlock1.trn[5]))
            DescartesStyleScaleY = fabs(-m_IntergraphHeader.IBlock1.trn[1] /
                                        sin(DescartesStyleRotation + DescartesStyleAffinity));
        else
            DescartesStyleScaleY = fabs(m_IntergraphHeader.IBlock1.trn[5] /
                                        cos(DescartesStyleRotation + DescartesStyleAffinity));

        // TR #128817 : The pixel size become the equal average values of pixel sizes.
        // To remain IRasB compliant.  Now we will support unsquared pixel size, so remove
        // the pixelsize averaging.
        /*
        if (m_IntergraphHeader.IBlock1.ver == 2)
        {
            DescartesStyleScaleX = (DescartesStyleScaleX + DescartesStyleScaleY) / 2.0;
            DescartesStyleScaleY = DescartesStyleScaleX;
        } */

        // I++ does not like affinity greater than 90 degrees ... adjust
        double MinAngle = -PI / 2;
        double RightAngle = PI / 2;
        double NormSkew = Skew;
        if ((NormSkew < MinAngle) || (NormSkew > RightAngle))
            {
            double HalfTurn = PI;
            while (NormSkew < MinAngle)
                {
                Skew += HalfTurn;
                NormSkew = Skew;
                }

            while (NormSkew > RightAngle)
                {
                Skew -= HalfTurn;
                NormSkew = Skew;
                }

            // Set scalings in affine (note the sign reversal in Y to compensate affinity)
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              DescartesStyleScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              -DescartesStyleScaleY);

            }
        else
            {

            // Set scalings in affine
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              DescartesStyleScaleX,
                                              0.0,
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              0.0,
                                              DescartesStyleScaleY);
            }

        // Set the rotation to parameter in ROT and SKW
        pNewAffine->SetRotation(Rotation);
        pNewAffine->SetAnorthogonality(Skew);

        // Extract Affine matrix
        HFCMatrix<3, 3> MyMatrix = pNewAffine->GetMatrix();

        // The rest depends on the SLO

        // SLO 0
        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              -MyMatrix[1][0],
                                              MyMatrix[1][1],
                                              MyMatrix[1][2],
                                              -MyMatrix[0][0],
                                              MyMatrix[0][1]);
            }
        // SLO 1
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            // Correct!
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              MyMatrix[1][0],
                                              -MyMatrix[1][1],
                                              MyMatrix[1][2],
                                              -MyMatrix[0][0],
                                              MyMatrix[0][1]);
            }
        // SLO 2
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              -MyMatrix[1][0],
                                              MyMatrix[1][1],
                                              MyMatrix[1][2],
                                              MyMatrix[0][0],
                                              -MyMatrix[0][1]);
            }
        // SLO 3
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              MyMatrix[1][0],
                                              -MyMatrix[1][1],
                                              MyMatrix[1][2],
                                              MyMatrix[0][0],
                                              -MyMatrix[0][1]);
            }
        // SLO 4
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              MyMatrix[0][0],
                                              -MyMatrix[0][1],
                                              MyMatrix[1][2],
                                              MyMatrix[1][0],
                                              -MyMatrix[1][1]);
            }
        // SLO 5
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              -MyMatrix[0][0],
                                              MyMatrix[0][1],
                                              MyMatrix[1][2],
                                              MyMatrix[1][0],
                                              -MyMatrix[1][1]);
            }
        // SLO 6
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              MyMatrix[0][0],
                                              -MyMatrix[0][1],
                                              MyMatrix[1][2],
                                              -MyMatrix[1][0],
                                              MyMatrix[1][1]);
            }
        // SLO 7
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(MyMatrix[0][2],
                                              -MyMatrix[0][0],
                                              MyMatrix[0][1],
                                              MyMatrix[1][2],
                                              -MyMatrix[1][0],
                                              MyMatrix[1][1]);
            }
#endif
        }

    HFCPtr<HGF2DTransfoModel> pModel =  pNewAffine->CreateSimplifiedModel();
    if (pModel != 0)
        m_pTransfoModel = pModel;
    else
        m_pTransfoModel = pNewAffine;
    }


//-----------------------------------------------------------------------------
// Protected
// Sets the transformation of the Intergraph monochrome file
// Fields in the intergraph header are updated to reflect the current
// global transformation of the file
//-----------------------------------------------------------------------------
bool HRFIntergraphMonochromeFile::SetGlobalTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    // Return value ... could be set
    bool Result = false;

    bool Valid = true;

    // Check if the transformation can be represented by a matrix.
    HASSERT(pi_rpTransfoModel->CanBeRepresentedByAMatrix());
    Valid = (pi_rpTransfoModel->CanBeRepresentedByAMatrix());

    // Extract the matrix
    HFCMatrix<3, 3> MyMatrix = pi_rpTransfoModel->GetMatrix();

    HASSERT(MyMatrix[2][0] == 0.0);
    HASSERT(MyMatrix[2][1] == 0.0);
    HASSERT(MyMatrix[2][2] == 1.0);

    Valid = Valid && (MyMatrix[2][0] == 0.0);
    Valid = Valid && (MyMatrix[2][1] == 0.0);
    Valid = Valid && (MyMatrix[2][2] == 1.0);

    // Obtain current scanline orientation
    HRFScanlineOrientation OriginalSLO = GetScanlineOrientation();

#if IRASB_STYLE

    double ScaleX;
    double ScaleY;
    double TranslationX;
    double TranslationY;

    // What IRAS/B supports is scalings only as absolute value and translations.
    // Technically it supports flips for SLO 0 and 4, but this is unofficial
    // and possibly accidental.

    TranslationX = MyMatrix[0][2];
    TranslationY = MyMatrix[1][2];

    // SLO 0
    if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
        {
        // MyMatrix[0][0] and [1][1] must be null
        HASSERT(MyMatrix[0][0] == 0.0);
        HASSERT(MyMatrix[1][1] == 0.0);
        Valid = Valid && (MyMatrix[0][0] == 0.0);
        Valid = Valid && (MyMatrix[1][1] == 0.0);

        // Absolute value of [0][1] and [1][0] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0]));

        ScaleX = MyMatrix[0][1];
        ScaleY = MyMatrix[1][0];
        }
    // SLO 1
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
        {
        // MyMatrix[0][0] and [1][1] must be null
        HASSERT(MyMatrix[0][0] == 0.0);
        HASSERT(MyMatrix[1][1] == 0.0);

        Valid = Valid && (MyMatrix[0][0] == 0.0);
        Valid = Valid && (MyMatrix[1][1] == 0.0);

        // Absolute value of [0][1] and [1][0] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0]));

        // Scaling of MyMatrix[0][1] must be negative
        HASSERT(MyMatrix[0][1] < 0.0);
        Valid = Valid && (MyMatrix[0][1] < 0.0);
        // and MyMatrix[1][0] must be negative
        HASSERT(MyMatrix[1][0] < 0.0);
        Valid = Valid && (MyMatrix[1][0] < 0.0);

        ScaleX = fabs(MyMatrix[0][1]);
        ScaleY = ScaleX;
        }
    // SLO 2
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
        {
        // MyMatrix[0][0] and [1][1] must be null
        HASSERT(MyMatrix[0][0] == 0.0);
        HASSERT(MyMatrix[1][1] == 0.0);

        Valid = Valid && (MyMatrix[0][0] == 0.0);
        Valid = Valid && (MyMatrix[1][1] == 0.0);

        // Absolute value of [0][1] and [1][0] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0]));

        // Scaling of MyMatrix[0][1] must be positive
        HASSERT(MyMatrix[0][1] > 0.0);
        Valid = Valid && (MyMatrix[0][1] > 0.0);
        // and MyMatrix[1][0] must be positive
        HASSERT(MyMatrix[1][0] > 0.0);
        Valid = Valid && (MyMatrix[1][0] > 0.0);

        ScaleX = MyMatrix[0][1];
        ScaleY = ScaleX;
        }
    // SLO 3
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
        {
        // MyMatrix[0][0] and [1][1] must be null
        HASSERT(MyMatrix[0][0] == 0.0);
        HASSERT(MyMatrix[1][1] == 0.0);

        Valid = Valid && (MyMatrix[0][0] == 0.0);
        Valid = Valid && (MyMatrix[1][1] == 0.0);

        // Absolute value of [0][1] and [1][0] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][1]), fabs(MyMatrix[1][0]));

        // Scaling of MyMatrix[0][1] must be negative
        HASSERT(MyMatrix[0][1] < 0.0);
        Valid = Valid && (MyMatrix[0][1] < 0.0);
        // and MyMatrix[1][0] must be positive
        HASSERT(MyMatrix[1][0] > 0.0);
        Valid = Valid && (MyMatrix[1][0] > 0.0);
        ScaleX = MyMatrix[0][1];
        ScaleY = ScaleX;
        }
    // SLO 4
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
        {
        // MyMatrix[0][1] and [1][0] must be null
        HASSERT(MyMatrix[0][1] == 0.0);
        HASSERT(MyMatrix[1][0] == 0.0);

        Valid = Valid && (MyMatrix[0][1] == 0.0);
        Valid = Valid && (MyMatrix[1][0] == 0.0);

        // Absolute value of [0][0] and [1][1] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1]));

        ScaleX = MyMatrix[0][0];
        ScaleY = MyMatrix[1][1];
        }
    // SLO 5
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
        {
        // MyMatrix[0][1] and [1][0] must be null
        HASSERT(MyMatrix[0][1] == 0.0);
        HASSERT(MyMatrix[1][0] == 0.0);

        Valid = Valid && (MyMatrix[0][1] == 0.0);
        Valid = Valid && (MyMatrix[1][0] == 0.0);

        // Absolute value of [0][0] and [1][1] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1]));

        // Scaling of MyMatrix[0][0] must be negative
        HASSERT(MyMatrix[0][0] < 0.0);
        Valid = Valid && (MyMatrix[0][0] < 0.0);
        // and MyMatrix[1][1] must be negative
        HASSERT(MyMatrix[1][1] < 0.0);
        Valid = Valid && (MyMatrix[1][1] < 0.0);

        ScaleX = fabs(MyMatrix[0][0]);
        ScaleY = ScaleX;
        }
    // SLO 6
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
        {
        // MyMatrix[0][1] and [1][0] must be null
        HASSERT(MyMatrix[0][1] == 0.0);
        HASSERT(MyMatrix[1][0] == 0.0);

        Valid = Valid && (MyMatrix[0][1] == 0.0);
        Valid = Valid && (MyMatrix[1][0] == 0.0);

        // Absolute value of [0][0] and [1][1] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1]));

        // Scaling of MyMatrix[0][0] must be positive
        HASSERT(MyMatrix[0][0] > 0.0);
        Valid = Valid && (MyMatrix[0][0] > 0.0);
        // and MyMatrix[1][1] must be positive
        HASSERT(MyMatrix[1][1] > 0.0);
        Valid = Valid && (MyMatrix[1][1] > 0.0);

        ScaleX = fabs(MyMatrix[0][0]);
        ScaleY = ScaleX;
        }
    // SLO 7
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
        {
        // MyMatrix[0][1] and [1][0] must be null
        HASSERT(MyMatrix[0][1] == 0.0);
        HASSERT(MyMatrix[1][0] == 0.0);

        Valid = Valid && (MyMatrix[0][1] == 0.0);
        Valid = Valid && (MyMatrix[1][0] == 0.0);

        // Absolute value of [0][0] and [1][1] must be equal
        HASSERT(HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1])));
        Valid = Valid && HDOUBLE_EQUAL_EPSILON(fabs(MyMatrix[0][0]), fabs(MyMatrix[1][1]));

        // Scaling of MyMatrix[0][0] must be positive
        HASSERT(MyMatrix[0][0] > 0.0);
        Valid = Valid && (MyMatrix[0][0] > 0.0);
        // and MyMatrix[1][1] must be positive
        HASSERT(MyMatrix[1][1] > 0.0);
        Valid = Valid && (MyMatrix[1][1] > 0.0);

        ScaleX = fabs(MyMatrix[0][0]);
        ScaleY = ScaleX;
        }

    if (Valid)
        {
        // Scaling and translation have been obtained ...
        // Set Intergraph header
        m_IntergraphHeader.IBlock1.trn[0]  = ScaleX;
        m_IntergraphHeader.IBlock1.trn[1]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[2]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[3]  = TranslationX;
        m_IntergraphHeader.IBlock1.trn[4]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[5]  = ScaleY;
        m_IntergraphHeader.IBlock1.trn[6]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[7]  = TranslationY;
        m_IntergraphHeader.IBlock1.trn[8]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[9]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[10] = 1.0;
        m_IntergraphHeader.IBlock1.trn[11] = 0.0;
        m_IntergraphHeader.IBlock1.trn[12] = 0.0;
        m_IntergraphHeader.IBlock1.trn[13] = 0.0;
        m_IntergraphHeader.IBlock1.trn[14] = 0.0;
        m_IntergraphHeader.IBlock1.trn[15] = 1.0;

        // Initialize (or re-initialize) obsolete parameters
        m_IntergraphHeader.IBlock1.xori = 0.0;
        m_IntergraphHeader.IBlock1.yor = 0.0;
        m_IntergraphHeader.IBlock1.zor = 0;
        m_IntergraphHeader.IBlock1.xdl = 0.0;
        m_IntergraphHeader.IBlock1.ydl = 0.0;
        m_IntergraphHeader.IBlock1.zdl = 1;
        m_IntergraphHeader.IBlock1.rot = 0.0;
        m_IntergraphHeader.IBlock1.skw = 0.0;

        Result = true;
        }

#else // Descartes Style

    // Descartes style of storing usually required that we approximate to the maximum rotations
    // using changes of SLO. Here however we will consider the SLO to be fixed and decided by the
    // user. We will not change it and store the full rotation in the ROT and SKW factors.
    // This does not impair in any way the interpretation of geo-reference by Descartes.

    // SLO Independent matrix
    HFCMatrix<3, 3> SLOIndependentMatrix;

    // Set constant values
    SLOIndependentMatrix[2][0] = 0.0;
    SLOIndependentMatrix[2][1] = 0.0;
    SLOIndependentMatrix[2][2] = 0.0;

    // Translation parameters are taken as are
    SLOIndependentMatrix[0][2] = MyMatrix[0][2];
    SLOIndependentMatrix[1][2] = MyMatrix[1][2];

    // The rest depend on the SLO
    // SLO 0
    if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
        {
        SLOIndependentMatrix[0][0] = -MyMatrix[1][0];
        SLOIndependentMatrix[0][1] = MyMatrix[1][1];
        SLOIndependentMatrix[1][0] = -MyMatrix[0][0];
        SLOIndependentMatrix[1][1] = MyMatrix[0][1];
        }
    // SLO 1
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
        {
        SLOIndependentMatrix[0][0] = -MyMatrix[1][0];
        SLOIndependentMatrix[0][1] = MyMatrix[1][1];
        SLOIndependentMatrix[1][0] = MyMatrix[0][0];
        SLOIndependentMatrix[1][1] = -MyMatrix[0][1];
        }
    // SLO 2
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
        {
        SLOIndependentMatrix[0][0] = MyMatrix[1][0];
        SLOIndependentMatrix[0][1] = -MyMatrix[1][1];
        SLOIndependentMatrix[1][0] = -MyMatrix[0][0];
        SLOIndependentMatrix[1][1] = MyMatrix[0][1];
        }
    // SLO 3
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
        {
        SLOIndependentMatrix[0][0] = MyMatrix[1][0];
        SLOIndependentMatrix[0][1] = -MyMatrix[1][1];
        SLOIndependentMatrix[1][0] = MyMatrix[0][0];
        SLOIndependentMatrix[1][1] = -MyMatrix[0][1];
        }
    // SLO 4
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
        {
        SLOIndependentMatrix[0][0] = MyMatrix[0][0];
        SLOIndependentMatrix[0][1] = -MyMatrix[0][1];
        SLOIndependentMatrix[1][0] = MyMatrix[1][0];
        SLOIndependentMatrix[1][1] = -MyMatrix[1][1];
        }
    // SLO 5
    else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
        {
        SLOIndependentMatrix[0][0] = -MyMatrix[0][0];
        SLOIndependentMatrix[0][1] = MyMatrix[0][1];
        SLOIndependentMatrix[1][0] = MyMatrix[1][0];
        SLOIndependentMatrix[1][1] = -MyMatrix[1][1];
        }
    // SLO 6
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
        {
        SLOIndependentMatrix[0][0] = MyMatrix[0][0];
        SLOIndependentMatrix[0][1] = -MyMatrix[0][1];
        SLOIndependentMatrix[1][0] = -MyMatrix[1][0];
        SLOIndependentMatrix[1][1] = MyMatrix[1][1];
        }
    // SLO 7
    else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
        {
        SLOIndependentMatrix[0][0] = -MyMatrix[0][0];
        SLOIndependentMatrix[0][1] = MyMatrix[0][1];
        SLOIndependentMatrix[1][0] = -MyMatrix[1][0];
        SLOIndependentMatrix[1][1] = MyMatrix[1][1];
        }

    // We now the SLO Independent matrix. We need to extract parameters using Descartes style

    if (Valid)
        {
        // Calculate rotation, scalings and affinity (Descartes style)
        // The difference between I++ parameters and Descartes is that
        // scaling factors are always positive and mirroring is distributed
        // along rotation and affinity
        double DescartesStyleRotation;
        double DescartesStyleAffinity;
        double DescartesStyleScaleX;
        double DescartesStyleScaleY;

        // Extract Descartes rotation
        DescartesStyleRotation = atan2(SLOIndependentMatrix[1][0],
                                       SLOIndependentMatrix[0][0]);

        // Extract Descartes affinity
        DescartesStyleAffinity = atan2(-SLOIndependentMatrix[0][1],
                                       SLOIndependentMatrix[1][1]) - DescartesStyleRotation;

        // The most precise calculation to X scaling
        if (fabs(SLOIndependentMatrix[1][0]) > fabs(SLOIndependentMatrix[0][0]))
            DescartesStyleScaleX = fabs(SLOIndependentMatrix[1][0] / sin(DescartesStyleRotation));
        else
            DescartesStyleScaleX = fabs(SLOIndependentMatrix[0][0] / cos(DescartesStyleRotation));

        // The most precise calculation to Y scaling
        if (fabs(SLOIndependentMatrix[0][1]) > fabs(SLOIndependentMatrix[1][1]))
            DescartesStyleScaleY = fabs(-SLOIndependentMatrix[0][1] /
                                        sin(DescartesStyleRotation + DescartesStyleAffinity));
        else
            DescartesStyleScaleY = fabs(SLOIndependentMatrix[1][1] /
                                        cos(DescartesStyleRotation + DescartesStyleAffinity));

        // From this we build the final matrix to be stored
        m_IntergraphHeader.IBlock1.trn[0]  = DescartesStyleScaleX;
        m_IntergraphHeader.IBlock1.trn[1]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[2]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[3]  = SLOIndependentMatrix[0][2];
        m_IntergraphHeader.IBlock1.trn[4]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[5]  = DescartesStyleScaleY;
        m_IntergraphHeader.IBlock1.trn[6]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[7]  = SLOIndependentMatrix[1][2];
        m_IntergraphHeader.IBlock1.trn[8]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[9]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[10] = 1.0;
        m_IntergraphHeader.IBlock1.trn[11] = 0.0;
        m_IntergraphHeader.IBlock1.trn[12] = 0.0;
        m_IntergraphHeader.IBlock1.trn[13] = 0.0;
        m_IntergraphHeader.IBlock1.trn[14] = 0.0;
        m_IntergraphHeader.IBlock1.trn[15] = 1.0;


        // Initialize (or re-initialize) obsolete parameters
        m_IntergraphHeader.IBlock1.xori = SLOIndependentMatrix[0][2];;
        m_IntergraphHeader.IBlock1.yor = SLOIndependentMatrix[1][2];;
        m_IntergraphHeader.IBlock1.zor = 0.0;
        m_IntergraphHeader.IBlock1.xdl = DescartesStyleScaleX;
        m_IntergraphHeader.IBlock1.ydl = DescartesStyleScaleY;
        m_IntergraphHeader.IBlock1.zdl = 1.0;
        m_IntergraphHeader.IBlock1.rot = DescartesStyleRotation;
        m_IntergraphHeader.IBlock1.skw = DescartesStyleAffinity;

        Result = true;
        }

#endif

#if (0)

// This was simply an attempt to change the SLO based on flips
// At the moment it should be disregarded
    Scaling = MyMatrix[0][0];
    TranslationX = MyMatrix[0][2];
    TranslationY = MyMatrix[1][2];

    // We will need to modify the SLO in order to preserve the flips
    // Horizontal slos remain horizontal and vertical slos remain vertical
    HRFScanlineOrientation OriginalSLO = GetScanlineOrientation();

    // Check if horizontal scaling factor is negative
    if (MyMatrix[0][0] < 0.0)
        {
        // Change slo 0 to slo 2
        if (OriginalSLO == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 2;
            TranslationY -= Scaling * GetWidth();
            }
        // Change slo 1 to slo 3
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 3;
            TranslationY -= Scaling * GetWidth();
            }
        // Change slo 2 to slo 0
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 0;
            TranslationY += Scaling * GetWidth();
            }
        // Change slo 3 to slo 1
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 1;
            TranslationY += Scaling * GetWidth();
            }
        // Change slo 4 to slo 5
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 5;
            TranslationY += Scaling * GetWidth();
            }
        // Change slo 5 to slo 4
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 4;
            TranslationX -= Scaling * GetWidth();
            }
        // Change slo 6 to slo 7
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 7;
            TranslationX += Scaling * GetWidth();
            }
        // Change slo 7 to slo 6
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 6;
            TranslationX -= Scaling * GetWidth();
            }
        }

    if (MyMatrix[1][1] < 0.0)
        {
        // Set scaling to absolute value
        MyMatrix[1][1] == fabs(MyMatrix[1][1]);

        // Change slo 0 to slo 1
        if (OriginalSLO == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 1;
            TranslationX -= Scaling * GetWidth();
            }
        // Change slo 1 to slo 0
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 0;
            }
        // Change slo 2 to slo 3
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 3;
            }
        // Change slo 3 to slo 2
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 3;
            }
        // Change slo 4 to slo 6
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 6;
            }
        // Change slo 5 to slo 7
        else if (OriginalSLO == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 7;
            }
        // Change slo 6 to slo 4
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 4;
            }
        // Change slo 7 to slo 5
        else if (OriginalSLO == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            m_IntergraphHeader.IBlock1.slo  = 5;
            }
        }
    }
#endif


return Result;
    }


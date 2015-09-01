//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphColorFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphColorFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFIntergraphColorFile.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphColorFile::HRFIntergraphColorFile(const HFCPtr<HFCURL>& pi_rURL,
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
HRFIntergraphColorFile::HRFIntergraphColorFile(const HFCPtr<HFCURL>& pi_rURL,
                                               HFCAccessMode         pi_AccessMode,
                                               uint64_t             pi_Offset,
                                               bool                 pi_DontOpenFile)
    : HRFIntergraphFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
//    SetBitPerPixel(1);
    }



//-----------------------------------------------------------------------------
// Destructor
// Closes the Intergraph color file
//-----------------------------------------------------------------------------
HRFIntergraphColorFile::~HRFIntergraphColorFile()
    {
    // This function must be called here before destruction of Intergraph color file
    // since the close function uses overridden virtual functions
    HRFIntergraphFile::Close();
    }


//-----------------------------------------------------------------------------
// AddPage
//-----------------------------------------------------------------------------
bool HRFIntergraphColorFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation with the capabilities if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);

    if (!pi_pPage->HasTransfoModel())
        {
        // No model specified. Add our default model (Y flip)
        HGF2DStretch Flip;
        Flip.SetYScaling(-1.0);

        pi_pPage->SetTransfoModel(Flip);
        }

    return HRFIntergraphFile::AddPage(pi_pPage);
    }


//-----------------------------------------------------------------------------
// GetTransfoModel
// Prepares and sets the internal transformation model
//-----------------------------------------------------------------------------
void HRFIntergraphColorFile::GetTransfoModel()
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    // The bits per pixel must be greater than 1
    HPRECONDITION(m_BitPerPixel > 1);

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
        int32_t NumPixelsPerScanline = GetWidth(0);
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
        // The Intergraph version is 3 or more and there is a valid matrix

        // Instanciate required affine
        pNewAffine = new HGF2DAffine();

        // SLO 0
        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -m_IntergraphHeader.IBlock1.trn[4],
                                              m_IntergraphHeader.IBlock1.trn[5],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -m_IntergraphHeader.IBlock1.trn[0],
                                              m_IntergraphHeader.IBlock1.trn[1]);

            }
        // SLO 1
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              m_IntergraphHeader.IBlock1.trn[4],
                                              -m_IntergraphHeader.IBlock1.trn[5],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -m_IntergraphHeader.IBlock1.trn[0],
                                              m_IntergraphHeader.IBlock1.trn[1]);

            }
        // SLO 2
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -m_IntergraphHeader.IBlock1.trn[4],
                                              m_IntergraphHeader.IBlock1.trn[5],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              m_IntergraphHeader.IBlock1.trn[0],
                                              -m_IntergraphHeader.IBlock1.trn[1]);
            }
        // SLO 3
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              m_IntergraphHeader.IBlock1.trn[4],
                                              -m_IntergraphHeader.IBlock1.trn[5],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              m_IntergraphHeader.IBlock1.trn[0],
                                              -m_IntergraphHeader.IBlock1.trn[1]);
            }
        // SLO 4
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              m_IntergraphHeader.IBlock1.trn[0],
                                              -m_IntergraphHeader.IBlock1.trn[1],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              m_IntergraphHeader.IBlock1.trn[4],
                                              -m_IntergraphHeader.IBlock1.trn[5]);
            }
        // SLO 5
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -m_IntergraphHeader.IBlock1.trn[0],
                                              m_IntergraphHeader.IBlock1.trn[1],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              m_IntergraphHeader.IBlock1.trn[4],
                                              -m_IntergraphHeader.IBlock1.trn[5]);
            }
        // SLO 6
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              m_IntergraphHeader.IBlock1.trn[0],
                                              -m_IntergraphHeader.IBlock1.trn[1],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -m_IntergraphHeader.IBlock1.trn[4],
                                              m_IntergraphHeader.IBlock1.trn[5]);
            }
        // SLO 7
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            pNewAffine->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                              -m_IntergraphHeader.IBlock1.trn[0],
                                              m_IntergraphHeader.IBlock1.trn[1],
                                              m_IntergraphHeader.IBlock1.trn[7],
                                              -m_IntergraphHeader.IBlock1.trn[4],
                                              m_IntergraphHeader.IBlock1.trn[5]);
            }
        }


    HFCPtr<HGF2DTransfoModel> pModel =  pNewAffine->CreateSimplifiedModel();
    if (pModel != 0)
        m_pTransfoModel = pModel;
    else
        m_pTransfoModel = pNewAffine;
    }

//-----------------------------------------------------------------------------
// Protected
// Sets the transformation of the Intergraph Color file
// Fields in the intergraph header are updated to reflect the current
// global transformation of the file
//-----------------------------------------------------------------------------
bool HRFIntergraphColorFile::SetGlobalTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_HasHeaderFilled);

    bool Result = false;

    // Check if the current file is expressed along intergraph world
    // or default world. In the case it is default world, there are
    // no geo-reference and thus nothing to store.
//HChkAR ???? Should we set it to intergraph then ... I do not believe so
    if (GetWorldIdentificator() == HGF2DWorld_INTERGRAPHWORLD)
        {
        // Make sure the model can be represented by a matrix.
        HASSERT(pi_rpTransfoModel->CanBeRepresentedByAMatrix());

        // Extract matrix
        HFCMatrix<3, 3> MyMatrix = pi_rpTransfoModel->GetMatrix();

        // The matrix must have projection parameters null
        // And global scale equal to 1.0
        HASSERT(MyMatrix[2][0] == 0.0);
        HASSERT(MyMatrix[2][1] == 0.0);
        HASSERT(MyMatrix[2][2] == 1.0);

        // Fill-in the geo-reference parameters that are fixed
        m_IntergraphHeader.IBlock1.trn[2]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[6]  = 0.0;
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

        // The matrix to store depends on the SLO
        // SLO 0
        if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = -MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[1]  = MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = -MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[5]  = MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 1
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = -MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[1]  = MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[5]  = -MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 2
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[1]  = -MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = -MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[5]  = MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 3
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[1]  = -MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[5]  = -MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 4
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[1]  = -MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[5]  = -MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 5
        else if (m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = -MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[1]  = MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[5]  = -MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 6
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[1]  = -MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = -MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[5]  = MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        // SLO 7
        else if (m_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            // Fill the affine matrix.
            m_IntergraphHeader.IBlock1.trn[0]  = -MyMatrix[0][0];
            m_IntergraphHeader.IBlock1.trn[1]  = MyMatrix[0][1];
            m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
            m_IntergraphHeader.IBlock1.trn[4]  = -MyMatrix[1][0];
            m_IntergraphHeader.IBlock1.trn[5]  = MyMatrix[1][1];
            m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
            }
        }

    Result = true;

#if (0)
    // Check if the transformation can be represented by a matrix.
    if (pi_rpTransfoModel->CanBeRepresentedByAMatrix())
        {
        HFCMatrix<3, 3> MyMatrix = pi_rpTransfoModel->GetMatrix();

        // Fill the affine matrix.
        m_IntergraphHeader.IBlock1.trn[0]  = MyMatrix[0][0];
        m_IntergraphHeader.IBlock1.trn[1]  = MyMatrix[0][1];
        m_IntergraphHeader.IBlock1.trn[2]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[3]  = MyMatrix[0][2];
        m_IntergraphHeader.IBlock1.trn[4]  = MyMatrix[1][0];
        m_IntergraphHeader.IBlock1.trn[5]  = MyMatrix[1][1];
        m_IntergraphHeader.IBlock1.trn[6]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[7]  = MyMatrix[1][2];
        m_IntergraphHeader.IBlock1.trn[8]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[9]  = 0.0;
        m_IntergraphHeader.IBlock1.trn[10] = 1.0;
        m_IntergraphHeader.IBlock1.trn[11] = 0.0;
        m_IntergraphHeader.IBlock1.trn[12] = MyMatrix[2][0];
        m_IntergraphHeader.IBlock1.trn[13] = MyMatrix[2][1];
        m_IntergraphHeader.IBlock1.trn[14] = 0.0;
        m_IntergraphHeader.IBlock1.trn[15] = MyMatrix[2][2];

//        HGF2DProjective *tempRasterTransfo = new HGF2DProjective();
        HGF2DAffine* tempRasterTransfo = new HGF2DAffine();

        double Degree = (PI/180);

//        HFCMatrix<3, 3> MyOtherMatrix;
//        MyOtherMatrix[0][0] = m_IntergraphHeader.IBlock1.trn[0];
//        MyOtherMatrix[0][1] = m_IntergraphHeader.IBlock1.trn[1];
//        MyOtherMatrix[0][2] = m_IntergraphHeader.IBlock1.trn[3];
//        MyOtherMatrix[1][0] = m_IntergraphHeader.IBlock1.trn[4];
//        MyOtherMatrix[1][1] = m_IntergraphHeader.IBlock1.trn[5];
//        MyOtherMatrix[1][2] = m_IntergraphHeader.IBlock1.trn[7];
//        MyOtherMatrix[2][2] = 1.0;

//        tempRasterTransfo->SetByMatrix(MyOtherMatrix);
        tempRasterTransfo->SetByMatrixParameters(m_IntergraphHeader.IBlock1.trn[3],
                                                 m_IntergraphHeader.IBlock1.trn[0],
                                                 m_IntergraphHeader.IBlock1.trn[1],
                                                 m_IntergraphHeader.IBlock1.trn[7],
                                                 m_IntergraphHeader.IBlock1.trn[4],
                                                 m_IntergraphHeader.IBlock1.trn[5]);

        m_IntergraphHeader.IBlock1.xori = m_IntergraphHeader.IBlock1.trn[3];
        m_IntergraphHeader.IBlock1.yor = m_IntergraphHeader.IBlock1.trn[7];
        m_IntergraphHeader.IBlock1.zor = 0;
        m_IntergraphHeader.IBlock1.xdl = tempRasterTransfo->GetXScaling();
        m_IntergraphHeader.IBlock1.ydl = tempRasterTransfo->GetYScaling();
        m_IntergraphHeader.IBlock1.zdl = 1;
        double tempRotAngle(tempRasterTransfo->GetRotation());
        m_IntergraphHeader.IBlock1.rot = tempRotAngle * Degree;
        double tempSkewAngle(tempRasterTransfo->GetAnorthogonality());
        m_IntergraphHeader.IBlock1.skw = tempSkewAngle * Degree;

        delete tempRasterTransfo;
        Result = true;
        }
#endif

    return Result;

    }


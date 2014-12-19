//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetTransformHandler.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFInternetTransformHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetTransformHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HGF2DProjective.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("affine-transform");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetTransformHandler::HRFInternetTransformHandler()
    : HRFInternetASCIIHandler("affine-transform")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetTransformHandler::~HRFInternetTransformHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetTransformHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                         HFCBuffer&              pio_rBuffer,
                                         HFCInternetConnection&  pi_rConnection)
    {
    double a11, a12, a13, a14;
    double a21, a22, a23, a24;
    double a31, a32, a33, a34;
    double a41, a42, a43, a44;

    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    // Scan the matrix parameters in the buffer

    // Place the buffer in a string in order to force a '\0' at the end in order to avoid an access violation
    if (sscanf(string((const char*)pio_rBuffer.GetData(), pio_rBuffer.GetDataSize()).c_str() + s_Label.size() + 1,
               "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
               &a11, &a12, &a13, &a14,
               &a21, &a22, &a23, &a24,
               &a31, &a32, &a33, &a34,
               &a41, &a42, &a43, &a44) != 16)
        throw HRFIIHandlerException(HRFII_INVALID_DATA_FORMAT_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::TRANSFORM);

    // Build a matrix from these params
    HFCMatrix<3, 3> Matrix;
    Matrix[0][0] = a11;
    Matrix[0][1] = a12;
    Matrix[0][2] = a14;
    Matrix[1][0] = a21;
    Matrix[1][1] = a22;
    Matrix[1][2] = a24;
    Matrix[2][0] = a41;
    Matrix[2][1] = a42;
    Matrix[2][2] = a44;

    // Build a transfo model from the matrix
    HFCPtr<HGF2DTransfoModel> pTransfo(new HGF2DProjective());
    ((HFCPtr<HGF2DProjective>&)pTransfo)->SetByMatrix(Matrix);

    // Set the transfo model in the internet image
    HFCMonitor Monitor(pi_rFile.m_DataKey);
    pi_rFile.m_pTransfoModel = pTransfo;
    }

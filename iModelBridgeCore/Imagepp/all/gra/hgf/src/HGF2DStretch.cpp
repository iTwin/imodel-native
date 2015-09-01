//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DStretch.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DStretch
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>


// The class declaration must be the last include file.
#include <Imagepp/all/h/HGF2DStretch.h>


/** -----------------------------------------------------------------------------
    Default Constructor
    Initializes the model to no transformation with units for all channels
    set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DStretch::HGF2DStretch()
    : HGF2DTransfoModel(),
      m_ScaleX(1.0),
      m_ScaleY(1.0),
      m_XTranslation(0.0),
      m_YTranslation(0.0)
    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Constructor
    This method constructs new instances of HGF2DStretch objects.
    Sets directly the transformation parameters. The dimension units are then
    taken from the provided displacement.

    @param pi_rTranslation IN A constant reference to an HGF2DDisplacement
                              object containing the description of the translation
                              component to set into the stretch.

    @param pi_ScalingX IN The scaling factor for x coordinates. This scaling
                          must be different from 0.0.

    @param pi_ScalingY IN The scaling factor for y coordinates. This scaling
                          must be different from 0.0.

    @code
        HGF2DDisplacement   MyTranslation (10.0, 10.0);
        HGF2DStretch        MyModel(MyTranslation, 2.0, 2.1);
    @end

    -----------------------------------------------------------------------------
*/
HGF2DStretch::HGF2DStretch(const HGF2DDisplacement& pi_rTranslation,
                           double                  pi_ScaleX,
                           double                  pi_ScaleY)
    : HGF2DTransfoModel(),
    m_XTranslation (pi_rTranslation.GetDeltaX()),
    m_YTranslation (pi_rTranslation.GetDeltaY()),
    m_ScaleX(pi_ScaleX),
    m_ScaleY(pi_ScaleY)
    {
    // Check that scales provided are not null
    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);

    Prepare();
    }




//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DStretch::HGF2DStretch(const HGF2DStretch& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy (pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destroyer.
//-----------------------------------------------------------------------------
HGF2DStretch::~HGF2DStretch()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DStretch& HGF2DStretch::operator=(const HGF2DStretch& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DStretch::IsConvertDirectThreadSafe() const
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DStretch::IsConvertInverseThreadSafe() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertDirect(double* pio_pXInOut,
                                 double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Copy input values since needed by both equations
    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Transform coordinates
    *pio_pXInOut = (X * m_PreparedDirectA1) + m_XTranslationPrime;
    *pio_pYInOut = (Y * m_PreparedDirectA2) + m_YTranslationPrime;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertDirect (double    pi_YIn,
                                       double    pi_XInStart,
                                       size_t    pi_NumLoc,
                                       double    pi_XInStep,
                                       double*   po_aXOut,
                                       double*   po_aYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double  ByProdY = (pi_YIn * m_PreparedDirectA2) + m_YTranslationPrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = (X * m_PreparedDirectA1) + m_XTranslationPrime;
        *pCurrentY = ByProdY;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertDirect (size_t    pi_NumLoc,
                                       double*   pio_aXInOut,
                                       double*   pio_aYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    double  ByProdY;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        ByProdY = (Y * m_PreparedDirectA2) + m_YTranslationPrime;

        pio_aXInOut[i] = (X * m_PreparedDirectA1) + m_XTranslationPrime;
        pio_aYInOut[i] = ByProdY;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertDirect(double   pi_XIn,
                                      double   pi_YIn,
                                      double*  po_pXOut,
                                      double*  po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Transform coordinates
    *po_pXOut = (pi_XIn * m_PreparedDirectA1) + m_XTranslationPrime;
    *po_pYOut = (pi_YIn * m_PreparedDirectA2) + m_YTranslationPrime;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertInverse(double* pio_pXInOut,
                                       double* pio_pYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    // Transform coordinates
    *pio_pXInOut = (X * m_PreparedInverseA1) + m_XTranslationInversePrime;
    *pio_pYInOut = (Y * m_PreparedInverseA2) + m_YTranslationInversePrime;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertInverse (double    pi_YIn,
                                        double    pi_XInStart,
                                        size_t    pi_NumLoc,
                                        double    pi_XInStep,
                                        double*   po_aXOut,
                                        double*   po_aYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double  ByProdY = (pi_YIn * m_PreparedInverseA2) + m_YTranslationInversePrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = (X * m_PreparedInverseA1) + m_XTranslationInversePrime;
        *pCurrentY = ByProdY;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertInverse (size_t    pi_NumLoc,
                                        double*   pio_aXInOut,
                                        double*   pio_aYInOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    double  ByProdY;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        ByProdY = (Y * m_PreparedInverseA2) + m_YTranslationInversePrime;

        pio_aXInOut[i] = (X * m_PreparedInverseA1) + m_XTranslationInversePrime;
        pio_aYInOut[i] = ByProdY;
        }

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DStretch::ConvertInverse(double  pi_XIn,
                                       double  pi_YIn,
                                       double* po_pXOut,
                                       double* po_pYOut) const
    {
    // Make sure that recipient variables are provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Transform coordinates
    *po_pXOut = (pi_XIn * m_PreparedInverseA1) + m_XTranslationInversePrime;
    *po_pYOut = (pi_YIn * m_PreparedInverseA2) + m_YTranslationInversePrime;

    return SUCCESS;
    }




//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DStretch::IsStretchable (double pi_AngleTolerance) const
    {
    return(true);
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DStretch::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DStretch::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DStretch::PreservesShape() const
    {
    // Shape is preserved only if there is no anisotropic scaling implied
    return (m_ScaleX == m_ScaleY);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DStretch::PreservesDirection() const
    {
    // Direction is preserved only if there is no anisotropic scaling implied
    return (m_ScaleX == m_ScaleY);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DStretch::CanBeRepresentedByAMatrix() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DStretch::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    return(GetMatrix(ReturnedMatrix));
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DStretch::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    po_rRecipient[0][2] = m_XTranslation;
    po_rRecipient[1][2] = m_YTranslation;
    po_rRecipient[0][0] = m_ScaleX;
    po_rRecipient[0][1] = 0.0;
    po_rRecipient[1][0] = 0.0;
    po_rRecipient[1][1] = m_ScaleY;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }



//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DStretch::IsIdentity () const
    {

    return ((m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0) &&
            (m_ScaleX == 1.0) &&
            (m_ScaleY == 1.0));
    }




//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DStretch::GetStretchParams (double* po_pScaleFactorX,
                                     double* po_pScaleFactorY,
                                     HGF2DDisplacement* po_pDisplacement) const
    {
    // Check that recipient variables are provided
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    // Extract stretch parameters
    *po_pScaleFactorX = m_ScaleX;
    *po_pScaleFactorY = m_ScaleY;

    *po_pDisplacement = GetTranslation();
    }



/** -----------------------------------------------------------------------------
    This method sets the translation component of the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing
                              the new translation component of the model.

    @code
        HGF2DDisplacement  Translation (10.0, 10.0);

        MyModel.SetTranslation  (Translation);
    @end

    @see GetTranslation()
    @see AddTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::SetTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation = pi_rTranslation.GetDeltaX();
    m_YTranslation = pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }



/** -----------------------------------------------------------------------------
    This method adds the specified translation component to the model.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing the
                              translation component to add to the model.

    @code
        HGF2DDisplacement  Translation (10.0, 10.0);

        MyModel.AddTranslation  (Translation);
    @end

    @see GetTranslation()
    @see SetTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::AddTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation += pi_rTranslation.GetDeltaX();
    m_YTranslation += pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }



/** -----------------------------------------------------------------------------
    This method sets scaling component of the model in the X direction.

    @param pi_ScaleX IN The scaling factor of the scaling component of the model
                        in the X direction.
                        This factor must be different than 0.0

    @code
        HGF2DStretch  MyModel;
        MyModel.SetXScaling (34.5);
    @end

    @see GetXScaling()
    @see SetYScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::SetXScaling (double pi_ScaleX)
    {
    // Check that scale provided is not null
    HPRECONDITION(pi_ScaleX != 0.0);

    // Set scale
    m_ScaleX = pi_ScaleX;

    // Compute prepared settings
    Prepare ();
    }


/** -----------------------------------------------------------------------------
    This method sets scaling component of the model in the Y direction.

    @param pi_ScaleY IN The scaling factor of the scaling component of the model
                        in the Y direction.
                        This factor must be different than 0.0

    @code
        HGF2DStretch  MyModel;
        MyModel.SetYScaling (34.5);
    @end

    @see GetXYcaling()
    @see SetXScaling()
    @see AddIsotropicScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::SetYScaling (double pi_ScaleY)
    {
    // Check that scale provided is not null
    HPRECONDITION(pi_ScaleY != 0.0);

    // Set scale
    m_ScaleY = pi_ScaleY;

    // Compute prepared settings
    Prepare ();
    }




/** -----------------------------------------------------------------------------
    This method adds an isotropic scaling around a specified center of scaling
    to the current components of the model.
    If center omitted, then the origin is used.
    Remember that when specifying a center different than the origin, a
    translation component is also added to the model.

    @param pi_Scale IN The scaling factor of the scaling component to add
                       to the model. This factor must be different from 0.0.


    @param pi_rXCenter IN OPTIONAL A double that
                                contains the X value of the center of the scaling
                                to add to the model.

    @param pi_rYCenter IN OPTIONAL A double that
                                contains the Y value of the center of the scaling
                                to add to the model.


    @see GetXScaling()
    @see GetYScaling()
    @see AddAnisotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::AddIsotropicScaling (double pi_Scale,
                                        double pi_XCenter,
                                        double pi_YCenter)
    {
    // Check that scale provided is not null
    HPRECONDITION(pi_Scale != 0.0);

    // Add scaling component
    m_ScaleX *= pi_Scale;
    m_ScaleY *= pi_Scale;

    // calculate new translation component
    double TransX = ((m_XTranslation * pi_Scale) +
                     (pi_XCenter - pi_XCenter * pi_Scale));
    double TransY = ((m_YTranslation * pi_Scale) +
                     (pi_YCenter - pi_YCenter * pi_Scale));

    // Set translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;

    // Compute prepared settings
    Prepare ();
    }



/** -----------------------------------------------------------------------------
    This method adds an anisotropic scaling (different in X and Y dimensions)
    around a specified center of scaling to the current components of the model.
    If center is omitted, then the origin is used.
    Remember that when specifying a center different than the origin, a
    translation component is also added to the model.

    @param pi_ScaleX IN The scaling factor of the scaling component to
                        add to the model in the X dimension.
                        This factor must be different from 0.0.

    @param pi_ScaleY IN The scaling factor of the scaling component to
                        add to the model in the Y dimension.
                        This factor must be different from 0.0.

    @param pi_rXCenter IN OPTIONAL A double that
                        contains the X value of the center of the scaling to
                        add to the model.

    @param pi_rYCenter IN OPTIONAL A double that
                        contains the Y value of the center of the scaling to
                        add to the model.

    @see GetXScaling()
    @see GetYScaling()
    @see AddIsotropicScaling()
    -----------------------------------------------------------------------------
*/
void HGF2DStretch::AddAnisotropicScaling (double pi_ScaleX,
                                          double pi_ScaleY,
                                          double pi_XCenter,
                                          double pi_YCenter)
    {
    // Check that scales provided are not null
    HPRECONDITION(pi_ScaleX != 0.0);
    HPRECONDITION(pi_ScaleY != 0.0);

    // Add scaling component
    m_ScaleX *= pi_ScaleX;
    m_ScaleY *= pi_ScaleY;

    // calculate new translation component
    double TransX = ((m_XTranslation * pi_ScaleX) +
                     (pi_XCenter - pi_XCenter * pi_ScaleX));
    double TransY = ((m_YTranslation * pi_ScaleY) +
                     (pi_YCenter - pi_YCenter * pi_ScaleY));

    // Set translation
    m_XTranslation = TransX;
    m_YTranslation = TransY;

    // Compute prepared settings
    Prepare ();
    }






//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DStretch::Reverse()
    {
    m_ScaleX = 1/m_ScaleX;
    m_ScaleY = 1/m_ScaleY;
    m_XTranslation = m_XTranslationInverse;
    m_YTranslation = m_YTranslationInverse;

    // Invoque reversing of ancester
    HGF2DTransfoModel::Reverse();

    Prepare();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DStretch::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    // The only models known are IDENTITY, TRANSLATION, SIMILITUDE and AFFINE
    if (pi_rModel.GetClassID() == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DStretch(*this);

        }
    else if (pi_rModel.GetClassID() == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be stretch
        HAutoPtr<HGF2DStretch> pMyStretch(new HGF2DStretch());

        // Cast model as a stretch
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute composed translation
        double TransX = m_XTranslation + pModelPrime->GetTranslation().GetDeltaX();
        double TransY = m_YTranslation + pModelPrime->GetTranslation().GetDeltaY();

        // Set parameters
        pMyStretch->m_ScaleX = m_ScaleX;
        pMyStretch->m_ScaleY = m_ScaleY;
        pMyStretch->m_XTranslation = TransX;
        pMyStretch->m_YTranslation = TransY;
        pMyStretch->Prepare();

        // Assign to smart pointer
        pResultModel = pMyStretch.release();
        }
    // Similitude and affine give similar results
    else if ((pi_rModel.GetClassID() == HGF2DAffine::CLASS_ID) ||
             (pi_rModel.GetClassID() == HGF2DSimilitude::CLASS_ID))
        {
        // A similitude must be representable by a matrix
        HASSERT(pi_rModel.CanBeRepresentedByAMatrix());

        // Obtain matrix from similitude
        HFCMatrix<3, 3> MyGivenMatrix(pi_rModel.GetMatrix());

        // Compose the two matrix together
        HAutoPtr<HGF2DAffine> pResultAffine(new HGF2DAffine());

        // Perform composition
        HFCMatrix<3, 3> MyNewMatrix = MyGivenMatrix * GetMatrix();

        // Make sure the projection parameters are null
        HASSERT(MyNewMatrix[2][0] == 0.0);
        HASSERT(MyNewMatrix[2][1] == 0.0);

        // Make sure gloabl scale is 1.0
        HASSERT(MyNewMatrix[2][2] == 1.0);

        // Set the affine
        pResultAffine->SetByMatrixParameters(MyNewMatrix[0][2],
                                             MyNewMatrix[0][0],
                                             MyNewMatrix[0][1],
                                             MyNewMatrix[1][2],
                                             MyNewMatrix[1][0],
                                             MyNewMatrix[1][1]);

        // Set result model
        pResultModel = pResultAffine.release();
        }
    else if (pi_rModel.GetClassID() == HGF2DStretch::CLASS_ID)
        {
        // We have two stretch models ... compose a stretch
        // Allocate
        HAutoPtr<HGF2DStretch> pMyStretch(new HGF2DStretch());


        // Cast model as a stretch
        const HGF2DStretch* pModelPrime = static_cast<const HGF2DStretch*>(&pi_rModel);

        // Calculate new translation
        double TransX = m_XTranslation * pModelPrime->m_ScaleX +
                        pModelPrime->m_XTranslation;

        double TransY = m_YTranslation * pModelPrime->m_ScaleY +
                        pModelPrime->m_YTranslation;


        // Set parameters
        pMyStretch->m_XTranslation = TransX;
        pMyStretch->m_YTranslation = TransY;
        pMyStretch->m_ScaleX = m_ScaleX * pModelPrime->m_ScaleX;
        pMyStretch->m_ScaleY = m_ScaleY * pModelPrime->m_ScaleY;
        pMyStretch->Prepare();

        // Assign to smart pointer
        pResultModel = pMyStretch.release();
        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return(pResultModel);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DStretch::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DStretch (*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DStretch::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    // The only models known are IDENTITY, TRANSLATION, SIMILITUDE, and AFFINE but stretch
    // would have processed itself so not processed.
    if (pi_rModel.GetClassID() == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DStretch(*this);
        }
    else if (pi_rModel.GetClassID() == HGF2DTranslation::CLASS_ID)
        {
        // Model is translation ... result will be stretch
        HAutoPtr<HGF2DStretch> pMyStretch(new HGF2DStretch());

        // Cast model as a stretch
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute composed translation
        double TransX = pModelPrime->GetTranslation().GetDeltaX() * m_ScaleX +
                        m_XTranslation;
        double TransY = pModelPrime->GetTranslation().GetDeltaY() * m_ScaleY +
                        m_YTranslation;

        // Set parameters
        pMyStretch->m_ScaleX = m_ScaleX;
        pMyStretch->m_ScaleY = m_ScaleY;
        pMyStretch->m_XTranslation = TransX;
        pMyStretch->m_YTranslation = TransY;
        pMyStretch->Prepare();

        // Assign to smart pointer
        pResultModel = pMyStretch.release();
        }
    else if (pi_rModel.GetClassID() == HGF2DSimilitude::CLASS_ID)
        {
        // Extract matrix of self
        HFCMatrix<3, 3> MySelfMatrix(GetMatrix());

        // Compose the two matrix together
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());

        HFCMatrix<3, 3> ResultMatrix = (MySelfMatrix * pi_rModel.GetMatrix());

        // The result should be an affine
        HASSERT(ResultMatrix[2][0] == 0);
        HASSERT(ResultMatrix[2][1] == 0);

        // Normalize matrix
        if (ResultMatrix[2][2] != 1.0)
            {
            ResultMatrix /= ResultMatrix[2][2];
            }

        // Set affine
        pMyAffine->SetByMatrixParameters(ResultMatrix[0][2], ResultMatrix[0][0], ResultMatrix[0][1],
                                         ResultMatrix[1][2], ResultMatrix[1][0], ResultMatrix[1][1]);

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
        }
    else if (pi_rModel.GetClassID() == HGF2DAffine::CLASS_ID)
        {
        HAutoPtr<HGF2DAffine> pMyAffine(new HGF2DAffine());

        HFCMatrix<3, 3> MySelfMatrix(GetMatrix());

        // Compose the two matrix together

        HFCMatrix<3, 3> NewMatrix =  MySelfMatrix * pi_rModel.GetMatrix();

        // The result projection parameters must be null
        // No this is not an error ... we want EXACT compare
        HASSERT(NewMatrix[2][0] == 0.0);
        HASSERT(NewMatrix[2][1] == 0.0);

        // The global scale must be 1
        // No this is not an error ... we want EXACT compare
        HASSERT(NewMatrix[2][2] == 1.0);

        pMyAffine->SetByMatrixParameters(NewMatrix[0][2],
                                         NewMatrix[0][0],
                                         NewMatrix[0][1],
                                         NewMatrix[1][2],
                                         NewMatrix[1][0],
                                         NewMatrix[1][1]);

        // Assign to smart pointer
        pResultModel = pMyAffine.release();
        }
    else
        {
        // Type is not known ... build a complex
        // To do this we call the ancester ComposeYourself
        pResultModel = HGF2DTransfoModel::ComposeYourself (pi_rModel);
        }

    return (pResultModel);


    }


//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DStretch::Prepare ()
    {
    double             InvScaleX;
    double             InvScaleY;

    // Compute X scale
    InvScaleX = 1 / m_ScaleX;

    // Compute Y scale
    InvScaleY = 1 / m_ScaleY;

    // Compute new translations
    m_XTranslationInverse = (-m_XTranslation / m_ScaleX);

    m_YTranslationInverse = (-m_YTranslation / m_ScaleY);

    // Prepare transformation factor for direct and inverse transformation
    m_PreparedDirectA1 = m_ScaleX;
    m_PreparedDirectA2 = m_ScaleY;

    m_PreparedInverseA1 = InvScaleX;
    m_PreparedInverseA2 = InvScaleY;

    // Set prime translations
    m_XTranslationInversePrime = m_XTranslationInverse;
    m_YTranslationInversePrime = m_YTranslationInverse;

    m_XTranslationPrime = m_XTranslation;
    m_YTranslationPrime = m_YTranslation;
    }



//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DStretch::Copy(const HGF2DStretch& pi_rObj)
    {
    // Copy stretch parameters
    m_XTranslation = pi_rObj.m_XTranslation;
    m_YTranslation = pi_rObj.m_YTranslation;
    m_ScaleX = pi_rObj.m_ScaleX;
    m_ScaleY = pi_rObj.m_ScaleY;

    // Copy prepared items
    m_PreparedDirectA1 = pi_rObj.m_PreparedDirectA1;
    m_PreparedInverseA1 = pi_rObj.m_PreparedInverseA1;
    m_PreparedDirectA2 = pi_rObj.m_PreparedDirectA2;
    m_PreparedInverseA2 = pi_rObj.m_PreparedInverseA2;
    m_XTranslationInverse = pi_rObj.m_XTranslationInverse;
    m_YTranslationInverse = pi_rObj.m_YTranslationInverse;
    m_XTranslationInversePrime = pi_rObj.m_XTranslationInversePrime;
    m_YTranslationInversePrime = pi_rObj.m_YTranslationInversePrime;
    m_XTranslationPrime = pi_rObj.m_XTranslationPrime;
    m_YTranslationPrime = pi_rObj.m_YTranslationPrime;
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DStretch::CreateSimplifiedModel() const
    {
    if (m_ScaleX == 1.0 && m_ScaleY == 1.0)
        {
        if ((m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0))
            {
            return new HGF2DIdentity();
            }
        else
            {
            return new HGF2DTranslation(GetTranslation());
            }
        }

    // If we get here, no simplification is possible.
    return 0;
    }

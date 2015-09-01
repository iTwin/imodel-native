//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DTranslation.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DTranslation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

// The class declaration must be the last include file.
#include <Imagepp/all/h/HGF2DTranslation.h>


/** -----------------------------------------------------------------------------
    Default constructor
    Initializes the translation to no translation with units for all channels
    set to meters.
    -----------------------------------------------------------------------------
*/
HGF2DTranslation::HGF2DTranslation()
    : HGF2DTransfoModel(),
       m_XTranslation(0.0),
       m_YTranslation(0.0)
    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Constructor
    Initializes the translation to given translation. The units are extracted from
    the displacement.
    -----------------------------------------------------------------------------
*/
HGF2DTranslation::HGF2DTranslation(const HGF2DDisplacement& pi_rTranslation)
    : HGF2DTransfoModel(),
    m_XTranslation(pi_rTranslation.GetDeltaX()),
    m_YTranslation(pi_rTranslation.GetDeltaY())
    {
    Prepare();
    }


/** -----------------------------------------------------------------------------
    Copy Constructor
    -----------------------------------------------------------------------------
*/
HGF2DTranslation::HGF2DTranslation(const HGF2DTranslation& pi_rObj)
    : HGF2DTransfoModel (pi_rObj)
    {
    Copy (pi_rObj);
    }

/** -----------------------------------------------------------------------------
    Destroyer
    -----------------------------------------------------------------------------
*/
HGF2DTranslation::~HGF2DTranslation()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DTranslation& HGF2DTranslation::operator=(const HGF2DTranslation& pi_rObj)
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
bool HGF2DTranslation::IsConvertDirectThreadSafe() const 
    { 
    return true; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alexandre.Gariepy               06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HGF2DTranslation::IsConvertInverseThreadSafe() const 
    { 
    return true; 
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertDirect(double* pio_pXInOut,
                                          double* pio_pYInOut) const
    {
    // Check variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Transform coordinates without unit conversions
    *pio_pXInOut += m_XTranslationPrime;
    *pio_pYInOut += m_YTranslationPrime;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertDirect (double    pi_YIn,
                                           double    pi_XInStart,
                                           size_t    pi_NumLoc,
                                           double    pi_XInStep,
                                           double*   po_aXOut,
                                           double*   po_aYOut) const
    {
    // Check arrays are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double    ResultY = pi_YIn + m_YTranslationPrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = X + m_XTranslationPrime;
        *pCurrentY = ResultY;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertDirect (size_t    pi_NumLoc,
                                           double*   pio_aXInOut,
                                           double*   pio_aYInOut) const
    {
    // Check arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        pio_aXInOut[i] = X + m_XTranslationPrime;
        pio_aYInOut[i] = Y + m_YTranslationPrime;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertDirect(double   pi_XIn,
                                          double   pi_YIn,
                                          double*  po_pXOut,
                                          double*  po_pYOut) const
    {
    // Check vars are provided
    HPRECONDITION (po_pXOut != 0);
    HPRECONDITION (po_pYOut != 0);

    // Perform transformation without unit conversion
    *po_pXOut = pi_XIn + m_XTranslationPrime;
    *po_pYOut = pi_YIn + m_YTranslationPrime;

    return SUCCESS;
    }
    


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertInverse(double* pio_pXInOut,
                                           double* pio_pYInOut) const
    {
    // Check vars are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    // Transform coordinates
    *pio_pXInOut += m_XTranslationInversePrime;
    *pio_pYInOut += m_YTranslationInversePrime;

    return SUCCESS;
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertInverse (double    pi_YIn,
                                            double    pi_XInStart,
                                            size_t    pi_NumLoc,
                                            double    pi_XInStep,
                                            double*   po_aXOut,
                                            double*   po_aYOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(po_aXOut != 0);
    HPRECONDITION(po_aYOut != 0);

    uint32_t Index;
    double  X;
    double* pCurrentX = po_aXOut;
    double* pCurrentY = po_aYOut;

    double ResultY = pi_YIn + m_YTranslationInversePrime;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        *pCurrentX = X + m_XTranslationInversePrime;
        *pCurrentY = ResultY;
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertInverse (size_t    pi_NumLoc,
                                            double*   pio_aXInOut,
                                            double*   pio_aYInOut) const
    {
    // Check arrays are provided
    HPRECONDITION(pio_aXInOut != 0);
    HPRECONDITION(pio_aYInOut != 0);

    double X;
    double Y;

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        X = pio_aXInOut[i];
        Y = pio_aYInOut[i];

        pio_aXInOut[i] = X + m_XTranslationInversePrime;
        pio_aYInOut[i] = Y + m_YTranslationInversePrime;
        }

    return SUCCESS;
    }
    
//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DTranslation::ConvertInverse(double  pi_XIn,
                                           double  pi_YIn,
                                           double* po_pXOut,
                                           double* po_pYOut) const
    {
    // Check variables were provided
    HPRECONDITION(po_pXOut != 0);
    HPRECONDITION(po_pYOut != 0);

    // Transform coordinates without unit conversion
    *po_pXOut = pi_XIn + m_XTranslationInversePrime;
    *po_pYOut = pi_YIn + m_YTranslationInversePrime;

    return SUCCESS;
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool HGF2DTranslation::PreservesLinearity () const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool HGF2DTranslation::PreservesParallelism() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool HGF2DTranslation::PreservesShape() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool HGF2DTranslation::PreservesDirection() const
    {
    return (true);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DTranslation::CanBeRepresentedByAMatrix() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DTranslation::GetMatrix() const
    {
    HFCMatrix<3, 3> ReturnedMatrix;

    ReturnedMatrix[0][0] = 1.0;
    ReturnedMatrix[0][1] = 0.0;
    ReturnedMatrix[0][2] = m_XTranslation;
    ReturnedMatrix[1][0] = 0.0;
    ReturnedMatrix[1][1] = 1.0;
    ReturnedMatrix[1][2] = m_YTranslation;
    ReturnedMatrix[2][0] = 0.0;
    ReturnedMatrix[2][1] = 0.0;
    ReturnedMatrix[2][2] = 1.0;

    return(ReturnedMatrix);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the model by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DTranslation::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    po_rRecipient[0][0] = 1.0;
    po_rRecipient[0][1] = 0.0;
    po_rRecipient[0][2] = m_XTranslation;
    po_rRecipient[1][0] = 0.0;
    po_rRecipient[1][1] = 1.0;
    po_rRecipient[1][2] = m_YTranslation;
    po_rRecipient[2][0] = 0.0;
    po_rRecipient[2][1] = 0.0;
    po_rRecipient[2][2] = 1.0;

    return(po_rRecipient);
    }

//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DTranslation::IsIdentity () const
    {
    return ((m_XTranslation == 0.0) &&
            (m_YTranslation == 0.0));
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DTranslation::IsStretchable (double pi_AngleTolerance) const
    {
    return (true);
    }



//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DTranslation::GetStretchParams (double* po_pScaleFactorX,
                                         double* po_pScaleFactorY,
                                         HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    // Extract stretch parameters
    *po_pScaleFactorX = 1.0;
    *po_pScaleFactorY = 1.0;

    *po_pDisplacement = GetTranslation();
    }

/** -----------------------------------------------------------------------------
    This method sets the translation component of the model.
    The units of the model are not modified.

    @param pi_rTranslation IN Reference to an HGF2DDisplacement representing
                              the new translation component of the model.

    @code
        HGF2DTranslation    MyModel;
        HGF2DDisplacement   Displacement(10.0, 12.3);
        MyModel.SetTranslation(Displacement);
    @end

    @see GetTranslation()
    -----------------------------------------------------------------------------
*/
void HGF2DTranslation::SetTranslation (const HGF2DDisplacement& pi_rTranslation)
    {
    // Set translation
    m_XTranslation = pi_rTranslation.GetDeltaX();
    m_YTranslation = pi_rTranslation.GetDeltaY();

    // Compute prepared settings
    Prepare ();
    }


//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transfomation model
//-----------------------------------------------------------------------------
void    HGF2DTranslation::Reverse()
    {
    // Reverse transformation parameters
    m_XTranslation = m_XTranslationInverse;
    m_YTranslation = m_YTranslationInverse;

    // Invoque reversing of ancester
    HGF2DTransfoModel::Reverse();

    // Prepare
    Prepare();
    }


//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DTranslation::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID   TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY and TRANSLATION
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DTranslation(*this);

        }
    else if (TheModelType == HGF2DTranslation::CLASS_ID)
        {
        // We have two translation models ... compose a translation
        // Allocate
        HAutoPtr<HGF2DTranslation> pMyTrans(new HGF2DTranslation());

        // Cast model as a translation
        const HGF2DTranslation* pModelPrime = static_cast<const HGF2DTranslation*>(&pi_rModel);

        // Compute composed translation
        double TransX = m_XTranslation +
                        pModelPrime->m_XTranslation;
        double TransY = m_YTranslation +
                        pModelPrime->m_YTranslation;

        // Set parameters
        pMyTrans->SetTranslation(HGF2DDisplacement(TransX, TransY));

        // Assign to smart pointer
        pResultModel = pMyTrans.release();
        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DTranslation::Clone () const
    {
    // Allocate object as copy and return
    return (new HGF2DTranslation (*this));
    }



//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given has failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned. The major difference with the Compose() method, is that the order
// of composition is reversed,
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DTranslation::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are IDENTITY and translation, but translation
    // would have processed itself so not processed.
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DTranslation(*this);
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
void HGF2DTranslation::Prepare ()
    {

    // Convert direct translation to inverse translation (negative value)
    // GetTranslation() is not used since compiler refused to inline it
    m_XTranslationInverse = - m_XTranslation;
    m_YTranslationInverse = - m_YTranslation;

    m_XTranslationInversePrime = m_XTranslationInverse;
    m_YTranslationInversePrime = m_YTranslationInverse;

    m_XTranslationPrime = m_XTranslation;
    m_YTranslationPrime = m_YTranslation;

    }



//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DTranslation::Copy(const HGF2DTranslation& pi_rObj)
    {

    // Copy translation parameters
    m_XTranslation = pi_rObj.m_XTranslation;
    m_YTranslation = pi_rObj.m_YTranslation;

    // Copy prepared items
    m_ConversionRequired        = pi_rObj.m_ConversionRequired;
    m_PreparedDirectA1          = pi_rObj.m_PreparedDirectA1;
    m_PreparedInverseA1         = pi_rObj.m_PreparedInverseA1;
    m_PreparedDirectA2          = pi_rObj.m_PreparedDirectA2;
    m_PreparedInverseA2         = pi_rObj.m_PreparedInverseA2;
    m_XTranslationInverse       = pi_rObj.m_XTranslationInverse;
    m_YTranslationInverse       = pi_rObj.m_YTranslationInverse;
    m_XTranslationInversePrime  = pi_rObj.m_XTranslationInversePrime;
    m_YTranslationInversePrime  = pi_rObj.m_YTranslationInversePrime;
    m_XTranslationPrime         = pi_rObj.m_XTranslationPrime;
    m_YTranslationPrime         = pi_rObj.m_YTranslationPrime;
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DTranslation::CreateSimplifiedModel() const
    {
    if ((m_XTranslation == 0.0) &&
        (m_YTranslation == 0.0))
        return new HGF2DIdentity();
    else
        return 0;
    }

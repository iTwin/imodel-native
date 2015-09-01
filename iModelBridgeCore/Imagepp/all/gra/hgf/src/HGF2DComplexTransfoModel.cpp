//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DComplexTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DComplexTransfoModel
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HGF2DComplexTransfoModel.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DUniverse.h>


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGF2DComplexTransfoModel::HGF2DComplexTransfoModel()
    : HGF2DTransfoModel(),
      m_hasDomain(false)
    {
    }


/** -----------------------------------------------------------------------------
    Copy constructor
    This create a new copy of the provided complex transformation model.
    A deep copy of the contained models is performed.

    @code
        HGF2DComplexTransfoModel        MyFirstModel ();
        HGF2DComplexTransfoModel        MySecondModel (MyFirstModel);
    @end
    -----------------------------------------------------------------------------
*/
HGF2DComplexTransfoModel::HGF2DComplexTransfoModel(const HGF2DComplexTransfoModel& pi_rObj)
    : HGF2DTransfoModel(),
      m_hasDomain(false)
    {
    try
        {
        Copy (pi_rObj);
        }
    catch(...)
        {
        // Destroy all models in the list
        List_TransfoModel::iterator Itr;

        for (Itr = m_ListOfModels.begin(); (Itr != m_ListOfModels.end()); ++Itr)
            {
            delete (*Itr);
            }

        // Clear list
        m_ListOfModels.clear();

        // Continue the exception propagation.
        throw;
        }
    }

/** -----------------------------------------------------------------------------
    Destroyer
    This destroys the complex transformation model. It therefore destroys all
    transformation models it contains in the list.
    -----------------------------------------------------------------------------
*/
HGF2DComplexTransfoModel::~HGF2DComplexTransfoModel()
    {
    // Destroy all models in the list
    List_TransfoModel::iterator Itr;

    for (Itr = m_ListOfModels.begin(); (Itr != m_ListOfModels.end()); ++Itr)
        {
        delete (*Itr);
        }

    // Clear list
    m_ListOfModels.clear();
    }

/** -----------------------------------------------------------------------------
    Assignment operator
    It copies the given HGF2DComplexTransfoModel to this one. The copy is deep.
    The models included in the list of transformation models from the source
    are themselves duplicated.

    @code
        HGF2DComplexTransfoModel    MyModel ();
        HGF2DComplexTransfoModel    MyOtherModel = MyModel;
    @end
    -----------------------------------------------------------------------------
*/
HGF2DComplexTransfoModel& HGF2DComplexTransfoModel::operator=(const HGF2DComplexTransfoModel& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        HGF2DTransfoModel::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }


/** -----------------------------------------------------------------------------
    This method adds a copy of the given transformation model at the end of the
    complex transformation model chain. The domain will be copied,
    compossed and transformed appropriately if needed.

    @param pi_rModelToAdd IN Constant reference to a transformation model to
                             add to complex..

    @code

        HGF2DComplexTransfoModel    MyModel;
        HGF2DSimilitude             MySimi(Meters, Meters, Meters, Meters);

        MyModel.AddModel(MySimi);

    @end
    -----------------------------------------------------------------------------
*/
void  HGF2DComplexTransfoModel::AddModel (const HGF2DTransfoModel& pi_rModelToAdd)
    {

    // Allocate dynamically a copy of the model
    HGF2DTransfoModel* pModelCopy = pi_rModelToAdd.Clone();

    if (m_ListOfModels.size() <= 0)
        {
        // This is the first model
        // Check if it has a domain defined
        if (pModelCopy->HasDomain())
            {
            // Transformation has domain ... we get these domains
            // Note that we do not copy the shapes as these are
            // shared objects and belong to a transformation model
            // copy owned by the present object.
            m_directDomain = pModelCopy->GetDirectDomain();
            m_inverseDomain = pModelCopy->GetInverseDomain();
            }
        }
    else
        {
        // We check if either transformation model to add or current 
        // state of the complex have domains defined.
        if (pModelCopy->HasDomain() || HasDomain())
            {
            // At least one has a domain ... the result will have a domain
            // Check if model to add has a domain
            if (pModelCopy->HasDomain())
                {
                // Given model has a model ... Check if complex already has one
                if (HasDomain())
                    {
                    // Both have domains. We must compose both domains together
                    // First we intersect the inverse domain of complex and direct domain of added.
                    HFCPtr<HGF2DShape> tempShape = GetInverseDomain()->IntersectShape(*(pModelCopy->GetDirectDomain()));

                    m_inverseDomain = tempShape->AllocTransformDirect(*pModelCopy);
                    m_directDomain = tempShape->AllocTransformInverse(*this);
                    }
                else
                    {
                    // Only the given model has a domain
                    // The inverse domain will be the model inverse domain while we
                    // must compute the direct domain by passing through the list of existing 
                    // transformation models in the list.
                    // Here we use the current complex model taking advantage of the fact it has not been modified yet.
                    m_inverseDomain = pModelCopy->GetInverseDomain(); 
                    m_directDomain = pModelCopy->GetDirectDomain()->AllocTransformInverse(*this);
                    }
                }
            else
                {
                // If we get here then the complex transformation model (self) must
                // already have a domain.
                HASSERT(HasDomain());

                // The direct domain remains the same yet the inverse domain must be modified
                // to be expressing in post-transformation coordinates.
                m_inverseDomain = m_inverseDomain->AllocTransformDirect(*pModelCopy);
                }
            }

        }

    // Add this pointer to the list
    m_ListOfModels.push_back (pModelCopy);
    }


/** -----------------------------------------------------------------------------
    PRIAVTE
    This method adds a copy of the given transformation model at the beginning of the
    complex transformation model chain. 
    Domain is composed or copied and transformed if required

    @param pi_rModelToAdd IN Constant reference to a transformation model to
                             add to front of complex..
    -----------------------------------------------------------------------------
*/
void  HGF2DComplexTransfoModel::AddFrontModel (const HGF2DTransfoModel& pi_rModelToAdd)
    {

    // Allocate dynamically a copy of the model
    HGF2DTransfoModel* pModelCopy = pi_rModelToAdd.Clone();

    // If this is first model added ... set direct units
    if (m_ListOfModels.size() <= 0)
        {
        // This is the first model
        // Check if it has a domain defined
        if (pModelCopy->HasDomain())
            {
            // Transformation has domain ... we get these domains
            // Note that we do not copy the shapes as these are
            // shared objects and belong to a transformation model
            // copy owned by the present object.
            m_directDomain = pModelCopy->GetDirectDomain();
            m_inverseDomain = pModelCopy->GetInverseDomain();
            }
        }
    else
        {
        // We check if either transformation model to add or current 
        // state of the complex have domains defined.
        if (pModelCopy->HasDomain() || HasDomain())
            {
            // At least one has a domain ... the result will have a domain
            // Check if model to add has a domain
            if (pModelCopy->HasDomain())
                {
                // Given model has a domain ... Check if complex already has one
                if (HasDomain())
                    {
                    // Both have domains. We must compose both domains together
                    // First we intersect the inverse domain of complex and direct domain of added.
                    HFCPtr<HGF2DShape> tempShape = GetDirectDomain()->IntersectShape(*(pModelCopy->GetInverseDomain()));

                    m_directDomain = tempShape->AllocTransformInverse(*pModelCopy);
                    m_inverseDomain = tempShape->AllocTransformDirect(*this);
                    }
                else
                    {
                    // Only the given model has a domain
                    // The direct domain will be the added model direct domain while we
                    // must compute the inverse domain by passing through the list of existing 
                    // transformation models in the list.
                    // Here we use the current complex model taking advantage of the fact it has not been modified yet.
                    m_directDomain = pModelCopy->GetDirectDomain(); 
                    m_inverseDomain = pModelCopy->GetInverseDomain()->AllocTransformDirect(*this);
                    }
                }
            else
                {
                // If we get here then the complex transformation model (self) must
                // already have a domain.
                HASSERT(HasDomain());

                // The inverse domain remains the same yet the direct domain must be modified
                // to be expressing in post-transformation coordinates.
                m_directDomain = m_directDomain->AllocTransformInverse(*pModelCopy);
                }
            }

        }

    // Add this pointer to the list
    m_ListOfModels.push_front (pModelCopy);
    }


/** -----------------------------------------------------------------------------
    This method returns the number of models in the complex

    @return The number of models in the complex model
    ----------------------------------------------------------------------------- 
*/
size_t  HGF2DComplexTransfoModel::GetNumberOfModels () const
    {
    return m_ListOfModels.size();
    }

/** -----------------------------------------------------------------------------
    This method returns a pointer to the internal model in the complex. 
    The returned transformation model shall not be modified nor deallocated.
    The model returned shall not be used unless the complex transformation model
    is maintained in memory.
    Note that if the complex transformation model has a domain, the component
    transformation model should not have any domain set unless the initial component 
    added had a domain.

    @return The indicated transformation model
    ----------------------------------------------------------------------------- 
*/
HGF2DTransfoModel* HGF2DComplexTransfoModel::GetModel (size_t modelNumber) 
    {
    HGF2DTransfoModel* result = NULL;

    List_TransfoModel::iterator itr = m_ListOfModels.begin();
    size_t currentModelIndex = 0;
    while (itr != m_ListOfModels.end() && currentModelIndex < modelNumber)
        {
        itr++;
        currentModelIndex++;
        }

    if (itr != m_ListOfModels.end())
        result = *itr;

    return result;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertDirect(double* pio_pXInOut,
                                                  double* pio_pYInOut) const
    {
    // Make sure recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    StatusInt status = SUCCESS;

    double NewX;
    double NewY;

    // Iterate if the model is complex
    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.end())); ++Itr)
        {
        status = (*Itr)->ConvertDirect (*pio_pXInOut, *pio_pYInOut, &NewX, &NewY);

        // Copy result
        *pio_pXInOut = NewX;
        *pio_pYInOut = NewY;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertDirect (double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, 
                                                   double* po_aXOut, double* po_aYOut) const
    {
    StatusInt status = SUCCESS;

    // Iterate if the model is complex
    List_TransfoModel::const_iterator Itr;

    for (Itr = m_ListOfModels.begin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.end())); ++Itr)
        {
        if (Itr == m_ListOfModels.begin())
            {
            status = (*Itr)->ConvertDirect (pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_aXOut, po_aYOut);
            }
        else
            {
            status = (*Itr)->ConvertDirect(pi_NumLoc, po_aXOut, po_aYOut);
            }
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
        *po_pXOut = pi_XIn;
        *po_pYOut = pi_YIn;
        return ConvertDirect(po_pXOut, po_pYOut);
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    StatusInt status = SUCCESS;

    // Iterate if the model is complex
    List_TransfoModel::const_iterator Itr;

    for (Itr = m_ListOfModels.begin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.end())); ++Itr)
        {
        status = (*Itr)->ConvertDirect(pi_NumLoc, pio_aXInOut, pio_aYInOut);
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertInverse(double* pio_pXInOut,
                                                     double* pio_pYInOut) const
    {
    // Make sure the recipient variables are provided
    HPRECONDITION(pio_pXInOut != 0);
    HPRECONDITION(pio_pYInOut != 0);

    StatusInt status = SUCCESS;
    double NewX;
    double NewY;

    // There are at least one model...
    // For each call direct transformation in reverse order
    List_TransfoModel::const_reverse_iterator Itr;
    for (Itr = m_ListOfModels.rbegin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.rend())); ++Itr)
        {
        status = (*Itr)->ConvertInverse (*pio_pXInOut, *pio_pYInOut, &NewX, &NewY);

        // Copy result
        *pio_pXInOut = NewX;
        *pio_pYInOut = NewY;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertInverse (double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, 
                                                    double* po_aXOut, double* po_aYOut) const
    {
    StatusInt status = SUCCESS;

    // Iterate if the model is complex
    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.end())); ++Itr)
        {
        if (Itr == m_ListOfModels.begin())
            {
            status = (*Itr)->ConvertInverse (pi_YIn, pi_XInStart, pi_NumLoc, pi_XInStep, po_aXOut, po_aYOut);
            }
        else
            {
            status = (*Itr)->ConvertInverse(pi_NumLoc, po_aXOut, po_aYOut);
            }
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
    {
        *po_pXOut = pi_XIn;
        *po_pYOut = pi_YIn;
        return ConvertInverse(po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DComplexTransfoModel::ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
    {
    StatusInt status = SUCCESS;

    // Iterate if the model is complex
    List_TransfoModel::const_iterator Itr;

    for (Itr = m_ListOfModels.begin(); ((SUCCESS == status || 1 == status/*&&AR*/) && (Itr != m_ListOfModels.end())); ++Itr)
        {
        status = (*Itr)->ConvertInverse(pi_NumLoc, pio_aXInOut, pio_aYInOut);
        }

    return status;
    }

//-----------------------------------------------------------------------------
// IsIdentity
//-----------------------------------------------------------------------------
inline bool   HGF2DComplexTransfoModel::IsIdentity () const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->IsIdentity();
        }

    return (ReturnValue);
    }


//-----------------------------------------------------------------------------
// IsStretchable
// This askes all the component model if they are stretchable. All must
// answer affirmative for the result to be true
//-----------------------------------------------------------------------------
inline bool   HGF2DComplexTransfoModel::IsStretchable (double pi_AngleTolerance) const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->IsStretchable(pi_AngleTolerance);
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// GetStretchParams
//-----------------------------------------------------------------------------
inline void    HGF2DComplexTransfoModel::GetStretchParams (double*  po_pScaleFactorX,
                                                           double*  po_pScaleFactorY,
                                                           HGF2DDisplacement* po_pDisplacement) const
    {
    HPRECONDITION(po_pScaleFactorX != 0);
    HPRECONDITION(po_pScaleFactorY != 0);
    HPRECONDITION(po_pDisplacement != 0);

    double             ScaleX;
    double             ScaleY;
    HGF2DDisplacement   Translation;

    // Init stretch parameters
    *po_pScaleFactorX = 1.0;
    *po_pScaleFactorY = 1.0;
    *po_pDisplacement = HGF2DDisplacement();

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); (Itr != m_ListOfModels.end()); ++Itr)
        {
        // Convert
        (*Itr)->GetStretchParams (&ScaleX, &ScaleY, &Translation);

        // Add stretch parameters together
        *po_pScaleFactorX *= ScaleX;
        *po_pScaleFactorY *= ScaleY;
        *po_pDisplacement += Translation;
        }
    }


//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DComplexTransfoModel::Reverse()
    {
    // Create copy of list of models
    List_TransfoModel  TempListOfModels (m_ListOfModels);

    // Clear present list
    m_ListOfModels.clear();

    List_TransfoModel::reverse_iterator rItr;

    // There are at least one model...
    // For each append to list in reverse order
    for (rItr = TempListOfModels.rbegin(); (rItr != TempListOfModels.rend()); ++rItr)
        {
        // Set the units of both channels (direct and inverse) to those of previewed
        // Input (direct) units
        // Copy ref to model
        m_ListOfModels.push_back((*rItr));
        }

    List_TransfoModel::iterator Itr;
    // For each model, reverse model
    for (Itr = m_ListOfModels.begin(); (Itr != m_ListOfModels.end()); ++Itr)
        {
        // Copy ref to model
        (*Itr)->Reverse ();
        }

    // Domain mamangement 
    if (HasDomain())
        {
        std::swap(m_directDomain, m_inverseDomain);
        }

    // Invoque reverse of ancester, wich will reverse the inverse and direct units
    HGF2DTransfoModel::Reverse();

    }

//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DComplexTransfoModel::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID     TheModelType  = pi_rModel.GetClassID();

    // The only models known are NEUTRAL and COMPLEX
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Model is neutral ... return copy of self
        pResultModel = new HGF2DComplexTransfoModel (*this);

        }
    else if (TheModelType == HGF2DComplexTransfoModel::CLASS_ID)
        {
        // Other model is also a complex ... we create a duplicate of this
        HGF2DComplexTransfoModel* pNewModel = new HGF2DComplexTransfoModel (*this);

        // Cast to a complex
        HGF2DComplexTransfoModel* pCmplxModel = (HGF2DComplexTransfoModel*)&pi_rModel;

        // For each model in given ... we add
        List_TransfoModel::iterator Itr;
        for (Itr = pCmplxModel->m_ListOfModels.begin(); (Itr != pCmplxModel->m_ListOfModels.end()); ++Itr)
            {
            pNewModel->AddModel (*(*Itr));
            }

        // Assign to smart pointer
        pResultModel = pNewModel;
        }
    else
        {
        // Model unknown add it at the end of copy
        // Make copy
        HGF2DComplexTransfoModel* pNewModel = new HGF2DComplexTransfoModel (*this);

        // Add model
        pNewModel->AddModel(pi_rModel);

        pResultModel = pNewModel;
        }

    return (pResultModel);
    }





//-----------------------------------------------------------------------------
// ComposeYourself
// This method is called for self when the given as failed to compose. It is a last
// resort, and will not call back the given transformation model. If self does not
// know the type of given, a complex transformation model is constructed and
// returned.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DComplexTransfoModel::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if the type of the given model is known
    HCLASS_ID TheModelType  = pi_rModel.GetClassID();

    // The only models known are NEUTRAL and complex but complex would have been processed
    if (TheModelType == HGF2DIdentity::CLASS_ID)
        {
        // Type is not known ... build a complex
        // Allocate new complex transformation model
        HGF2DComplexTransfoModel* pMyComplex = new HGF2DComplexTransfoModel (*this);


        pResultModel = pMyComplex;
        }
    else
        {
        // Unknown model ... add it at the beginning of current complex copy

        // Allocate new complex transformation model
        HGF2DComplexTransfoModel* pMyComplex = new HGF2DComplexTransfoModel (*this);

        // This is the same proceduire as a AddModel() except we add at the front
        pMyComplex->AddFrontModel(pi_rModel);

        pResultModel = pMyComplex;
        }

    return (pResultModel);

    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DComplexTransfoModel::Clone() const
    {
    return (new HGF2DComplexTransfoModel (*this));
    }


//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DComplexTransfoModel::Copy(const HGF2DComplexTransfoModel& pi_rObj)
    {
    List_TransfoModel::const_iterator Itr;
    for (Itr = pi_rObj.m_ListOfModels.begin(); (Itr != pi_rObj.m_ListOfModels.end()); ++Itr)
        {
        // Convert
        m_ListOfModels.push_back ((*Itr)->Clone());
        }


    m_hasDomain = pi_rObj.m_hasDomain;
    if (m_hasDomain)
        {
        m_directDomain = static_cast<HGF2DShape*>(pi_rObj.m_directDomain->Clone());
        m_inverseDomain = static_cast<HGF2DShape*>(pi_rObj.m_inverseDomain->Clone());
        }
    }

//-----------------------------------------------------------------------------
//  Prepare
//  Copy method
//-----------------------------------------------------------------------------
void HGF2DComplexTransfoModel::Prepare()
    {
    // This method is only called for a change of units
    // It insures that the units are matched one to another for the
    // transformation models included in the complex model

    // Check if there are models in complex
    if (m_ListOfModels.size() > 0)
        {
        // Check that the input units of first model are identical to those of the complex model
        List_TransfoModel::iterator Itr = m_ListOfModels.begin();


        // Check that the inverse units of last model are identical to those of complex model
        List_TransfoModel::reverse_iterator rItr = m_ListOfModels.rbegin();
        }
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DComplexTransfoModel::PreservesLinearity () const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->PreservesLinearity();
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DComplexTransfoModel::PreservesParallelism() const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->PreservesParallelism();
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DComplexTransfoModel::PreservesShape() const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->PreservesShape();
        }

    return (ReturnValue);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DComplexTransfoModel::PreservesDirection() const
    {
    bool   ReturnValue = true;

    List_TransfoModel::const_iterator Itr;
    for (Itr = m_ListOfModels.begin(); ReturnValue && (Itr != m_ListOfModels.end()); ++Itr)
        {
        // There are at least one model...
        ReturnValue = (*Itr)->PreservesDirection();
        }

    return (ReturnValue);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DComplexTransfoModel::CanBeRepresentedByAMatrix() const
    {
    return false;
    }


//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the affine by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DComplexTransfoModel::GetMatrix() const
    {
    // A complex transformation model cannot be represented as a matrix
    HASSERT(0);

    return(HFCMatrix<3, 3>());
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the affine by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3>& HGF2DComplexTransfoModel::GetMatrix(HFCMatrix<3, 3>& po_rRecipient) const
    {
    // A complex transformation model cannot be represented as a matrix
    HASSERT(0);

    return(po_rRecipient);
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DComplexTransfoModel::CreateSimplifiedModel() const
    {
    HFCPtr<HGF2DTransfoModel> pResult;

    // It would be next to impossible to create a simplified domain given 
    // we have a domain that implies we bear a complicated model within.
    if (HasDomain())
        return pResult;

    if (m_ListOfModels.empty())
        {
        // A complex should never be empty, but just in case...
        pResult = new HGF2DIdentity();
        }
    else
        {
        if (m_ListOfModels.size() == 1)
            {
            // Should never happen, but if it does, simply return
            // the single model inside.
            pResult = m_ListOfModels.front()->CreateSimplifiedModel();
            if (pResult == 0)
                pResult = m_ListOfModels.front()->Clone();
            }
        else
            {
            bool AtLeastOneSimplified = false;

            List_TransfoModel::const_iterator ModelItr(m_ListOfModels.begin());
            while (ModelItr != m_ListOfModels.end())
                {
                // Try to simplify the current model
                HFCPtr<HGF2DTransfoModel> pSimpleModel((*ModelItr)->CreateSimplifiedModel());

                if (!AtLeastOneSimplified)
                    {
                    if (pSimpleModel != 0)
                        {
                        // We've found the first model that can be simplified.
                        AtLeastOneSimplified = true;

                        // Start from the beginning and compose everything up to here
                        List_TransfoModel::const_iterator TempItr(m_ListOfModels.begin());
                        while (TempItr != ModelItr)
                            {
                            if (pResult == 0)
                                pResult = (*TempItr)->Clone();
                            else
                                pResult = pResult->ComposeInverseWithDirectOf(**TempItr);

                            ++TempItr;
                            }

                        // Compose result with current simplified model
                        if (pResult == 0)
                            pResult = pSimpleModel;
                        else
                            pResult = pResult->ComposeInverseWithDirectOf(*pSimpleModel);
                        }
                    }
                else
                    {
                    // We already started to simplify. Continue to compose...

                    if (pSimpleModel != 0)
                        {
                        pResult = pResult->ComposeInverseWithDirectOf(*pSimpleModel);
                        }
                    else
                        {
                        pResult = pResult->ComposeInverseWithDirectOf(**ModelItr);
                        }
                    }

                ++ModelItr;
                }
            }
        }

    return pResult;
    }




/** -----------------------------------------------------------------------------
    @bsimethod                                         Alain Robert 2014/06
    -----------------------------------------------------------------------------
*/
bool  HGF2DComplexTransfoModel::HasDomain() const
    {
    return m_hasDomain;
    }

/** -----------------------------------------------------------------------------
    @bsimethod                                         Alain Robert 2014/06
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DShape>  HGF2DComplexTransfoModel::GetDirectDomain() const
    {
    // Default implementation has no domain implying there is no limit
    if (m_hasDomain)
        return m_directDomain;
    else
        return new HGF2DUniverse();
    }


/** -----------------------------------------------------------------------------
    @bsimethod                                         Alain Robert 2014/06
    -----------------------------------------------------------------------------
*/
HFCPtr<HGF2DShape>  HGF2DComplexTransfoModel::GetInverseDomain() const
    {
    // Default implementation has no domain implying there is no limit
    if (m_hasDomain)
        return m_inverseDomain;
    else
        return new HGF2DUniverse();
    }

bool HGF2DComplexTransfoModel::IsConvertDirectThreadSafe() const
    {
    bool isThreadSafe = true;
    for (auto itr = m_ListOfModels.begin(); (isThreadSafe && itr != m_ListOfModels.end()); ++itr)
        {
        isThreadSafe = (*itr)->IsConvertDirectThreadSafe();
        }
    return isThreadSafe;
    }

bool HGF2DComplexTransfoModel::IsConvertInverseThreadSafe() const
    {
    bool isThreadSafe = true;
    for (auto itr = m_ListOfModels.begin(); (isThreadSafe && itr != m_ListOfModels.end()); ++itr)
        {
        isThreadSafe = (*itr)->IsConvertInverseThreadSafe();
        }
    return isThreadSafe;
    }

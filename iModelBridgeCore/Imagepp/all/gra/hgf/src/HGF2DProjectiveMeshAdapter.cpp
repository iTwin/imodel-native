//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DProjectiveMeshAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DProjectiveMeshAdapter
//-----------------------------------------------------------------------------

#include "hstdcpp.h"    // must be first for PreCompiledHeader Option

#include "HGF2DIdentity.h"

#include "HGF2DProjectiveMeshAdapter.h"
#include "HGFMatrixOps.h"
#include "HGF2DComplexTransfoModel.h"
#include "HGF2DAffine.h"
#include "HGF2DProjective.h"
#include "HVE2DGenFacet.h"
#include "MatrixFromTiePts.h"
#include "HVE2DVoidShape.h"


//-----------------------------------------------------------------------------
// Default Constructor
// Protected
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::HGF2DProjectiveMeshAdapter()
    : HGF2DTransfoModelAdapter()
    {
    m_pPreTransfoModel = new HGF2DIdentity();
    m_pPostTransfoModel = new HGF2DIdentity();
    m_AdaptAsAffine = false;
    m_pBaseMesh = 0;
    m_Reversed = false;
    m_ModelsCreated = false;
    m_NumConverted = 0;
    m_LastInverseFacetInitialized = false;
    m_LastDirectFacetInitialized = false;
    m_LastInverseHit = 0;
    m_LastDirectHit = 0;
    m_LastDirectModelPresent = false;
    m_LastInverseModelPresent = false;
    m_LastTransformedDirectFacetPresent = false;
    m_LastTransformedInverseFacetPresent = false;

    }

//-----------------------------------------------------------------------------
// Normal constructor.  It adapts the given mesh based model
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& i_rMeshBasedTransfoModel)
    : HGF2DTransfoModelAdapter(i_rMeshBasedTransfoModel)
    {
    m_pPreTransfoModel = new HGF2DIdentity();
    m_pPostTransfoModel = new HGF2DIdentity();
//    m_pBaseMesh = i_rMeshBasedTransfoModel.GetMesh();
    m_pBaseMesh = (static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel()))->GetMesh());
    m_AdaptAsAffine = false;
    m_Reversed = false;
    m_ModelsCreated = false;
    m_NumConverted = 0;
    m_LastInverseFacetInitialized = false;
    m_LastDirectFacetInitialized = false;
    m_LastInverseHit = 0;
    m_LastDirectHit = 0;
    m_LastDirectModelPresent = false;
    m_LastInverseModelPresent = false;
    m_LastTransformedDirectFacetPresent = false;
    m_LastTransformedInverseFacetPresent = false;


    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// PRIVATE CONSTRUCTOR
// Normal constructor.  It adapts the given mesh based model with specified
// pre and post models which must be linear
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& i_rMeshBasedTransfoModel,
                                                       const HGF2DTransfoModel& i_rPreTransfo,
                                                       const HGF2DTransfoModel& i_rPostTransfo,
                                                       bool                     i_AdaptAsAffine)
    : HGF2DTransfoModelAdapter(i_rMeshBasedTransfoModel)
    {
    m_pPreTransfoModel = i_rPreTransfo.Clone();
    m_pPostTransfoModel = i_rPostTransfo.Clone();
    m_pBaseMesh = static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel()))->GetMesh();
//    m_pBaseMesh = i_rMeshBasedTransfoModel.GetMesh();
    m_AdaptAsAffine = i_AdaptAsAffine;
    m_Reversed = false;
    m_ModelsCreated = false;
    m_NumConverted = 0;
    m_LastInverseFacetInitialized = false;
    m_LastDirectFacetInitialized = false;
    m_LastInverseHit = 0;
    m_LastDirectHit = 0;
    m_LastDirectModelPresent = false;
    m_LastInverseModelPresent = false;
    m_LastTransformedDirectFacetPresent = false;
    m_LastTransformedInverseFacetPresent = false;



    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// PRIVATE CONSTRUCTOR
// Normal constructor.  It adapts the given mesh based model with specified
// pre and post models which must be linear
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& i_rMeshBasedTransfoModel,
                                                       const HGF2DTransfoModel& i_rPreTransfo,
                                                       const HGF2DTransfoModel& i_rPostTransfo,
                                                       bool                     i_AdaptAsAffine,
                                                       bool                     i_ModelsCreated,
                                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_DirectModels,
                                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_InverseModels,
                                                       const HGF2DTransfoModel& i_rDefaultModel,
                                                       bool                      i_Reversed)

    : HGF2DTransfoModelAdapter(i_rMeshBasedTransfoModel)
    {
    m_pPreTransfoModel = i_rPreTransfo.Clone();
    m_pPostTransfoModel = i_rPostTransfo.Clone();
    m_pBaseMesh = static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel()))->GetMesh();
//    m_pBaseMesh = i_rMeshBasedTransfoModel.GetMesh();
    m_AdaptAsAffine = i_AdaptAsAffine;
    m_Reversed = i_Reversed;
    m_ModelsCreated = i_ModelsCreated;
    m_NumConverted = 0;
    m_pMeshOfDirectModels = i_DirectModels;
    m_pMeshOfInverseModels = i_InverseModels;
    m_pDefaultModel = i_rDefaultModel.Clone();
    m_LastInverseFacetInitialized = false;
    m_LastDirectFacetInitialized = false;
    m_LastInverseHit = 0;
    m_LastDirectHit = 0;
    m_LastDirectModelPresent = false;
    m_LastInverseModelPresent = false;
    m_LastTransformedDirectFacetPresent = false;
    m_LastTransformedInverseFacetPresent = false;




    HINVARIANTS;
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::HGF2DProjectiveMeshAdapter(const HGF2DProjectiveMeshAdapter& i_rObj)
    : HGF2DTransfoModelAdapter(i_rObj)
    {
    // Force given to create models
    if (!i_rObj.m_ModelsCreated)
        i_rObj.CreateModels();

    Copy(i_rObj);

    m_LastInverseFacetInitialized = false;
    m_LastDirectFacetInitialized = false;
    m_LastInverseHit = 0;
    m_LastDirectHit = 0;
    m_LastDirectModelPresent = false;
    m_LastInverseModelPresent = false;
    m_LastTransformedDirectFacetPresent = false;
    m_LastTransformedInverseFacetPresent = false;



    if (!m_ModelsCreated)
        CreateModels();

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter::~HGF2DProjectiveMeshAdapter()
    {
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjectiveMeshAdapter& HGF2DProjectiveMeshAdapter::operator=(const HGF2DProjectiveMeshAdapter& i_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &i_rObj)
        {
        // Force given to create models
        if (!i_rObj.m_ModelsCreated)
            i_rObj.CreateModels();

        // Call ancestor operator=
        HGF2DTransfoModelAdapter::operator=(i_rObj);
        Copy(i_rObj);

        m_LastInverseFacetInitialized = false;
        m_LastDirectFacetInitialized = false;
        m_LastInverseHit = 0;
        m_LastDirectHit = 0;
        m_LastDirectModelPresent = false;
        m_LastInverseModelPresent = false;
        m_LastTransformedDirectFacetPresent = false;
        m_LastTransformedInverseFacetPresent = false;

        if (!m_ModelsCreated)
            CreateModels();
        }

    // Return reference to self
    return (*this);
    }



//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertDirect(double* pio_pXInOut,
                                               double* pio_pYInOut) const
    {
    HINVARIANTS;

    // Variables must be provided
    HPRECONDITION(pio_pXInOut != NULL);
    HPRECONDITION(pio_pYInOut != NULL);

    if (!m_ModelsCreated)
        {
        CreateModels();
        }

#if (1)
    // EQUIVALENT TO
    //    m_pPreTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
    //    m_pAdaptedTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
    //    m_pPostTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    m_pPreTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);

    // Obtain the facet
    HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > pFacet =
        static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(&(*m_pMeshOfDirectModels->GetFacetAt(HGF2DPosition(*pio_pXInOut, *pio_pYInOut))));


    // Obtain appropriate model
    HFCPtr<HGF2DTransfoModel> pModel = (pFacet == 0 ? m_pDefaultModel : pFacet->GetAttribute());


    if (m_LastDirectFacetInitialized && pFacet == m_pLastDirectFacet)
        {
        ++m_LastDirectHit;

        if (m_LastDirectHit == 10)
            {
            // Create last inverse model
            if (m_Reversed)
                m_pLastDirectModel = m_pPreTransfoModel->ComposeInverseWithInverseOf(*pModel);
            else
                m_pLastDirectModel = m_pPreTransfoModel->ComposeInverseWithDirectOf(*pModel);

            m_pLastDirectModel = m_pLastDirectModel->ComposeInverseWithDirectOf(*m_pPostTransfoModel);

            // Indicate that a model is present
            m_LastDirectModelPresent = true;


            m_LastTransformedDirectFacetPresent = (pFacet != 0);

            // Check if it is default model used
            if (pFacet != 0)
                {
                // Obtain matrix of pre-model
                HFCMatrix<3, 3> PreMatrix = m_pPreTransfoModel->GetMatrix();

                // Obtain a transformed facet
                m_pLastTransformedDirectFacet = static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(pFacet->AllocateTransformed(PreMatrix));

                // A facet must have been allocated
                HASSERT(m_pLastTransformedDirectFacet != 0);
                }

            }
        }
    else
        {
        // Save last facet
        m_pLastDirectFacet = pFacet;
        m_LastDirectHit = 0;
        m_LastDirectFacetInitialized = true;
        m_LastTransformedDirectFacetPresent=false;
        m_LastDirectModelPresent = false;
        }

    if (m_LastDirectFacetInitialized && (pFacet == m_pLastDirectFacet) && (m_LastDirectModelPresent))
        {
        // Use the last model
        *pio_pXInOut = X;
        *pio_pYInOut = Y;
        m_pLastDirectModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        if (m_Reversed)
            {
            // Convert coordinate
            pModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
            }
        else
            {
            // Convert coordinate
            pModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
            }

        m_pPostTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
#else
    m_pPreTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
    if (m_Reversed)
        {
        // Convert coordinate
        m_pDefaultModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        // Convert coordinate
        m_pDefaultModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
    m_pPostTransfoModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
#endif
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertDirect(double pi_XIn,
                                               double pi_YIn,
                                               double* po_pXOut,
                                               double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);


    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    ConvertDirect(po_pXOut, po_pYOut);

    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertDirect(double    pi_YIn,
                                               double    pi_XInStart,
                                               uint32_t   pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
#if (0)
        if (m_LastTransformedDirectFacetPresent &&
            m_pLastTransformedDirectFacet->IsPointIn(HGF2DPosition(X, pi_YIn)))
            {
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            m_pLastDirectModel->ConvertDirect(pCurrentX, pCurrentY);
            }
        else
            {
#endif
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            ConvertDirect(pCurrentX, pCurrentY);
#if (0)
            }
#endif
        }
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertDirect(uint32_t  pi_NumLoc,
                                               double*   pio_aXInOut,
                                               double*   pio_aYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_aXInOut != NULL);
    HPRECONDITION(pio_aYInOut != NULL);

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        ConvertDirect(pio_aXInOut + i, pio_aYInOut + i);
        }
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertInverse(double* pio_pXInOut,
                                                double* pio_pYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_pXInOut != NULL);
    HPRECONDITION(pio_pYInOut != NULL);

    if (!m_ModelsCreated)
        {
        CreateModels();
        }

#if (1)
    // EQUIVALENT TO
    //     m_pPostTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
    //     m_pAdaptedTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
    //     m_pPreTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;
    m_pPostTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);

    // Obtain the facet
    HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > pFacet =
        static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(&(*m_pMeshOfInverseModels->GetFacetAt(HGF2DPosition(
                                                                    *pio_pXInOut,
                                                                    *pio_pYInOut))));


    // Obtain appropriate model
    HFCPtr<HGF2DTransfoModel> pModel = (pFacet == 0 ? m_pDefaultModel : pFacet->GetAttribute());


    if (m_LastInverseFacetInitialized && pFacet == m_pLastInverseFacet)
        {
        ++m_LastInverseHit;

        if (m_LastInverseHit == 10)
            {
            // Create last inverse model
            if (m_Reversed)
                m_pLastInverseModel = m_pPreTransfoModel->ComposeInverseWithInverseOf(*pModel);
            else
                m_pLastInverseModel = m_pPreTransfoModel->ComposeInverseWithDirectOf(*pModel);

            m_pLastInverseModel = m_pLastInverseModel->ComposeInverseWithDirectOf(*m_pPostTransfoModel);

            // Indicate that a model is present
            m_LastInverseModelPresent = true;

            m_LastTransformedInverseFacetPresent = (pFacet != 0);

            // Check if it is default model used
            if (pFacet != 0)
                {
                // Obtain matrix of pre-model
                HFCPtr<HGF2DTransfoModel> pTempPostModel = m_pPostTransfoModel->Clone();
                pTempPostModel->Reverse();
                HFCMatrix<3, 3> PreMatrix = pTempPostModel->GetMatrix();

                // Obtain a transformed facet
                m_pLastTransformedInverseFacet = static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(pFacet->AllocateTransformed(PreMatrix));

                // A facet must have been allocated
                HASSERT(m_pLastTransformedInverseFacet != 0);
                }
            }
        }
    else
        {
        // Save last facet
        m_pLastInverseFacet = pFacet;
        m_LastInverseHit = 0;
        m_LastInverseFacetInitialized = true;
        m_LastTransformedInverseFacetPresent=false;
        m_LastInverseModelPresent = false;
        }

    if (m_LastInverseFacetInitialized && (pFacet == m_pLastInverseFacet) && (m_LastInverseModelPresent))
        {
        // Use the last model
        *pio_pXInOut = X;
        *pio_pYInOut = Y;
        m_pLastInverseModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        if (m_Reversed)
            {
            // Convert coordinate
            pModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
            }
        else
            {
            // Convert coordinate
            pModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
            }

        m_pPreTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
#else
    m_pPostTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
    if (m_Reversed)
        {
        // Convert coordinate
        m_pDefaultModel->ConvertDirect(pio_pXInOut, pio_pYInOut);
        }
    else
        {
        // Convert coordinate
        m_pDefaultModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
        }
    m_pPreTransfoModel->ConvertInverse(pio_pXInOut, pio_pYInOut);
#endif
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertInverse(double pi_XIn,
                                                double pi_YIn,
                                                double* po_pXOut,
                                                double* po_pYOut) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    ConvertInverse(po_pXOut, po_pYOut);


    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertInverse(double    pi_YIn,
                                                double    pi_XInStart,
                                                uint32_t   pi_NumLoc,
                                                double    pi_XInStep,
                                                double*   po_pXOut,
                                                double*   po_pYOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
#if (0)
        if (m_LastTransformedInverseFacetPresent &&
            m_pLastTransformedInverseFacet->IsPointIn(HGF2DPosition(X, pi_YIn)))
            {
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            m_pLastInverseModel->ConvertInverse(pCurrentX, pCurrentY);
            }
        else
            {
#endif
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            ConvertInverse(pCurrentX, pCurrentY);
#if (0)
            }
#endif
        }
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::ConvertInverse(uint32_t  pi_NumLoc,
                                                double*   pio_aXInOut,
                                                double*   pio_aYInOut) const
    {
    HINVARIANTS;

    HPRECONDITION(pio_aXInOut != NULL);
    HPRECONDITION(pio_aYInOut != NULL);

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
        ConvertInverse(pio_aXInOut + i, pio_aYInOut + i);
        }
    }



//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveMeshAdapter::PreservesLinearity () const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveMeshAdapter::PreservesParallelism() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveMeshAdapter::PreservesShape() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HGF2DProjectiveMeshAdapter::PreservesDirection() const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HGF2DProjectiveMeshAdapter::CanBeRepresentedByAMatrix() const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HGF2DProjectiveMeshAdapter::IsIdentity() const
    {
    HINVARIANTS;

    return(false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HGF2DProjectiveMeshAdapter::IsStretchable(double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return(false);
    }


//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::GetStretchParams(double* po_pScaleFactorX,
                                                  double* po_pScaleFactorY,
                                                  HGF2DDisplacement* po_pDisplacement) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pScaleFactorX != NULL);
    HPRECONDITION(po_pScaleFactorY != NULL);
    HPRECONDITION(po_pDisplacement != NULL);


    if (!m_ModelsCreated)
        {
        CreateModels();
        }


    HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > pFacet;

    // Check if reversed
    if (m_Reversed)
        {
//        pFacet = static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(&(*m_pMeshOfInverseModels->GetFacet(0)));
//        pFacet->GetAttribute()->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
        HFCPtr<HGF2DTransfoModel> pTempModel = m_pDefaultModel->Clone();
        pTempModel->Reverse();
        pTempModel->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
        }
    else
        {
//        pFacet = static_cast<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > *>(&(*m_pMeshOfDirectModels->GetFacet(0)));
//        pFacet->GetAttribute()->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
        m_pDefaultModel->GetStretchParams(po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
        }

    }



//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HGF2DProjectiveMeshAdapter::GetMatrix() const
    {
    HINVARIANTS;

    // Should not be called
    HASSERT(0);

    HFCMatrix<3, 3> Matrix;

    return(Matrix);
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HGF2DProjectiveMeshAdapter::Reverse()
    {
    HINVARIANTS;

#if (0)
    m_Reversed = !m_Reversed;
#else
    HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > pSwapMesh = m_pMeshOfDirectModels;
    m_pMeshOfDirectModels = m_pMeshOfInverseModels;
    m_pMeshOfInverseModels = pSwapMesh;
    m_Reversed = !m_Reversed;

#if (0)
    if (m_ModelsCreated)
        {

        HFCPtr<HGF2DTransfoModel> pNewDefault = m_pDefaultModel->Clone();
        m_pDefaultModel = pNewDefault;
        m_pDefaultModel->Reverse();
        }
#endif
#endif

    // Swap pre and post transformation model
    HFCPtr<HGF2DTransfoModel> pDumb = m_pPreTransfoModel;
    m_pPreTransfoModel = m_pPostTransfoModel;
    m_pPostTransfoModel = pDumb;

    // Reverse these transfo models
    m_pPreTransfoModel->Reverse();
    m_pPostTransfoModel->Reverse();

    // This reverses the adapted model
    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModelAdapter::Reverse();

    HINVARIANTS;
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveMeshAdapter::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    if (!m_ModelsCreated)
        CreateModels();

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity())
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DProjectiveMeshAdapter(*this);

        }
    else
        {
        if (pi_rModel.PreservesLinearity())
            {
            HFCPtr<HGF2DTransfoModel> pNewPostModel = m_pPostTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

            pResultModel = new HGF2DProjectiveMeshAdapter(*GetAdaptedMeshBasedModel(),
                                                          *m_pPreTransfoModel,
                                                          *pNewPostModel,
                                                          m_AdaptAsAffine,
                                                          m_ModelsCreated,
                                                          m_pMeshOfDirectModels,
                                                          m_pMeshOfInverseModels,
                                                          *m_pDefaultModel,
                                                          m_Reversed);
            }
        else
            {
            // Model is not known ... ask other
            pResultModel = CallComposeOf(pi_rModel);
            }
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DProjectiveMeshAdapter::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return(new HGF2DProjectiveMeshAdapter(*this));
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveMeshAdapter::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    if (!m_ModelsCreated)
        CreateModels();

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsIdentity())
        {
        pResultModel = new HGF2DProjectiveMeshAdapter(*this);
        }
    // Check if the type of the given model can be represented by a matrix
    else if (pi_rModel.PreservesLinearity())
        {
#if (0)
        // Compute the new pre-model
        HFCMatrix<3, 3> MySelfMatrix(m_pPreTransfoModel->GetMatrix());

        HFCPtr<HGF2DTransfoModel> pResultPreModel;

        if (m_AdaptAsAffine)
            {
            // Compose the two matrix together
            pResultPreModel = new HGF2DAffine();
            }
        else
            {
            // Compose the two matrix together
            pResultPreModel = new HGF2DProjective();
            }


        HFCMatrix<3, 3> ResMatrix = MySelfMatrix * pi_rModel.GetMatrix();

        // The projection parameters must be null
        HASSERT(ResMatrix[2][0] == 0.0);
        HASSERT(ResMatrix[2][1] == 0.0);

        if (m_AdaptAsAffine)
            {
            ((HGF2DAffine*)(&(*pResultPreModel)))->SetByMatrixParameters(ResMatrix[0][2],
                                                                         ResMatrix[0][0],
                                                                         ResMatrix[0][1],
                                                                         ResMatrix[1][2],
                                                                         ResMatrix[1][0],
                                                                         ResMatrix[1][1]);
            }
        else
            {
            ((HGF2DProjective*)(&(*pResultPreModel)))->SetByMatrix(MySelfMatrix * pi_rModel.GetMatrix());
            }
#endif

        HFCPtr<HGF2DTransfoModel> pNewPreModel = pi_rModel.ComposeInverseWithDirectOf(*m_pPreTransfoModel);

        // Create the final result model
        pResultModel = new HGF2DProjectiveMeshAdapter(*GetAdaptedMeshBasedModel(),
                                                      *pNewPreModel,
                                                      *m_pPostTransfoModel,
                                                      m_AdaptAsAffine,
                                                      m_ModelsCreated,
                                                      m_pMeshOfDirectModels,
                                                      m_pMeshOfInverseModels,
                                                      *m_pDefaultModel,
                                                      m_Reversed);

        }
    else
        {
        // Type is not known ... build a complex
        // To do this we call the ancester ComposeYourself
        pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);
        }

    return (pResultModel);
    }




//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::Prepare()
    {
    // Obtain conversion ratio for direct X to inverse X units

    // Invoque preparation of ancester (required)
    HGF2DTransfoModelAdapter::Prepare();
    }



//-----------------------------------------------------------------------------
// PRIVATE
// Copy
// Copy method
//-----------------------------------------------------------------------------
void HGF2DProjectiveMeshAdapter::Copy(const HGF2DProjectiveMeshAdapter& pi_rObj)
    {
    // Copy data
    m_pPreTransfoModel = pi_rObj.m_pPreTransfoModel->Clone();
    m_pPostTransfoModel = pi_rObj.m_pPostTransfoModel->Clone();
    m_pBaseMesh = pi_rObj.m_pBaseMesh;
    m_AdaptAsAffine = pi_rObj.m_AdaptAsAffine;

    m_ModelsCreated = pi_rObj.m_ModelsCreated;
    m_pMeshOfDirectModels = pi_rObj.m_pMeshOfDirectModels;
    m_pMeshOfInverseModels = pi_rObj.m_pMeshOfInverseModels;
    m_pDefaultModel = pi_rObj.m_pDefaultModel;

    m_Reversed = pi_rObj.m_Reversed;

    m_NumConverted = 0;

//    CreateModels();
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HGF2DProjectiveMeshAdapter::CreateSimplifiedModel() const
    {
    HINVARIANTS;

    // Declare recipient variable
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel;

    // If we get here, no simplification is possible.
    return pSimplifiedModel;
    }



//-----------------------------------------------------------------------------
// PRIVATE
// CreateModels
//
// This method creates the linear models for each and every area part of the mesh
//-----------------------------------------------------------------------------
bool HGF2DProjectiveMeshAdapter::CreateModels() const
    {


    // From now the reversal is not applicable
    m_Reversed = false;
    m_NumConverted = 0;

    m_pBaseMesh = (static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel()))->GetMesh());

    // Create the default model first
    CreateDefaultModel();

#if (0)
    HFCPtr<HGF2DCoordSys> pPreModelCS = new HGF2DCoordSys(*m_pPreTransfoModel, m_pBaseMesh->GetCoordSys());
#else
    HFCPtr<HGF2DCoordSys> pPreModelCS = new HGF2DCoordSys(HGF2DIdentity(), m_pBaseMesh->GetCoordSys());
#endif

    // Create a coordinate system related to base mesh coordinate system
    // through the adapted model ... this system will only be used in the current
    // method
    // Create the direct mesh
    HFCPtr<HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> > > pGenMeshOfDirectModels = new HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> >(pPreModelCS);
    HFCPtr<HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> > > pGenMeshOfInverseModels = new HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> >(m_pBaseMesh->GetCoordSys());

    m_pMeshOfDirectModels = pGenMeshOfDirectModels;
    m_pMeshOfInverseModels = pGenMeshOfInverseModels;



// #ifndef _DEBUG

    // These lines are executed only when in release mode
    pGenMeshOfDirectModels->SetFacetValidation(false);

    pGenMeshOfInverseModels->SetFacetValidation(false);
// #endif


    // For every facet part of the mesh ...
    long NumberOfFacets = m_pBaseMesh->CountFacets();

    for (long i = 0 ; i < NumberOfFacets ; ++i)
        {
        // Obtain the facet
        HFCPtr<HVE2DRawFacet> pFacet = m_pBaseMesh->GetFacet(i);

        // Obtain the shape of the facet and create a copy
        // This copy will be essential later in the method
        HFCPtr<HVE2DShape> pShape = static_cast<HVE2DShape*>(pFacet->GetShape().Clone());

        // Set the shape coordinate system to the base mesh coordinate system
        pShape->SetCoordSys(m_pBaseMesh->GetCoordSys());

        // Transform shape into pre-model space
        HFCPtr<HVE2DShape> pNewShape = static_cast<HVE2DShape*>(pShape->AllocateCopyInCoordSys(pPreModelCS));

        // Drop the shape
        HGF2DLocationCollection ShapePoints;
        pNewShape->Drop(&ShapePoints, pNewShape->GetTolerance());

#if (0)


        // Remove points that are equal one to the other
        long i2;
        long j;
        for (i2 = 0 ; i2 < ShapePoints.size() - 1 ; ++i2)
            {
            for(j = i2+1 ; j < ShapePoints.size() ; ++ j)
                {
                // Check if points are equal
                if (ShapePoints[i2].IsEqualTo(ShapePoints[j]))
                    {
                    // since they are equal, we erase one of them
                    ShapePoints.erase(&(ShapePoints.at(j)));

                    j--;
                    }
                }

            }
#else
        // Removing double points accelerates operation
        ShapePoints.pop_back();
#endif

        // Add the centroid
        // This is because of a bug concerning model generation using tie points
        unsigned long k;
        double MeanX = 0.0;
        double MeanY = 0.0;

        for (k = 0 ; k < ShapePoints.size() ; ++k)
            {
            MeanX += ShapePoints[k].GetX();
            MeanY += ShapePoints[k].GetY();
            }

        MeanX /= ShapePoints.size();
        MeanY /= ShapePoints.size();

        ShapePoints.push_back(HGF2DLocation(MeanX, MeanY, pPreModelCS));

        // Calculate the number of tie points needed
        size_t NumberOfPoints = ShapePoints.size();

        // Allocate Tie Points (6 double per tie points pair)
        HArrayAutoPtr<double> pTiePoints(new double[NumberOfPoints * 6]);

        // For every points part of the shape ...
        double CurrentX;
        double CurrentY;
        double TempX;
        double TempY;
        unsigned short CurrentTiePointVal = 0;

        HGF2DLocationCollection::iterator ShapePointItr;
        for (ShapePointItr = ShapePoints.begin(); ShapePointItr != ShapePoints.end() ; ++ShapePointItr)
            {
            // Obtain coordinates
            CurrentX = ShapePointItr->GetX();
            CurrentY = ShapePointItr->GetY();

            // Add point to list of points.
            pTiePoints[CurrentTiePointVal] = CurrentX;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = CurrentY;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = 0.0;
            ++CurrentTiePointVal;


#if (0)
            // Convert point through pre/adapted/post model
            m_pPreTransfoModel->ConvertDirect(CurrentX, CurrentY, &TempX, &TempY);

            // Apply non-linear transformation
            m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

            // Apply the post-transformation
            m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);
#else
            m_pAdaptedTransfoModel->ConvertDirect(CurrentX, CurrentY, &TempX, &TempY);
#endif

            // Convert to the direct channel units
            // Since we want the matrix not to contain unit conversion

            pTiePoints[CurrentTiePointVal] = TempX;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = TempY;
            ++CurrentTiePointVal;
            pTiePoints[CurrentTiePointVal] = 0.0;
            ++CurrentTiePointVal;
            }

        // Compute the result projective matrix
        double MyFlatMatrix[4][4];

        if (m_AdaptAsAffine)
            {
            // Create matrix
            GetAffineTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, CurrentTiePointVal, pTiePoints);
            }
        else
            {
            // Create matrix
            GetProjectiveTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, CurrentTiePointVal, pTiePoints);
            }

        // Format result flat matrix as an HFCMatrix
        HFCMatrix<3, 3> MyMatrix;
        MyMatrix[0][0] = MyFlatMatrix[0][0];
        MyMatrix[0][1] = MyFlatMatrix[0][1];
        MyMatrix[0][2] = MyFlatMatrix[0][3];
        MyMatrix[1][0] = MyFlatMatrix[1][0];
        MyMatrix[1][1] = MyFlatMatrix[1][1];
        MyMatrix[1][2] = MyFlatMatrix[1][3];
        MyMatrix[2][0] = MyFlatMatrix[3][0];
        MyMatrix[2][1] = MyFlatMatrix[3][1];
        MyMatrix[2][2] = MyFlatMatrix[3][3];

        // Create result model pointer
        HFCPtr<HGF2DTransfoModel> pModel;

        // Create the projective or affine model to assign to new facet
        if (m_AdaptAsAffine)
            {
            // The projection parameters must be null
            HASSERT(MyMatrix[2][0] == 0.0);
            HASSERT(MyMatrix[2][1] == 0.0);

            // create model
            HFCPtr<HGF2DAffine> pAffine = new HGF2DAffine();
            pAffine->SetByMatrixParameters(MyMatrix[0][2],
                                           MyMatrix[0][0],
                                           MyMatrix[0][1],
                                           MyMatrix[1][2],
                                           MyMatrix[1][0],
                                           MyMatrix[1][1]);


            //    HFCPtr<HGF2DTransfoModel> pModel = new HGF2DAffine(MyMatrix);
            pModel = pAffine;
            }
        else
            {
            // create model
            HFCPtr<HGF2DProjective> pProjective = new HGF2DProjective();
            pProjective->SetByMatrix(MyMatrix);

            pModel = pProjective;
            }

        // Model adaptation validation
#ifdef _DEBUG
#if (0)
            {
            // The purpose of this section is to debug the projective generation
            // process
            HGF2DExtent ShapeExtent(pNewShape->GetExtent());

            double ShapeXMin = ShapeExtent.GetXMin();
            double ShapeYMin = ShapeExtent.GetYMin();
            double ShapeXMax = ShapeExtent.GetXMax();
            double ShapeYMax = ShapeExtent.GetYMax();

            double ShapeWidth = ShapeXMax - ShapeXMin;
            double ShapeHeight = ShapeYMax - ShapeYMin;
            double XStep = (ShapeWidth / 20.0);
            double YStep = (ShapeHeight / 20.0);

            double MyX;
            double MyY;

            // Convert in temporary variables
            double TempX;
            double TempY;

            double MaxError = 0.0;
            double StatSumX = 0.0;
            double StatSumY = 0.0;
            uint32_t StatNumSamples = 0;

            double TempX1;
            double TempY1;

            for (MyX = ShapeXMin ; MyX < ShapeXMax ; MyX += XStep)
                {
                for (MyY = ShapeYMin ; MyY < ShapeYMax ; MyY += YStep)
                    {
                    // Check if point is in shape
                    if (pNewShape->IsPointIn(HGF2DLocation(MyX, MyY, pPreModelCS)))
                        {
                        // the point is inside the shape


                        // Convert point through pre/adapted/post model
                        m_pPreTransfoModel->ConvertDirect(MyX, MyY, &TempX, &TempY);

                        // Apply non-linear transformation
                        m_pAdaptedTransfoModel->ConvertDirect(&TempX, &TempY);

                        // Apply the post-transformation
                        m_pPostTransfoModel->ConvertDirect(&TempX, &TempY);

                        // Convert to the direct channel units
                        // Since we want the matrix not to contain unit conversion

                        // Convert one way
                        pModel->ConvertDirect(MyX, MyY, &TempX1, &TempY1);


                        // Compute difference (drift)
                        double DeltaX = fabs(TempX - TempX1);
                        double DeltaY = fabs(TempY - TempY1);

                        // Add deltas
                        StatSumX += DeltaX;
                        StatSumY += DeltaY;
                        StatNumSamples++;

                        MaxError = MAX(MaxError, MAX(DeltaX, DeltaY));

                        }
                    }
                }

            if (MaxError > 0.00001)
                {
                MaxError = MaxError;
                }
            }

#endif
#endif



        // Create facet
        HFCPtr<HVE2DGenFacet<HFCPtr<HGF2DTransfoModel> > >pNewDirectFacet = new HVE2DGenFacet<HFCPtr<HGF2DTransfoModel> >(*pNewShape, pModel);

        // Add to direct model mesh
        pGenMeshOfDirectModels->AddFacet(pNewDirectFacet);


        //    HFCPtr<HGF2DCoordSys> pTempInverseSystem  = new HGF2DCoordSys(*GetAdaptedTransfoModel(), m_pBaseMesh->GetCoordSys());
        HFCPtr<HGF2DTransfoModel> pModelToto = pModel->Clone();

        pModelToto->Reverse();

        HFCPtr<HGF2DCoordSys> pTempInverseSystem  = new HGF2DCoordSys(*pModelToto, pPreModelCS);

        // Change the shape
#if (1)
        // TEMPORARY... WAITING IMMAGE++ FIX (version 27 ?)
        HFCPtr<HVE2DShape> pInverseShape;

        // Obtain the polysegment
        // Drop
        HGF2DLocationCollection TheShapePoints;
        pNewShape->Drop(&TheShapePoints, pNewShape->GetTolerance());

        HVE2DPolySegment NewPolySegment(TheShapePoints);


        HVE2DLinear* pMyPrimaryResultLinear = static_cast<HVE2DLinear*>(NewPolySegment.AllocateCopyInCoordSys(pTempInverseSystem));

        // Check if result linear is complex or polysegment
        HASSERT(pMyPrimaryResultLinear->IsABasicLinear());

        // A basic linear must be a polysegment
        HASSERT(((HVE2DBasicLinear*)(pMyPrimaryResultLinear))->GetBasicLinearType() == HVE2DPolySegment::CLASS_ID);

        // Cast into a polysegment
        HVE2DPolySegment* pMyPolySegment = static_cast<HVE2DPolySegment*>(pMyPrimaryResultLinear);

        pMyPolySegment->RemoveAutoContiguousNeedles(true);

        if (pMyPolySegment->GetSize() < 3)
            {
            // There are less than 3 segments ... empty shape
            pInverseShape = new HVE2DVoidShape(pTempInverseSystem);
            }
        else
            {
            pInverseShape = static_cast<HVE2DShape*>(pNewShape->AllocateCopyInCoordSys(pTempInverseSystem));
            }

        delete pMyPrimaryResultLinear;

#else
        HFCPtr<HVE2DShape> pInverseShape = static_cast<HVE2DShape*>(pNewShape->AllocateCopyInCoordSys(pTempInverseSystem));
#endif

        // Set the coordinate system of inverse shape to base mesh system
        pInverseShape->SetCoordSys(m_pBaseMesh->GetCoordSys());

//        HGF2DExtent MShapeExtent = pNewShape->GetExtent();
//        HGF2DExtent InverseShapeExtent = pInverseShape->GetExtent();


        // Add shape only if it exists
        if (pInverseShape->CalculateArea() != 0.0)
            {

            // Create inverse facet
            HFCPtr<HVE2DGenFacet<HFCPtr<HGF2DTransfoModel> > > pNewInverseFacet = new HVE2DGenFacet<HFCPtr<HGF2DTransfoModel> >(*pInverseShape, pModel);

            // Add to inverse model mesh
            pGenMeshOfInverseModels->AddFacet(pNewInverseFacet);
            }


        // Model reverse validation
#ifdef _DEBUG
#if (0)
            {
            // The purpose of this section is to debug the projective generation
            // process
            HGF2DExtent ShapeExtent(pNewShape->GetExtent());

            double ShapeXMin = ShapeExtent.GetXMin();
            double ShapeYMin = ShapeExtent.GetYMin();
            double ShapeXMax = ShapeExtent.GetXMax();
            double ShapeYMax = ShapeExtent.GetYMax();

            double ShapeWidth = ShapeXMax - ShapeXMin;
            double ShapeHeight = ShapeYMax - ShapeYMin;
            double XStep = (ShapeWidth / 20.0);
            double YStep = (ShapeHeight / 20.0);

            double MyX;
            double MyY;

            // Convert in temporary variables
            double TempX;
            double TempY;

            double MaxError = 0.0;
            double StatSumX = 0.0;
            double StatSumY = 0.0;
            uint32_t StatNumSamples = 0;

            double TempX1;
            double TempY1;

            for (MyX = ShapeXMin ; MyX < ShapeXMax ; MyX += XStep)
                {
                for (MyY = ShapeYMin ; MyY < ShapeYMax ; MyY += YStep)
                    {
                    // Check if point is in shape
                    if (pNewShape->IsPointIn(HGF2DLocation(MyX, MyY, pPreModelCS)))
                        {
                        // the point is inside the shape
                        pModel->ConvertDirect(MyX, MyY, &TempX, &TempY);

                        // Make sure that the coordinate is in the shape
                        HASSERT(pInverseShape->IsPointIn(HGF2DLocation(TempX, TempY, pInverseShape->GetCoordSys())));


                        pModelToto->ConvertDirect(TempX, TempY, &TempX1, &TempY1);

                        // Compute difference (drift)
                        double DeltaX = fabs(MyX - TempX1);
                        double DeltaY = fabs(MyY - TempY1);

                        // Add deltas
                        StatSumX += DeltaX;
                        StatSumY += DeltaY;
                        StatNumSamples++;

                        MaxError = MAX(MaxError, MAX(DeltaX, DeltaY));

                        }
                    }
                }

            if (MaxError > 0.00001)
                {
                MaxError = MaxError;
                }
            }

#endif
#endif


        }

    pGenMeshOfDirectModels->SetFacetValidation(true);

    pGenMeshOfInverseModels->SetFacetValidation(true);


    m_ModelsCreated = true;

    return(true);
    }


//-----------------------------------------------------------------------------
// PROTECTED
//
// GetAdaptedMeshBasedModel
// Returns the adapted mesh based model. It is the same model as GetAdaptedModel but
// the type returned varies.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DMeshBasedTransfoModel> HGF2DProjectiveMeshAdapter::GetAdaptedMeshBasedModel() const
    {
    return(static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel())));
    }



/** -----------------------------------------------------------------------------
    Studies the reversibility of the model over a region using the given step.
    Since Mentor models are notably un-reversible when region of
    operation is far from usual region of application, it is recommended
    to estimate the reversibility of the model before using. The method
    will sample coordinate transformation by converting direct then inverse
    this result. The deviation from the original value is used in the
    calculation of mean and maximum error which are returned.

    @param pi_rPrecisionArea An extent over which to perform the study. The
                             area may not be empty.

    @param pi_Step The step used in X and Y for sampling. This value must be
                   greater than 0.0


    @param po_pMeanError Pointer to double that receives the mean error.

    @param po_pMaxError  Pointer to double that receives the maximum error.

    -----------------------------------------------------------------------------
*/
void HGF2DProjectiveMeshAdapter::StudyReversibilityPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                                                 double                pi_Step,
                                                                 double*               po_pMeanError,
                                                                 double*               po_pMaxError) const
    {
    // The extent of area must not be empty
    HPRECONDITION(pi_PrecisionArea.GetWidth() != 0.0);
    HPRECONDITION(pi_PrecisionArea.GetHeight() != 0.0);

    // The step may not be null nor negative
    HPRECONDITION(pi_Step > 0.0);

    // Recipient variables must be provided
    HPRECONDITION(po_pMeanError != NULL);
    HPRECONDITION(po_pMaxError != NULL);

    // Convert in temporary variables
    double TempX;
    double TempY;

    double MaxError = 0.0;
    double StatSumX = 0.0;
    double StatSumY = 0.0;
    uint32_t StatNumSamples = 0;

    double TempX1;
    double TempY1;

    double CurrentX;
    double CurrentY;

    for (CurrentY = pi_PrecisionArea.GetYMin() ;
         CurrentY < pi_PrecisionArea.GetYMax() ;
         CurrentY += pi_Step)
        {
        for (CurrentX = pi_PrecisionArea.GetXMin() ;
             CurrentX < pi_PrecisionArea.GetXMax() ;
             CurrentX += pi_Step)
            {
            // Convert one way
            ConvertDirect(CurrentX, CurrentY, &TempX, &TempY);


            // Convert back
            ConvertInverse(TempX, TempY, &TempX1, &TempY1);

            // Compute difference (drift)
            double DeltaX = fabs(CurrentX - TempX1);
            double DeltaY = fabs(CurrentY - TempY1);

            // Add deltas
            StatSumX += DeltaX;
            StatSumY += DeltaY;
            StatNumSamples++;

            MaxError = MAX(MaxError, MAX(DeltaX, DeltaY));
            }
        }

    // Compute precision results
    *po_pMaxError = MaxError;
    *po_pMeanError = (StatNumSamples > 0 ? (StatSumX + StatSumY) / (2 * StatNumSamples) : 0.0);
    }





//-----------------------------------------------------------------------------
// PRIVATE
// CreateDefaultModel
//
// This method creates the linear default model
//-----------------------------------------------------------------------------
bool HGF2DProjectiveMeshAdapter::CreateDefaultModel() const
    {
    // Obtain the extent of the mesh
    HGF2DExtent MyMeshExtent = m_pBaseMesh->GetExtent();
    HGF2DLiteExtent MyLiteMeshExtent(MyMeshExtent.GetXMin(),
                                     MyMeshExtent.GetYMin(),
                                     MyMeshExtent.GetXMax(),
                                     MyMeshExtent.GetYMax());

    // Allocate Tie Points (6 double per tie points pair which are 4 in this particular case)
    HArrayAutoPtr<double> pTiePoints(new double[4 * 6]);
    unsigned short CurrentTiePointVal = 0;
    double TempX;
    double TempY;
#if (0)
    double PreX;
    double PreY;
#endif

    // Append 4 point pairs, one for each extent corner


    // XMin, YMin
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetXMin();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetYMin();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    m_pAdaptedTransfoModel->ConvertDirect(MyLiteMeshExtent.GetXMin(),
                                          MyLiteMeshExtent.GetYMin(),
                                          &TempX,
                                          &TempY);

    pTiePoints[CurrentTiePointVal] = TempX;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = TempY;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    // XMin, YMax
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetXMin();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetYMax();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    m_pAdaptedTransfoModel->ConvertDirect(MyLiteMeshExtent.GetXMin(),
                                          MyLiteMeshExtent.GetYMax(),
                                          &TempX,
                                          &TempY);

    pTiePoints[CurrentTiePointVal] = TempX;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = TempY;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    // XMax, YMax
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetXMax();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetYMax();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    m_pAdaptedTransfoModel->ConvertDirect(MyLiteMeshExtent.GetXMax(),
                                          MyLiteMeshExtent.GetYMax(),
                                          &TempX,
                                          &TempY);

    pTiePoints[CurrentTiePointVal] = TempX;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = TempY;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    // XMax, YMin
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetXMax();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = MyLiteMeshExtent.GetYMin();
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    m_pAdaptedTransfoModel->ConvertDirect(MyLiteMeshExtent.GetXMax(),
                                          MyLiteMeshExtent.GetYMin(),
                                          &TempX,
                                          &TempY);

    pTiePoints[CurrentTiePointVal] = TempX;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = TempY;
    ++CurrentTiePointVal;
    pTiePoints[CurrentTiePointVal] = 0.0;
    ++CurrentTiePointVal;

    // Compute the result affine / projective matrix
    double MyFlatMatrix[4][4];

    if (m_AdaptAsAffine)
        {
        // Create matrix
        GetAffineTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, CurrentTiePointVal, pTiePoints);
        }
    else
        {
        // Create matrix
        GetProjectiveTransfoMatrixFromScaleAndTiePts(MyFlatMatrix, CurrentTiePointVal, pTiePoints);
        }

    // Format result flat matrix as an HFCMatrix
    HFCMatrix<3, 3> MyMatrix;
    MyMatrix[0][0] = MyFlatMatrix[0][0];
    MyMatrix[0][1] = MyFlatMatrix[0][1];
    MyMatrix[0][2] = MyFlatMatrix[0][3];
    MyMatrix[1][0] = MyFlatMatrix[1][0];
    MyMatrix[1][1] = MyFlatMatrix[1][1];
    MyMatrix[1][2] = MyFlatMatrix[1][3];
    MyMatrix[2][0] = MyFlatMatrix[3][0];
    MyMatrix[2][1] = MyFlatMatrix[3][1];
    MyMatrix[2][2] = MyFlatMatrix[3][3];

    // Create the projective or affine model to assign to default model
    if (m_AdaptAsAffine)
        {
        // The projection parameters must be null
        HASSERT(MyMatrix[2][0] == 0.0);
        HASSERT(MyMatrix[2][1] == 0.0);

        // create model
        HFCPtr<HGF2DAffine> pAffine = new HGF2DAffine();
        pAffine->SetByMatrixParameters(MyMatrix[0][2],
                                       MyMatrix[0][0],
                                       MyMatrix[0][1],
                                       MyMatrix[1][2],
                                       MyMatrix[1][0],
                                       MyMatrix[1][1]);


        m_pDefaultModel = pAffine;
        }
    else
        {
        // create model
        HFCPtr<HGF2DProjective> pProjective = new HGF2DProjective();
        pProjective->SetByMatrix(MyMatrix);

        m_pDefaultModel = pProjective;
        }

    return(true);
    }


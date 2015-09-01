//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DProjectiveTriangleMeshAdapter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DProjectiveTriangleMeshAdapter
//-----------------------------------------------------------------------------

#include "hstdcpp.h"    // must be first for PreCompiledHeader Option

#include "HGF2DIdentity.h"

#include "HGF2DProjectiveTriangleMeshAdapter.h"
#include "KGIMatrixOps.h"
#include "HGF2DComplexTransfoModel.h"
#include "HGF2DAffine.h"
#include "HGF2DProjective.h"
#include "HVE2DTriangleFacet.h"
#include "MatrixFromTiePts.h"
#include "HVE2DVoidShape.h"
#include "HVE2DTriangleMesh.h"


#define TOTO (10)
//-----------------------------------------------------------------------------
// Default Constructor
// Protected
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter::HGF2DProjectiveTriangleMeshAdapter()
    : HGF2DProjectiveMeshAdapter()
    {
    }

//-----------------------------------------------------------------------------
// Normal constructor.  It adapts the given mesh based model
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter::HGF2DProjectiveTriangleMeshAdapter(const HGF2DTriangleMeshBasedTransfoModel& i_rTriangleMeshBasedTransfoModel)
    : HGF2DProjectiveMeshAdapter(i_rTriangleMeshBasedTransfoModel)
    {
    KINVARIANTS;
    }


//-----------------------------------------------------------------------------
// PRIVATE CONSTRUCTOR
// Normal constructor.  It adapts the given mesh based model with specified
// pre and post models which must be linear
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter::HGF2DProjectiveTriangleMeshAdapter(const HGF2DTriangleMeshBasedTransfoModel& i_rTriangleMeshBasedTransfoModel,
                                                                       const HGF2DTransfoModel& i_rPreTransfo,
                                                                       const HGF2DTransfoModel& i_rPostTransfo,
                                                                       bool                     i_AdaptAsAffine,
                                                                       bool                     i_ModelsCreated,
                                                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_DirectModels,
                                                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_InverseModels,
                                                                       const HGF2DTransfoModel& i_rDefaultModel,
                                                                       bool                      i_Reversed)

    : HGF2DProjectiveMeshAdapter(i_rTriangleMeshBasedTransfoModel,
                                 i_rPreTransfo,
                                 i_rPostTransfo,
                                 i_AdaptAsAffine,
                                 i_ModelsCreated,
                                 i_DirectModels,
                                 i_InverseModels,
                                 i_rDefaultModel,
                                 i_Reversed)
    {
    KINVARIANTS;
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter::HGF2DProjectiveTriangleMeshAdapter(const HGF2DProjectiveTriangleMeshAdapter& pi_rObj)
    : HGF2DProjectiveMeshAdapter(pi_rObj)
    {
    KINVARIANTS;
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter::~HGF2DProjectiveTriangleMeshAdapter()
    {
    KINVARIANTS;
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HGF2DProjectiveTriangleMeshAdapter& HGF2DProjectiveTriangleMeshAdapter::operator=(const HGF2DProjectiveTriangleMeshAdapter& i_rObj)
    {
    KINVARIANTS;

    // Check that object to copy is not self
    if (this != &i_rObj)
        {
        // Call ancestor operator=
        HGF2DProjectiveMeshAdapter::operator=(i_rObj);
        }

    // Return reference to self
    return (*this);
    }



//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveTriangleMeshAdapter::ConvertDirect(double    pi_YIn,
                                                            double    pi_XInStart,
                                                            uint32_t  pi_NumLoc,
                                                            double    pi_XInStep,
                                                            double*   po_pXOut,
                                                            double*   po_pYOut) const
    {
    KINVARIANTS;

    // Make sure recipient arrays are provided
    KPRECONDITION(po_pXOut);
    KPRECONDITION(po_pYOut);

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;


    StatusInt status = SUCCESS;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = SUCCESS;
        if (m_LastTransformedDirectFacetPresent &&
            m_pLastTransformedDirectFacet->IsPointIn(HGF2DPosition(X, pi_YIn)))
            {

            long NumToDo = 0;
            while ((pi_NumLoc - Index +NumToDo > TOTO) &&
                   m_pLastTransformedDirectFacet->IsPointIn(HGF2DPosition(X + ((NumToDo + TOTO) * pi_XInStep), pi_YIn)))
                {
                NumToDo += TOTO;
                }

            if (NumToDo >= TOTO)
                {
                tempStatus = m_pLastDirectModel->ConvertDirect(pi_YIn, X, NumToDo, pi_XInStep, pCurrentX, pCurrentY);
                pCurrentX += (NumToDo - 1);
                pCurrentY += (NumToDo - 1);
                X += ((NumToDo - 1) * pi_XInStep);
                Index += (NumToDo - 1);
                }
            else
                {
                *pCurrentX = X;
                *pCurrentY = pi_YIn;
                tempStatus = m_pLastDirectModel->ConvertDirect(pCurrentX, pCurrentY);
                }
            }
        else
            {
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            tempStatus = HGF2DProjectiveMeshAdapter::ConvertDirect(pCurrentX, pCurrentY);
            }

        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }
    return status;
    }




//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HGF2DProjectiveTriangleMeshAdapter::ConvertInverse(double    pi_YIn,
                                                             double    pi_XInStart,
                                                             uint32_t  pi_NumLoc,
                                                             double    pi_XInStep,
                                                             double*   po_pXOut,
                                                             double*   po_pYOut) const
    {
    HINVARIANTS;

    // Make sure recipient arrays are provided
    KPRECONDITION(po_pXOut);
    KPRECONDITION(po_pYOut);


    StatusInt status = SUCCESS;

    double  X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {

        StatusInt tempStatus = SUCCESS;

        if (m_LastTransformedInverseFacetPresent &&
            m_pLastTransformedInverseFacet->IsPointIn(HGF2DPosition(X, pi_YIn)))
            {
            long NumToDo = 0;
            while ((pi_NumLoc - Index + NumToDo > TOTO) &&
                   m_pLastTransformedInverseFacet->IsPointIn(HGF2DPosition(X + ((NumToDo + TOTO) * pi_XInStep), pi_YIn)))
                {
                NumToDo += TOTO;
                }

            if (NumToDo >= TOTO)
                {
                m_pLastInverseModel->ConvertInverse(pi_YIn, X, NumToDo, pi_XInStep, pCurrentX, pCurrentY);
                pCurrentX += (NumToDo - 1);
                pCurrentY += (NumToDo - 1);
                X += ((NumToDo - 1) * pi_XInStep);
                Index += (NumToDo - 1);
                }
            else
                {
                *pCurrentX = X;
                *pCurrentY = pi_YIn;
                m_pLastInverseModel->ConvertInverse(pCurrentX, pCurrentY);
                }
            }
        else
            {
            *pCurrentX = X;
            *pCurrentY = pi_YIn;
            HGF2DProjectiveMeshAdapter::ConvertInverse(pCurrentX, pCurrentY);
            }
        // We will return the first non SUCCESS return status only yet continue on with all coordinates
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;

        }
    return status;
    }






//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveTriangleMeshAdapter::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
    {
    KINVARIANTS;

    if (!m_ModelsCreated)
        CreateModels();

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity())
        {
        // Model is identity ... return copy of self
        pResultModel = new HGF2DProjectiveTriangleMeshAdapter(*this);

        }
    else
        {
        if (pi_rModel.PreservesLinearity())
            {
            HFCPtr<HGF2DTransfoModel> pNewPostModel = m_pPostTransfoModel->ComposeInverseWithDirectOf(pi_rModel);

            HFCPtr<HGF2DTriangleMeshBasedTransfoModel> pAdapted = static_cast<HGF2DTriangleMeshBasedTransfoModel*>(&(*GetAdaptedMeshBasedModel()));

            pResultModel = new HGF2DProjectiveTriangleMeshAdapter(*pAdapted,
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
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HGF2DProjectiveTriangleMeshAdapter::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    KINVARIANTS;

    if (!m_ModelsCreated)
        CreateModels();

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    if (pi_rModel.IsIdentity())
        {
        pResultModel = new HGF2DProjectiveTriangleMeshAdapter(*this);
        }
    // Check if the type of the given model can be represented by a matrix
    else if (pi_rModel.PreservesLinearity())
        {
        HFCPtr<HGF2DTransfoModel> pNewPreModel = pi_rModel.ComposeInverseWithDirectOf(*m_pPreTransfoModel);

        HFCPtr<HGF2DTriangleMeshBasedTransfoModel> pAdapted = static_cast<HGF2DTriangleMeshBasedTransfoModel*>(&(*GetAdaptedMeshBasedModel()));

        // Create the final result model
        pResultModel = new HGF2DProjectiveTriangleMeshAdapter(*pAdapted,
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
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HGF2DProjectiveTriangleMeshAdapter::Clone () const
    {
    KINVARIANTS;

    // Allocate object as copy and return
    return(new HGF2DProjectiveTriangleMeshAdapter(*this));
    }


//-----------------------------------------------------------------------------
// PRIVATE
// CreateModels
//
// This method creates the linear models for each and every area part of the mesh
//-----------------------------------------------------------------------------
bool HGF2DProjectiveTriangleMeshAdapter::CreateModels() const
    {
    // From now the reversal is not applicable
    m_Reversed = false;
    m_NumConverted = 0;

    m_pBaseMesh = (static_cast<HGF2DMeshBasedTransfoModel*>(&(*GetAdaptedTransfoModel()))->GetMesh());

    // Create the default model first
    CreateDefaultModel();

    HFCPtr<HGF2DCoordSys> pPreModelCS = new HGF2DCoordSys(HGF2DIdentity(), m_pBaseMesh->GetCoordSys());

    // Create a coordinate system related to base mesh coordinate system
    // through the adapted model ... this system will only be used in the current
    // method
    // Create the direct mesh
    HFCPtr<HVE2DTriangleMesh<HFCPtr<HGF2DTransfoModel> > > pTriangleMeshOfDirectModels = new HVE2DTriangleMesh<HFCPtr<HGF2DTransfoModel> >(pPreModelCS);
    HFCPtr<HVE2DTriangleMesh<HFCPtr<HGF2DTransfoModel> > > pTriangleMeshOfInverseModels = new HVE2DTriangleMesh<HFCPtr<HGF2DTransfoModel> >(m_pBaseMesh->GetCoordSys());

    m_pMeshOfDirectModels = pTriangleMeshOfDirectModels;
    m_pMeshOfInverseModels = pTriangleMeshOfInverseModels;

    // These lines are executed only when in release mode
    pTriangleMeshOfDirectModels->SetFacetValidation(false);

    pTriangleMeshOfInverseModels->SetFacetValidation(false);

    // For every facet part of the mesh ...
    long NumberOfFacets = m_pBaseMesh->CountFacets();

    for (long i = 0 ; i < NumberOfFacets ; ++i)
        {
        // Obtain the facet
        HFCPtr<HVE2DRawFacet> pFacet = m_pBaseMesh->GetFacet(i);

#if (0)
        HFCPtr<HVE2DTriangleFacet> pTriangleFacet = static_cast<HVE2DTriangleFacet*>(&(*pFacet));
#endif

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

        // Removing double points accelerates operation
        ShapePoints.pop_back();

        // Add the centroid
        // This is because of a bug concerning model generation using tie points
        long k;
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
        uint32_t NumberOfPoints = ShapePoints.size();

        // Allocate Tie Points (6 double per tie points pair)
        HArrayAutoPtr<double> pTiePoints(new double[NumberOfPoints * 6]);

        // For every points part of the shape ...
        double CurrentX;
        double CurrentY;
        double TempX;
        double TempY;
        long CurrentTiePointVal = 0;

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

            m_pAdaptedTransfoModel->ConvertDirect(CurrentX, CurrentY, &TempX, &TempY);

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


            pModel = pAffine;
            }
        else
            {
            // create model
            HFCPtr<HGF2DProjective> pProjective = new HGF2DProjective();
            pProjective->SetByMatrix(MyMatrix);

            pModel = pProjective;
            }


        // Cast shape as a polygon of segments
        KASSERT(pNewShape->IsSimple());
        HVE2DPolygonOfSegments& rPolygon = static_cast<HVE2DPolygonOfSegments&>(*pNewShape);

        // Create facet
        HFCPtr<HVE2DTriangleFacet<HFCPtr<HGF2DTransfoModel> > > pNewDirectFacet = new HVE2DTriangleFacet<HFCPtr<HGF2DTransfoModel> >(rPolygon, pModel);

        // Add to direct model mesh
        pTriangleMeshOfDirectModels->AddFacet(pNewDirectFacet);

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


        // Add shape only if it exists
        if (pInverseShape->CalculateArea() != 0.0)
            {
            // Cast shape as a polygon of segments
            KASSERT(pInverseShape->IsSimple());
            HVE2DPolygonOfSegments& rInvPolygon = static_cast<HVE2DPolygonOfSegments&>(*pInverseShape);


            // Create inverse facet
            HFCPtr<HVE2DTriangleFacet<HFCPtr<HGF2DTransfoModel> > > pNewInverseFacet = new HVE2DTriangleFacet<HFCPtr<HGF2DTransfoModel> >(rInvPolygon, pModel);

            // Add to inverse model mesh
            pTriangleMeshOfInverseModels->AddFacet(pNewInverseFacet);
            }
        }

    pTriangleMeshOfDirectModels->SetFacetValidation(true);

    pTriangleMeshOfInverseModels->SetFacetValidation(true);


    m_ModelsCreated = true;

    return(true);
    }



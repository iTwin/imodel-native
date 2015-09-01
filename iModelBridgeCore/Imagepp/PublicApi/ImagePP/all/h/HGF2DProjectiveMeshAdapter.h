//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DProjectiveMeshAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DProjectiveMeshAdapter
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model adapted upon a mesh.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

#include "HGF2DMeshBasedTransfoModel.h"
#include "HVE2DMesh.h"
#include "HVE2DGenMesh.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    The purpose of the projective mesh adapter is to linearize or accelerate a
    non-linear model based upon a mesh in order
    to accelerate the conversion process by providing an approximation of a projective
    for each and every facet. Note that the adapted model must be linear by facets
    The projective mesh adapter is valid for any coordinate value. If coordinates to
    convert fall outside the original mesh, then the default comportement
    of the adapted model is also linearized based on a pseudo facet based on the four corners
    of the extent

    Since the adaptation is an approximation of the transformation model, an error may be
    introduced by adaptation using such a projective adapter. The error can
    be studied using the StudyPrecisionOver() method.

    Study of precision can be performed on the original area or on any other area.

    -----------------------------------------------------------------------------
*/
class HGF2DProjectiveMeshAdapter : public HGF2DTransfoModelAdapter
    {
    HDECLARE_CLASS_ID(HGF2DProjectiveId_MeshAdapter, HGF2DTransfoModelAdapter)

public:

    // Primary methods
                                HGF2DProjectiveMeshAdapter();

                                HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& pi_rMeshBasedTransfoModel);


                                HGF2DProjectiveMeshAdapter(const HGF2DProjectiveMeshAdapter& pi_rObj);
    virtual                     ~HGF2DProjectiveMeshAdapter();
    HGF2DProjectiveMeshAdapter& operator=(const HGF2DProjectiveMeshAdapter& pi_rObj);

#if (0)

    // From HGF2DMeshBasedTransfoModel
    virtual HFCPtr<HVE2DRawMesh>
                                GetMesh() const;
#endif


    void                        StudyReversibilityPrecisionOver(const HGF2DLiteExtent& pi_PrecisionArea,
                                                                double                pi_Step,
                                                                double*               po_pMeanError,
                                                                double*               po_pMaxError) const;


    virtual bool IsConvertDirectThreadSafe() const override {return false;}
    virtual bool IsConvertInverseThreadSafe() const override {return false;}

    // From HGF2DTransfoModel
    virtual StatusInt           ConvertDirect(double*   pio_pXInOut,
                                              double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertDirect(double    pi_YIn,
                                              double    pi_XInStart,
                                              uint32_t  pi_NumLoc,
                                              double    pi_XInStep,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;

    virtual StatusInt           ConvertDirect(double    pi_XIn,
                                              double    pi_YIn,
                                              double*   po_pXOut,
                                              double*   po_pYOut) const override;


    virtual StatusInt           ConvertDirect(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const override;

    virtual StatusInt           ConvertInverse(double*   pio_pXInOut,
                                               double*   pio_pYInOut) const override;

    virtual StatusInt           ConvertInverse(double    pi_YIn,
                                               double    pi_XInStart,
                                               uint32_t  pi_NumLoc,
                                               double    pi_XInStep,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           ConvertInverse(double    pi_XIn,
                                               double    pi_YIn,
                                               double*   po_pXOut,
                                               double*   po_pYOut) const override;

    virtual StatusInt           ConvertDirect(size_t    pi_NumLoc,
                                              double*   pio_aXInOut,
                                              double*   pio_aYInOut) const override;



    virtual bool                IsIdentity      () const;
    virtual bool                IsStretchable   (double pi_AngleTolerance = 0) const;
    virtual void                GetStretchParams(double*           po_pScaleFactorX,
                                                 double*           po_pScaleFactorY,
                                                 HGF2DDisplacement* po_pDisplacement) const;

    virtual HGF2DTransfoModel*  Clone () const override;
    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;


    virtual bool                CanBeRepresentedByAMatrix() const;
    virtual HFCMatrix<3, 3>     GetMatrix() const;

    virtual HFCPtr<HGF2DTransfoModel>
                                CreateSimplifiedModel() const;

    virtual bool                PreservesLinearity() const;
    virtual bool                PreservesParallelism() const;
    virtual bool                PreservesShape() const;
    virtual bool                PreservesDirection() const;

    virtual void                Reverse ();

protected:

    virtual void                Prepare ();
    virtual HFCPtr<HGF2DTransfoModel>
                                ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;

    HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& i_rMeshBasedTransfoModel,
                               const HGF2DTransfoModel& i_rPreTransfo,
                               const HGF2DTransfoModel& i_rPostTransfo,
                               bool                     i_AdaptAsAffine);

    HGF2DProjectiveMeshAdapter(const HGF2DMeshBasedTransfoModel& i_rMeshBasedTransfoModel,
                               const HGF2DTransfoModel& i_rPreTransfo,
                               const HGF2DTransfoModel& i_rPostTransfo,
                               bool                     i_AdaptAsAffine,
                               bool                     i_ModelsCreated,
                               HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_DirectModels,
                               HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_InverseModels,
                               const HGF2DTransfoModel& i_rDefaultModel,
                               bool                     i_Reversed);


    HFCPtr<HGF2DMeshBasedTransfoModel>
                                GetAdaptedMeshBasedModel() const;

    // This private method is intentionaly virtual as we expect derived classes to 
    // override it in order to implement a new mesh model algorithm.
    virtual bool                CreateModels() const;



    void               CheckInvariants() const
        {
        // There must always be a pre and post model defined
        HASSERT(m_pPreTransfoModel != 0);
        HASSERT(m_pPostTransfoModel != 0);

        HASSERT(m_pPreTransfoModel->CanBeRepresentedByAMatrix());
        HASSERT(m_pPostTransfoModel->CanBeRepresentedByAMatrix());

// The following should have been true, but sometimes a facets is scaled out by minusculization
//            HASSERT(!m_ModelsCreated || (m_pMeshOfDirectModels->CountFacets() == m_pMeshOfInverseModels->CountFacets()));
//            HASSERT(!m_ModelsCreated || (m_pMeshOfDirectModels->CountFacets() == m_pBaseMesh->CountFacets()));
#ifdef HVERIFYCONTRACT
        HGF2DTransfoModelAdapter::ValidateInvariants();
#endif
        }


    // Private methods
    void               Copy (const HGF2DProjectiveMeshAdapter& pi_rObj);
    bool               CreateDefaultModel() const;

    // Primary attributes
    HFCPtr<HGF2DTransfoModel>           m_pPreTransfoModel;
    HFCPtr<HGF2DTransfoModel>           m_pPostTransfoModel;
    mutable HFCPtr<HVE2DRawMesh>                m_pBaseMesh;

    bool                               m_AdaptAsAffine;

    // Acceleration attributes
    // The reversed member indicates that the follwoing meshes are to be used in the inverse direction of their definition
    mutable bool                        m_Reversed;
    mutable bool                        m_ModelsCreated;

    // The following models exist only if m_Modelscreated is set to true
    // though the meshes are not destroyed if models are to be updated.
#if (0)
    mutable HFCPtr<HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> > > m_pMeshOfDirectModels;
    mutable HFCPtr<HVE2DGenMesh<HFCPtr<HGF2DTransfoModel> > > m_pMeshOfInverseModels;
#else
    mutable HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > m_pMeshOfDirectModels;
    mutable HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > m_pMeshOfInverseModels;
#endif
    mutable HFCPtr<HGF2DTransfoModel> m_pDefaultModel;

    mutable long m_NumConverted;

    mutable bool m_LastDirectFacetInitialized;
    mutable long m_LastDirectHit;
    mutable bool m_LastDirectModelPresent;
    mutable bool m_LastTransformedDirectFacetPresent;
    mutable HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > m_pLastDirectFacet;
    mutable HFCPtr<HGF2DTransfoModel> m_pLastDirectModel;
    mutable HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > m_pLastTransformedDirectFacet;

    mutable bool m_LastInverseFacetInitialized;
    mutable long m_LastInverseHit;
    mutable bool m_LastInverseModelPresent;
    mutable bool m_LastTransformedInverseFacetPresent;
    mutable HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > m_pLastInverseFacet;
    mutable HFCPtr<HGF2DTransfoModel> m_pLastInverseModel;
    mutable HFCPtr<HVE2DFacet<HFCPtr<HGF2DTransfoModel> > > m_pLastTransformedInverseFacet;

private:
    };

END_IMAGEPP_NAMESPACE
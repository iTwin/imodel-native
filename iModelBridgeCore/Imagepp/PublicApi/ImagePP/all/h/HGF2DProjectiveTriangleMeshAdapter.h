//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DProjectiveTriangleMeshAdapter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DProjectiveTriangleMeshAdapter
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model adapted upon a mesh.
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTransfoModelAdapter.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"

#include "HGF2DProjectiveMeshAdapter.h"
#include "HGF2DTriangleMeshBasedTransfoModel.h"
#include "HVE2DMesh.h"
#include "HVE2DGenMesh.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    The purpose of the projective triangle mesh adapter is to linearize or accelerate a
    non-linear model based upon a triangle mesh in order
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

    This class is a specialization of the general Projective mesh adapter which is slower
    for triangle meshes.

    -----------------------------------------------------------------------------
*/
class HGF2DProjectiveTriangleMeshAdapter : public HGF2DProjectiveMeshAdapter
    {
    HDECLARE_CLASS_ID(HGF2DProjectiveId_TriangleMeshAdapter, HGF2DProjectiveMeshAdapter)

public:

    // Primary methods
    HGF2DProjectiveTriangleMeshAdapter();

    HGF2DProjectiveTriangleMeshAdapter(const HGF2DTriangleMeshBasedTransfoModel& i_rTriangleMeshBasedTransfoModel);


    HGF2DProjectiveTriangleMeshAdapter(const HGF2DProjectiveTriangleMeshAdapter& i_rObj);
    virtual         ~HGF2DProjectiveTriangleMeshAdapter();
    HGF2DProjectiveTriangleMeshAdapter&    operator=(const HGF2DProjectiveTriangleMeshAdapter& i_rObj);



    // From HGF2DTransfoModel
    virtual void    ConvertDirect(double    pi_YIn,
                                  double    pi_XInStart,
                                  uint32_t   pi_NumLoc,
                                  double    pi_XInStep,
                                  double*   po_pXOut,
                                  double*   po_pYOut) const override;


    virtual void    ConvertInverse(double    pi_YIn,
                                   double    pi_XInStart,
                                   uint32_t   pi_NumLoc,
                                   double    pi_XInStep,
                                   double*   po_pXOut,
                                   double*   po_pYOut) const override;



    virtual HFCPtr<HGF2DTransfoModel>
    ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const;




    virtual HGF2DTransfoModel* Clone () const override;

protected:

    virtual HFCPtr<HGF2DTransfoModel>
    ComposeYourself (const HGF2DTransfoModel& pi_rModel) const;



    HGF2DProjectiveTriangleMeshAdapter(const HGF2DTriangleMeshBasedTransfoModel& i_rTriangleMeshBasedTransfoModel,
                                       const HGF2DTransfoModel& i_rPreTransfo,
                                       const HGF2DTransfoModel& i_rPostTransfo,
                                       bool                     i_AdaptAsAffine,
                                       bool                     i_ModelsCreated,
                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_DirectModels,
                                       HFCPtr<HVE2DMesh<HFCPtr<HGF2DTransfoModel> > > i_InverseModels,
                                       const HGF2DTransfoModel& i_rDefaultModel,
                                       bool                     i_Reversed);



    virtual bool    CreateModels() const;

    void            CheckInvariants() const
        {
        HGF2DProjectiveMeshAdapter::CheckInvariants();
        }



private:


    };


END_IMAGEPP_NAMESPACE
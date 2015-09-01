//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DMeshBasedTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DMeshBasedTransfoModel
//-----------------------------------------------------------------------------
// Description of 2D projective transformation model.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HVE2DRawMesh;

// HPM_DECLARE_HEADER(HGF2DMeshBasedTransfoModel)

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    A mesh based transformation model is a model that bases its definition upon a 2D
    mesh. The transformation inside each of the facets part of the mesh is usually
    different, although the transformation at the facet boundary and jucntion must
    identical.

    For such model, it is possible to extract the mesh to obtain the definition
    areas of the mesh based model
    -----------------------------------------------------------------------------
*/
class HGF2DMeshBasedTransfoModel : public HGF2DTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DTransfoModelId_MeshBased, HGF2DTransfoModel)

public:

    // Primary methods
    HGF2DMeshBasedTransfoModel();

    HGF2DMeshBasedTransfoModel(const HGF2DMeshBasedTransfoModel& pi_rObj);
    virtual         ~HGF2DMeshBasedTransfoModel();
    HGF2DMeshBasedTransfoModel&    operator=(const HGF2DMeshBasedTransfoModel& pi_rObj);


    // Unique function
    //virtual HFCPtr<HVE2DMesh>
    //                GetMesh() const = 0;


protected:

    void               ValidateInvariants() const
        {
        }


private:

    };

END_IMAGEPP_NAMESPACE
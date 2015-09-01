//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTriangleMeshBasedTransfoModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DTriangleMeshBasedTransfoModel
//-----------------------------------------------------------------------------
// Description of 2D triangle mesh based transformation model
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DDisplacement.h"
#include "HGF2DTriangleMeshBasedTransfoModel.h"
#include "HFCMatrix.h"
#include "HGF2DLiteExtent.h"
#include "HGF2DMeshBasedTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    A triangle mesh based transformation model is a model that bases its definition upon a 2D
    triangle mesh. The transformation inside each of the facets part of the mesh is usually
    different, although the transformation at the facet boundary and jucntion must
    identical.

    For such model, it is possible to extract the mesh to obtain the definition
    areas of the triangle mesh based model
    -----------------------------------------------------------------------------
*/
class HGF2DTriangleMeshBasedTransfoModel : public HGF2DMeshBasedTransfoModel
    {
    HDECLARE_CLASS_ID(HGF2DTransfoModelId_TriangleMeshBased, HGF2DMeshBasedTransfoModel)

public:

    // Primary methods
    HGF2DTriangleMeshBasedTransfoModel();

    HGF2DTriangleMeshBasedTransfoModel(const HGF2DTriangleMeshBasedTransfoModel& pi_rObj);
    virtual         ~HGF2DTriangleMeshBasedTransfoModel();
    HGF2DTriangleMeshBasedTransfoModel&    operator=(const HGF2DTriangleMeshBasedTransfoModel& pi_rObj);


protected:


private:

    void               ValidateInvariants() const
        {
        HGF2DMeshBasedTransfoModel::ValidateInvariants();
        }

    };
END_IMAGEPP_NAMESPACE

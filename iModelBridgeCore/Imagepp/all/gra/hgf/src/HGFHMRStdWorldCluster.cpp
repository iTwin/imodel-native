//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFHMRStdWorldCluster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>


HPM_REGISTER_CLASS(HGFHMRStdWorldCluster, HGF2DWorldCluster)

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGFHMRStdWorldCluster::HGFHMRStdWorldCluster()
    {
    // Create a new world for UNKNOWN DOWN axis
    HFCPtr<HGF2DWorld> pUnknownDownWorld = new HGF2DWorld(HGF2DWorld_UNKNOWNWORLD);

    // Create HMR World, Origin Bottom-Left
    HGF2DStretch NewModel;
    NewModel.SetYScaling(-1.0);
    HFCPtr<HGF2DWorld> pHMRWorld = new HGF2DWorld(NewModel,
                                                  (HFCPtr<HGF2DCoordSys>&)pUnknownDownWorld,
                                                  HGF2DWorld_HMRWORLD);

    // DGN CoordSys, Origin Bottom-Left
    HFCPtr<HGF2DWorld> pDGNWorld = new HGF2DWorld(HGF2DIdentity(),
                                                  (HFCPtr<HGF2DCoordSys>&)pHMRWorld,
                                                  HGF2DWorld_DGNWORLD);

    HFCPtr<HGF2DWorld> pIntergraphWorld = new HGF2DWorld(HGF2DIdentity(),
                                                         (HFCPtr<HGF2DCoordSys>&)pDGNWorld,
                                                         HGF2DWorld_INTERGRAPHWORLD);

    // Create ITIFF world, Top-Left origin
    HFCPtr<HGF2DWorld> pITiffWorld = new HGF2DWorld(HGF2DIdentity(),
                                                    (HFCPtr<HGF2DCoordSys>&)pUnknownDownWorld,
                                                    HGF2DWorld_ITIFFWORLD);

    // Geographic
    HFCPtr<HGF2DWorld> pGeographic = new HGF2DWorld(HGF2DIdentity(),
                                                    (HFCPtr<HGF2DCoordSys>&)pHMRWorld,
                                                    HGF2DWorld_GEOGRAPHIC);

    HFCPtr<HGF2DWorld> pGeotiffUnknown = new HGF2DWorld(HGF2DIdentity(),
                                                        (HFCPtr<HGF2DCoordSys>&)pHMRWorld,
                                                        HGF2DWorld_GEOTIFFUNKNOWN);


    // Add these worlds to the cluster
    AddWorldReference(pUnknownDownWorld);
    AddWorldReference(pHMRWorld);
    AddWorldReference(pDGNWorld);
    AddWorldReference(pIntergraphWorld);
    AddWorldReference(pITiffWorld);
    AddWorldReference(pGeographic);
    AddWorldReference(pGeotiffUnknown);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HGFHMRStdWorldCluster::~HGFHMRStdWorldCluster()
    {
    }


//-----------------------------------------------------------------------------
// GetWorldReference
// This overload returns unknown if the needed world is not found
//-----------------------------------------------------------------------------
HFCPtr<HGF2DWorld> HGFHMRStdWorldCluster::GetWorldReference(
    HGF2DWorldIdentificator pi_WorldID) const
    {
    // Find the world designated
    HFCPtr<HGF2DWorld> pFoundWorld = HGF2DWorldCluster::GetWorldReference(pi_WorldID);

    // Check if found
    if (pFoundWorld == 0)
        {
        // World not found ... return standard default
        pFoundWorld = HGF2DWorldCluster::GetWorldReference(HGF2DWorld_UNKNOWNWORLD);
        }

    HPOSTCONDITION(pFoundWorld != 0);

    return(pFoundWorld);
    }


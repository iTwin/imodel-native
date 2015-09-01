//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hps/src/HPSWorldCluster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HPSWorldCluster
//-----------------------------------------------------------------------------


#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HPSWorldCluster.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

HPM_REGISTER_CLASS(HPSWorldCluster, HGF2DWorldCluster)

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HPSWorldCluster::HPSWorldCluster()
    {
    // Create a new world for UNKNOWN DOWN axis

    HFCPtr<HGF2DWorld> pUnknownDownWorld = new HGF2DWorld(HGF2DWorld_UNKNOWNWORLD);

    HGF2DStretch FlipModel;
    FlipModel.SetYScaling(-1.0);


    // Create HMR World, bottom-Left origin

    HFCPtr<HGF2DWorld> pHMRWorld = new HGF2DWorld(FlipModel,
                                                  (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
                                                  HGF2DWorld_HMRWORLD);

    // DGN world, bottom-Left origin

    HFCPtr<HGF2DWorld> pDGNWorld = new HGF2DWorld(FlipModel,
                                                  (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
                                                  HGF2DWorld_DGNWORLD);

    // Intergraph world, bottom-left origin

    HFCPtr<HGF2DWorld> pIntergraphWorld = new HGF2DWorld(FlipModel,
                                                         (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
                                                         HGF2DWorld_INTERGRAPHWORLD);

    // Create ITIFF world, top-left origin

    HFCPtr<HGF2DWorld> pITiffWorld = new HGF2DWorld(HGF2DIdentity(),
                                                    (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
                                                    HGF2DWorld_ITIFFWORLD);

    // Geographic
    HFCPtr<HGF2DWorld> pGeographic = new HGF2DWorld(FlipModel,
                                                    (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
                                                    HGF2DWorld_GEOGRAPHIC);

    HFCPtr<HGF2DWorld> pGeotiffUnknown = new HGF2DWorld(FlipModel,
                                                        (HFCPtr<HGF2DCoordSys>)pUnknownDownWorld,
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
HPSWorldCluster::~HPSWorldCluster()
    {
    // Nothing to do!
    }

//-----------------------------------------------------------------------------
// Alters an existing relation between two worlds, or create a new world
// with specified relation.  Returns false if the base world can't be found.
//-----------------------------------------------------------------------------
bool HPSWorldCluster::SetRelation(HGF2DWorldIdentificator pi_WorldID,
                                   const HGF2DTransfoModel& pi_rRelation,
                                   HGF2DWorldIdentificator pi_BaseWorldID)
    {
    HFCPtr<HGF2DCoordSys> pBase = &*GetWorldReference(pi_BaseWorldID);
    if (pBase == 0)
        return false;

    HFCPtr<HGF2DCoordSys> pWorld = &*GetWorldReference(pi_WorldID);
    if (pWorld == 0)
        {
        pWorld = new HGF2DWorld(pi_rRelation, pBase, pi_WorldID);
        AddWorldReference((HFCPtr<HGF2DWorld>&)pWorld);
        }
    else
        pWorld->SetReference(pi_rRelation, pBase);

    return true;
    }

//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordUtility.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/all/h/HGF2DWorld.h>

class HGF2DWorldCluster;
class HRFRasterFile;
class HGF2DTransfoModel;
class HCPGCoordModel;

// ----------------------------------------------------------------------------
//  HGFGCoordException
// ----------------------------------------------------------------------------
class HCPGCoordUtility
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HCPGCoordUtility)

public:

    //This method compares two coordinate systems from BaseGeoCoord pointers
    bool AreBaseGCSEquivalent(const IRasterBaseGcsPtr& pi_rpBaseGeoCoord1, const IRasterBaseGcsPtr& pi_rpBaseGeoCoord2);

    // This method creates a GCoord model from projection description
    HFCPtr<HCPGCoordModel>
    CreateGCoordModel(IRasterBaseGcsPtr  pi_SourceProjection,
                      IRasterBaseGcsPtr  pi_DestinationProjection) const;

    // This method creates an adapted GCoord model from projection description
    HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(IRasterBaseGcsPtr              pi_SourceProjection,
                             IRasterBaseGcsPtr              pi_DestinationProjection,
                             const HGF2DLiteExtent&         pi_rExtent,
                             double                         pi_Step,
                             double                         pi_ExpectedMeanError,
                             double                         pi_ExpectedMaxError,
                             double*                        po_pAdaptationMeanError = 0,
                             double*                        po_pAdaptationMaxError = 0,
                             double*                        po_pReversibilityMeanError = 0,
                             double*                        po_pReversibilityMaxError = 0) const;
    // This method creates an adapted GCoord model from projection description
    // raster file extent and threshold errors.
    HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(IRasterBaseGcsPtr              pi_SourceProjection,
                             IRasterBaseGcsPtr              pi_DestinationProjection,
                             const HFCPtr<HRFRasterFile>&   pi_rpRasterFile,
                             uint32_t                      pi_Page,
                             const HGF2DWorldCluster&       pi_rCluster,
                             HGF2DWorldIdentificator        pi_DestinationBaseWorld,
                             double                         pi_Step,
                             double                         pi_ExpectedMeanError,
                             double                         pi_ExpectedMaxError,
                             double*                        po_pAdaptationMeanError = 0,
                             double*                        po_pAdaptationMaxError = 0,
                             double*                        po_pReversibilityMeanError = 0,
                             double*                        po_pReversibilityMaxError = 0) const;


    // Destructor
    virtual ~HCPGCoordUtility();

protected:
    // the model is a friend
    friend class HCPGCoordModel;
    // the projection is a friend

    // Constructor
    HCPGCoordUtility();


private:


    // Disabled methods
    HCPGCoordUtility(const HCPGCoordUtility&);
    HCPGCoordUtility& operator=(const HCPGCoordUtility&);
    };


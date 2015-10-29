//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCPGCoordUtility.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCMacros.h>
#include <Imagepp/all/h/HGF2DWorld.h>
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

BEGIN_IMAGEPP_NAMESPACE

class HGF2DWorldCluster;
class HRFRasterFile;
class HGF2DTransfoModel;
class HCPGCoordModel;

// ----------------------------------------------------------------------------
//  HGFGCoordException
// ----------------------------------------------------------------------------
class HCPGCoordUtility
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HCPGCoordUtility)

public:

    //This method compares two coordinate systems from BaseGeoCoord pointers
    bool AreBaseGCSEquivalent(IRasterBaseGcsCP pi_rpBaseGeoCoord1, IRasterBaseGcsCP pi_rpBaseGeoCoord2);

    // This method creates a GCoord model from projection description
    HFCPtr<HCPGCoordModel>
    CreateGCoordModel(IRasterBaseGcsR  pi_SourceProjection,
                      IRasterBaseGcsR  pi_DestinationProjection) const;

    // This method creates an adapted GCoord model from projection description
    HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(IRasterBaseGcsR                pi_SourceProjection,
                             IRasterBaseGcsR                pi_DestinationProjection,
                             const HGF2DLiteExtent&         pi_rExtent,
                             double                         pi_Step,
                             double                         pi_ExpectedMeanError,
                             double                         pi_ExpectedMaxError,
                             double*                        po_pAdaptationMeanError = 0,
                             double*                        po_pAdaptationMaxError = 0,
                             double*                        po_pReversibilityMeanError = 0,
                             double*                        po_pReversibilityMaxError = 0) const;

    IMAGEPP_EXPORT static HFCPtr<HGF2DTransfoModel> CreateAdaptedModel(HGF2DTransfoModel& transforModel,
                                                                   const HGF2DLiteExtent& pi_rExtent,
                                                                   double  pi_Step,
                                                                   double  pi_ExpectedMeanError,
                                                                   double  pi_ExpectedMaxError,
                                                                   double* po_pAdaptationMeanError,
                                                                   double* po_pAdaptationMaxError,
                                                                   double* po_pReversibilityMeanError,
                                                                   double* po_pReversibilityMaxError);

    // This method creates an adapted GCoord model from projection description
    // raster file extent and threshold errors.
    HFCPtr<HGF2DTransfoModel>
    CreateGCoordAdaptedModel(IRasterBaseGcsR               pi_SourceProjection,
                             IRasterBaseGcsR               pi_DestinationProjection,
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



    static StatusInt GetGeoDomain(IRasterBaseGcsCR                rasterGcs,
                                  vector<HGF2DCoord<double> >&    shape);


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

END_IMAGEPP_NAMESPACE
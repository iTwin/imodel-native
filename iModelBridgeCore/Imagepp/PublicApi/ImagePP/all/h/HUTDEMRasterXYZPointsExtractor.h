//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTDEMRasterXYZPointsExtractor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HGFHMRStdWorldCluster.h"
#include "HRABitmap.h"
#include "HRFRasterFile.h"
#include "HUTDEMRasterXYZPointsIterator.h"

BEGIN_IMAGEPP_NAMESPACE
class HUTDEMRasterXYZPointsExtractor
    {
public:

    friend class                    HUTDEMRasterXYZPointsIterator;

    IMAGEPP_EXPORT                          HUTDEMRasterXYZPointsExtractor         (const WString&         pi_rDEMRasterFileName,
                                                                            const HFCPtr<HPMPool>& pi_rpMemPool);                

    // NTERAY: For TR#330441, added this overload in order to ensure that we won't break civil work-flow. We should
    // probably preserve only the most permissive version (this one) and remove the redundant flag.
    IMAGEPP_EXPORT                          HUTDEMRasterXYZPointsExtractor         (const WString&         pi_rDEMRasterFileName, 
                                                                            const HFCPtr<HPMPool>& pi_rpMemPool,
                                                                            bool                   pi_legacyPixelTypeSupportOnly);   

    IMAGEPP_EXPORT virtual                  ~HUTDEMRasterXYZPointsExtractor        ();

    IMAGEPP_EXPORT void                     GetXYZPoints                           (uint64_t*  po_pNumberPoints,
                                                                            double*& po_prXYZPoints) const;

    IMAGEPP_EXPORT void                     GetDimensionInPixels                   (uint64_t* po_pWidthInPixels,
                                                                            uint64_t* po_pHeightInPixels) const;

    IMAGEPP_EXPORT void                     GetNumberOfPoints                      (uint64_t* po_pNumberPoints) const;

    IMAGEPP_EXPORT IRasterBaseGcsCP         GetDEMRasterCoordSysCP() const;

    IMAGEPP_EXPORT const IRasterBaseGcsCP   GetGeocodingCP() const;

    // Deprecated. NTERAY: Move to private section
    IMAGEPP_EXPORT const double*            GetNoDataValue                         () const;

    IMAGEPP_EXPORT void                     Get2DCoordMinMaxValues                 (double* po_pXMin, double* po_pXMax,
                                                                            double* po_pYMin, double* po_pYMax) const;

    IMAGEPP_EXPORT bool                     GetZCoordMinMaxValues                  (double* po_pZMin, double* po_pZMax) const;


    IMAGEPP_EXPORT size_t                   GetMaxPointQtyForXYZPointsIterator     (double  pi_ScaleFactor = 1.0);


    // Deprecated. Use CreateXYZPointsIteratorWithNoDataValueRemoval
    IMAGEPP_EXPORT HUTDEMRasterXYZPointsIterator*
                                    CreateXYZPointsIterator                (const WString& pi_rDestCoordSysKeyName = WString(L""),
                                                                            double         pi_ScaleFactor = 1.0);

    // NTERAY: Should be removed once support for legacy use is removed
    IMAGEPP_EXPORT HUTDEMRasterXYZPointsIterator*
                                    CreateXYZPointsIteratorWithNoDataValueRemoval
                                                                            (const WString& pi_rDestCoordSysKeyName = WString(L""),
                                                                             double         pi_ScaleFactor = 1.0);


    IMAGEPP_EXPORT HFCPtr<HGF2DCoordSys>    GetXYCoordSysPtr                       () const; 
    IMAGEPP_EXPORT HFCPtr<HGF2DCoordSys>    GetPhysicalLowerLeftPixelCoordSysPtr   () const;


    // TDORAY: Deprecated. Remove.
    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel>
                                    GetAppliedSourceToTargetXYTransform    () const;
   
    // NTERAY: Those may be temporary addition to this interface but will 
    // be required in order to be able to augment functionality/fix TRs 
    // from Descartes if we find MS to be already released and we still
    // need access to some parameters we forgot to provide from this API.
    IMAGEPP_EXPORT const HRFRasterFile&     GetRasterFile                          () const;
    IMAGEPP_EXPORT const HRARaster&         GetRaster                              () const;


private:
    void                            LoadRasterFile                         (HFCPtr<HRFRasterFile>&              pi_rpRasterFile,
                                                                            const HFCPtr<HPMPool>&              pi_rpMemPool,
                                                                            HFCPtr<HRARaster>&                  po_rpRaster) const;



    double                         GetFactorToMeterForZ                   () const;

    const HFCPtr<HGF2DCoordSys>&    GetXYCoordSystP                        () const;


    HFCPtr<HRFRasterFile>           m_pRasterFile;
    HFCPtr<HRARaster>               m_pRaster;
    HFCPtr<HGF2DCoordSys>           m_pXYCoordSys;
    uint64_t                       m_HeightInPixels;
    uint64_t                       m_WidthInPixels;

    static HFCPtr<HGFHMRStdWorldCluster>&    GetHMRWorldCluster();
    static HFCPtr<HGFHMRStdWorldCluster>    m_spWorldCluster;


    // Disable copies of any kind
    HUTDEMRasterXYZPointsExtractor(const HUTDEMRasterXYZPointsExtractor& pi_rObj);
    HUTDEMRasterXYZPointsExtractor& operator=(const HUTDEMRasterXYZPointsExtractor& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

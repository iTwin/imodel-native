//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTDEMRasterXYZPointsIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HIMStripAdapter.h"
#include "HRABitmap.h"
#include "HRFRasterFile.h"



BEGIN_IMAGEPP_NAMESPACE
class HUTDEMRasterXYZPointsExtractor;

class HUTDEMRasterXYZPointsIterator
    {
public :


    // Deprecated. Factor from the extractor
    IMAGEPP_EXPORT                          HUTDEMRasterXYZPointsIterator          (HUTDEMRasterXYZPointsExtractor*    pi_pRasterPointExtractor,
                                                                            const WString&                     pi_rDestCoordSysKeyName,
                                                                            double                             pi_ScaleFactor);

    IMAGEPP_EXPORT virtual                  ~HUTDEMRasterXYZPointsIterator         ();

    IMAGEPP_EXPORT size_t                   GetMaxXYZPointQty                      () const;

    template <typename PointType>
    size_t                          GetXYZPoints                           (PointType*                          po_pPoints,
                                                                            size_t                              pi_Capacity) const;

    // Deprecated. Use template overload.
    IMAGEPP_EXPORT const double*           GetXYZPoints                           (uint32_t*                             po_pNumberPoints) const;

    IMAGEPP_EXPORT void                     GetNumberOfFilteredPoints              (uint64_t*                            po_pNumberOfPoints) const;

    IMAGEPP_EXPORT void                     GetFilteredDimensionInPixels           (uint64_t*                            po_pWidthInPixels,
                                                                            uint64_t*                            po_pHeightInPixels) const;

    IMAGEPP_EXPORT bool                    IsDestCoordSysCreationFailed           () const;

    // NTERAY: May rename to Next once Next is removed.
    IMAGEPP_EXPORT bool                    NextBlock();

    // Deprecated. Use NextBlock.
    IMAGEPP_EXPORT void                     Next();

    IMAGEPP_EXPORT void                     Reset();

private :
    friend class                    HUTDEMRasterXYZPointsExtractor;

    struct                          Impl;

    static HUTDEMRasterXYZPointsIterator*
    CreateFor                              (HUTDEMRasterXYZPointsExtractor&    pi_rExtractor,
                                            const WString&                     pi_rDestCoordSysKeyName,
                                            double                             pi_ScaleFactor);

    explicit                        HUTDEMRasterXYZPointsIterator          (Impl*                              pi_pImpl,
                                                                            const WString&                     pi_rDestCoordSysKeyName,
                                                                            double                             pi_ScaleFactor);


    void                            Init                                   (HUTDEMRasterXYZPointsExtractor&    pi_rExtractor,
                                                                            const WString&                     pi_rDestCoordSysKeyName,
                                                                            double                             pi_ScaleFactor);

    uint32_t                       ComputeStripHeight                     () const;
    void                            InitStripForDebug                      ();
    HFCPtr<HGF2DTransfoModel>       ComputeTransfoModel                    () const;

    IMAGEPP_EXPORT size_t                   GetXYZPointsImpl                       (void*                               po_pPointsBuffer,
                                                                            size_t                              pi_CapacityInPoints) const;

    std::unique_ptr<Impl>             m_pImpl;

    //Reprojection related members
    bool                           m_IsDestCoordSysCreationFailed;
    IRasterBaseGcsPtr              m_pDestCoorSys;
    HFCPtr<HGF2DTransfoModel>      m_pReprojectionModel;

    HFCPtr<HRABitmap>              m_pStrip;
    mutable HAutoPtr<double>       m_pXYZPoints;
    double                         m_CurrentStripPosInSourceRasterPhyCS;
    double                         m_ScaleFactorX;
    double                         m_ScaleFactorY;
    uint64_t                       m_StripWidthInPixels;
    uint64_t                       m_FilteredHeightInPixels;
    double                         m_StripHeightInSourceRasterPhyCS;
    };


template <typename PointType>
size_t HUTDEMRasterXYZPointsIterator::GetXYZPoints (PointType* po_pPoints, size_t pi_Capacity) const
    {
    // Ensure that user has a correctly sized point type
    HSTATICASSERT(sizeof(PointType) == 3*sizeof(double));
    HPRECONDITION(pi_Capacity >= GetMaxXYZPointQty());

    return (pi_Capacity < GetMaxXYZPointQty()) ? 0 : GetXYZPointsImpl(po_pPoints, pi_Capacity);
    }
END_IMAGEPP_NAMESPACE

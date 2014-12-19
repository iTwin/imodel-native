//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HUTDEMRasterXYZPointsIterator.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HIMStripAdapter.h"
#include "HRABitmap.h"
#include "HRFRasterFile.h"



class HUTDEMRasterXYZPointsExtractor;

class HUTDEMRasterXYZPointsIterator
    {
public :


    // Deprecated. Factor from the extractor
    _HDLLg                          HUTDEMRasterXYZPointsIterator          (HUTDEMRasterXYZPointsExtractor*    pi_pRasterPointExtractor,
                                                                            const WString&                     pi_rDestCoordSysKeyName,
                                                                            double                             pi_ScaleFactor);

    _HDLLg virtual                  ~HUTDEMRasterXYZPointsIterator         ();

    _HDLLg size_t                   GetMaxXYZPointQty                      () const;

    template <typename PointType>
    size_t                          GetXYZPoints                           (PointType*                          po_pPoints,
                                                                            size_t                              pi_Capacity) const;

    // Deprecated. Use template overload.
    _HDLLg const double*           GetXYZPoints                           (uint32_t*                             po_pNumberPoints) const;

    _HDLLg void                     GetNumberOfFilteredPoints              (uint64_t*                            po_pNumberOfPoints) const;

    _HDLLg void                     GetFilteredDimensionInPixels           (uint64_t*                            po_pWidthInPixels,
                                                                            uint64_t*                            po_pHeightInPixels) const;

    _HDLLg bool                    IsDestCoordSysCreationFailed           () const;

    // NTERAY: May rename to Next once Next is removed.
    _HDLLg bool                    NextBlock();

    // Deprecated. Use NextBlock.
    _HDLLg void                     Next();

    _HDLLg void                     Reset();

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

    _HDLLg size_t                   GetXYZPointsImpl                       (void*                               po_pPointsBuffer,
                                                                            size_t                              pi_CapacityInPoints) const;

    std::auto_ptr<Impl>             m_pImpl;

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
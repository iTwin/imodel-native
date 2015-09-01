//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPieceWiseModel.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DLiteQuadrilateral.h>
#include <Imagepp/all/h/HGF2DTriangle.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    QuadrilateralFacet
    -----------------------------------------------------------------------------
*/
class QuadrilateralFacet
    {
public:
    QuadrilateralFacet ();
    QuadrilateralFacet (const HFCPtr<HGF2DTransfoModel>&      pi_rpModel,
                        const HGF2DLiteQuadrilateral&   pi_rShape);

    QuadrilateralFacet (const QuadrilateralFacet& pi_rObj);
    ~QuadrilateralFacet();

    QuadrilateralFacet&
    operator= (const QuadrilateralFacet& pi_rObj);

    HGF2DLiteExtent GetExtent() const;

    bool           IsPointIn(double pi_x, double pi_y) const;
    HGF2DLiteQuadrilateral
    GetQuadrilateral() const;

    HFCPtr<HGF2DTransfoModel>
    GetTransfoModel() const;

    void            Dump (ofstream& outStream) const;

private:
    HFCPtr<HGF2DTransfoModel> m_pModel;
    HGF2DLiteQuadrilateral    m_Shape;
    HGF2DLiteExtent           m_Extent;
    };

/** -----------------------------------------------------------------------------
    TriangleFacet
    -----------------------------------------------------------------------------
*/
class TriangleFacet
    {
public:
    TriangleFacet ();
    TriangleFacet (const HFCPtr<HGF2DTransfoModel>&      pi_rpModel,
                   const HGF2DTriangle&   pi_rShape);

    TriangleFacet (const TriangleFacet& pi_rObj);
    ~TriangleFacet();

    TriangleFacet&  operator= (const TriangleFacet& pi_rObj);

    HGF2DLiteExtent GetExtent() const;

    bool           IsPointIn(double pi_x, double pi_y) const;

    HFCPtr<HGF2DTransfoModel>
    GetTransfoModel() const;

    HGF2DTriangle    GetTriangle() const;

    void             Dump (ofstream& outStream) const;

private:
    HFCPtr<HGF2DTransfoModel> m_pModel;
    HGF2DTriangle             m_Shape;
    HGF2DLiteExtent           m_Extent;
    };


/** -----------------------------------------------------------------------------
    HGF2DPieceWiseModel
    -----------------------------------------------------------------------------
*/
class HGF2DPieceWiseModel : public HFCShareableObject<HGF2DPieceWiseModel>
    {
public:

    HGF2DPieceWiseModel();
    virtual                 ~HGF2DPieceWiseModel();

    virtual HFCPtr<HGF2DTransfoModel>
    GetModelFromCoordinate(double pi_X, double pi_Y) const = 0;
    virtual HFCPtr<HGF2DTransfoModel>
    GetModelFromInverseCoordinate(double pi_X, double pi_Y) const = 0;


    typedef enum
        {
        DIRECT,
        INVERSE
        } ChannelMode;

    static HGF2DLiteExtent  ScaleExtent (HGF2DLiteExtent const& pi_rExtent,
                                         double                 pi_Scale);

    static  HGF2DLiteExtent ConvertLiteExtent(const HGF2DLiteExtent&   pi_rExtent,
                                              const HGF2DTransfoModel& pi_rpModel,
                                              const ChannelMode        pi_channel);
    static HGF2DLiteExtent  ConvertLiteExtent(const HGF2DLiteExtent&   pi_rExtent,
                                              uint32_t                 nCols,
                                              uint32_t                 nRows,
                                              const HGF2DTransfoModel& pi_rpModel,
                                              const ChannelMode        pi_channel);
    };

END_IMAGEPP_NAMESPACE
#include <Imagepp/all/h/HGF2DPieceWiseModel.hpp>
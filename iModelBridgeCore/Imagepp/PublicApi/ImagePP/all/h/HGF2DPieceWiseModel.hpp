//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DPieceWiseModel.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// QuadrilateralFacet
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline QuadrilateralFacet::QuadrilateralFacet ()
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline QuadrilateralFacet::QuadrilateralFacet (const HFCPtr<HGF2DTransfoModel>& pi_rpModel,
                                               const HGF2DLiteQuadrilateral&    pi_rShape)
    : m_pModel(pi_rpModel),
      m_Shape(pi_rShape),
      m_Extent(pi_rShape.GetExtent())
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline QuadrilateralFacet::QuadrilateralFacet (const QuadrilateralFacet& pi_rObj)
    {
    m_pModel = pi_rObj.m_pModel;
    m_Shape  = pi_rObj.m_Shape;
    m_Extent = pi_rObj.m_Extent;
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline QuadrilateralFacet::~QuadrilateralFacet()
    {
    }

//-----------------------------------------------------------------------------
// operator=
//-----------------------------------------------------------------------------
inline QuadrilateralFacet& QuadrilateralFacet::operator= (const QuadrilateralFacet& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pModel = pi_rObj.m_pModel;
        m_Shape  = pi_rObj.m_Shape;
        m_Extent = pi_rObj.m_Extent;
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent QuadrilateralFacet::GetExtent() const
    {
    return m_Extent;
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
inline HGF2DLiteQuadrilateral QuadrilateralFacet::GetQuadrilateral() const
    {
    return m_Shape;
    }

//-----------------------------------------------------------------------------
// IsPointIn
//-----------------------------------------------------------------------------
inline bool QuadrilateralFacet::IsPointIn(double pi_x, double pi_y) const
    {
    return m_Shape.IsPointOuterIn(pi_x, pi_y);
    }

//-----------------------------------------------------------------------------
// GetTransfoModel
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> QuadrilateralFacet::GetTransfoModel () const
    {
    return m_pModel;
    }

//-----------------------------------------------------------------------------
// Dump
//-----------------------------------------------------------------------------
inline void QuadrilateralFacet::Dump (ofstream& outStream) const
    {
    double x0, y0, x1, y1, x2, y2, x3, y3;
    m_Shape.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2, &x3, &y3);

    outStream << "place smartline" << endl;
    outStream << "xy=" << x0 << "," << y0 << endl;
    outStream << "xy=" << x1 << "," << y1 << endl;
    outStream << "xy=" << x2 << "," << y2 << endl;
    outStream << "xy=" << x3 << "," << y3 << endl;
    outStream << "xy=" << x0 << "," << y0 << endl;
    }


//-----------------------------------------------------------------------------
// TriangleFacet
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline TriangleFacet::TriangleFacet ()
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline TriangleFacet::TriangleFacet (const HFCPtr<HGF2DTransfoModel>& pi_rpModel,
                                     const HGF2DTriangle&             pi_rShape)
    : m_pModel(pi_rpModel),
      m_Shape(pi_rShape),
      m_Extent(pi_rShape.GetExtent())
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline TriangleFacet::TriangleFacet (const TriangleFacet& pi_rObj)
    {
    m_pModel = pi_rObj.m_pModel;
    m_Shape  = pi_rObj.m_Shape;
    m_Extent = pi_rObj.m_Extent;
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline TriangleFacet::~TriangleFacet()
    {
    }

//-----------------------------------------------------------------------------
// operator=
//-----------------------------------------------------------------------------
inline TriangleFacet& TriangleFacet::operator= (const TriangleFacet& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pModel = pi_rObj.m_pModel;
        m_Shape  = pi_rObj.m_Shape;
        m_Extent = pi_rObj.m_Extent;
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent TriangleFacet::GetExtent() const
    {
    return m_Extent;
    }

//-----------------------------------------------------------------------------
// IsPointIn
//-----------------------------------------------------------------------------
inline bool TriangleFacet::IsPointIn(double pi_x, double pi_y) const
    {
    return m_Shape.IsPointOuterIn(pi_x, pi_y);
    }

//-----------------------------------------------------------------------------
// GetTransfoModel
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> TriangleFacet::GetTransfoModel () const
    {
    return m_pModel;
    }
//-----------------------------------------------------------------------------
// GetTransfoModel
//-----------------------------------------------------------------------------
inline HGF2DTriangle TriangleFacet::GetTriangle () const
    {
    return m_Shape;
    }

//-----------------------------------------------------------------------------
// Dump
//-----------------------------------------------------------------------------
inline void TriangleFacet::Dump (ofstream& outStream) const
    {
    double x0, y0, x1, y1, x2, y2;
    m_Shape.GetCorners(&x0, &y0, &x1, &y1, &x2, &y2);

    outStream << "place smartline" << endl;
    outStream << "xy=" << x0 << "," << y0 << endl;
    outStream << "xy=" << x1 << "," << y1 << endl;
    outStream << "xy=" << x2 << "," << y2 << endl;
    outStream << "xy=" << x0 << "," << y0 << endl;
    }

END_IMAGEPP_NAMESPACE
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HVE2DTriangleFacet.hpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include "HVE2DTriangleFacet.h"
#include "HGF2DProjective.h"



/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Facet to duplicate.
-----------------------------------------------------------------------------*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> HVE2DTriangleFacet<ATTRIBUTE>::HVE2DTriangleFacet(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rObj)
    : HVE2DFacet<ATTRIBUTE>(pi_rObj),
      m_FirstPoint(pi_rObj.m_FirstPoint),
      m_SecondPoint(pi_rObj.m_SecondPoint),
      m_ThirdPoint(pi_rObj.m_ThirdPoint)
    {
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying triangle only (using polygon)

 @param pi_rPolygon Polygon of segments that must constain exactly 3 segments
                    that must not be autocontiguous nor null in length.
                    the area of the triangle must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleFacet<ATTRIBUTE>::HVE2DTriangleFacet(const HVE2DPolygonOfSegments& pi_rPolygon)
    {
    // Make sure that the polygon has exactly 3 segments
    KPRECONDITION(pi_rPolygon.GetLinear().GetNumberOfLinears() == 3);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);

    HGF2DLocationCollection MyPoints;
    pi_rPolygon.Drop(&MyPoints, pi_rPolygon.GetTolerance());
    HASSERT(MyPoints.size() == 4);

    m_FirstPoint = MyPoints[0].GetPosition();
    m_SecondPoint = MyPoints[1].GetPosition();
    m_ThirdPoint = MyPoints[2].GetPosition();
    }

/**----------------------------------------------------------------------------
 Constructor for this class specifying triangle (using polygon) and atttribute

 @param pi_rPolygon Polygon of segments that must constain exactly 3 segments
                    that must not be autocontiguous nor null in length.
                    the area of the triangle must be non null.

 @param pi_rAttribute Attribute to assign to facet
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleFacet<ATTRIBUTE>::HVE2DTriangleFacet(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                                            const ATTRIBUTE& pi_rAttribute)
    {
    // Make sure that the polygon has exactly 3 segments
    HPRECONDITION(pi_rPolygon.GetLinear().GetNumberOfLinears() == 3);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);

    // Set attribute
    SetAttribute(pi_rAttribute);

    HGF2DLocationCollection MyPoints;
    pi_rPolygon.Drop(&MyPoints, pi_rPolygon.GetTolerance());
    HASSERT(MyPoints.size() == 4);

    m_FirstPoint = MyPoints[0].GetPosition();
    m_SecondPoint = MyPoints[1].GetPosition();
    m_ThirdPoint = MyPoints[2].GetPosition();

    }





/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleFacet<ATTRIBUTE>::~HVE2DTriangleFacet()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator

 @param pi_rObj Facet to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleFacet<ATTRIBUTE>& HVE2DTriangleFacet<ATTRIBUTE>::operator=(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_FirstPoint  = pi_rObj.m_FirstPoint ;
        m_SecondPoint = pi_rObj.m_SecondPoint;
        m_ThirdPoint  = pi_rObj.m_ThirdPoint ;
        HVE2DFacet<ATTRIBUTE>::operator=(pi_rObj);
        }

    return(*this);
    }





/**----------------------------------------------------------------------------
 This method returns a reference to the internal polygon of segments
 that represents the triangle.

 @return A constant reference to internal polygon of segments that contains
         the triangle.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DPolygonOfSegments& HVE2DTriangleFacet<ATTRIBUTE>::GetTriangle() const
    {
    return(static_cast<const HVE2DPolygonOfSegments&>(GetShape()));
    }

/**----------------------------------------------------------------------------
 This method sets the triangle of the facet.

 @param pi_rPolygon Polygon of segments that must constain exactly 3 segments
                    that must not be autocontiguous nor null in length.
                    the area of the triangle must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> void HVE2DTriangleFacet<ATTRIBUTE>::SetTriangle(const HVE2DPolygonOfSegments& pi_rPolygon)
    {
    // Make sure that the polygon has exactly 3 segments
    KPRECONDITION(pi_rPolygon.GetSize() == 4);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);
    }



/**----------------------------------------------------------------------------
 This method clones the facet

 @return A newly allocated triangle facet containing duplicates of all information and
         member. The new instance must be freed using delete when needed no more.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>* HVE2DTriangleFacet<ATTRIBUTE>::Clone() const
    {
    return(new HVE2DTriangleFacet<ATTRIBUTE>(*this));
    }



/**----------------------------------------------------------------------------
 This method returns the first summit

 @return The first summit
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> inline const HGF2DPosition& HVE2DTriangleFacet<ATTRIBUTE>::GetFirstSummit() const
    {
    return(m_FirstPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the second summit

 @return The second summit
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> inline const HGF2DPosition& HVE2DTriangleFacet<ATTRIBUTE>::GetSecondSummit() const
    {
    return(m_SecondPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the third summit

 @return The third summit
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> inline const HGF2DPosition& HVE2DTriangleFacet<ATTRIBUTE>::GetThirdSummit() const
    {
    return(m_ThirdPoint);
    }

/**----------------------------------------------------------------------------
 This method indicates if point is in facet

 @param i_rPoint The point to check if it is in triangle

 @return true if point is in, false otherwise
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DTriangleFacet<ATTRIBUTE>::IsPointIn(const HGF2DLocation& i_rPoint) const
    {
#if (0)
    return(GetShape().IsPointIn(i_rPoint));
#else
    return(IsPointIn(i_rPoint.GetPosition()));
#endif
    }


/**----------------------------------------------------------------------------
 This method indicates if point is in facet

 @param i_rPoint The point to check if it is in triangle

 @return true if point is in, false otherwise
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DTriangleFacet<ATTRIBUTE>::IsPointIn(const HGF2DPosition& i_rPoint) const
    {
    bool IsIn = false;

    double x  = i_rPoint.GetX();
    double y  = i_rPoint.GetY();
    double x1 = m_FirstPoint.GetX();
    double y1 = m_FirstPoint.GetY();

    double x2 = m_SecondPoint.GetX();
    double y2 = m_SecondPoint.GetY();

    double x3 = m_ThirdPoint.GetX();
    double y3 = m_ThirdPoint.GetY();


    double Det1 = (y - y1) * (x2 - x1) - (x - x1) * (y2 - y1);
    double Det2 = (y - y2) * (x3 - x2) - (x - x2) * (y3 - y2);
    double Det3 = (y - y3) * (x1 - x3) - (x - x3) * (y1 - y3);


#if (0)
    if (HNumeric<double>::EQUAL_EPSILON(Det1, 0.0) ||
        HNumeric<double>::EQUAL_EPSILON(Det2, 0.0) ||
        HNumeric<double>::EQUAL_EPSILON(Det3, 0.0))
        {
        // Point is on boundary
        IsIn = true;
        }
    else
#endif
        if (Det1 > 0.0 && Det2 > 0.0 && Det3 > 0.0)
            {
            IsIn = true;
            }
        else if (Det1 < 0.0 && Det2 < 0.0 && Det3 < 0.0)
            {
            IsIn = true;
            }

    // The point must be inside if it is declared in
    HASSERT(!IsIn || (IsIn && (GetShape().IsPointIn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())) ||
                               GetShape().IsPointOn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())))));

#if (0)
    return(GetShape().IsPointIn(HGF2DLocation(i_rPoint, GetShape().GetCoordSys())));
#else
    return(IsIn);
#endif

    }




/**----------------------------------------------------------------------------
 This method allocates a transformed coy of the facet

 @param i_rTransformMatrix A 2D transformation matrix that contains the
                           transformation to apply to copy. The determinant of
                           the matrix may not be null.

 @return A newly allocated facet containing duplicates of all information and
         member. The new instance must be freed using delete when needed no more.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>* HVE2DTriangleFacet<ATTRIBUTE>::AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const
    {
    HVE2DFacet<ATTRIBUTE>* pResultFacet = 0;

    // Create projective transformation model based upon the matrix
    HGF2DProjective MyModel(i_rTransformMatrix);

    // Create dumb coordinate system with relation to current coordinate system
    HFCPtr<HGF2DCoordSys> pDumbCoordSys = new HGF2DCoordSys(MyModel, GetShape().GetCoordSys());

    // Allocate transformed copy of shape
    HFCPtr<HVE2DShape> pNewShape = static_cast<HVE2DShape*>(GetShape().AllocateCopyInCoordSys(pDumbCoordSys));

    // Change coordinate system
    pNewShape->SetCoordSys(GetShape().GetCoordSys());

    // Make sure that the shape is simple
    // and that it is a polygon of segments (sometimes, the transformation eliminates the shape)
    if (pNewShape->IsSimple() && pNewShape->IsCompatibleWith(HVE2DPolygonOfSegments::CLASS_ID))
        {
        // Cast a reference into a polygon of segments
        HVE2DPolygonOfSegments& rPolygon = static_cast<HVE2DPolygonOfSegments&>(*pNewShape);

        // Make sure that the polygon has exactly 3 segments
        HASSERT(rPolygon.GetLinear().GetNumberOfLinears() == 3);

        // Create new triangle facet
        pResultFacet = new HVE2DTriangleFacet<ATTRIBUTE>(rPolygon, GetAttribute());
        }

    return(pResultFacet);
    }
END_IMAGEPP_NAMESPACE




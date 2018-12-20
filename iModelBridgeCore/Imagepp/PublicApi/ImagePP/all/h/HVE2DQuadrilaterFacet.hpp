//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DQuadrilaterFacet.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HVE2DQuadrilaterFacet.h"


BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Facet to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::HVE2DQuadrilaterFacet(const HVE2DQuadrilaterFacet<ATTRIBUTE>& pi_rObj)
    : HVE2DFacet<ATTRIBUTE>(pi_rObj)
    {
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying quadrilater only (using polygon)

 @param pi_rPolygon Polygon of segments that must constain exactly 4 segments
                    that must not be autocontiguous nor null in length.
                    the area of the quadrilater must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::HVE2DQuadrilaterFacet(const HVE2DPolygonOfSegments& pi_rPolygon)
    {
    // Make sure that the polygon has exactly 4 segments
    HPRECONDITION(pi_rPolygon.GetSize() == 5);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);
    }

/**----------------------------------------------------------------------------
 Constructor for this class specifying quadrilater (using polgon) and atttribute

 @param pi_rPolygon Polygon of segments that must constain exactly 4 segments
                    that must not be autocontiguous nor null in length.
                    the area of the quadrilater must be non null.

 @param pi_rAttribute Attribute to assign to facet
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::HVE2DQuadrilaterFacet(const HVE2DPolygonOfSegments& pi_rPolygon,
        const ATTRIBUTE& pi_rAttribute)
    {
    // Make sure that the polygon has exactly 4 segments
    HPRECONDITION(pi_rPolygon.GetSize() == 5);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);

    // Set attribute
    SetAttribute(pi_rAttribute);
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying quadrilater only (using rectangle)

 @param pi_rRectangle Rectangle specifying the quadrilater of the facet.
                    the area of the quadrilater must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::HVE2DQuadrilaterFacet(const HVE2DRectangle& pi_rRectangle)
    {
    // Convert to polygon of segment then set
    SetShape(HVE2DPolygonOfSegments(pi_rRectangle));
    }


/**----------------------------------------------------------------------------
 Constructor for this class specifying quadrilater (using rectangle) and attribute

 @param pi_rRectangle Rectangle specifying the quadrilater of the facet.
                    the area of the quadrilater must be non null.

 @param pi_rAttribute Attribute to assign to facet
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::HVE2DQuadrilaterFacet(const HVE2DRectangle& pi_rRectangle,
        const ATTRIBUTE& pi_rAttribute)
    {
    // Convert to polygon of segment then set
    SetShape(HVE2DPolygonOfSegments(pi_rRectangle));

    // Set the attribute
    SetAttribute(pi_rAttribute);
    }


/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DQuadrilaterFacet<ATTRIBUTE>::~HVE2DQuadrilaterFacet()
    {
    }



/**----------------------------------------------------------------------------
 This method returns a reference to the internal polygon of segments
 that represents the quadrilater.

 @return A constant reference to internal polygon of segments that contains
         the quadrilater.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DPolygonOfSegments& HVE2DQuadrilaterFacet<ATTRIBUTE>::GetQuadrilater() const
    {
    return(static_cast<const HVE2DPolygonOfSegments&>(GetShape()));
    }

/**----------------------------------------------------------------------------
 This method sets the quadrilater of the facet.

 @param pi_rPolygon Polygon of segments that must constain exactly 4 segments
                    that must not be autocontiguous nor null in length.
                    the area of the quadrilater must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> void HVE2DQuadrilaterFacet<ATTRIBUTE>::SetQuadrilater(const HVE2DPolygonOfSegments& pi_rPolygon)
    {
    // Make sure that the polygon has exactly 4 segments
    HPRECONDITION(pi_rPolygon.GetSize() == 5);

    // Now that the nature has been validated ... set it
    SetShape(pi_rPolygon);
    }

/**----------------------------------------------------------------------------
 This method sets the quadrilater of the facet using a rectangle.

 @param pi_rRectangle Rectangle specifying the quadrilater of the facet.
                    the area of the quadrilater must be non null.

-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> void HVE2DQuadrilaterFacet<ATTRIBUTE>::SetQuadrilater(const HVE2DRectangle& pi_rRectangle)
    {
    // Convert to polygon of segment then set
    SetShape(HVE2DPolygonOfSegments(pi_rRectangle));
    }

/**----------------------------------------------------------------------------
 This method clones the facet

 @return A newly allocated quadrilater facet containing duplicates of all information and
         member. The new instance must be freed using delete when needed no more.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>* HVE2DQuadrilaterFacet<ATTRIBUTE>::Clone() const
    {
    return(new HVE2DQuadrilaterFacet<ATTRIBUTE>(*this));
    }

END_IMAGEPP_NAMESPACE







//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DTriangleMesh.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Mesh to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleMesh<ATTRIBUTE>::HVE2DTriangleMesh(const HVE2DTriangleMesh<ATTRIBUTE>& pi_rObj)
    : HVE2DMesh<ATTRIBUTE>(pi_rObj)
    {
    }



/**----------------------------------------------------------------------------
 PRIVATE
 DefaultConstructor for this class
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleMesh<ATTRIBUTE>::HVE2DTriangleMesh()
    {
    }


/**----------------------------------------------------------------------------
 Constructor for this class
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleMesh<ATTRIBUTE>::HVE2DTriangleMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DMesh<ATTRIBUTE>(pi_rpCoordSys)
    {
    }


/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleMesh<ATTRIBUTE>::~HVE2DTriangleMesh()
    {
    }


/**----------------------------------------------------------------------------
 Assignement operator

 @param pi_rObj Mesh to copy from.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DTriangleMesh<ATTRIBUTE>& HVE2DTriangleMesh<ATTRIBUTE>::operator=(const HVE2DTriangleMesh<ATTRIBUTE>& pi_rObj)
    {
    HVE2DMesh<ATTRIBUTE>::operator=(pi_rObj);
    }



/**----------------------------------------------------------------------------
 This method adds a facet to the mesh.

 @param pi_rFacet Constant reference to facet to add to mesh. The facet
                  is duplicated. The new facet added must be located outside
                  all other facets located in the mesh, though these facets
                  can be contiguous by many points and to many facets at
                  the same time.

 @return true if the facet could be added and false otherwise.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DTriangleMesh<ATTRIBUTE>::AddFacet(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rFacet)
    {
    // Now that the facet type is validated (by compiler) we ask ancester to process
    return(HVE2DMesh<ATTRIBUTE>::AddFacet(pi_rFacet));
    }

/**----------------------------------------------------------------------------
 This method adds a facet to the mesh.

 @param i_rpFacet Constant reference to facet to add to mesh. The facet
                  is not duplicated. The new facet added must be located outside
                  all other facets located in the mesh, though these facets
                  can be contiguous by many points and to many facets at
                  the same time.

 @return true if the facet could be added and false otherwise.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DTriangleMesh<ATTRIBUTE>::AddFacet(HVE2DTriangleFacet<ATTRIBUTE>& i_rpFacet)
    {
    // Now that the facet type is validated (by compiler) we ask ancester to process
    return HVE2DMesh<ATTRIBUTE>::AddFacet(i_rpFacet);
    }




/**----------------------------------------------------------------------------
 This method returns a pointer to the internal facet located at this location

 @return A constant pointer to internal facet at the given location.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DFacet<ATTRIBUTE>* HVE2DTriangleMesh<ATTRIBUTE>::GetFacetAt(const HGF2DLocation& pi_rLocation) const
    {
    // Check if last hit defined
    if (m_pLastHit)
        {
        HGF2DPosition TestPoint = pi_rLocation.GetPosition();

        double x  = TestPoint.GetX();
        double y  = TestPoint.GetY();

        double Det1 = (y - m_y1) * (m_x2Mx1) - (x - m_x1) * (m_y2My1);
        double Det2 = (y - m_y2) * (m_x3Mx2) - (x - m_x2) * (m_y3My2);
        double Det3 = (y - m_y3) * (m_x1Mx3) - (x - m_x3) * (m_y1My3);

        bool IsIn = ((Det1 > 0.0 && Det2 > 0.0 && Det3 > 0.0) ||
                     (Det1 < 0.0 && Det2 < 0.0 && Det3 < 0.0));

        // The point must be inside if it is declared in
        HASSERT(!IsIn || (IsIn && (m_pLastHit->GetShape().IsPointIn(pi_rLocation) || m_pLastHit->GetShape().IsPointOn(pi_rLocation))));
//        HASSERT(IsIn == m_pLastHit->GetShape().IsPointIn(pi_rLocation));

        if (!IsIn)
            m_pLastHit = 0;

        }

    if (!m_pLastHit)
        {
        HVE2DMesh<ATTRIBUTE>::GetFacetAt(pi_rLocation);

        // If a facet was found
        if (m_pLastHit)
            {
            // Update acceleration attributes

            HGF2DPosition Point1(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetFirstSummit());
            HGF2DPosition Point2(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetSecondSummit());
            HGF2DPosition Point3(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetThirdSummit());

            m_x1 = Point1.GetX();
            m_y1 = Point1.GetY();

            m_x2 = Point2.GetX();
            m_y2 = Point2.GetY();

            m_x3 = Point3.GetX();
            m_y3 = Point3.GetY();

            m_x2Mx1 = (m_x2 - m_x1);
            m_x3Mx2 = (m_x3 - m_x2);
            m_x1Mx3 = (m_x1 - m_x3);

            m_y2My1 = (m_y2 - m_y1);
            m_y3My2 = (m_y3 - m_y2);
            m_y1My3 = (m_y1 - m_y3);
            }
        }

    return(static_cast<HVE2DFacet<ATTRIBUTE> *>(&(*m_pLastHit)));
    }


/**----------------------------------------------------------------------------
 This method returns a pointer to the internal facet located at this location

 @return A constant pointer to internal facet at the given location.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DFacet<ATTRIBUTE>* HVE2DTriangleMesh<ATTRIBUTE>::GetFacetAt(const HGF2DPosition& i_rPosition) const
    {
    // Check if last hit defined
    if (m_pLastHit)
        {
        double x  = i_rPosition.GetX();
        double y  = i_rPosition.GetY();

        double Det1 = (y - m_y1) * (m_x2Mx1) - (x - m_x1) * (m_y2My1);
        double Det2 = (y - m_y2) * (m_x3Mx2) - (x - m_x2) * (m_y3My2);
        double Det3 = (y - m_y3) * (m_x1Mx3) - (x - m_x3) * (m_y1My3);

        bool IsIn = ((Det1 > 0.0 && Det2 > 0.0 && Det3 > 0.0) ||
                     (Det1 < 0.0 && Det2 < 0.0 && Det3 < 0.0));

        // The point must be inside if it is declared in
//        HASSERT(!IsIn || (IsIn && (m_pLastHit->GetShape().IsPointIn(HGF2DLocation(i_rPosition, GetCoordSys())) ||
//                                   m_pLastHit->GetShape().IsPointOn(HGF2DLocation(i_rPosition, GetCoordSys())))));

        if (!IsIn)
            m_pLastHit = 0;

        }

    if (!m_pLastHit)
        {
        HVE2DMesh<ATTRIBUTE>::GetFacetAt(i_rPosition);

        // If a facet was found
        if (m_pLastHit)
            {
            // Update acceleration attributes

            HGF2DPosition Point1(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetFirstSummit());
            HGF2DPosition Point2(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetSecondSummit());
            HGF2DPosition Point3(static_cast<HVE2DTriangleFacet<ATTRIBUTE> *>(&(*(m_pLastHit)))->GetThirdSummit());

            m_x1 = Point1.GetX();
            m_y1 = Point1.GetY();

            m_x2 = Point2.GetX();
            m_y2 = Point2.GetY();

            m_x3 = Point3.GetX();
            m_y3 = Point3.GetY();

            m_x2Mx1 = (m_x2 - m_x1);
            m_x3Mx2 = (m_x3 - m_x2);
            m_x1Mx3 = (m_x1 - m_x3);

            m_y2My1 = (m_y2 - m_y1);
            m_y3My2 = (m_y3 - m_y2);
            m_y1My3 = (m_y1 - m_y3);
            }
        }

    return(static_cast<HVE2DFacet<ATTRIBUTE> *>(&(*m_pLastHit)));
    }

END_IMAGEPP_NAMESPACE

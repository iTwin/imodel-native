//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DMesh.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 This method returns the extent

 @return The extent of all mesh
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HGF2DExtent HVE2DMesh<ATTRIBUTE>::GetExtent() const
    {
#if (0)
    // Declare temp extent
    HGF2DExtent TotalExtent(GetCoordSys());
#else
    HGF2DExtent TotalExtent;
#endif

    // Obtain list of facets
    HAutoPtr<ListOfFacets::ObjectList> pListOfFacets(m_ListOfFacets.Query(HIDXSearchCriteria((uint32_t)&m_ListOfFacets,
                                                                          new HIDXCriteria())));

    ListOfFacets::ObjectList::iterator Itr;

    for (Itr = pListOfFacets->begin(); Itr != pListOfFacets->end() ; ++Itr)
        {

        TotalExtent.Add((*Itr)->GetShape().GetExtent());
        }


    return(TotalExtent);
    }



/**----------------------------------------------------------------------------
 PRIVATE
 Default Constructor for this class. The default constructor allocates
 a new coordinate system.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DMesh<ATTRIBUTE>::HVE2DMesh()
    : m_ListOfFacets(new HGF2DCoordSys()),
      m_ExtentUpToDate(false),
      m_TotalShapeUpToDate(false),
      m_pTotalShape(0),
      m_pLastHit(0),
      m_ValidateFacetAddition(true)
    {
    }

/**----------------------------------------------------------------------------
 Constructor using coordinate system only for this class
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DMesh<ATTRIBUTE>::HVE2DMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_ListOfFacets(pi_rpCoordSys),
      m_pLastHit(0),
      m_ExtentUpToDate(false),
      m_TotalShapeUpToDate(false),
      m_pTotalShape(0),
      m_ValidateFacetAddition(true)
    {
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DMesh<ATTRIBUTE>::~HVE2DMesh()
    {
    m_pLastHit = 0;
    }


/**----------------------------------------------------------------------------
    SetFacetValidation

    This method sets the facet validation active or inactive depending
    on provided value.

    @param i_ActiveValidation - If true, then validation of facets addition
                                will be performed, otherwise the facet validation
                                will not be performed.


-----------------------------------------------------------------------------*/
template<class ATTRIBUTE>  void HVE2DMesh<ATTRIBUTE>::SetFacetValidation(bool i_ActiveValidation)
    {
    m_ValidateFacetAddition = i_ActiveValidation;
    }

/**----------------------------------------------------------------------------
    GetFacetValidation

    This method gets the current facet validation active or inactive setting

    @return - Returns true, then validation of facets addition
              is performed, otherwise the facet validation
              is not performed.


-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> bool HVE2DMesh<ATTRIBUTE>::GetFacetValidation() const
    {
    return(m_ValidateFacetAddition);
    }


#if (0)
/**----------------------------------------------------------------------------
 GetCoordSys
-----------------------------------------------------------------------------*/
inline const HFCPtr<HGF2DCoordSys>& HVE2DRawMesh::GetCoordSys() const
    {
    return(m_pCoordSys);
    }



/**----------------------------------------------------------------------------
 This method returns the extent of the mesh

 @return The extent of the mesh. If the mesh is empty, an empty extent
         is returned (usually 0,0,0,0)
-----------------------------------------------------------------------------*/
inline const HGF2DLiteExtent& HVE2DRawMesh::GetLiteExtent() const
    {
    if (!m_ExtentUpToDate)
        {
        GetExtent();

        HASSERT(m_ExtentUpToDate);
        }

    return(m_LiteExtent);
    }

#endif


/**----------------------------------------------------------------------------
 This method returns a pointer to the internal facet located at this location

 @return A constant pointer to internal facet at the given location.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DFacet<ATTRIBUTE>* HVE2DMesh<ATTRIBUTE>::GetFacetAt(const HGF2DLocation& pi_rLocation) const
    {
#if (0)
    // Check if last hit defined
    // anbd point is in last hit ...
    if (!m_pLastHit || !(m_pLastHit->IsPointIn(pi_rLocation)))
        {
        // Re-initialize
        m_pLastHit = 0;
#endif

        // Create extent for query
        double HalfEpsilon = HNumeric<double>::GLOBAL_EPSILON() / 2;

        // The query is performed on an area of an EPSILON in size
        HGF2DExtent LocationExtent(pi_rLocation.GetX() - HalfEpsilon,
                                   pi_rLocation.GetY() - HalfEpsilon,
                                   pi_rLocation.GetX() + HalfEpsilon,
                                   pi_rLocation.GetY() + HalfEpsilon,
                                   pi_rLocation.GetCoordSys());


        // Perform the spatial query
        HAutoPtr<ListOfFacets::ObjectList> pListOfFacets(m_ListOfFacets.Query(HIDXSearchCriteria(&m_ListOfFacets,
                                                                              new HGFSpatialCriteria(LocationExtent))));

        // Note that the query may return many facets when the location is located at a multiply
        // shared facet boundaries.

        // return the first obtained
        return(pListOfFacets->size() > 0 ? (*pListOfFacets->begin()) : 0);
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
    template<class ATTRIBUTE> bool HVE2DMesh<ATTRIBUTE>::AddFacet(const HVE2DFacet<ATTRIBUTE>& pi_rFacet)
        {
        // Re-init extent
        m_ExtentUpToDate = false;

        // Check if this facets is located outside all other facets
        // Obtain the facet shape
        const HVE2DShape& rFacetShape = pi_rFacet.GetShape();

        // Obtain the list of facets that may interact with new facet (spatially)
        HAutoPtr<ListOfFacets::ObjectList> pListOfFacets(m_ListOfFacets.Query(HIDXSearchCriteria((uint32_t)&m_ListOfFacets,
                                                                              new HGFSpatialCriteria(rFacetShape.GetExtent()))));

        // On this list ... process
        ListOfFacets::ObjectList::const_iterator Itr;
        bool IsOut = true;
        // For all facets
        for(Itr = pListOfFacets->begin() ; IsOut && Itr != pListOfFacets->end() ; ++Itr)
            {
            // Check if shape of facet to add is outside shape of current facet
            IsOut = (rFacetShape.CalculateSpatialPositionOf((*Itr)->GetShape()) == HVE2DShape::S_OUT) &&
                    ((*Itr)->GetShape().CalculateSpatialPositionOf(rFacetShape) == HVE2DShape::S_OUT);
            }

        // If the facet shape is located outside all other facets ... add it
        if (IsOut)
            m_ListOfFacets.Add(pi_rFacet.Clone());

        // Return true if facet could be added.
        return(IsOut);
        }

#if (0)
    /**----------------------------------------------------------------------------
     This method returns the extent of the mesh

     @return The extent of the mesh. If the mesh is empty, an empty extent
             is returned (usually 0,0,0,0)
    -----------------------------------------------------------------------------*/
    const HGF2DExtent& HVE2DMesh<ATTRIBUTE>::GetExtent() const
        {
        if (!m_ExtentUpToDate)
            {
            // Re-init extent
            m_Extent = HGF2DExtent(GetCoordSys());

            BaseListOfFacets::const_iterator Itr;
            for (Itr = m_BaseListOfFacets.begin() ; Itr != m_BaseListOfFacets.end() ; ++Itr)
                {
                m_Extent.Add((*Itr)->GetShape().GetExtent());
                }

            // Check if the result extent is defined (if there are no triangles then the extent will not be defined)
            // and if defeined then set the lite extent accordingly ... otherwise leave lite extent
            // uninitialized ...
            if (m_Extent.IsDefined())
                {
                m_LiteExtent.Set(m_Extent.GetXMin(),
                                 m_Extent.GetYMin(),
                                 m_Extent.GetYMax(),
                                 m_Extent.GetYMax());
                }

            m_ExtentUpToDate = true;
            }

        return(m_Extent);
        }



    /**----------------------------------------------------------------------------
     This method returns a pointer to the indicated facet

     @return Pointer to facet
    -----------------------------------------------------------------------------*/
    const HVE2DFacet<ATTRIBUTE>* HVE2DRawMesh::GetFacet(size_t i_Index) const
        {
        HPRECONDITION(i_Index < m_BaseListOfFacets.size());

        return(m_BaseListOfFacets[i_Index]);
        }

    /**----------------------------------------------------------------------------
     This method returns a pointer to the indicated facet (not const)

     @return Pointer to facet
    -----------------------------------------------------------------------------*/
    HVE2DFacet<ATTRIBUTE>* HVE2DRawMesh::GetFacet(size_t i_Index)
        {
        HPRECONDITION(i_Index < m_BaseListOfFacets.size());

        return(m_BaseListOfFacets[i_Index]);
        }

    /**----------------------------------------------------------------------------
     This method returns a pointer to the indicated facet (not const)

     @return Pointer to facet
    -----------------------------------------------------------------------------*/
    long HVE2DMesh<ATTRIBUTE>::CountFacets() const
        {
        return(m_BaseListOfFacets.size());
        }


    /**----------------------------------------------------------------------------
     This method returns the total shape of DTM

     @return The total shape of all triangle part of the TIN
    -----------------------------------------------------------------------------*/
    const HVE2DShape& HVE2DRawMesh::GetShape() const
        {
        if (!m_TotalShapeUpToDate)
            {
            // Intialise total shape to void
            m_pTotalShape = new HVE2DVoidShape(GetCoordSys());

            // Declare recipient list
            list<HVE2DFacet<ATTRIBUTE>* > MyList;

            // Obtain facets near extent of given facet
            if (m_AltListOfFacets.GetAll(MyList) > 0)
                {
                // For each and every one of the facets found
                list<HVE2DFacet<ATTRIBUTE>* >::iterator MyItr;
                for(MyItr = MyList.begin() ; MyItr != MyList.end() ; ++MyItr)
                    {
                    m_pTotalShape = m_pTotalShape->UnifyShape((*MyItr)->GetShape());
                    }
                }

            m_TotalShapeUpToDate = true;
            }

        return(*m_pTotalShape);
        }




#endif





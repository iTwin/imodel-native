//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSpatialIndex.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE



/**----------------------------------------------------------------------------
 Constructor for this class. The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::HGF2DSpatialIndexBase(size_t pi_SplitTreshold)
    : m_SplitTreshold(pi_SplitTreshold),
      m_HasMaxExtent(false)
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Constructor for this class. This is the constructor that limits the outter
 growth of the spatial index. An initial extent is provided and
 all items added in the spatial index must fall completely inside this extent.
 The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_MaxExtent IN The maximum extent of the spatial index.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE>
HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::HGF2DSpatialIndexBase(EXTENT const& pi_MaxExtent, size_t pi_SplitTreshold)
    : m_SplitTreshold(pi_SplitTreshold),
      m_HasMaxExtent(true),
      m_MaxExtent(i_MaxExtent)
    {
    // In this particular case, the root node is created immediately
    m_pRootNode = new NODE(m_SplitTreshold, pi_MaxExtent);

    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::HGF2DSpatialIndexBase(const HGF2DSpatialIndexBase& pi_rObj)
    : m_RootNode(pi_rObj.m_RootNode)
    {
    m_LastNode = 0;
    m_SplitTreshold = pi_rObj.m_SplitTreshold;
    m_MaxExtent = pi_rObj.m_MaxExtent;
    m_HasMaxExtent = pi_rObj.m_HaxMaxExtent;

    HINVARIANTS;
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::~HGF2DSpatialIndexBase()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator
 Alike the copy constructor, the nodes are duplicated while the spatial objects
 are not, but only their reference.
 IMPORTANT!: Take note that the previously indexed spatial objects are removed
 prior to copy of assigned spatial index.

 @param pi_rObj IN The spatial index to copy content from

 @return Reference to spatial index
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> HGF2DSpatialIndexBase<COORD, EXTENT, NODE>&
HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::operator=(const HGF2DSpatialIndexBase& pi_rObj)
    {
    HINVARIANTS;

    if (this != &pi_rObj)
        {
        m_RootNode = pi_rObj.m_RootNode;
        m_LastNode = 0;
        m_SplitTreshold = pi_rObj.m_SplitTreshold;
        m_HasMaxExtent = pi_rObj.m_HasMaxExtent;
        m_MaxExtent = pi_rObj.m_MaxExtent;
        }

    return(*this);
    }




/**----------------------------------------------------------------------------
 PROTECTED
 Returns the split treshold value for index. This treshold value is the
 maximum number of items idexed by a single node before splitting occurs.

 @return The treshold value.

-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> size_t HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::GetSplitTreshold() const
    {
    HINVARIANTS;

    return(m_SplitTreshold);
    }

/**----------------------------------------------------------------------------
 PROTECTED
 Sets the split treshold value for index. This treshold value is the maximum
 number of items indexed by a single node before splitting occurs.

 @param pi_SplitTreshold IN The new treshold value. This value may not be less than
 the minimum of 2.
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> void HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::SetSplitTreshold(size_t pi_SplitTreshold)
    {
    HINVARIANTS;
    HPRECONDITION(pi_SplitTreshold >= 2);

    m_SplitTreshold = pi_SplitTreshold;

    // Check if there is a root node
    if (m_pRootNode != 0)
        {
        // There is a root node ... we set the treshold
        m_pRootNode->SetSplitTreshold(m_SplitTreshold);
        }

    }


/**----------------------------------------------------------------------------
 PROTECTED
 Gets the limiting outter extent (maximum extent) if the spatial index
 was created with a limiting maximum extent. Prior to calling this method
 It must be verified that the spatial effectively has a maximum extent using
 the HasMaxExtent() method.

 @return Returns the maximum extent of spatial index.
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> EXTENT HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::GetMaxExtent() const
    {
    HPRECONDITION (HasMaxExtent());

    return m_MaxExtent;
    }


/**----------------------------------------------------------------------------
 PROTECTED
 Indicates if the index has a limiting outter extent (maximum extent).
 In order to have a limited extent the spatial index must have been created with
 a limiting maximum extent.

 @return true if the spatial index has a maximum extent.
-----------------------------------------------------------------------------*/
template<class COORD, class EXTENT, class NODE> bool HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::HasMaxExtent() const
    {
    return m_HasMaxExtent;
    }







#if (0)
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

/**----------------------------------------------------------------------------
 Constructor for this class. The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::HGF2DSpatialPtrIndex(size_t pi_SplitTreshold)
    : HGF2DSpatiaIndexBase (pi_SplitTreshold),
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Constructor for this class. This is the constructor that limits the outter
 growth of the spatial index. An initial extent is provided and
 all items added in the spatial index must fall completely inside this extent.
 The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_MaxExtent IN The maximum extent of the spatial index.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE>
HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::HGF2DSpatialPtrIndex(EXTENT const& pi_MaxExtent, size_t pi_SplitTreshold)
    : HGF2DSpatialIndexBase(pi_MaxExtent, pi_SplitTreshold),
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::HGF2DSpatialPtrIndex(const HGF2DSpatialPtrIndex& pi_rObj)
    : HGF2DSpatialIndexBase (pi_rObj)
    {
    HINVARIANTS;
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::~HGF2DSpatialPtrIndex()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator
 Alike the copy constructor, the nodes are duplicated while the spatial objects
 are not, but only their reference.
 IMPORTANT!: Take note that the previously indexed spatial objects are removed
 prior to copy of assigned spatial index.

 @param pi_rObj IN The spatial index to copy content from

 @return Reference to spatial index
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>&
HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::operator=(const HGF2DSpatialPtrIndex& pi_rObj)
    {
    HINVARIANTS;

    if (this != &pi_rObj)
        {
        HGF2DSpatialIndexBase::operator= (pi_rObj);
        }

    return(*this);
    }


/**----------------------------------------------------------------------------
 This method adds a spatial object reference in the spatial index

 @param pi_rpSpatialObject IN A smart pointer to spatial object to index.
 No check are performed to verify that the spatial object is already indexed or not.
 If the spatial object is already indexed then it will be indexed twice.

 @return true if item is added and false otherwise. It is possible to refuse addition
 of an item if the spatial index is limited in size (HasMaxExtent()) and the
 item extent does not completely lie within this maximum extent (GetMaxExtent()).

-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> bool HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::AddItem(const SPATIAL_PTR& pi_rpSpatialObject)
    {
    HINVARIANTS;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        m_pRootNode = new NODE(m_SplitTreshold, HGFIndexGeometryOpProvider<SPATIAL>::ExtentExtract(pi_rpSpatialObject));
        }

    // Make sure that the node extent contains the object extent
    while (!((m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL>::ExtentExtract(pi_rpSpatialObject).GetOrigin())) &&
             (m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL>::ExtentExtract(pi_rpSpatialObject).GetCorner()))))
        {
        // If the item is not in root node and extent is limited then it is impossible to add item
        if (m_HasMaxExtent)
            return false;

        // The extent is not contained... we must create a new node
        PushRootDown(HGFIndexGeometryOpProvider<SPATIAL>::ExtentExtract(pi_rpSpatialObject));
        }

    // The root node contains the spatial object ... add it
    m_pRootNode->AddItem(pi_rpSpatialObject);

    return true;
    }


/**----------------------------------------------------------------------------
 This method removes a spatial object reference from the spatial index.

 Notice that if the object is indexed many times in the spatial index then it
 will only be removed once.

 @param pi_rpSpatialObject IN Pointer to spatial index to remove.

 @return true if the object was removed and false otherwise
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> bool HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::RemoveItem(const SPATIAL_PTR& pi_rpSpatialObject)
    {
    HINVARIANTS;

    bool Removed = false;

    // Check if initial node allocated
    if (m_pRootNode != 0)
        {
        // The root node present ... remove
        Removed = m_pRootNode->Remove(pi_rpSpatialObject);
        }

    return(Removed);
    }










/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the neighborhood of given coordinate
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> size_t HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::GetAt(const COORD& pi_rCoord,
        list<SPATIAL_PTR>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the given extent
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> size_t HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::GetIn(const EXTENT& pi_rExtent,
        list<SPATIAL_PTR>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of all pointer to all spatial objects that are located in
 the index. These objects are returned somewhat ordered spatially
-----------------------------------------------------------------------------*/
template<class SPATIAL_PTR, class SPATIALFINDER, class NODE> size_t HGF2DSpatialPtrIndex<SPATIAL_PTR, SPATIALFINDER, NODE>::GetAll(list<SPATIAL_PTR>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    if (m_pRootNode != 0)
        m_pRootNode->GetIn(m_pRootNode->GetExtent(), pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

#endif

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGF2DSpatialIndex Class

/**----------------------------------------------------------------------------
 Constructor for this class. The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DSpatialIndex(size_t pi_SplitTreshold)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_SplitTreshold)
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Constructor for this class. This is the constructor that limits the outter
 growth of the spatial index. An initial extent is provided and
 all items added in the spatial index must fall completely inside this extent.
 The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_MaxExtent IN The maximum extent of the spatial index.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE>
HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DSpatialIndex(EXTENT const& pi_MaxExtent, size_t pi_SplitTreshold)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_MaxExtent, pi_SplitTreshold),
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DSpatialIndex(const HGF2DSpatialIndex& pi_rObj)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_rObj)
    {
    HINVARIANTS;
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::~HGF2DSpatialIndex()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator
 Alike the copy constructor, the nodes are duplicated while the spatial objects
 are not, but only their reference.
 IMPORTANT!: Take note that the previously indexed spatial objects are removed
 prior to copy of assigned spatial index.

 @param pi_rObj IN The spatial index to copy content from

 @return Reference to spatial index
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>&
HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::operator=(const HGF2DSpatialIndex& pi_rObj)
    {
    HINVARIANTS;

    if (this != &pi_rObj)
        {
        HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::operator= (pi_rObj);
        }

    return(*this);
    }


/**----------------------------------------------------------------------------
 This method adds a spatial object reference in the spatial index

 @param pi_rpSpatialObject IN A smart pointer to spatial object to index.
 No check are performed to verify that the spatial object is already indexed or not.
 If the spatial object is already indexed then it will be indexed twice.

 @return true if item is added and false otherwise. It is possible to refuse addition
 of an item if the spatial index is limited in size (HasMaxExtent()) and the
 item extent does not completely lie within this maximum extent (GetMaxExtent()).

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> bool HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::AddItem(const SPATIAL pi_rpSpatialObject)
    {
    HINVARIANTS;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_HasMaxExtent)
            m_pRootNode = new NODE(m_SplitTreshold, m_MaxExtent);
        else
            m_pRootNode = new NODE(m_SplitTreshold, HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    if (!m_pRootNode->AddItemConditional (pi_rpSpatialObject, m_HasMaxExtent))
        {
        // Item could not be added

        // Make sure that the node extent contains the object extent
        while (!((m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin())) &&
                 (m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
            }


        // The root node contains the spatial object ... add it
        m_pRootNode->AddItem(pi_rpSpatialObject);
        }

    return true;
    }


/**----------------------------------------------------------------------------
 This method removes a spatial object reference from the spatial index.

 Notice that if the object is indexed many times in the spatial index then it
 will only be removed once.

 @param pi_rpSpatialObject IN Pointer to spatial index to remove.

 @return true if the object was removed and false otherwise
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> bool HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::RemoveItem(const SPATIAL pi_rpSpatialObject)
    {
    HINVARIANTS;

    bool Removed = false;

    // Check if initial node allocated
    if (m_pRootNode != 0)
        {
        // The root node present ... remove
        Removed = m_pRootNode->Remove(pi_rpSpatialObject);
        }

    return(Removed);
    }




/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the neighborhood of given coordinate
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetAt(const COORD& pi_rCoord,
        list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the given extent
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetIn(const EXTENT& pi_rExtent,
        list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of all pointer to all spatial objects that are located in
 the index. These objects are returned somewhat ordered spatially
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetAll(list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    if (m_pRootNode != 0)
        m_pRootNode->GetIn(m_pRootNode->GetExtent(), pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 PRIVATE METHOD
 This method inserts a new root that contains the old root in order
 to attempt inclusion of given extent. The root is pushed down by one level.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> void HGF2DSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::PushRootDown(const EXTENT& pi_rObjectExtent)
    {
    HINVARIANTS;

    // The new node will be twice the sizes of current root node
    EXTENT RootExtent = m_pRootNode->GetExtent();

    // Calculate center of current root extent
    COORD RootExtentCenter((RootExtent.GetXMax() + RootExtent.GetXMin()) / 2.0,
                           (RootExtent.GetYMax() + RootExtent.GetYMin()) / 2.0);

    // Calculate center of spatial object
    COORD ObjectExtentCenter((pi_rObjectExtent.GetXMax() + pi_rObjectExtent.GetXMin()) / 2.0,
                             (pi_rObjectExtent.GetYMax() + pi_rObjectExtent.GetYMin()) / 2.0);


    // Find out what sub-node of new root will current root be
    // and create subnodes appropriately
    HFCPtr<NODE> aSubNodes[4];
    EXTENT NewRootExtent;
    if (ObjectExtentCenter.GetX() > RootExtentCenter.GetX())
        {
        if (ObjectExtentCenter.GetY() > RootExtentCenter.GetY())
            {
            // Oldroot becomes number 2
            NewRootExtent.Set(RootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMax() + RootExtent.GetWidth(), RootExtent.GetYMax() + RootExtent.GetHeight());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMax(), NewRootExtent.GetYMax()));
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMax(), NewRootExtent.GetXMax(), NewRootExtent.GetYMax()));
            aSubNodes[2] = m_pRootNode;
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMin(), NewRootExtent.GetXMax(), RootExtent.GetYMax()));
            }
        else
            {
            // Oldroot becomes number 0
            NewRootExtent.Set(RootExtent.GetXMin(), RootExtent.GetYMin() - RootExtent.GetHeight(), RootExtent.GetXMax() + RootExtent.GetWidth(), RootExtent.GetYMax());
            aSubNodes[0] = m_pRootNode;
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMin(), NewRootExtent.GetXMax(), RootExtent.GetYMax()));
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMin()));
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), NewRootExtent.GetYMin(), NewRootExtent.GetXMax(), NewRootExtent.GetYMax()));
            }
        }
    else
        {
        if (ObjectExtentCenter.GetY() > RootExtentCenter.GetY())
            {
            // Oldroot becomes number 3
            NewRootExtent.Set(RootExtent.GetXMin() - RootExtent.GetWidth(), RootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMax() + RootExtent.GetHeight());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMin(), NewRootExtent.GetYMax()));
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMax(), NewRootExtent.GetYMax()));
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMax()));
            aSubNodes[3] = m_pRootNode;
            }
        else
            {
            // Oldroot becomes number 1
            NewRootExtent.Set(RootExtent.GetXMin() - RootExtent.GetWidth(), RootExtent.GetYMin() - RootExtent.GetHeight(), RootExtent.GetXMax(), RootExtent.GetYMax());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMax()));
            aSubNodes[1] = m_pRootNode;
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMin()));
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMin()));
            }
        }


    // Create new node
    HFCPtr<NODE> pNewRootNode = new NODE(m_SplitTreshold, NewRootExtent);

    // Set subnodes of new root
    pNewRootNode->SetSubNodes(aSubNodes);

    // Set new root as root
    m_pRootNode = pNewRootNode;
    }


//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// HGF2DPooledSpatialIndex Class

/**----------------------------------------------------------------------------
 Constructor for this class. The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DPooledSpatialIndex(FastVectorCountLimitedPool<SPATIAL>* pool, size_t pi_SplitTreshold)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_SplitTreshold),
      m_pool(pool)
    {
    HINVARIANTS;

    }


/**----------------------------------------------------------------------------
 Constructor for this class. This is the constructor that limits the outter
 growth of the spatial index. An initial extent is provided and
 all items added in the spatial index must fall completely inside this extent.
 The split threshold is used to indicate the maximum
 amount of spatial objects to be indexed by an index node after which the node is
 split. Since the split treshold is an optional parameter the constructor also serves as
 a default constructor.

    @param pi_MaxExtent IN The maximum extent of the spatial index.

    @param pi_SplitTreshold IN OPTIONAL The maximum number of items per spatial index
                               node after which the node may be split.

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE>
HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DPooledSpatialIndex(EXTENT const& pi_MaxExtent, FastVectorCountLimitedPool<SPATIAL>* pool, size_t pi_SplitTreshold)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_MaxExtent, pi_SplitTreshold),
      m_pool (pool)
    {
    HINVARIANTS;
    }


/**----------------------------------------------------------------------------
 Copy Constructor
 The internal index items (nodes) are copied but the indexed spatial objects
 are not.

 @param pi_rObj IN The spatial index to copy construct from.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::HGF2DPooledSpatialIndex(const HGF2DPooledSpatialIndex& pi_rObj)
    : HGF2DSpatialIndexBase<COORD, EXTENT, NODE> (pi_rObj)
    {
    HINVARIANTS;
    if (pi_rObj != this)
        m_pool = pi_rObj->GetPool();
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::~HGF2DPooledSpatialIndex()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator
 Alike the copy constructor, the nodes are duplicated while the spatial objects
 are not, but only their reference.
 IMPORTANT!: Take note that the previously indexed spatial objects are removed
 prior to copy of assigned spatial index.

 @param pi_rObj IN The spatial index to copy content from

 @return Reference to spatial index
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>&
HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::operator=(const HGF2DPooledSpatialIndex& pi_rObj)
    {
    HINVARIANTS;

    if (this != &pi_rObj)
        {
        HGF2DSpatialIndexBase<COORD, EXTENT, NODE>::operator= (pi_rObj);
        m_pool = pi_rObj->GetPool();
        }

    return(*this);
    }


/**----------------------------------------------------------------------------
 This method returns the pool

 @return The pool

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> FastVectorCountLimitedPool<SPATIAL>* HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetPool() const
    {
    HINVARIANTS;
    return m_pool;
    }

/**----------------------------------------------------------------------------
 This method adds a spatial object reference in the spatial index

 @param pi_rpSpatialObject IN A smart pointer to spatial object to index.
 No check are performed to verify that the spatial object is already indexed or not.
 If the spatial object is already indexed then it will be indexed twice.

 @return true if item is added and false otherwise. It is possible to refuse addition
 of an item if the spatial index is limited in size (HasMaxExtent()) and the
 item extent does not completely lie within this maximum extent (GetMaxExtent()).

-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> bool HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::AddItem(const SPATIAL pi_rpSpatialObject)
    {
    HINVARIANTS;

    // Check if initial node allocated
    if (m_pRootNode == 0)
        {
        // There is no root node at the moment
        // Allocate root node the size of the object extent
        if (m_HasMaxExtent)
            m_pRootNode = new NODE(m_SplitTreshold, m_MaxExtent, m_pool);
        else
            m_pRootNode = new NODE(m_SplitTreshold, HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject), m_pool);
        }


    // Attempt to add the spatial object to the node ... The node may decide to increase its extent if possible to add it, given
    // The maximum extent is not imposed
    if (!m_pRootNode->AddItemConditional (pi_rpSpatialObject, m_HasMaxExtent))
        {
        // Item could not be added

        // Make sure that the node extent contains the object extent
        while (!((m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetOrigin())) &&
                 (m_pRootNode->GetExtent().IsPointIn(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject).GetCorner()))))
            {
            // If the item is not in root node and extent is limited then it is impossible to add item
            if (m_HasMaxExtent)
                return false;

            // The extent is not contained... we must create a new node
            PushRootDown(HGFIndexGeometryOpProvider<SPATIAL, COORD, EXTENT>::ExtentExtract(pi_rpSpatialObject));
            }


        // The root node contains the spatial object ... add it
        m_pRootNode->AddItem(pi_rpSpatialObject);
        }

    return true;
    }


/**----------------------------------------------------------------------------
 This method removes a spatial object reference from the spatial index.

 Notice that if the object is indexed many times in the spatial index then it
 will only be removed once.

 @param pi_rpSpatialObject IN Pointer to spatial index to remove.

 @return true if the object was removed and false otherwise
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> bool HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::RemoveItem(const SPATIAL pi_rpSpatialObject)
    {
    HINVARIANTS;

    bool Removed = false;

    // Check if initial node allocated
    if (m_pRootNode != 0)
        {
        // The root node present ... remove
        Removed = m_pRootNode->Remove(pi_rpSpatialObject);
        }

    return(Removed);
    }




/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the neighborhood of given coordinate
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetAt(const COORD& pi_rCoord,
        list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetAt(pi_rCoord, pio_rListOfObjects, 0, &m_pLastNode);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of pointer to spatial objects that are located in
 the given extent
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetIn(const EXTENT& pi_rExtent,
        list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    // If a last node access exists ... use it
    if (m_pLastNode != 0)
        m_pLastNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Otherwise use root node
    else if (m_pRootNode != 0)
        m_pRootNode->GetIn(pi_rExtent, pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 This method returns a list of all pointer to all spatial objects that are located in
 the index. These objects are returned somewhat ordered spatially
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> size_t HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::GetAll(list<SPATIAL>& pio_rListOfObjects) const
    {
    HINVARIANTS;

    // Save the number of objects currently in list
    size_t InitialNumberOfObjects = pio_rListOfObjects.size();

    if (m_pRootNode != 0)
        m_pRootNode->GetIn(m_pRootNode->GetExtent(), pio_rListOfObjects);

    // Return number of newly found objects
    return(pio_rListOfObjects.size()- InitialNumberOfObjects);
    }

/**----------------------------------------------------------------------------
 PRIVATE METHOD
 This method inserts a new root that contains the old root in order
 to attempt inclusion of given extent. The root is pushed down by one level.
-----------------------------------------------------------------------------*/
template<class SPATIAL, class COORD, class EXTENT, class SPATIALFINDER, class NODE> void HGF2DPooledSpatialIndex<SPATIAL, COORD, EXTENT, SPATIALFINDER, NODE>::PushRootDown(const EXTENT& pi_rObjectExtent)
    {
    HINVARIANTS;

    // The new node will be twice the sizes of current root node
    EXTENT RootExtent = m_pRootNode->GetExtent();

    // Calculate center of current root extent
    COORD RootExtentCenter((RootExtent.GetXMax() + RootExtent.GetXMin()) / 2.0,
                           (RootExtent.GetYMax() + RootExtent.GetYMin()) / 2.0);

    // Calculate center of spatial object
    COORD ObjectExtentCenter((pi_rObjectExtent.GetXMax() + pi_rObjectExtent.GetXMin()) / 2.0,
                             (pi_rObjectExtent.GetYMax() + pi_rObjectExtent.GetYMin()) / 2.0);


    // Find out what sub-node of new root will current root be
    // and create subnodes appropriately
    HFCPtr<NODE> aSubNodes[4];
    EXTENT NewRootExtent;
    if (ObjectExtentCenter.GetX() > RootExtentCenter.GetX())
        {
        if (ObjectExtentCenter.GetY() > RootExtentCenter.GetY())
            {
            // Oldroot becomes number 2
            NewRootExtent.Set(RootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMax() + RootExtent.GetWidth(), RootExtent.GetYMax() + RootExtent.GetHeight());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMax(), NewRootExtent.GetYMax()), GetPool());
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMax(), NewRootExtent.GetXMax(), NewRootExtent.GetYMax()), GetPool());
            aSubNodes[2] = m_pRootNode;
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMin(), NewRootExtent.GetXMax(), RootExtent.GetYMax()), GetPool());
            }
        else
            {
            // Oldroot becomes number 0
            NewRootExtent.Set(RootExtent.GetXMin(), RootExtent.GetYMin() - RootExtent.GetHeight(), RootExtent.GetXMax() + RootExtent.GetWidth(), RootExtent.GetYMax());
            aSubNodes[0] = m_pRootNode;
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), RootExtent.GetYMin(), NewRootExtent.GetXMax(), RootExtent.GetYMax()), GetPool());
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMin()), GetPool());
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMax(), NewRootExtent.GetYMin(), NewRootExtent.GetXMax(), NewRootExtent.GetYMax()), GetPool());
            }
        }
    else
        {
        if (ObjectExtentCenter.GetY() > RootExtentCenter.GetY())
            {
            // Oldroot becomes number 3
            NewRootExtent.Set(RootExtent.GetXMin() - RootExtent.GetWidth(), RootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMax() + RootExtent.GetHeight());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMin(), NewRootExtent.GetYMax()), GetPool());
            aSubNodes[1] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), RootExtent.GetYMax(), RootExtent.GetXMax(), NewRootExtent.GetYMax()), GetPool());
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMax()), GetPool());
            aSubNodes[3] = m_pRootNode;
            }
        else
            {
            // Oldroot becomes number 1
            NewRootExtent.Set(RootExtent.GetXMin() - RootExtent.GetWidth(), RootExtent.GetYMin() - RootExtent.GetHeight(), RootExtent.GetXMax(), RootExtent.GetYMax());
            aSubNodes[0] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), RootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMax()), GetPool());
            aSubNodes[1] = m_pRootNode;
            aSubNodes[2] = new NODE(m_SplitTreshold, EXTENT(NewRootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMin(), RootExtent.GetYMin()), GetPool());
            aSubNodes[3] = new NODE(m_SplitTreshold, EXTENT(RootExtent.GetXMin(), NewRootExtent.GetYMin(), RootExtent.GetXMax(), RootExtent.GetYMin()), GetPool());
            }
        }


    // Create new node
    HFCPtr<NODE> pNewRootNode = new NODE(m_SplitTreshold, NewRootExtent, GetPool());

    // Set subnodes of new root
    pNewRootNode->SetSubNodes(aSubNodes);

    // Set new root as root
    m_pRootNode = pNewRootNode;
    }
END_IMAGEPP_NAMESPACE
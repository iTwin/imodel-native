//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSRegion.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HGSRegion
//:>-----------------------------------------------------------------------------
//:> inline method for class HGSRegion.
//:>-----------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
inline HGSRegion::~HGSRegion()
    {
    }

/**----------------------------------------------------------------------------
 Clone.

 @return HGSSurfaceOption*
-----------------------------------------------------------------------------*/
inline HGSSurfaceOption* HGSRegion::Clone() const
    {
    return new HGSRegion(*this);
    }

/**----------------------------------------------------------------------------
 Get base shape.

 @return const HFCPtr<HVEShape>&
-----------------------------------------------------------------------------*/
inline const HFCPtr<HVEShape>& HGSRegion::GetBaseShape() const
    {
    HPRECONDITION(m_pBaseShape != 0);

    m_pBaseShape->ChangeCoordSys(m_pCoordSys);
    return m_pBaseShape;
    }

/**----------------------------------------------------------------------------
 Set base shape

 @param pi_rpBaseShape
-----------------------------------------------------------------------------*/
inline void HGSRegion::SetBaseShape(const HFCPtr<HVEShape>&  pi_rpBaseShape)
    {
    HPRECONDITION(pi_rpBaseShape != 0);

    m_pBaseShape = pi_rpBaseShape;
    m_pBaseShape->ChangeCoordSys(m_pCoordSys);

    }

/**----------------------------------------------------------------------------
 Add a new operation.

 @param pi_Operation
 @param pi_rpRegion
-----------------------------------------------------------------------------*/
inline void HGSRegion::AddOperation(HGSRegion::Operation      pi_Operation,
                                    const HFCPtr<HGSRegion>&  pi_rpRegion)
    {
    HPRECONDITION(pi_rpRegion != 0);

    if (pi_rpRegion.operator ==(this))
        {
        HASSERT(0);
        }
    HFCPtr<RegionOperation> pOperation(new RegionOperation());

    pOperation->m_Operation = pi_Operation;
    pOperation->m_pRegion   = pi_rpRegion;

    m_RegionOperations.push_back(pOperation);
    }

/**----------------------------------------------------------------------------
 Add a new operation.

 @param pi_Operation
 @param pi_pPoints
 @param pi_BufferSize
-----------------------------------------------------------------------------*/
inline void HGSRegion::AddOperation(HGSRegion::Operation  pi_Operation,
                                    const double*        pi_pPoints,
                                    size_t                pi_BufferSize)
    {
    HPRECONDITION(pi_pPoints != 0);
    HPRECONDITION(pi_BufferSize > 3);

    // create a shape with the HGSRegion coordsys
    HFCPtr<HVEShape> pShape = new HVEShape(&pi_BufferSize,
                                           const_cast<double*>(pi_pPoints),  // after verification,
                                           m_pCoordSys);                    // the data not change

    HFCPtr<HGSRegion> pRegion = new HGSRegion(pShape, m_pCoordSys);
    AddOperation(pi_Operation, pRegion);
    }

/**----------------------------------------------------------------------------
 Get the last operation.

-----------------------------------------------------------------------------*/
inline void HGSRegion::RemoveLastOperation()
    {
    HPRECONDITION(m_RegionOperations.size() > 0);

    m_RegionOperations.pop_back();
    }

/**----------------------------------------------------------------------------
 Get a specific operation.

 @param pi_OperationIndex
 @param po_pOperation
 @param po_pRegion
-----------------------------------------------------------------------------*/
inline void HGSRegion::GetOperation(uint32_t                pi_OperationIndex,
                                    HGSRegion::Operation*   po_pOperation,
                                    HFCPtr<HGSRegion>&      po_pRegion) const
    {
    HPRECONDITION(pi_OperationIndex < m_RegionOperations.size());
    HPRECONDITION(po_pOperation != 0);

    const HFCPtr<RegionOperation> pOperation = m_RegionOperations[pi_OperationIndex];
    HPOSTCONDITION(pOperation != 0);
    *po_pOperation = pOperation->m_Operation;
    po_pRegion = pOperation->m_pRegion;
    }

/**----------------------------------------------------------------------------
 Get the number of operation.

 @return UInt32
-----------------------------------------------------------------------------*/
inline uint32_t HGSRegion::CountOperations() const
    {
    return (uint32_t)m_RegionOperations.size();
    }


/**----------------------------------------------------------------------------
Return the shape composition.

@return bool
-----------------------------------------------------------------------------*/
inline bool HGSRegion::IsScanlinesShape() const
    {
    return (m_pScanlines != 0);
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------



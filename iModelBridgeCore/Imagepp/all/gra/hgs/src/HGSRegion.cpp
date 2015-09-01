//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgs/src/HGSRegion.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HGSRegion
//:>-----------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGF2DWorld.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGFScanLines.h>

/**----------------------------------------------------------------------------
 Constructor for this class.
-----------------------------------------------------------------------------*/
HGSRegion::HGSRegion(const HFCPtr<HVEShape>&        pi_rpBaseShape,
                     const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys)
    : m_pBaseShape(pi_rpBaseShape),
      m_pCoordSys(pi_rpCoordSys),
      m_pScanlines(0)
    {
    HPRECONDITION(pi_rpBaseShape != 0);
    HPRECONDITION(pi_rpCoordSys != 0);

    m_pBaseShape->ChangeCoordSys(m_pCoordSys);
    }


/**----------------------------------------------------------------------------
 Constructor for this class.

 This constructor take a copy of the HGSRegion and change the coordsys

 @param pi_rpRegion
 @param pi_rpTransfoModel
-----------------------------------------------------------------------------*/
HGSRegion::HGSRegion(const HFCPtr<HGSRegion>&    pi_rpRegion,
                     const HGF2DTransfoModel&    pi_rTransfoModel)
    : m_pScanlines(0)
    {
    HPRECONDITION(pi_rpRegion != 0);

    // create a new coordsys
    m_pCoordSys = new HGF2DCoordSys(pi_rTransfoModel, pi_rpRegion->m_pCoordSys);
    m_pBaseShape = new HVEShape(*pi_rpRegion->m_pBaseShape);
    m_pBaseShape->ChangeCoordSys(m_pCoordSys);

    // scan region and take a copy of each HGSRegion

    HGSRegion::Operation    Operation;
    HFCPtr<HGSRegion>       pRegion;
    HFCPtr<HGSRegion>       pNewRegion;
    for (uint32_t i = 0; i < pi_rpRegion->CountOperations(); i++)
        {
        pi_rpRegion->GetOperation(i,
                                  &Operation,
                                  pRegion);
        pNewRegion = new HGSRegion(pRegion, pi_rTransfoModel);

        AddOperation(Operation, pNewRegion);
        }
    }

/**----------------------------------------------------------------------------
 Constructor for this class.

 This constructor take a copy of the HGSRegion and change the coordsys

 @param pi_pPoints
 @param pi_BufferSize
-----------------------------------------------------------------------------*/
HGSRegion::HGSRegion(const double*                pi_pPoints,
                     size_t                        pi_BufferSize)
    : m_pScanlines(0)
    {
    HPRECONDITION(pi_pPoints != 0);
    HPRECONDITION(pi_BufferSize > 2);
    HPRECONDITION(pi_BufferSize <= LONG_MAX);

    // create a default coordsys
    m_pCoordSys = new HGF2DWorld(HGF2DWorld_UNKNOWNWORLD);
    m_pBaseShape = new HVEShape(&pi_BufferSize,
                                const_cast<double*>(pi_pPoints),
                                m_pCoordSys);
    }


/**----------------------------------------------------------------------------
Constructor for this class.

This constructor take a copy of the HGSRegion and change the coordsys

@param pi_pPoints
@param pi_BufferSize
-----------------------------------------------------------------------------*/
HGSRegion::HGSRegion(const HGFScanLines*  pi_pScanlines)
    : m_pScanlines(pi_pScanlines)
    {
    HPRECONDITION(pi_pScanlines != 0);

    }

/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_pObj
-----------------------------------------------------------------------------*/
HGSRegion::HGSRegion(const HGSRegion&   pi_rObj)
    {
    // create a new coordsys
    m_pCoordSys = pi_rObj.m_pCoordSys;
    if (pi_rObj.m_pBaseShape != 0)
        m_pBaseShape = new HVEShape(*pi_rObj.m_pBaseShape);

    // HGSRegion can be set with HGFScanlines
    m_pScanlines = pi_rObj.m_pScanlines;

    // scan region and take a copy of each HGSRegion

    HGSRegion::Operation    Operation;
    HFCPtr<HGSRegion>       pRegion;
    HFCPtr<HGSRegion>       pNewRegion;
    for (uint32_t i = 0; i < pi_rObj.CountOperations(); i++)
        {
        pi_rObj.GetOperation(i,
                             &Operation,
                             pRegion);
        pNewRegion = new HGSRegion(*pRegion);

        AddOperation(Operation, pNewRegion);
        }
    }

/**----------------------------------------------------------------------------
 Get the extent of the region

 @param po_pXMin
 @param po_pYMin
 @param po_pXMax
 @param po_pYMax
-----------------------------------------------------------------------------*/
void HGSRegion::GetExtent(double*   po_pXMin,
                          double*   po_pYMin,
                          double*   po_pXMax,
                          double*   po_pYMax) const
    {
    HPRECONDITION(po_pXMin != 0);
    HPRECONDITION(po_pYMin != 0);
    HPRECONDITION(po_pXMax != 0);
    HPRECONDITION(po_pYMax != 0);

    if (!m_Extent.IsDefined())
        {
        ComputeExtent();
        HPOSTCONDITION(m_Extent.IsDefined());
        }


    *po_pXMin = m_Extent.GetXMin();
    *po_pYMin = m_Extent.GetYMin();
    *po_pXMax = m_Extent.GetXMax();
    *po_pYMax = m_Extent.GetYMax();
    }


/**----------------------------------------------------------------------------
 Get the shape of the region

 @return HFCPtr<HVEShape>
-----------------------------------------------------------------------------*/
HFCPtr<HVEShape> HGSRegion::GetShape() const
    {
    HPRECONDITION(m_pBaseShape != 0);

    // take a copy of the base shape
    m_pBaseShape->ChangeCoordSys(m_pCoordSys);
    HFCPtr<HVEShape> pShape(new HVEShape(*m_pBaseShape));

    // scan all operation
    HGSRegion::Operation    Operation;
    HFCPtr<HGSRegion>       pRegion;
    HAutoPtr<HVEShape>      pShapeOperation;
    for (uint32_t i = 0; i < CountOperations(); i++)
        {
        GetOperation(i,
                     &Operation,
                     pRegion);

        pShapeOperation = pRegion->GetShape();

        switch(Operation)
            {
            case UNIFY:
                pShape->Unify(*pShapeOperation);
                break;

            case DIFFERENTIATE:
                pShape->Differentiate(*pShapeOperation);
                break;

            case INTERSECT:
                pShape->Intersect(*pShapeOperation);
                break;

            default:
                HASSERT(0);
            }
        }
    return pShape;
    }

/**----------------------------------------------------------------------------
Get the scanlines

@return HGFScanlines*
-----------------------------------------------------------------------------*/
const HGFScanLines* HGSRegion::GetScanlines() const
    {
    HPRECONDITION(m_pScanlines != 0);

    return m_pScanlines;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Compute the region extent.

-----------------------------------------------------------------------------*/
void HGSRegion::ComputeExtent() const
    {
    // scan the region to calculate the extent
    m_pBaseShape->ChangeCoordSys(m_pCoordSys);
    m_Extent = m_pBaseShape->GetExtent();

    HGSRegion::Operation    Operation;
    HFCPtr<HGSRegion>       pRegion;
    for (uint32_t i = 0; i < CountOperations(); i++)
        {
        GetOperation(i,
                     &Operation,
                     pRegion);

        switch(Operation)
            {
            case UNIFY:
                {
                pRegion->ComputeExtent();
                HVEShape RegionExtent(pRegion->m_Extent);
                RegionExtent.ChangeCoordSys(m_pCoordSys);

                if (!m_Extent.IsDefined())
                    m_Extent = RegionExtent.GetExtent();
                else
                    m_Extent.Add(RegionExtent.GetExtent());
                break;
                }

            case DIFFERENTIATE:
                {
                pRegion->ComputeExtent();
                HVEShape RegionExtent(pRegion->m_Extent);

                RegionExtent.ChangeCoordSys(m_pCoordSys);
                m_Extent.Differentiate(RegionExtent.GetExtent());
                break;
                }

            case INTERSECT:
                {
                pRegion->ComputeExtent();
                HVEShape RegionExtent(pRegion->m_Extent);

                RegionExtent.ChangeCoordSys(m_pCoordSys);
                m_Extent.Intersect(RegionExtent.GetExtent());
                break;
                }

            default:
                HASSERT(0);
            }
        }
    }



/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
HGSRegion::~HGSRegion()
    {
    }

/**----------------------------------------------------------------------------
 Clone.

 @return HGSRegion*
-----------------------------------------------------------------------------*/
HGSRegion* HGSRegion::Clone() const
    {
    return new HGSRegion(*this);
    }

/**----------------------------------------------------------------------------
 Get base shape.

 @return const HFCPtr<HVEShape>&
-----------------------------------------------------------------------------*/
const HFCPtr<HVEShape>& HGSRegion::GetBaseShape() const
    {
    HPRECONDITION(m_pBaseShape != 0);

    m_pBaseShape->ChangeCoordSys(m_pCoordSys);
    return m_pBaseShape;
    }

/**----------------------------------------------------------------------------
 Set base shape

 @param pi_rpBaseShape
-----------------------------------------------------------------------------*/
void HGSRegion::SetBaseShape(const HFCPtr<HVEShape>&  pi_rpBaseShape)
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
void HGSRegion::AddOperation(HGSRegion::Operation      pi_Operation,
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
void HGSRegion::AddOperation(HGSRegion::Operation  pi_Operation,
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
void HGSRegion::RemoveLastOperation()
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
void HGSRegion::GetOperation(uint32_t                pi_OperationIndex,
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
uint32_t HGSRegion::CountOperations() const
    {
    return (uint32_t)m_RegionOperations.size();
    }


/**----------------------------------------------------------------------------
Return the shape composition.

@return bool
-----------------------------------------------------------------------------*/
bool HGSRegion::IsScanlinesShape() const
    {
    return (m_pScanlines != 0);
    }




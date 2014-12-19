//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSRegion.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HGSRegion
//:>-----------------------------------------------------------------------------
//:> General class for HGSRegion.
//:>-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceOption.h"
#include "HVEShape.h"


class HGSRegion : public HGSSurfaceOption
    {
    HDECLARE_CLASS_ID(1714, HGSSurfaceOption)

public:

    enum Operation
        {
        UNIFY,
        DIFFERENTIATE,
        INTERSECT
        };


    // Primary methods
    _HDLLg              HGSRegion(const HFCPtr<HVEShape>&       pi_rpBaseShape,
                                  const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys);
    _HDLLg              HGSRegion(const HFCPtr<HGSRegion>&      pi_rpRegion,
                                  const HGF2DTransfoModel&      pi_rTransfoModel);
    _HDLLg              HGSRegion(const double*                pi_pPoints,
                                  size_t                        pi_BufferSize);
    _HDLLg              HGSRegion(const HGFScanLines*           pi_pScanlines);
    _HDLLg              HGSRegion(const HGSRegion&              pi_rObj);
    _HDLLg virtual      ~HGSRegion();

    virtual HGSSurfaceOption*
    Clone() const;

    const HFCPtr<HVEShape>&
    GetBaseShape() const;

    void            SetBaseShape(const HFCPtr<HVEShape>&    pi_rpBaseShape);

    void            AddOperation(HGSRegion::Operation       pi_Operation,
                                 const HFCPtr<HGSRegion>&   pi_rpRegion);

    void            AddOperation(HGSRegion::Operation       pi_Operation,
                                 const double*             pi_pPoints,
                                 size_t                     pi_BufferSize);

    void            RemoveLastOperation();

    void            GetOperation(uint32_t                   pi_OperationIndex,
                                 HGSRegion::Operation*      po_pOperation,
                                 HFCPtr<HGSRegion>&         po_rpRegion) const;

    uint32_t        CountOperations() const;

    void            GetExtent(double*  po_pXMin,
                              double*  po_pYMin,
                              double*  po_pXMax,
                              double*  po_pYMax) const;

    HFCPtr<HVEShape>
    GetShape() const;

    bool           IsScanlinesShape() const;
    const HGFScanLines*   GetScanlines() const;


protected:

private:

    // type definition
    struct RegionOperation : HFCShareableObject<RegionOperation>
        {
        HFCPtr<HGSRegion>   m_pRegion;
        Operation           m_Operation;
        };
    typedef vector<HFCPtr<RegionOperation>, allocator<HFCPtr<RegionOperation> > > RegionOperations;

    // members
    const HGFScanLines*             m_pScanlines;
    mutable HFCPtr<HVEShape>        m_pBaseShape;
    HFCPtr<HGF2DCoordSys>   m_pCoordSys;
    RegionOperations        m_RegionOperations;
    mutable HGF2DExtent             m_Extent;

    // methods
    void            ComputeExtent() const;

    // disabled methods
    HGSRegion&      operator=(const HGSRegion& pi_rObj);
    };


#include "HGSRegion.hpp"

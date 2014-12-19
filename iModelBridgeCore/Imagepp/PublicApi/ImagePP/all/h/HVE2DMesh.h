//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DMesh.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DMesh
//-----------------------------------------------------------------------------
// Description of a mesh
//-----------------------------------------------------------------------------

#pragma once


#include "HVE2DFacet.h"
#include "HGFHVVHSpatialIndex.h" // One
#include "HGFSpatialIndex.h"    // or the other

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a general mesh interface. This is an abstract template
    class that defines the basic mesh interface. A mesh is composed of any
    number of facets containing a shape and an attribute of the template
    argument type. The shape of all facets located in the mesh must not overlap
    although they may be contiguous in any way.
    In a mesh, a single position may belong to at most
    one facet.

    -----------------------------------------------------------------------------
*/
template<class ATTRIBUTE> class HVE2DMesh
    {

public:

    // Primary methods
    HVE2DMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    virtual            ~HVE2DMesh();



    const HVE2DFacet<ATTRIBUTE>*
    GetFacetAt(const HGF2DLocation& pi_rLocation) const;

    HGF2DExtent        GetExtent() const;
    bool              AddFacet(const HVE2DFacet<ATTRIBUTE>& pi_rFacet);

#if (0) // Removed by Stephane Poulain
    const HFCPtr<HVE2DFacet<ATTRIBUTE> >
    GetFacetAt(const HGF2DPosition& i_rPosition) const;
    const HGF2DLiteExtent&
    GetLiteExtent() const;
    virtual const HFCPtr<HVE2DRawFacet>
    GetFacet(long i_Index) const;
    virtual HFCPtr<HVE2DRawFacet>
    GetFacet(long i_Index);
    virtual long       CountFacets() const;

    const HFCPtr<HGF2DCoordSys>&
    GetCoordSys() const;

    virtual const HVE2DShape&  GetShape() const;


#endif

    virtual void       SetFacetValidation(bool i_ActiveValidation);
    virtual bool       GetFacetValidation() const;

protected:


    typedef HGFHVVHSpatialIndex< HVE2DFacet<ATTRIBUTE>* > ListOfFacets;
    ListOfFacets m_ListOfFacets;

    mutable HFCPtr<HVE2DFacet<ATTRIBUTE> >  m_pLastHit;

#if (0) // Removed by Stephane Poulain
    bool              AddFacet(const HVE2DRawFacet& pi_rFacet);
    bool              AddFacet(HFCPtr<HVE2DRawFacet>& i_rpFacet);
    HGF2DSpatialIndex<HVE2DRawFacet, HGF2DSpatialIndexUniqueInPointFinder<HVE2DRawFacet> >
    m_AltListOfFacets;
#endif
    mutable bool              m_TotalShapeUpToDate;
    mutable HFCPtr<HVE2DShape> m_pTotalShape;
    mutable bool            m_ExtentUpToDate;
private:
    bool                    m_ValidateFacetAddition;

#if (0) // Removed by Stephane Poulain

    mutable HGF2DExtent     m_Extent;
    mutable HGF2DLiteExtent m_LiteExtent;


    bool IsNewFacetValid(const HVE2DRawFacet& i_rFacet) const;

    // Desactivated
    HFCPtr<HGF2DCoordSys> m_pCoordSys;
#endif
    // Desactivated
    HVE2DMesh();
    HVE2DMesh(const HVE2DMesh&   pi_rObject);
    HVE2DMesh&         operator=(const HVE2DMesh& pi_rObj);

    };

#include "HVE2DMesh.hpp"


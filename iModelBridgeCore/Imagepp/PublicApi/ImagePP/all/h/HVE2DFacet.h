//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DFacet.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DFacet
//-----------------------------------------------------------------------------
// Description of a facet
//-----------------------------------------------------------------------------

#pragma once


#include "HVE2DPolygonOfSegments.h"
#include "HVE2DRectangle.h"
#include "HGF2DProjective.h"



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a general facet template class. A facet is composed
    of a shape and an attribute of the template argument type. Facets are used
    to bind spatial portions of the world to such an attribute and added
    to a mesh that will manage multiple facets
    The shape of a facet must enclose a non-null area.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> class HVE2DFacet
    {

public:

    // Primary methods
    HVE2DFacet(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DFacet(const HVE2DSimpleShape& pi_rShape);
    HVE2DFacet(const HVE2DSimpleShape& pi_rShape,
               const ATTRIBUTE& pi_rAttribute);
    HVE2DFacet(const HVE2DFacet<ATTRIBUTE>& pi_rObject);
    virtual            ~HVE2DFacet();

    HVE2DFacet&
    operator=(const HVE2DFacet<ATTRIBUTE>& pi_rObj);

    //  Facet specific interface
    const HVE2DShape&          GetShape() const;
    HGF2DExtent                GetExtent() const;
    const HGF2DLiteExtent&     GetLiteExtent() const;


    // Attribute management
    const ATTRIBUTE&           GetAttribute() const;
    void                       SetAttribute(const ATTRIBUTE& pi_rAttribute);

    virtual HVE2DFacet<ATTRIBUTE>*
                               Clone() const;

    /**----------------------------------------------------------------------------
     This method allocates a transformed coy of the facet

     @param i_rTransformMatrix A 2D transformation matrix that contains the
                               transformation to apply to copy. The determinant of
                               the matrix may not be null.

     @return A newly allocated facet containing duplicates of all information and
             member. The new instance must be freed using delete when needed no more.
    -----------------------------------------------------------------------------*/
    virtual HVE2DFacet<ATTRIBUTE>*
                               AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const;

protected:
    // Shape setting
    void                       SetShape(const HVE2DShape& pi_rShape);

    // Default constructor
    HVE2DFacet();

private:


    HFCPtr<HVE2DShape> m_pShape;
    ATTRIBUTE          m_Attribute;
    mutable HGF2DLiteExtent m_LiteExtent;

    };
END_IMAGEPP_NAMESPACE

#include "HVE2DFacet.hpp"


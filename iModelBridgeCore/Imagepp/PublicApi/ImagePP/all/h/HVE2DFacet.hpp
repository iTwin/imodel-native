//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DFacet.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Constructor for this class specifying shape

 @param pi_rShape  Shape to set facet to
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>::HVE2DFacet(const HVE2DSimpleShape& pi_rShape)
    {
    // Now that the nature has been validated ... set it
    SetShape(pi_rShape);
    }

/**----------------------------------------------------------------------------
 Constructor for this class specifying shape and atttribute

 @param pi_rShape  Shape to set facet to

 @param pi_rAttribute Attribute to assign to facet
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>::HVE2DFacet(const HVE2DSimpleShape& pi_rShape,
                                                            const ATTRIBUTE& pi_rAttribute)
    {
    // Now that the nature has been validated ... set it
    SetShape(pi_rShape);

    // Set attribute
    SetAttribute(pi_rAttribute);
    }



/**----------------------------------------------------------------------------
 PROTECTED
 Default Constructor for this class.

 @param pi_rObj Buffer to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>::HVE2DFacet()
    {
    m_pShape = 0;
    }


/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Facet to duplicate.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>::HVE2DFacet(const HVE2DFacet<ATTRIBUTE>& pi_rObj)
    {
    m_pShape = static_cast<HVE2DShape*>(pi_rObj.m_pShape->Clone());
    m_Attribute = pi_rObj.m_Attribute;
    m_LiteExtent = pi_rObj.m_LiteExtent;
    }


/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>::~HVE2DFacet()
    {
    }

/**----------------------------------------------------------------------------
 Assignment operator
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>& HVE2DFacet<ATTRIBUTE>::operator=(const HVE2DFacet<ATTRIBUTE>& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pShape = static_cast<HVE2DShape*>(pi_rObj.m_pShape->Clone());
        m_Attribute = pi_rObj.m_Attribute;
        m_LiteExtent = pi_rObj.m_LiteExtent;
        }

    return(*this);
    }



/**----------------------------------------------------------------------------
 This method returns a reference to the internal shape
 that represents the shape of the facet.

 @return A constant reference to internal shape that
         represents the area.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HVE2DShape& HVE2DFacet<ATTRIBUTE>::GetShape() const
    {
    return(*m_pShape);
    }

/**----------------------------------------------------------------------------
 This method returns the extent of the facet which is of course the extent
 of the shape. This method is required to implement the spatial index in
 the meshes.

 @return The extent of the facet. The extent returned is expressed in the
         coordinate system of the shape.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HGF2DExtent HVE2DFacet<ATTRIBUTE>::GetExtent() const
    {
    return(m_pShape->GetExtent());
    }

/**----------------------------------------------------------------------------
 This method returns the extent of the facet which is of course the extent
 of the shape. This method is required to implement the spatial index in
 the meshes.

 @return The extent of the facet.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const HGF2DLiteExtent& HVE2DFacet<ATTRIBUTE>::GetLiteExtent() const
    {
    return(m_LiteExtent);
    }


/**----------------------------------------------------------------------------
 PROTECTED
 This method sets the shape of the facet.

 @param pi_rShape The shape to set to facet. The area of the shape
                  must be non null.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> void HVE2DFacet<ATTRIBUTE>::SetShape(const HVE2DShape& pi_rShape)
    {
    // Make sure the area is non null
    HPRECONDITION(pi_rShape.CalculateArea() != 0.0);

    // Copy the shape
    m_pShape = static_cast<HVE2DShape*>(pi_rShape.Clone());

    HGF2DExtent MyShapeExtent(m_pShape->GetExtent());

    if (MyShapeExtent.IsDefined())
        {
        m_LiteExtent = HGF2DLiteExtent(MyShapeExtent.GetXMin(),
                                       MyShapeExtent.GetYMin(),
                                       MyShapeExtent.GetXMax(),
                                       MyShapeExtent.GetYMax());
        }
    else
        {
        m_LiteExtent = HGF2DLiteExtent(0.0, 0.0, 0.0, 0.0);
        }
    }



/**----------------------------------------------------------------------------
 This method returns a reference to the internal attribute of the facet

 @return A reference to the internal attribute of the facet.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> const ATTRIBUTE& HVE2DFacet<ATTRIBUTE>::GetAttribute() const
    {
    return(m_Attribute);
    }

/**----------------------------------------------------------------------------
 This method sets the attribute of the facet

 @param pi_rAttribute The new attribute to assign to facet.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> void HVE2DFacet<ATTRIBUTE>::SetAttribute(const ATTRIBUTE& pi_rAttribute)
    {
    m_Attribute = pi_rAttribute;
    }

/**----------------------------------------------------------------------------
 This method clones the facet

 @return A newly allocated facet containing duplicates of all information and
         member. The new instance must be freed using delete when needed no more.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>* HVE2DFacet<ATTRIBUTE>::Clone() const
    {
    return(new HVE2DFacet<ATTRIBUTE>(*this));
    }


/**----------------------------------------------------------------------------
 This method allocates a transformed copy of the facet

 @param i_rTransformMatrix A 2D transformation matrix that contains the
                           transformation to apply to copy. The determinant of
                           the matrix may not be null.

 @return A newly allocated facet containing duplicates of all information and
         member.
-----------------------------------------------------------------------------*/
template<class ATTRIBUTE> HVE2DFacet<ATTRIBUTE>* HVE2DFacet<ATTRIBUTE>::AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const
    {
    // Create projective transformation model based upon the matrix
    HGF2DProjective MyModel(i_rTransformMatrix);

    // Create dumb coordinate system with relation to current coordinate system
    HFCPtr<HGF2DCoordSys> pDumbCoordSys = new HGF2DCoordSys(MyModel, GetShape().GetCoordSys());

    // Allocate transformed copy of shape
    HFCPtr<HVE2DShape> pNewShape = static_cast<HVE2DShape*>(GetShape().AllocateCopyInCoordSys(pDumbCoordSys));

    // Change coordinate system
    pNewShape->SetCoordSys(GetShape().GetCoordSys());

    // Make sure it is simple
    if (pNewShape->IsSimple())
        {
        HFCPtr<HVE2DSimpleShape> pSimpleResultShape = static_cast<HVE2DSimpleShape*>(&*pNewShape);

        // Create new facet
        return (new HVE2DFacet<ATTRIBUTE>(*pSimpleResultShape, GetAttribute()));
        }

    // Not a simple shape ... too bad
    return NULL;
    }

END_IMAGEPP_NAMESPACE





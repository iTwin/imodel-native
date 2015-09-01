//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DPolygon.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DPolygon
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DSimpleShape.h"

BEGIN_IMAGEPP_NAMESPACE
class HVE2DRectangle;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the simple shape of type polygon. A polygon is
    constituted of linked linears which form a path in 2D space which does
    not auto intersect and closes on itself. This type of simple shape is
    the generic polygon  which can be composed of any number and type
    of HVE2DLinear object, as long as they are linked one to the
    other to form a continuous path with the required restrictions
    mentionned above.



    Polygons are far more slower to operate surface
    operations (Union, Intersect and Differentiate) than the
    more specific simple shapes HVE2DRectangle and HVE2DPolygon,
    but permit representation of any possible simple shape into space.

    -----------------------------------------------------------------------------
*/
class HVE2DPolygon : public HVE2DSimpleShape
    {

    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HVE2DPolygonId)

public:


    // Primary methods
    /** -----------------------------------------------------------------------------

        Constructors and copy constructor. These methods permit to
        instantiate a new HVE2DPolygon object. The first one is the default
        constructor. The second one only sets the interpretation coordinate
        system. The third one enables specification of polygon by giving a
        complex linear. This complex linear must close on itself. The fourth
        constructor permits definition by specification of a HVE2DRectangle
        object. An equivalent polygon is constructed with four segments. In
        all constructor where no coordinate system is specificaly specified,
        the interpretation coordinate system is the one of the defining object.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used in the
                             interpretation of the shape.

        @param pi_rComplex Constant reference to a complex linear which
                           must not auto intersect, auto-close.
        @param pi_rRectangle An HVE2DRectangle object from which is
                             constructed a polygon.
        @param pi_rObject Constant reference to a HVE2DPolygon to duplicate.

        Example:
        @code
        @end

        @see IsComplex()
        @see HVE2DBasicLinear
        @see HVE2DComplexLinear
        -----------------------------------------------------------------------------
    */
    HVE2DPolygon();
    HVE2DPolygon(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    HVE2DPolygon(const HVE2DComplexLinear& pi_rComplex);
    HVE2DPolygon(const HVE2DRectangle& pi_rRectangle);
    HVE2DPolygon(const HVE2DPolygon&   pi_rObject);
    virtual            ~HVE2DPolygon();

    HVE2DPolygon&      operator=(const HVE2DPolygon& pi_rObj);

    // Setting
    virtual void       SetLinear(const HVE2DLinear& pi_rLinear);

    // From HVE2DSimpleShape
    virtual HVE2DComplexLinear    GetLinear() const;
    virtual HVE2DComplexLinear    GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;
    virtual HVE2DComplexLinear*   AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const;


    // From HVE2DShape
    virtual bool                  IsEmpty() const;
    virtual HVE2DShapeTypeId      GetShapeType() const;
    virtual double                CalculateArea() const;
    virtual double                CalculatePerimeter() const;
    IMAGEPP_EXPORT virtual bool           IsPointIn(const HGF2DLocation& pi_rPoint,
                                            double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual void                  MakeEmpty();

    virtual void                  Drop(HGF2DLocationCollection* po_pPoint,
                                       double                   pi_Tolerance) const;

    IMAGEPP_EXPORT virtual HGF2DShape*    GetLightShape() const;


    // From HVE2DVector
    virtual HGF2DLocation    CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual size_t    Intersect(const HVE2DVector& pi_rVector,
                                       HGF2DLocationCollection* po_pCrossPoints) const;
    IMAGEPP_EXPORT virtual size_t    ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                        HGF2DLocationCollection* po_pContiguousnessPoints) const;
    IMAGEPP_EXPORT virtual void      ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* pi_pFirstContiguousnessPoint,
                                                          HGF2DLocation* pi_pSecondContiguousnessPoint) const;
    IMAGEPP_EXPORT virtual HVE2DVector*     AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    IMAGEPP_EXPORT virtual bool             Crosses(const HVE2DVector& pi_rVector) const;
    virtual bool             AreContiguous(const HVE2DVector& pi_rVector) const;
    virtual bool             AreAdjacent(const HVE2DVector& pi_rVector) const;
    virtual bool             IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                       HVE2DVector::ExtremityProcessing
                                       pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                       double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool             IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing
                                          pi_ExtremityProcessing = HVE2DVector::INCLUDE_EXTREMITIES,
                                          double pi_Tolerance = HVE_USE_INTERNAL_EPSILON) const;
    virtual bool             AreContiguousAt(const HVE2DVector& pi_rVector,
                                             const HGF2DLocation& pi_rPoint) const;
    IMAGEPP_EXPORT virtual HGFBearing       CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                              HVE2DVector::ArbitraryDirection
                                              pi_Direction = HVE2DVector::BETA) const;
    IMAGEPP_EXPORT virtual double    CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                              HVE2DVector::ArbitraryDirection
                                              pi_Direction = HVE2DVector::BETA) const;

    // From HGFGraphicObject
    virtual HGF2DExtent      GetExtent() const;
    virtual void             Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void             Scale(double pi_ScaleFactor,
                                   const HGF2DLocation& pi_rScaleOrigin);

    virtual void             SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    virtual void             SetTolerance(double pi_Tolerance);
    virtual void             SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance);

    // From HPMPersistentObject
    virtual HPMPersistentObject*    Clone() const;
    // Debugging
    IMAGEPP_EXPORT  virtual void            PrintState(ostream& po_rOutput) const;

protected:

    IMAGEPP_EXPORT  virtual void       SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

private:

    virtual HVE2DSimpleShape::RotationDirection    CalculateRotationDirection() const;
    virtual double                                 CalculateRawArea() const;


    // Member attribute ... a single complex linear
    HVE2DComplexLinear      m_ComplexLinear;
    };
END_IMAGEPP_NAMESPACE


#include "HVE2DPolygon.hpp"

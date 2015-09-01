//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DShape.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DShape
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DVector.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DSimpleShape;
class HGF2DHoledShape;
class HGF2DLinear;
class HGFScanLines;
class HGF2DTransfoModel;
class HGF2DComplexTransfoModel;

typedef uint32_t HGF2DShapeTypeId;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This abstract class encapsulates all attributes common to all shape
    elements. Shape elements are here defined as closed linear lines that
    enclose an area. The boundary of a shape is a continuous 2D line that
    does not auto-cross.

    Since a shape is also a vector, then it possesses two arbitrary direction
    HGF2DVector::ALPHA and HGF2DVector::BETA used to define direction along
    its path. These arbitrary directions are undefined, except for the fact
    that the BETA direction is the reverse of the ALPHA direction.

    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DShape : public HGF2DVector
    {
    HDECLARE_CLASS_ID(HGF2DShapeId_Base, HGF2DVector)

public:

    // Used in the management and iteration of shapes which can contain other shapes
    typedef list<HGF2DShape*, allocator<HGF2DShape*> >   ShapeList;

    // Used in the management and iteration of simple shapes
    typedef list<HGF2DSimpleShape*, allocator<HGF2DSimpleShape*> >   SimpleShapeList;

    // Used in the management and iteration of shapes which can contain holes.
    typedef SimpleShapeList  HoleList;

    // Return value for the spatial position of an object according to shape
    enum SpatialPosition
        {
        S_IN,
        S_OUT,
        S_ON,
        S_PARTIALY_IN
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor. These methods permit to
        instantiate a new HGF2DShape object. The first one is the default
        constructor. The second one only sets the interpretation coordinate system.

        @param pi_rpCoordSys Constant reference to a smart pointer to
                             coordinate system that will be used
                             in the interpretation of the shape.

        @param pi_rObject Constant reference to a HGF2DRectangle to duplicate.


        Example:
        @code
        @end

        -----------------------------------------------------------------------------
    */
                                    HGF2DShape();
                                    HGF2DShape(const HGF2DShape& pi_rObject);
    virtual                         ~HGF2DShape();

    HGF2DShape&                     operator=(const HGF2DShape& pi_rObj);

    // Shape type determination
    /** -----------------------------------------------------------------------------
        This method returns true if the shape is a descendant of HGF2DSimpleShape.
        Simple shape are characterized by the fact that they define one
        closed area which cannot contain holes.

        @return true if the shape is a simple shape, and false otherwise.

        Example:
        @code
        @end

        @see HGF2DSimpleShape
        @see IsComplex()
        -----------------------------------------------------------------------------
    */
    virtual bool                    IsSimple() const = 0;

    /** -----------------------------------------------------------------------------
        This method returns true if the shape is complex and false
        otherwise. A complex shape is characterized by the fact that is
        can be composed of many disjoint parts, that is constituted of many shapes.

        @return true if the shape is complex, and false otherwise.

        Example:
        @code
        @end

        @see HGF2DComplexShape
        @see IsSimple()
        -----------------------------------------------------------------------------
    */
    virtual bool                    IsComplex() const = 0;

    /** -----------------------------------------------------------------------------
        Returns true if the shape does not define any area.

        @return true if no area is defined by the shape, and false otherwise.

        Example:
        @code
        @end

        @see CalculateArea()
        -----------------------------------------------------------------------------
    */
    virtual bool                    IsEmpty     () const = 0;


    /** -----------------------------------------------------------------------------
        This method returns the class id of the self-shape.

        @return The class id of the shape.

        Example:
        @code
        @end
        -----------------------------------------------------------------------------
    */
    virtual HGF2DShapeTypeId        GetShapeType() const = 0;

    // Iteration on component shapes
    /** -----------------------------------------------------------------------------
        This method returns a constant reference to the internal list of
        constituent shapes for complex shapes. The shape must be complex if this
        method is to be used.

        @return A constant reference to the internal shape list of
                the complex shape. This list follows in all points the
                interface of list<> STL template.

        Example:
        @code
        @end

        @see IsComplex()
        @see GetHoleList()
        -----------------------------------------------------------------------------
    */
    virtual const ShapeList&            GetShapeList() const = 0;

    // Iteration on component holes
    /** -----------------------------------------------------------------------------
        This method returns true if the given linear has holes. This includes
        only holed shape of some kind that actually contain holes. Complex
        shapes which are made of many other shapes which themselves can
        contain holes will return false.

        @return true if shape has holes.

        Example:
        @code
        @end

        @see HGF2DHoledShape
        @see GetHoleList()
        -----------------------------------------------------------------------------
    */
    virtual bool                        HasHoles() const = 0;

    /** -----------------------------------------------------------------------------
        This method returns a constant reference to the internal list of
        constituent holes for a shape that has holes. The shape must both
        have the capacity of containing holes and have holes. It must
        therefore answer true when HasHoles() is called.

        @return A constant reference to the internal hole list of the shape.
                This list follows in all points the interface
                of list<> STL template

        Example:
        @code
        @end

        @see HasHoles()
        @see GetShapeList()
        -----------------------------------------------------------------------------
    */
    virtual const HoleList&             GetHoleList() const = 0;

    // Scanlines generation
    virtual void                        Rasterize(HGFScanLines& pio_rScanlines) const;


    // Area oriented operations
    /** -----------------------------------------------------------------------------
        This method returns a pointer to a dynamically allocated shape that
        defines an area containing the area defined by one shape which is
        not located in the other shape.
        The method returns the area contained in the given shape but
        not contained by self. It thus performs the differentiation of
        the two shapes. When the result shape is not needed anymore
        the caller must delete it.
        The method requires that the given shape be expressed in the
        same coordinate system as self.

        @param pi_rShape Constant reference to a shape to perform
                         differentiation of with self.

        @return A pointer to a dynamically allocated shape containing
                the difference of the two shapes.

        Example:
        @code
        @end

        @see DifferentiateShape()
        @see IntersectShape()
        @see UnifyShape
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGF2DShape*       DifferentiateFromShape(const HGF2DShape& pi_rShape) const = 0;
    /** -----------------------------------------------------------------------------
        This method returns a pointer to a dynamically allocated shape that
        defines an area containing the area defined by one shape which is
        not located in the other shape.
        The method differentiates the given from self.

        It thus performs the differentiation of the two shapes.
        When the result shape is not needed anymore the caller must
        delete it.

        The method requires that the given shape be expressed in the
        same coordinate system as self.

        @param pi_rShape Constant reference to a shape to perform
                         differentiation of with self.

        @return A pointer to a dynamically allocated shape containing
                the difference of the two shapes.

        Example:
        @code
        @end

        @see DifferentiateShape()
        @see DifferentiateFromShape()
        @see IntersectShape()
        @see UnifyShape
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGF2DShape*       DifferentiateShape(const HGF2DShape& pi_rShape) const = 0;

    /** -----------------------------------------------------------------------------
        These methods returns a pointer to a dynamically allocated shape that
        defines an area containing the area defined by both self and the
        given shape. It therefore performs the intersection of the two shapes.
        When the result shape is not needed anymore it must be deleted by
        the caller.

        The method requires that the given shape be expressed in the
        same coordinate system as self.

        @param pi_rShape Constant reference to a shape to perform
                         intersection with self.

        @return A pointer to a dynamically allocated shape containing
                the intersection of the two shapes.

        Example:
        @code
        @end

        @see DifferentiateShape()
        @see IntersectShape()
        @see UnifyShape()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGF2DShape*       IntersectShape(const HGF2DShape& pi_rShape) const = 0;

    /** -----------------------------------------------------------------------------
        This methods returns a pointer to a dynamically allocated
        shape that defines an area containing the area defined by the
        self and the given shape. It therefore performs the union of the
        two shapes. When the result shape is not needed anymore it
        must be deleted by the caller.

        The method requires that the given shape be expressed in the
        same coordinate system as self.

        @param pi_rShape Constant reference to a shape to perform
                         union with self.

        @return A pointer to a dynamically allocated shape containing
                the union of the two shapes.

        Example:
        @code
        @end

        @see DifferentiateShape()
        @see IntersectShape()
        @see UnifyShape()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGF2DShape*       UnifyShape(const HGF2DShape& pi_rShape) const = 0;


    // Geometric properties and information
    /** -----------------------------------------------------------------------------
        This method calculates and returns the area enclosed by the
        shape. The units ot the area are the area units derived from
        the distance unit used by the X dimension of the coordinate
        system of the shape.

        @return The area of the shape.

        Example:
        @code
        @end

        @see CalculatePerimeter()
        -----------------------------------------------------------------------------
    */
    virtual double                          CalculateArea() const = 0;

    /** -----------------------------------------------------------------------------
        This method returns the length of the perimeter of the
        HGF2DShape object. 

        @return The perimeter of shape.

        Example:
        @code
        HGF2DShape     MyFirst();
        HGF2DShape    MySecond();
        ...
        double TotalLength = MyFirst.CalculatePerimeter() + MySecond.CalculatePerimeter();
        @end

        @see CalculateArea()
        -----------------------------------------------------------------------------
    */
    virtual double                          CalculatePerimeter() const = 0;


    IMAGEPP_EXPORT virtual SpatialPosition   CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual SpatialPosition   CalculateSpatialPositionOfNonCrossingLinear(const HGF2DLinear& pi_rLinear) const;
    IMAGEPP_EXPORT virtual SpatialPosition   CalculateSpatialPositionOf(const HGF2DPosition& pi_rPosition) const;

    /** -----------------------------------------------------------------------------
        This method checks if the given point is located inside the area defined
        by the shape. If the point is located on the shape boundary, then
        false will be returned. If a tolerance is provided it will be
        use, otherwise the current tolerance of the use is used.

        @param pi_rPosition The HGF2DPosition of which the spatial
                            position of must be determined.

        @param pi_Tolerance A double expressing the tolerance to apply during
                            positional determination. The value is interpreted in the
                            units of the coordinate system the shape is interpreted in.

        @return true if the point is in the shape area, and false otherwise.

        Example:
        @code
        @end

        @see HGF2DPosition
        @see Locate()
        @see CalculateSpatialPositionOf()
        @see IsPointOn()
        -----------------------------------------------------------------------------
    */
    virtual bool                            IsPointIn(const HGF2DPosition& pi_rPoint, double Tolerance = HGF_USE_INTERNAL_EPSILON) const = 0;

    virtual bool                            IsPointOn(const HGF2DPosition& pi_rPoint, HGF2DVector::ExtremityProcessing pi_ExtremityProcessing = HGF2DVector::INCLUDE_EXTREMITIES,
                                                      double Tolerance = HGF_USE_INTERNAL_EPSILON) const = 0;



    // Modification
    virtual void                            MakeEmpty() = 0;

    virtual void                            Drop(HGF2DPositionCollection* po_pPoint,
                                                 double                   pi_rTolerance) const = 0;


    virtual HGF2DVector::Location           Locate(const HGF2DPosition& pi_rPoint) const;

    // From HGF2DVector
    virtual HGF2DVectorTypeId               GetMainVectorType() const;
    virtual bool                            IsAtAnExtremity(const HGF2DPosition& pi_rPosition,
                                                            double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;
    virtual bool                            IsNull() const;

    // Debugging
    virtual void                            PrintState(ostream& po_rOutput) const;


    // Should be somehow hidden
    IMAGEPP_EXPORT virtual SpatialPosition   CalculateSpatialPositionOfSingleComponentVector(const HGF2DVector& pi_rVector) const;
    IMAGEPP_EXPORT virtual SpatialPosition   CalculateSpatialPositionOfNonCrossingSimpleShape(const HGF2DSimpleShape& pi_rSimpleShape) const;



    /** -----------------------------------------------------------------------------
        AllocTransformDirect()
        This method returns a transformed copy of the shape passing through the model 
        provided. The process will make use of various geometry methods provided to
        obtain a cohesive solution that may or may not be of the same original
        shape type. The process is supposed to take into account high or low scale value
        that may result into shape folding and vanishing, becoming smaller than tolerance
        and the process will then return a void shape. The method will also process
        correctly shapes that become autocontiguous or autocrossing as a result
        of high scale or the presence of a vanishing point or a vanishing line. The
        result shape is guaranteed to comform to all common shape properties, autocrossing 
        points being removed by introduction of a complex shape with flirting or almost flirting 
        result components.
        -----------------------------------------------------------------------------
    */
    virtual HFCPtr<HGF2DShape>              AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const = 0;
    /** -----------------------------------------------------------------------------
        AllocTransformInverse()
        This method is the complement of the AllocTransformDirect() method. The default 
        implementation will clone the model, reverse it and call AllocTransformDirect().
        For this reason, the method is necessarily slower to execute than the former
        method and care should be given to manually reverse the model and call
        AllocTransformDirect() instead when more than one shape is involved.
        -----------------------------------------------------------------------------
    */
    virtual HFCPtr<HGF2DShape>              AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const;

protected:


private:

    SpatialPosition                         CalculateSpatialPositionOfMultipleComponentVector(const HGF2DVector& pi_rVector) const;
    SpatialPosition                         CalculateSpatialPositionOfComplexShape(const HGF2DShape& pi_rShape) const;
    SpatialPosition                         CalculateSpatialPositionOfHoledShape(const HGF2DHoledShape& pi_rHoledShape) const;

    };

END_IMAGEPP_NAMESPACE

#include "HGF2DShape.hpp"

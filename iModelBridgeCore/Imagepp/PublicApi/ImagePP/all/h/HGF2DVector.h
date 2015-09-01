//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DVector.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DVector
//-----------------------------------------------------------------------------
// Description of a vector
//-----------------------------------------------------------------------------
#pragma once

#include "HGFLiteTolerance.h"
#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE
class HGF2DLinear;


// Use to indicate that internal epsilon is to be use for tolerance when tolerance can be overridden
// Note that any negative value will do, but the var is provided for clarity
const double HGF_USE_INTERNAL_EPSILON = -1.0;

// Type used for the main vector type
typedef uint32_t HGF2DVectorTypeId;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 


    The nature of all descending vector types enables the class to recognize
    certain caracteristics. Vectors, whatever their type, are constructed of
    linear elements located in a 2D coordinate system. All the vector types
    therefore possess a set of lines in space that can be interpreted differently
    upon their nature. This line or lines can be paths into space, boundaries of
    areas, graphical representation support etc.

    Each of these interpretation lines is assigned a direction applicable for
    all points of the line. These arbitrary directions are called ALPHA and BETA.
    The ALPHA direction is either one of the directions on the lines for a
    specified point, and BETA is by definition, the other direction. These
    directions must be consistent. If for some point, another given point
    is located in the ALPHA direction, therefore, the first point is located in
    the BETA direction of the second. The definition of the ALPHA and BETA
    direction is therefore fixed and cannot change along the constituting line
    of the vectors.


    The vector class also recognizes the optional ability of constituting lines
    to have extremities to their definition. Some line may have no extremities,
    as will be the case in shape elements, but some may have.

    The properties and behavior common to all vectors is thoroughly explained
    in the "HMR Vector Handbook". Please refer to this document for the
    details of vector implementation.

    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DVector : public HFCShareableObject<HGF2DVector>
    {

    HDECLARE_BASECLASS_ID(HGF2DVectorId_Base)


public:

    enum Location
        {
        S_INSIDE,
        S_ON_BOUNDARY,
        S_OUTSIDE
        };

    enum ArbitraryDirection
        {
        ALPHA,
        BETA
        };

    enum ExtremityProcessing
        {
        INCLUDE_EXTREMITIES,
        EXCLUDE_EXTREMITIES
        };


    // Primary methods
    /** -----------------------------------------------------------------------------

        Default constructor, constructor and copy constructor. These methods
        permit to instantiate a new HGF2DVector object. When the default
        constructor is used, the vector will be assigned a new coordinate system
        not related to any other. It is strongly advised not to use "unreferenced"
        vectors for normal usage nor for an extended period. Default
        construction is mainly provided only for the purpose of creating vector
        objects before they can know their proper coordinate system. The vector
        should subsequently be linked to a specified coordinate system with
        the use of the SetCoordSys() method.

        @param pi_rObject Constant reference to a HGF2DVector to duplicate.


        Example:
        @code
        @end

        -----------------------------------------------------------------------------
    */
    HGF2DVector();
    HGF2DVector(const HGF2DVector&    pi_rObject);
    virtual            ~HGF2DVector();

    HGF2DVector&       operator=(const HGF2DVector& pi_rObj);

    /** -----------------------------------------------------------------------------
        Returns the extent of this object.

        @return A constant reference to a HGF2DLiteExtent object that describes the extent
                of this vector object
        @see HGF2DLiteExtent
        -----------------------------------------------------------------------------
    */
    virtual HGF2DLiteExtent GetExtent() const = 0;

    // Operations
    /** -----------------------------------------------------------------------------
        This method returns the closest point on vector boundary from
        given point. If many points of the vector are both closest and at
        the same distance from the given location, then one of them will
        be returned, but which one is undefined.

        @param pi_rPoint Constant reference to an HGF2DPosition containing the
                         point to find closest point on vector boundary.

        @return The closest point on vector boundary.

        Example:
        @code
        @end

        -----------------------------------------------------------------------------
    */
    virtual HGF2DPosition
    CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const = 0;

    /** -----------------------------------------------------------------------------
        This method returns the bearing of vector boundary at given point
        in specified arbitrary direction. The point must be located on
        the vector boundary (IsPointOn()). The bearing returned in the bearing
        in the direction of the next point in the specified direction.
        Therefore if a discontinuity in the slope exists at the specified
        point, the bearing in the ALPHA and BETA direction will be different
        by a value different from PI.

        @param pi_rPoint Constant reference to a HGF2DPosition located on the
                         vector at which the bearing must be evaluated.

        @param pi_Direction This optional parameter indicates in which of the two
                            arbitrary direction on the vector boundary the bearing
                            must be evaluated at. The default is to
                            use the BETA direction.

        @return The bearing at point on vector boundary.

        Example:
        @code
        @end

        @see CalculateAngularAcceleration()
        @see CalculatePerpendicularBearingAt()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGFBearing CalculateBearing(const HGF2DPosition& pi_rPoint,
                                           ArbitraryDirection pi_Direction = BETA) const = 0;

    // Bearing calculations
    /** -----------------------------------------------------------------------------
        This method returns the bearing of vector perpendicular to the boundary
        at given point in the right direction when traveling on this
        boundary in the specified arbitrary direction. The point must
        be located on the vector boundary (IsPointOn()). The bearing returned
        is the bearing perpendicular to the direction of the next point
        in the specified direction. If the bearing of the boundary at the
        point is different in ALPHA and BETA direction (Discontinuity),
        then the mean bearing is used in the calculation of the perpendicular
        bearing. It follows that the perpendicular bearing in one direction
        is always 180 degrees different from the other direction. Optionaly
        if a pointer to existing angle is provided, the sweep at the point
        is also returend. If the bearing in ALPHA and BETA direction are
        different by 180 degrees then 180 degrees is returned. If the
        different is a different value, then this value is returned. Note
        that the angle is the local sweep at the point to the right of
        the point when traveling in specified arbitrary direction.

        @param pi_rPoint Constant reference to a HGF2DPosition located on the
                         vector at which the bearing must be evaluated.

        @param pi_Direction This optional parameter indicates in which of the two
                            arbitrary direction on the vector boundary the bearing
                            must be evaluated to the right of. The default is
                            to use the BETA direction

        @param po_pSweep Optional pointer to existing double that
                         will receive the sweep of the angle formed at point.

        @return The perpendicular bearing at point on vector boundary.

        Example:
        @code
        @end

        @see CalculateAngularAcceleration()
        @see CalculateBearing()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual HGFBearing CalculatePerpendicularBearingAt(const HGF2DPosition&            pi_rPoint,
                                                          HGF2DVector::ArbitraryDirection pi_DirectionToRight = HGF2DVector::BETA,
                                                          double*                         po_pSweep = 0) const;

    /** -----------------------------------------------------------------------------
        This method returns the angular acceleration on vector at given point
        in specified arbitrary direction. The point must be located on the vector
        boundary (IsPointOn()). The angular acceleration returned is a measure of
        the local slope variation in the direction of the next point in the
        specified direction.

        @param pi_rPoint Constant reference to a HGF2DPosition located on the
                         vector boundary at which the angular acceleration must
                         be evaluated

        @param pi_Direction This optional parameter indicates in which of the two
                            arbitrary direction on the vector boundary the angular
                            acceleration must be evaluated at. The default is to
                            use the BETA direction.

        @return The angular acceleration at point on vector boundary.

        Example:
        @code
        @end

        @see CalculateBearing()
        -----------------------------------------------------------------------------
    */
    virtual double    CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                 ArbitraryDirection pi_Direction = BETA) const = 0;


    /** -----------------------------------------------------------------------------
        This method returns the cross point between self and the given vector if any.

        @param pi_rObject Constant reference to a HGF2DVector to find cross points with.

        @param po_pCrossPoints An HGF2DPositionCollection that is appended all the
                               cross points found.


        @return The number of cross points found.

        Example:
        @code
        @end

        @see HGF2DPosition
        -----------------------------------------------------------------------------
    */
    virtual size_t     Intersect(const HGF2DVector& pi_rVector,
                                 HGF2DPositionCollection* po_pCrossPoints) const = 0;


    /** -----------------------------------------------------------------------------
        The method returns all the contiguousness points between self and the
        given vector. Copntiguousness points are the start and end points of
        contiguousness regions between two vectors. The second method returns
        the two contiguousness points of the contiguousness region the specified
        point is part of. The vectors must be contiguous to call these methods.
        The points are returned in increasing order of BETA direction. This
        means that the first point is always positioned in the ALPHA direction
        to the other returned points, the second is in the ALPHA direction to
        all next points and so on. The points are returned by pair. There is
        always an even number of points returned. The first indicates the start
        of a contiguousness region, the second indicates the end of this same
        contiguousness region, The third point indicates the start of
        the next contiguousness region and so on.

        There is a few cases, where a contiguousness region will not possess
        any contiguousness points. This can occur notably when two shapes are
        identical or equivalent. They are contiguous for all points of their
        definition, but there is no start nor end point to the contiguousness
        region. If no contiguousness points are returned, the interpretation
        must be base on the types of the vectors at hand.


        @param pi_rObject Constant reference to a HGF2DVector to find
                          contiguousness points with.

        @param po_pContiguousnessPoints    An HGF2DPositionCollection to which is
                                        appended all the contiguousness points found.

        @return The number of contiguousness points found.

        Example:
        @code
        @end

        @see HGF2DPosition
        @see AreContiguous()
        @see ObtainContiguousnessPointsAt()
        -----------------------------------------------------------------------------
    */
    virtual size_t     ObtainContiguousnessPoints(const HGF2DVector& pi_rVector,
                                                  HGF2DPositionCollection* po_pContiguousnessPoints) const = 0;


    /** -----------------------------------------------------------------------------
        The method returns the contiguousness points between self and the
        given vector at specified location. Contiguousness points are the
        start and end points of contiguousness regions between two vectors.
        The method returns the two contiguousness points of the contiguousness
        region the specified point is part of.
        The vectors must be contiguous in region of specified point to call these
        methods.
        The points are returned in increasing order of BETA direction. This
        means that the first point is always positioned in the ALPHA direction
        to the other returned point.
        The first point indicates the start of the contiguousness region,
        the second indicates the end of this same
        contiguousness region.

        There is a few cases, where a contiguousness region will not possess
        any contiguousness points. This can occur notably when two shapes are
        identical or equivalent. They are contiguous for all points of their
        definition, but there is no start nor end point to the contiguousness
        region. In such case, the contiguousness points returned will be
        undefined. The interpretation must be base on the types of the
        vectors at hand.


        @param pi_rObject Constant reference to a HGF2DVector to find
                          contiguousness points with.

        @param pi_rPoint Constant reference to the point part of a contiguousness
                         region between the vectors of which we want to obtain
                         the contiguousness points.

        @param po_pFirstContiguousnessPoint Pointer to a HGF2DPosition which will receive
                                            the start of the studied contiguousness region.
        @param po_pSecondContiguousnessPoint Pointer to a HGF2DPosition which will receive
                                             the end of the studied contiguousness region.

        Example:
        @code
        @end

        @see HGF2DPosition
        @see AreContiguousAt()
        @see ObtainContiguousnessPoints()
        -----------------------------------------------------------------------------
    */
    virtual void       ObtainContiguousnessPointsAt(
        const HGF2DVector& pi_rVector,
        const HGF2DPosition& pi_rPoint,
        HGF2DPosition* pi_pFirstContiguousnessPoint,
        HGF2DPosition* pi_pSecondContiguousnessPoint) const = 0;


    // Vector property determination
//        HDLL virtual bool      Touches(const HGF2DVector& pi_rVector) const = 0;
//        HDLL virtual bool      Flirts(const HGF2DVector& pi_rVector) const = 0;
    /** -----------------------------------------------------------------------------
        This method determines if the two vectors cross each others boundary or not.

        @param pi_rVector Constant reference to other vector object.

        @return true if the vectors cross, and false otherwise.

        Example:
        @code
        @end

        @see Intersect()
        -----------------------------------------------------------------------------
    */
    virtual bool      Crosses(const HGF2DVector& pi_rVector) const = 0;

    /** -----------------------------------------------------------------------------
        The method determines if the two vectors are contiguous. Contiguousness
        is characterized by portions of vector boundaries shared by the
        two vectors. When they touch by an infinite number of points,
        then the vectors are contiguous.

        If any one of the two vectors is null, then they cannot be contiguous.

        @param pi_rVector Constant reference to other vector object.

        @return true if the vectors are contiguous and false otherwise.

        Example:
        @code
        @end

        @see AreContiguousAt()
        @see ObtainContiguousnessPoints()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual bool      AreContiguous(const HGF2DVector& pi_rVector) const = 0;

    /** -----------------------------------------------------------------------------
        This method determines if the two vectors are adjacent to each
        other or not. Adjacency is characterized by the vector boundaries
        which connect at a finite or infinite number of points, and the slope
        at these connection points is identical for both vectors. This is
        a required condition for contiguousness, but some flirting are caused
        by adjacency.


        @param pi_rVector Constant reference to other vector object.

        @return true if the vectors are adjacent, and false otherwise.

        Example:
        @code
        @end

        @see AreContiguous()
        @see CalculateBearing()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual bool      AreAdjacent(const HGF2DVector& pi_rVector) const = 0;


    /** -----------------------------------------------------------------------------
        This method determines if the given point is located on the
        vector boundary. An optional parameter is provided to
        indicate if extremities are to be considered for the
        operation or not. This method applies by default the
        current vector tolerance or the given tolerance.
        The same tolerance is also applied when extremities
        are to be excluded.

        @param pi_rPoint Constant reference to test point.

        @param pi_ExtremityProcessing An optional flag indicating if the extremities are to be
                                      included or excluded from the vector boundary.
                                      This parameter can take one of the following values :
                                        HGF2DVector::INCLUDE_EXTREMITIES
                                        HGF2DVector::EXCLUDE_EXTREMITIES
                                      Notice that the value of this parameter may be
                                      irrelevant to certain types of vector. For example,
                                      a shape has not extremities, therefore the parameter
                                      is ignored. The default is to include extremitites

        @param pi_Tolerance An optional parameter used to specify the tolerance to apply
                            for the operation. If the parameter is not specified then
                            the vector internal tolerance will be used. This value
                            must be greater than 0.0.

        @return true if the point is on the vector boundary, and false otherwise.

        Example:
        @code
        @end

        @see IsPointOnSCS()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual bool      IsPointOn(
        const HGF2DPosition& pi_rTestPoint,
        ExtremityProcessing pi_ExtremProcess = INCLUDE_EXTREMITIES,
        double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const = 0;




    IMAGEPP_EXPORT virtual bool      IsConnectedBy(const HGF2DLinear& pi_rLinear) const;


    /** -----------------------------------------------------------------------------
        The method determines if the two vectors are contiguous in the area
        of the specified point which must be located on both vectors. Contiguousness
        is characterized by portions of vector boundaries shared by the
        two vectors. When they touch by an infinite number of points,
        then the vectors are contiguous.

        If any one of the two vectors is null, then they cannot be contiguous.

        @param pi_rVector Constant reference to other vector object.

        @param pi_rPosition Constant reference to a HGF2DPosition containing
                            the position of point which must be located on
                            both vector objects (IsPointOn()).

        @return true if the vectors are contiguous and false otherwise.

        Example:
        @code
        @end

        @see AreContiguous()
        @see ObtainContiguousnessPointsAt()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual bool      AreContiguousAt(const HGF2DVector& pi_rVector,
                                              const HGF2DPosition& pi_rPoint) const = 0;

    /** -----------------------------------------------------------------------------
        This method checks if the given point is located at an extremity of the
        vector within a tolerance.

        @param pi_rPosition Constant reference to point to check if it
                            is located upon an extremity of the vector.

        @param pi_Tolerance An optional parameter used to specify the tolerance to apply
                            for the operation. If the parameter is not specified then
                            the vector internal tolerance will be used. This value
                            must be greater than 0.0.

        @return true if the point is at an extremeity and false otherwise.

        Example
        @code
        @end

        @see HGF2DPosition
        -----------------------------------------------------------------------------
    */
    virtual bool      IsAtAnExtremity(const HGF2DPosition& pi_rPosition,
                                       double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const = 0;


    /** -----------------------------------------------------------------------------
        This method determines if there is any boundary to the vector. If
        no boundary exists either because it is not located in space
        yet, or because the length of the boundary is zero, the vector
        is NULL. This may be a temporary condition or depending on the nature
        of object.


        @return true if the vector is null, and false otherwise.

        Example
        @code
        @end
        -----------------------------------------------------------------------------
    */
    virtual bool      IsNull() const = 0;

    // Classification
    /** -----------------------------------------------------------------------------
        This method returns the main vector type. This main vector type
        id is simple the Persistent object class identification number of
        the class that directly inherits from HGF2DVector (Such as
        HGF2DShape::TYPE_ID or HGF2DLinear) of the vector object.

        @return The main vector type id.

        Example
        @code
        @end
        -----------------------------------------------------------------------------
    */
    virtual HGF2DVectorTypeId    GetMainVectorType() const = 0;

    // Tolerance application
    bool              IsAutoToleranceActive() const;
    virtual void       SetAutoToleranceActive(bool pi_ActiveAutoTolerance);
    double            GetTolerance() const;
    virtual void       SetTolerance(double pi_Tolerance);

    IMAGEPP_EXPORT HFCPtr<HGFLiteTolerance>    GetStrokeTolerance() const;
    IMAGEPP_EXPORT virtual void                SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance);


    bool              TendsToTheLeftOfSCS(const HGF2DVector&   pi_rVector,
                                           const HGF2DPosition& pi_rPoint,
                                           ArbitraryDirection   pi_Direction) const;

    IMAGEPP_EXPORT virtual void       PrintState(ostream& po_rOutput) const;

    // Location and Transformation

    /** -----------------------------------------------------------------------------
        Moves this graphic object in its coordinate system by the specified displacement.

        @param pi_rDisplacement IN A constant reference to a displacement object which
                                   specifies the distance and direction of the move.
        @see HGF2DDisplacement
        @see Scale()
        -----------------------------------------------------------------------------
    */
    virtual void        Move(const HGF2DDisplacement& pi_rDisplacement) = 0;

    /** -----------------------------------------------------------------------------
        Scales this graphic object in its coordinate system with regard to its own origin
        or with regard to an origin specified as parameter.
        The same scale factor is applied along both the X and Y axis.

        @param pi_ScaleFactor IN The scaling factor.

        @param pi_rOrigin IN OPTIONAL A constant reference to a location object that
                          specifies the point of origin of the scaling.  If this parameter
                          is not specified, the object is scaled with regard to its
                          own origin.
        @see HGF2DLocation
        @see Move()
        -----------------------------------------------------------------------------
    */
    virtual void        Scale(double pi_ScaleFactor,
                              const HGF2DPosition& pi_rOrigin) = 0;

    /** -----------------------------------------------------------------------------
        The method determines if the two vectors are contiguous in the area
        of the specified point which must be located on both vectors. Contiguousness
        is characterized by portions of vector boundaries shared by the
        two vectors. When they touch by an infinite number of points,
        then the vectors are contiguous.
        If the vectors are contiguous then the contiguousness points are
        returned in provided locations

        If any one of the two vectors is null, then they cannot be contiguous.

        @param pi_rVector Constant reference to other vector object.

        @param pi_rPosition Constant reference to a HGF2DPosition containing
                            the position of point which must be located on
                            both vector objects (IsPointOn()).
        @param po_pFirstPoint Pointer to HGF2DPosition which receives the
                              first contiguousness point if any.

        @param po_pSecondPoint Pointer to HGF2DPosition which receives
                               the second contiguousness point if any.

        @return true if the vectors are contiguous and false otherwise.

        Example:
        @code
        @end

        @see AreContiguousAt()
        @see ObtainContiguousnessPointsAt()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT virtual bool      AreContiguousAtAndGet(
        const HGF2DVector& pi_rVector,
        const HGF2DPosition& pi_rPoint,
        HGF2DPosition* po_pFirstContiguousnessPoint,
        HGF2DPosition* po_pSecondContiguousnessPoint) const;

    virtual Location
    Locate(const HGF2DPosition& pi_rPoint) const;

    // From HPMPersistenObject
    virtual HGF2DVector*
    Clone() const = 0;



protected:

    IMAGEPP_EXPORT virtual bool      IntersectsAtSplitPoint(const HGF2DVector& pi_rVector,
                                                     const HGF2DPosition& pi_rTestPoint,
                                                     const HGF2DPosition& pi_rNextEndPoint,
                                                     bool pi_ProcessNext) const;

    mutable HFCPtr<HGFLiteTolerance>
    m_pStrokeTolerance;

private:
    double              m_Tolerance;
    bool                m_AutoToleranceActive;
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DVector.hpp"

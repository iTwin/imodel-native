/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/



// ===================================================================================

/// <summary>
/// Read xml content and create an ILineSegment
/// Returns false if the reader is not positioned on an element with name LineSegment
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;LineSegment&gt;
///    &lt;startPoint&gt;... &lt;/startPoint&gt;
///    &lt;endPoint&gt;... &lt;/endPoint&gt;
///  &lt;/LineSegment&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadILineSegment (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "LineSegment"))
        {
        CGLineSegmentDetail detail;
        CGLineSegmentFlags flags;

        flags.startPoint_defined = ReadTagDPoint3d (value, "startPoint", detail.startPoint);
        flags.endPoint_defined = ReadTagDPoint3d (value, "endPoint", detail.endPoint);

        result = m_factory.Create (detail);//LineSegment(detail.startPoint,detail.endPoint);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ICircularArc
/// Returns false if the reader is not positioned on an element with name CircularArc
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CircularArc&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radius&gt;... &lt;/radius&gt;
///    &lt;startAngle&gt;... &lt;/startAngle&gt;
///    &lt;sweepAngle&gt;... &lt;/sweepAngle&gt;
///  &lt;/CircularArc&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularArc (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CircularArc"))
        {
        CGCircularArcDetail detail;
        CGCircularArcFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radius_defined = ReadTagdouble (value, "radius", detail.radius);
        flags.startAngle_defined = ReadTagAngle (value, "startAngle", detail.startAngle);
        flags.sweepAngle_defined = ReadTagAngle (value, "sweepAngle", detail.sweepAngle);

        result = m_factory.Create (detail);//CircularArc(detail.placement,detail.radius,detail.startAngle,detail.sweepAngle);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IDgnBox
/// Returns false if the reader is not positioned on an element with name DgnBox
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnBox&gt;
///    &lt;baseOrigin&gt;... &lt;/baseOrigin&gt;
///    &lt;topOrigin&gt;... &lt;/topOrigin&gt;
///    &lt;vectorX&gt;... &lt;/vectorX&gt;
///    &lt;vectorY&gt;... &lt;/vectorY&gt;
///    &lt;baseX&gt;... &lt;/baseX&gt;
///    &lt;baseY&gt;... &lt;/baseY&gt;
///    &lt;topX&gt;... &lt;/topX&gt;
///    &lt;topY&gt;... &lt;/topY&gt;
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/DgnBox&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnBox (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnBox"))
        {
        CGDgnBoxDetail detail;
        CGDgnBoxFlags flags;

        flags.baseOrigin_defined = ReadTagDPoint3d (value, "baseOrigin", detail.baseOrigin);
        flags.topOrigin_defined = ReadTagDPoint3d (value, "topOrigin", detail.topOrigin);
        flags.vectorX_defined = ReadTagDVector3d (value, "vectorX", detail.vectorX);
        flags.vectorY_defined = ReadTagDVector3d (value, "vectorY", detail.vectorY);
        flags.baseX_defined = ReadTagdouble (value, "baseX", detail.baseX);
        flags.baseY_defined = ReadTagdouble (value, "baseY", detail.baseY);
        flags.topX_defined = ReadTagdouble (value, "topX", detail.topX);
        flags.topY_defined = ReadTagdouble (value, "topY", detail.topY);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);

        result = m_factory.Create (detail);//DgnBox(detail.baseOrigin,detail.topOrigin,detail.vectorX,detail.vectorY,detail.baseX,detail.baseY,detail.topX,detail.topY,detail.capped);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IDgnSphere
/// Returns false if the reader is not positioned on an element with name DgnSphere
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnSphere&gt;
///    &lt;center&gt;... &lt;/center&gt;
///    &lt;vectorX&gt;... &lt;/vectorX&gt;
///    &lt;vectorZ&gt;... &lt;/vectorZ&gt;
///    &lt;radiusXY&gt;... &lt;/radiusXY&gt;
///    &lt;radiusZ&gt;... &lt;/radiusZ&gt;
///    &lt;startLatitude&gt;... &lt;/startLatitude&gt;
///    &lt;latitudeSweep&gt;... &lt;/latitudeSweep&gt;
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/DgnSphere&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnSphere (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnSphere"))
        {
        CGDgnSphereDetail detail;
        CGDgnSphereFlags flags;

        flags.center_defined = ReadTagDPoint3d (value, "center", detail.center);
        flags.vectorX_defined = ReadTagDVector3d (value, "vectorX", detail.vectorX);
        flags.vectorZ_defined = ReadTagDVector3d (value, "vectorZ", detail.vectorZ);
        flags.radiusXY_defined = ReadTagdouble (value, "radiusXY", detail.radiusXY);
        flags.radiusZ_defined = ReadTagdouble (value, "radiusZ", detail.radiusZ);
        flags.startLatitude_defined = ReadTagAngle (value, "startLatitude", detail.startLatitude);
        flags.latitudeSweep_defined = ReadTagAngle (value, "latitudeSweep", detail.latitudeSweep);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);

        result = m_factory.Create (detail);//DgnSphere(detail.center,detail.vectorX,detail.vectorZ,detail.radiusXY,detail.radiusZ,detail.startLatitude,detail.latitudeSweep,detail.capped);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IDgnCone
/// Returns false if the reader is not positioned on an element with name DgnCone
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnCone&gt;
///    &lt;centerA&gt;... &lt;/centerA&gt;
///    &lt;centerB&gt;... &lt;/centerB&gt;
///    &lt;vectorX&gt;... &lt;/vectorX&gt;
///    &lt;vectorY&gt;... &lt;/vectorY&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/DgnCone&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnCone (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnCone"))
        {
        CGDgnConeDetail detail;
        CGDgnConeFlags flags;

        flags.centerA_defined = ReadTagDPoint3d (value, "centerA", detail.centerA);
        flags.centerB_defined = ReadTagDPoint3d (value, "centerB", detail.centerB);
        flags.vectorX_defined = ReadTagDVector3d (value, "vectorX", detail.vectorX);
        flags.vectorY_defined = ReadTagDVector3d (value, "vectorY", detail.vectorY);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);

        result = m_factory.Create (detail);//DgnCone(detail.centerA,detail.centerB,detail.vectorX,detail.vectorY,detail.radiusA,detail.radiusB,detail.capped);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IDgnTorusPipe
/// Returns false if the reader is not positioned on an element with name DgnTorusPipe
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnTorusPipe&gt;
///    &lt;center&gt;... &lt;/center&gt;
///    &lt;vectorX&gt;... &lt;/vectorX&gt;
///    &lt;vectorY&gt;... &lt;/vectorY&gt;
///    &lt;majorRadius&gt;... &lt;/majorRadius&gt;
///    &lt;minorRadius&gt;... &lt;/minorRadius&gt;
///    &lt;sweepAngle&gt;... &lt;/sweepAngle&gt;
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/DgnTorusPipe&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnTorusPipe (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnTorusPipe"))
        {
        CGDgnTorusPipeDetail detail;
        CGDgnTorusPipeFlags flags;

        flags.center_defined = ReadTagDPoint3d (value, "center", detail.center);
        flags.vectorX_defined = ReadTagDVector3d (value, "vectorX", detail.vectorX);
        flags.vectorY_defined = ReadTagDVector3d (value, "vectorY", detail.vectorY);
        flags.majorRadius_defined = ReadTagdouble (value, "majorRadius", detail.majorRadius);
        flags.minorRadius_defined = ReadTagdouble (value, "minorRadius", detail.minorRadius);
        flags.sweepAngle_defined = ReadTagAngle (value, "sweepAngle", detail.sweepAngle);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);

        result = m_factory.Create (detail);//DgnTorusPipe(detail.center,detail.vectorX,detail.vectorY,detail.majorRadius,detail.minorRadius,detail.sweepAngle,detail.capped);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IBlock
/// Returns false if the reader is not positioned on an element with name Block
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Block&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;cornerA&gt;... &lt;/cornerA&gt;
///    &lt;cornerB&gt;... &lt;/cornerB&gt;
///    &lt;bSolidFlag&gt;... &lt;/bSolidFlag&gt;
///  &lt;/Block&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIBlock (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Block"))
        {
        CGBlockDetail detail;
        CGBlockFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.cornerA_defined = ReadTagDPoint3d (value, "cornerA", detail.cornerA);
        flags.cornerB_defined = ReadTagDPoint3d (value, "cornerB", detail.cornerB);
        flags.bSolidFlag_defined = ReadTagbool (value, "bSolidFlag", detail.bSolidFlag);

        result = m_factory.Create (detail);//Block(detail.placement,detail.cornerA,detail.cornerB,detail.bSolidFlag);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ICircularCone
/// Returns false if the reader is not positioned on an element with name CircularCone
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CircularCone&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;height&gt;... &lt;/height&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///    &lt;bSolidFlag&gt;... &lt;/bSolidFlag&gt;
///  &lt;/CircularCone&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularCone (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CircularCone"))
        {
        CGCircularConeDetail detail;
        CGCircularConeFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.height_defined = ReadTagdouble (value, "height", detail.height);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);
        flags.bSolidFlag_defined = ReadTagbool (value, "bSolidFlag", detail.bSolidFlag);

        result = m_factory.Create (detail);//CircularCone(detail.placement,detail.height,detail.radiusA,detail.radiusB,detail.bSolidFlag);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ICircularCylinder
/// Returns false if the reader is not positioned on an element with name CircularCylinder
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CircularCylinder&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;height&gt;... &lt;/height&gt;
///    &lt;radius&gt;... &lt;/radius&gt;
///    &lt;bSolidFlag&gt;... &lt;/bSolidFlag&gt;
///  &lt;/CircularCylinder&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularCylinder (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CircularCylinder"))
        {
        CGCircularCylinderDetail detail;
        CGCircularCylinderFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.height_defined = ReadTagdouble (value, "height", detail.height);
        flags.radius_defined = ReadTagdouble (value, "radius", detail.radius);
        flags.bSolidFlag_defined = ReadTagbool (value, "bSolidFlag", detail.bSolidFlag);

        result = m_factory.Create (detail);//CircularCylinder(detail.placement,detail.height,detail.radius,detail.bSolidFlag);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ICircularDisk
/// Returns false if the reader is not positioned on an element with name CircularDisk
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CircularDisk&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radius&gt;... &lt;/radius&gt;
///  &lt;/CircularDisk&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularDisk (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CircularDisk"))
        {
        CGCircularDiskDetail detail;
        CGCircularDiskFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radius_defined = ReadTagdouble (value, "radius", detail.radius);

        result = m_factory.Create (detail);//CircularDisk(detail.placement,detail.radius);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ICoordinate
/// Returns false if the reader is not positioned on an element with name Coordinate
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Coordinate&gt;
///    &lt;xyz&gt;... &lt;/xyz&gt;
///  &lt;/Coordinate&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICoordinate (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Coordinate"))
        {
        CGCoordinateDetail detail;
        CGCoordinateFlags flags;

        flags.xyz_defined = ReadTagDPoint3d (value, "xyz", detail.xyz);

        result = m_factory.Create (detail);//Coordinate(detail.xyz);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IEllipticArc
/// Returns false if the reader is not positioned on an element with name EllipticArc
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;EllipticArc&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///    &lt;startAngle&gt;... &lt;/startAngle&gt;
///    &lt;sweepAngle&gt;... &lt;/sweepAngle&gt;
///  &lt;/EllipticArc&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIEllipticArc (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "EllipticArc"))
        {
        CGEllipticArcDetail detail;
        CGEllipticArcFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);
        flags.startAngle_defined = ReadTagAngle (value, "startAngle", detail.startAngle);
        flags.sweepAngle_defined = ReadTagAngle (value, "sweepAngle", detail.sweepAngle);

        result = m_factory.Create (detail);//EllipticArc(detail.placement,detail.radiusA,detail.radiusB,detail.startAngle,detail.sweepAngle);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IEllipticDisk
/// Returns false if the reader is not positioned on an element with name EllipticDisk
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;EllipticDisk&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///  &lt;/EllipticDisk&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIEllipticDisk (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "EllipticDisk"))
        {
        CGEllipticDiskDetail detail;
        CGEllipticDiskFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);

        result = m_factory.Create (detail);//EllipticDisk(detail.placement,detail.radiusA,detail.radiusB);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ISingleLineText
/// Returns false if the reader is not positioned on an element with name SingleLineText
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SingleLineText&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;textString&gt;... &lt;/textString&gt;
///    &lt;fontName&gt;... &lt;/fontName&gt;
///    &lt;characterXSize&gt;... &lt;/characterXSize&gt;
///    &lt;characterYSize&gt;... &lt;/characterYSize&gt;
///    &lt;justification&gt;... &lt;/justification&gt;
///  &lt;/SingleLineText&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISingleLineText (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SingleLineText"))
        {
        CGSingleLineTextDetail detail;
        CGSingleLineTextFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.textString_defined = ReadTagString (value, "textString", detail.textString);
        flags.fontName_defined = ReadTagString (value, "fontName", detail.fontName);
        flags.characterXSize_defined = ReadTagdouble (value, "characterXSize", detail.characterXSize);
        flags.characterYSize_defined = ReadTagdouble (value, "characterYSize", detail.characterYSize);
        flags.justification_defined = ReadTagint (value, "justification", detail.justification);

        result = m_factory.Create (detail);//SingleLineText(detail.placement,detail.textString,detail.fontName,detail.characterXSize,detail.characterYSize,detail.justification);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ISkewedCone
/// Returns false if the reader is not positioned on an element with name SkewedCone
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SkewedCone&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;centerB&gt;... &lt;/centerB&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///    &lt;bSolidFlag&gt;... &lt;/bSolidFlag&gt;
///  &lt;/SkewedCone&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISkewedCone (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SkewedCone"))
        {
        CGSkewedConeDetail detail;
        CGSkewedConeFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.centerB_defined = ReadTagDPoint3d (value, "centerB", detail.centerB);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);
        flags.bSolidFlag_defined = ReadTagbool (value, "bSolidFlag", detail.bSolidFlag);

        result = m_factory.Create (detail);//SkewedCone(detail.placement,detail.centerB,detail.radiusA,detail.radiusB,detail.bSolidFlag);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ISphere
/// Returns false if the reader is not positioned on an element with name Sphere
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Sphere&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radius&gt;... &lt;/radius&gt;
///  &lt;/Sphere&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISphere (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Sphere"))
        {
        CGSphereDetail detail;
        CGSphereFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radius_defined = ReadTagdouble (value, "radius", detail.radius);

        result = m_factory.Create (detail);//Sphere(detail.placement,detail.radius);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an ITorusPipe
/// Returns false if the reader is not positioned on an element with name TorusPipe
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;TorusPipe&gt;
///    &lt;placement&gt;... &lt;/placement&gt;
///    &lt;radiusA&gt;... &lt;/radiusA&gt;
///    &lt;radiusB&gt;... &lt;/radiusB&gt;
///    &lt;startAngle&gt;... &lt;/startAngle&gt;
///    &lt;sweepAngle&gt;... &lt;/sweepAngle&gt;
///    &lt;bSolidFlag&gt;... &lt;/bSolidFlag&gt;
///  &lt;/TorusPipe&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadITorusPipe (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "TorusPipe"))
        {
        CGTorusPipeDetail detail;
        CGTorusPipeFlags flags;

        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.radiusA_defined = ReadTagdouble (value, "radiusA", detail.radiusA);
        flags.radiusB_defined = ReadTagdouble (value, "radiusB", detail.radiusB);
        flags.startAngle_defined = ReadTagAngle (value, "startAngle", detail.startAngle);
        flags.sweepAngle_defined = ReadTagAngle (value, "sweepAngle", detail.sweepAngle);
        flags.bSolidFlag_defined = ReadTagbool (value, "bSolidFlag", detail.bSolidFlag);

        result = m_factory.Create (detail);//TorusPipe(detail.placement,detail.radiusA,detail.radiusB,detail.startAngle,detail.sweepAngle,detail.bSolidFlag);
        return true;
        }
    return false;
    }



// ===================================================================================

/// <summary>
/// Read xml content and create an IVector
/// Returns false if the reader is not positioned on an element with name Vector
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Vector&gt;
///    &lt;xyz&gt;... &lt;/xyz&gt;
///    &lt;vector&gt;... &lt;/vector&gt;
///  &lt;/Vector&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIVector (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Vector"))
        {
        CGVectorDetail detail;
        CGVectorFlags flags;

        flags.xyz_defined = ReadTagDPoint3d (value, "xyz", detail.xyz);
        flags.vector_defined = ReadTagDVector3d (value, "vector", detail.vector);

        result = m_factory.Create (detail);//Vector(detail.xyz,detail.vector);
        return true;
        }
    return false;
    }






/// <summary>
/// Read xml content and create an IIndexedMesh
/// Returns false if the reader is not positioned on an element with name IndexedMesh
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;IndexedMesh&gt;
///    &lt;ListOfCoord&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfCoord&gt;
///    &lt;ListOfCoordIndex&gt;
///         &lt;id&gt;...&lt;/id&gt;
///    &lt;/ListOfCoordIndex&gt;
///    &lt;ListOfParam&gt;
///         &lt;uv&gt;...&lt;/uv&gt;
///    &lt;/ListOfParam&gt;
///    &lt;ListOfParamIndex&gt;
///         &lt;id&gt;...&lt;/id&gt;
///    &lt;/ListOfParamIndex&gt;
///    &lt;ListOfNormal&gt;
///         &lt;normal&gt;...&lt;/normal&gt;
///    &lt;/ListOfNormal&gt;
///    &lt;ListOfNormalIndex&gt;
///         &lt;id&gt;...&lt;/id&gt;
///    &lt;/ListOfNormalIndex&gt;
///    &lt;ListOfColor&gt;
///         &lt;color&gt;...&lt;/color&gt;
///    &lt;/ListOfColor&gt;
///    &lt;ListOfColorIndex&gt;
///         &lt;id&gt;...&lt;/id&gt;
///    &lt;/ListOfColorIndex&gt;
///  &lt;/IndexedMesh&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIIndexedMesh (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "IndexedMesh"))
        {
        CGIndexedMeshDetail detail;
        CGIndexedMeshFlags flags;

        flags.xyzArray_defined = ReadListOfDPoint3d (value, "ListOfCoord", "Coord", detail.xyzArray);
        flags.coordIndexArray_defined = ReadListOfint (value, "ListOfCoordIndex", "CoordIndex", detail.coordIndexArray);
        flags.uvArray_defined = ReadListOfDPoint2d (value, "ListOfParam", "Param", detail.uvArray);
        flags.paramIndexArray_defined = ReadListOfint (value, "ListOfParamIndex", "ParamIndex", detail.paramIndexArray);
        flags.normalArray_defined = ReadListOfDVector3d (value, "ListOfNormal", "Normal", detail.normalArray);
        flags.normalIndexArray_defined = ReadListOfint (value, "ListOfNormalIndex", "NormalIndex", detail.normalIndexArray);
        flags.colorArray_defined = ReadListOfDVector3d (value, "ListOfColor", "Color", detail.colorArray);
        flags.colorIndexArray_defined = ReadListOfint (value, "ListOfColorIndex", "ColorIndex", detail.colorIndexArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IAdjacentSurfacePatches
/// Returns false if the reader is not positioned on an element with name AdjacentSurfacePatches
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;AdjacentSurfacePatches&gt;
///    &lt;ListOfPatch&gt;
///         &lt;Patch&gt;...&lt;/Patch&gt;
///    &lt;/ListOfPatch&gt;
///  &lt;/AdjacentSurfacePatches&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIAdjacentSurfacePatches (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "AdjacentSurfacePatches"))
        {
        CGAdjacentSurfacePatchesDetail detail;
        CGAdjacentSurfacePatchesFlags flags;

        flags.patchArray_defined = ReadListOfISurfacePatch (value, "ListOfPatch", "Patch", detail.patchArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IBsplineCurve
/// Returns false if the reader is not positioned on an element with name BsplineCurve
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;BsplineCurve&gt;
///    &lt;Order&gt;... &lt;/Order&gt;
///    &lt;Closed&gt;... &lt;/Closed&gt;
///    &lt;ListOfControlPoint&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfControlPoint&gt;
///    &lt;ListOfWeight&gt;
///         &lt;Weight&gt;...&lt;/Weight&gt;
///    &lt;/ListOfWeight&gt;
///    &lt;ListOfKnot&gt;
///         &lt;Knot&gt;...&lt;/Knot&gt;
///    &lt;/ListOfKnot&gt;
///  &lt;/BsplineCurve&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIBsplineCurve (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "BsplineCurve"))
        {
        CGBsplineCurveDetail detail;
        CGBsplineCurveFlags flags;

        flags.order_defined = ReadTagint (value, "order", detail.order);
        flags.closed_defined = ReadTagbool (value, "closed", detail.closed);
        flags.controlPointArray_defined = ReadListOfDPoint3d (value, "ListOfControlPoint", "ControlPoint", detail.controlPointArray);
        flags.weightArray_defined = ReadListOfdouble (value, "ListOfWeight", "Weight", detail.weightArray);
        flags.knotArray_defined = ReadListOfdouble (value, "ListOfKnot", "Knot", detail.knotArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IBsplineSurface
/// Returns false if the reader is not positioned on an element with name BsplineSurface
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;BsplineSurface&gt;
///    &lt;OrderU&gt;... &lt;/OrderU&gt;
///    &lt;ClosedU&gt;... &lt;/ClosedU&gt;
///    &lt;NumUControlPoint&gt;... &lt;/NumUControlPoint&gt;
///    &lt;OrderV&gt;... &lt;/OrderV&gt;
///    &lt;ClosedV&gt;... &lt;/ClosedV&gt;
///    &lt;NumVControlPoint&gt;... &lt;/NumVControlPoint&gt;
///    &lt;ListOfControlPoint&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfControlPoint&gt;
///    &lt;ListOfWeight&gt;
///         &lt;Weight&gt;...&lt;/Weight&gt;
///    &lt;/ListOfWeight&gt;
///    &lt;ListOfKnotU&gt;
///         &lt;KnotU&gt;...&lt;/KnotU&gt;
///    &lt;/ListOfKnotU&gt;
///    &lt;ListOfKnotV&gt;
///         &lt;KnotV&gt;...&lt;/KnotV&gt;
///    &lt;/ListOfKnotV&gt;
///  &lt;/BsplineSurface&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIBsplineSurface (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "BsplineSurface"))
        {
        CGBsplineSurfaceDetail detail;
        CGBsplineSurfaceFlags flags;

        flags.orderU_defined = ReadTagint (value, "orderU", detail.orderU);
        flags.closedU_defined = ReadTagbool (value, "closedU", detail.closedU);
        flags.numUControlPoint_defined = ReadTagint (value, "numUControlPoint", detail.numUControlPoint);
        flags.orderV_defined = ReadTagint (value, "orderV", detail.orderV);
        flags.closedV_defined = ReadTagbool (value, "closedV", detail.closedV);
        flags.numVControlPoint_defined = ReadTagint (value, "numVControlPoint", detail.numVControlPoint);
        flags.controlPointArray_defined = ReadListOfDPoint3d (value, "ListOfControlPoint", "ControlPoint", detail.controlPointArray);
        flags.weightArray_defined = ReadListOfdouble (value, "ListOfWeight", "Weight", detail.weightArray);
        flags.knotUArray_defined = ReadListOfdouble (value, "ListOfKnotU", "KnotU", detail.knotUArray);
        flags.knotVArray_defined = ReadListOfdouble (value, "ListOfKnotV", "KnotV", detail.knotVArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IGeometryPtr
/// Returns false if the reader is not positioned on an element with name CurveChain
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CurveChain&gt;
///    &lt;ListOfCurve&gt;
///         &lt;Curve&gt;...&lt;/Curve&gt;
///    &lt;/ListOfCurve&gt;
///  &lt;/CurveChain&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICurveChain (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CurveChain"))
        {
        CGCurveChainDetail detail;
        CGCurveChainFlags flags;

        flags.curveArray_defined = ReadListOf_AnyICurvePrimitive (value, "ListOfCurve", "Curve", detail.curveArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ICurveGroup
/// Returns false if the reader is not positioned on an element with name CurveGroup
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CurveGroup&gt;
///    &lt;ListOfCurve&gt;
///         &lt;Curve&gt;...&lt;/Curve&gt;
///    &lt;/ListOfCurve&gt;
///  &lt;/CurveGroup&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICurveGroup (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CurveGroup"))
        {
        CGCurveGroupDetail detail;
        CGCurveGroupFlags flags;

        flags.curveArray_defined = ReadListOfICurve (value, "ListOfCurve", "Curve", detail.curveArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ICurveReference
/// Returns false if the reader is not positioned on an element with name CurveReference
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;CurveReference&gt;
///    &lt;Reversed&gt;... &lt;/Reversed&gt;
///    &lt;ParentCurve&gt;... &lt;/ParentCurve&gt;
///  &lt;/CurveReference&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICurveReference (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "CurveReference"))
        {
        CGCurveReferenceDetail detail;
        CGCurveReferenceFlags flags;

        flags.reversed_defined = ReadTagbool (value, "reversed", detail.reversed);
        flags.parentCurve_defined = ReadTag_AnyCurve (value, "parentCurve", detail.parentCurve);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IGroup
/// Returns false if the reader is not positioned on an element with name Group
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Group&gt;
///    &lt;ListOfMember&gt;
///         &lt;Member&gt;...&lt;/Member&gt;
///    &lt;/ListOfMember&gt;
///  &lt;/Group&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIGroup (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Group"))
        {
        CGGroupDetail detail;
        CGGroupFlags flags;

        flags.memberArray_defined = ReadListOfIGeometry (value, "ListOfMember", "Member", detail.memberArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IInterpolatingCurve
/// Returns false if the reader is not positioned on an element with name InterpolatingCurve
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;InterpolatingCurve&gt;
///    &lt;EndConditionCode&gt;... &lt;/EndConditionCode&gt;
///    &lt;KnotCode&gt;... &lt;/KnotCode&gt;
///    &lt;StartVector&gt;... &lt;/StartVector&gt;
///    &lt;EndVector&gt;... &lt;/EndVector&gt;
///    &lt;ListOfPoint&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfPoint&gt;
///    &lt;ListOfKnot&gt;
///         &lt;knot&gt;...&lt;/knot&gt;
///    &lt;/ListOfKnot&gt;
///  &lt;/InterpolatingCurve&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIInterpolatingCurve (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "InterpolatingCurve"))
        {
        CGInterpolatingCurveDetail detail;
        CGInterpolatingCurveFlags flags;

        flags.endConditionCode_defined = ReadTagint (value, "endConditionCode", detail.endConditionCode);
        flags.knotCode_defined = ReadTagint (value, "knotCode", detail.knotCode);
        flags.startVector_defined = ReadTagDVector3d (value, "startVector", detail.startVector);
        flags.endVector_defined = ReadTagDVector3d (value, "endVector", detail.endVector);
        flags.PointArray_defined = ReadListOfDPoint3d (value, "ListOfPoint", "Point", detail.PointArray);
        flags.KnotArray_defined = ReadListOfdouble (value, "ListOfKnot", "Knot", detail.KnotArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ILineString
/// Returns false if the reader is not positioned on an element with name LineString
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;LineString&gt;
///    &lt;ListOfPoint&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfPoint&gt;
///  &lt;/LineString&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadILineString (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "LineString"))
        {
        CGLineStringDetail detail;
        CGLineStringFlags flags;

        flags.PointArray_defined = ReadListOfDPoint3d (value, "ListOfPoint", "Point", detail.PointArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IOperation
/// Returns false if the reader is not positioned on an element with name Operation
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Operation&gt;
///    &lt;Name&gt;... &lt;/Name&gt;
///    &lt;ListOfMember&gt;
///         &lt;Member&gt;...&lt;/Member&gt;
///    &lt;/ListOfMember&gt;
///  &lt;/Operation&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIOperation (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Operation"))
        {
        CGOperationDetail detail;
        CGOperationFlags flags;

        flags.name_defined = ReadTagString (value, "name", detail.name);
        flags.memberArray_defined = ReadListOfIGeometry (value, "ListOfMember", "Member", detail.memberArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IParametricSurfacePatch
/// Returns false if the reader is not positioned on an element with name ParametricSurfacePatch
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;ParametricSurfacePatch&gt;
///    &lt;LoopType&gt;... &lt;/LoopType&gt;
///    &lt;Surface&gt;... &lt;/Surface&gt;
///    &lt;ListOfCurveChain&gt;
///         &lt;CurveChain&gt;...&lt;/CurveChain&gt;
///    &lt;/ListOfCurveChain&gt;
///  &lt;/ParametricSurfacePatch&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIParametricSurfacePatch (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "ParametricSurfacePatch"))
        {
        CGParametricSurfacePatchDetail detail;
        CGParametricSurfacePatchFlags flags;

        flags.loopType_defined = ReadTagLoopType (value, "loopType", detail.loopType);
        flags.surface_defined = ReadTag_AnyParametricSurface (value, "surface", detail.surface);
        flags.loopArray_defined = ReadListOfICurveChain (value, "ListOfCurveChain", "CurveChain", detail.loopArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IPointChain
/// Returns false if the reader is not positioned on an element with name PointChain
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;PointChain&gt;
///    &lt;ListOfPoint&gt;
///         &lt;Point&gt;...&lt;/Point&gt;
///    &lt;/ListOfPoint&gt;
///  &lt;/PointChain&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIPointChain (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "PointChain"))
        {
        CGPointChainDetail detail;
        CGPointChainFlags flags;

        flags.PointArray_defined = ReadListOfISinglePoint (value, "ListOfPoint", "Point", detail.PointArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IPointGroup
/// Returns false if the reader is not positioned on an element with name PointGroup
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;PointGroup&gt;
///    &lt;ListOfMember&gt;
///         &lt;Member&gt;...&lt;/Member&gt;
///    &lt;/ListOfMember&gt;
///  &lt;/PointGroup&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIPointGroup (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "PointGroup"))
        {
        CGPointGroupDetail detail;
        CGPointGroupFlags flags;

        flags.memberArray_defined = ReadListOfIPoint (value, "ListOfMember", "Member", detail.memberArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IPolygon
/// Returns false if the reader is not positioned on an element with name Polygon
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Polygon&gt;
///    &lt;ListOfPoint&gt;
///         &lt;xyz&gt;...&lt;/xyz&gt;
///    &lt;/ListOfPoint&gt;
///  &lt;/Polygon&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIPolygon (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Polygon"))
        {
        CGPolygonDetail detail;
        CGPolygonFlags flags;

        flags.pointArray_defined = ReadListOfDPoint3d (value, "ListOfPoint", "Point", detail.pointArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IPrimitiveCurveReference
/// Returns false if the reader is not positioned on an element with name PrimitiveCurveReference
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;PrimitiveCurveReference&gt;
///    &lt;Reversed&gt;... &lt;/Reversed&gt;
///    &lt;ParentCurve&gt;... &lt;/ParentCurve&gt;
///  &lt;/PrimitiveCurveReference&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIPrimitiveCurveReference (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "PrimitiveCurveReference"))
        {
        CGPrimitiveCurveReferenceDetail detail;
        CGPrimitiveCurveReferenceFlags flags;

        flags.reversed_defined = ReadTagbool (value, "reversed", detail.reversed);
        flags.parentCurve_defined = ReadTag_AnyICurvePrimitive (value, "parentCurve", detail.parentCurve);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IPartialCurve
/// Returns false if the reader is not positioned on an element with name PartialCurve
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;PartialCurve&gt;
///    &lt;Fraction0&gt;... &lt;/Fraction0&gt;
///    &lt;Fraction1&gt;... &lt;/Fraction1&gt;
///    &lt;ParentCurve&gt;... &lt;/ParentCurve&gt;
///  &lt;/PartialCurve&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIPartialCurve (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "PartialCurve"))
        {
        CGPartialCurveDetail detail;
        CGPartialCurveFlags flags;

        flags.fraction0_defined = ReadTagdouble (value, "fraction0", detail.fraction0);
        flags.fraction1_defined = ReadTagdouble (value, "fraction1", detail.fraction1);
        flags.parentCurve_defined = ReadTag_AnyICurvePrimitive (value, "parentCurve", detail.parentCurve);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISharedGroupDef
/// Returns false if the reader is not positioned on an element with name SharedGroupDef
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SharedGroupDef&gt;
///    &lt;Name&gt;... &lt;/Name&gt;
///    &lt;Geometry&gt;... &lt;/Geometry&gt;
///  &lt;/SharedGroupDef&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISharedGroupDef (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SharedGroupDef"))
        {
        CGSharedGroupDefDetail detail;
        CGSharedGroupDefFlags flags;

        flags.name_defined = ReadTagString (value, "name", detail.name);
        flags.geometry_defined = ReadTag_AnyGeometry (value, "geometry", detail.geometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISharedGroupInstance
/// Returns false if the reader is not positioned on an element with name SharedGroupInstance
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SharedGroupInstance&gt;
///    &lt;SharedGroupName&gt;... &lt;/SharedGroupName&gt;
///    &lt;Transform&gt;... &lt;/Transform&gt;
///  &lt;/SharedGroupInstance&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISharedGroupInstance (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SharedGroupInstance"))
        {
        CGSharedGroupInstanceDetail detail;
        CGSharedGroupInstanceFlags flags;

        flags.sharedGroupName_defined = ReadTagString (value, "sharedGroupName", detail.sharedGroupName);
        flags.transform_defined = ReadTagTransform (value, "transform", detail.transform);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IShelledSolid
/// Returns false if the reader is not positioned on an element with name ShelledSolid
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;ShelledSolid&gt;
///    &lt;BoundingSurface&gt;... &lt;/BoundingSurface&gt;
///  &lt;/ShelledSolid&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIShelledSolid (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "ShelledSolid"))
        {
        CGShelledSolidDetail detail;
        CGShelledSolidFlags flags;

        flags.BoundingSurface_defined = ReadTag_AnySurface (value, "BoundingSurface", detail.BoundingSurface);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISolidBySweptSurface
/// Returns false if the reader is not positioned on an element with name SolidBySweptSurface
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SolidBySweptSurface&gt;
///    &lt;BaseGeometry&gt;... &lt;/BaseGeometry&gt;
///    &lt;RailCurve&gt;... &lt;/RailCurve&gt;
///  &lt;/SolidBySweptSurface&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISolidBySweptSurface (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SolidBySweptSurface"))
        {
        CGSolidBySweptSurfaceDetail detail;
        CGSolidBySweptSurfaceFlags flags;

        flags.baseGeometry_defined = ReadTag_AnySurface (value, "baseGeometry", detail.baseGeometry);
        flags.railCurve_defined = ReadTag_AnyCurve (value, "railCurve", detail.railCurve);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISolidByRuledSweep
/// Returns false if the reader is not positioned on an element with name SolidByRuledSweep
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SolidByRuledSweep&gt;
///    &lt;ListOfSection&gt;
///         &lt;section&gt;...&lt;/section&gt;
///    &lt;/ListOfSection&gt;
///  &lt;/SolidByRuledSweep&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISolidByRuledSweep (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SolidByRuledSweep"))
        {
        CGSolidByRuledSweepDetail detail;
        CGSolidByRuledSweepFlags flags;

        flags.SectionArray_defined = ReadListOf_AnyCurveVector (value, "ListOfSection", "Section", detail.SectionArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISurfaceByRuledSweep
/// Returns false if the reader is not positioned on an element with name SurfaceByRuledSweep
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SurfaceByRuledSweep&gt;
///    &lt;ListOfSection&gt;
///         &lt;section&gt;...&lt;/section&gt;
///    &lt;/ListOfSection&gt;
///  &lt;/SurfaceByRuledSweep&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISurfaceByRuledSweep (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SurfaceByRuledSweep"))
        {
        CGSurfaceByRuledSweepDetail detail;
        CGSurfaceByRuledSweepFlags flags;

        flags.SectionArray_defined = ReadListOf_AnyCurveVector (value, "ListOfSection", "Section", detail.SectionArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISolidGroup
/// Returns false if the reader is not positioned on an element with name SolidGroup
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SolidGroup&gt;
///    &lt;ListOfSolid&gt;
///         &lt;Solid&gt;...&lt;/Solid&gt;
///    &lt;/ListOfSolid&gt;
///  &lt;/SolidGroup&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISolidGroup (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SolidGroup"))
        {
        CGSolidGroupDetail detail;
        CGSolidGroupFlags flags;

        flags.solidArray_defined = ReadListOfISolid (value, "ListOfSolid", "Solid", detail.solidArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISpiral
/// Returns false if the reader is not positioned on an element with name Spiral
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;Spiral&gt;
///    &lt;SpiralType&gt;... &lt;/SpiralType&gt;
///    &lt;StartPoint&gt;... &lt;/StartPoint&gt;
///    &lt;StartBearing&gt;... &lt;/StartBearing&gt;
///    &lt;StartCurvature&gt;... &lt;/StartCurvature&gt;
///    &lt;EndPoint&gt;... &lt;/EndPoint&gt;
///    &lt;EndBearing&gt;... &lt;/EndBearing&gt;
///    &lt;EndCurvature&gt;... &lt;/EndCurvature&gt;
///    &lt;Geometry&gt;... &lt;/Geometry&gt;
///  &lt;/Spiral&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISpiral (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "Spiral"))
        {
        CGSpiralDetail detail;
        CGSpiralFlags flags;

        flags.spiralType_defined = ReadTagString (value, "spiralType", detail.spiralType);
        flags.startPoint_defined = ReadTagDPoint3d (value, "startPoint", detail.startPoint);
        flags.startBearing_defined = ReadTagAngle (value, "startBearing", detail.startBearing);
        flags.startCurvature_defined = ReadTagdouble (value, "startCurvature", detail.startCurvature);
        flags.endPoint_defined = ReadTagDPoint3d (value, "endPoint", detail.endPoint);
        flags.endBearing_defined = ReadTagAngle (value, "endBearing", detail.endBearing);
        flags.endCurvature_defined = ReadTagdouble (value, "endCurvature", detail.endCurvature);
        flags.geometry_defined = ReadTag_AnyGeometry (value, "geometry", detail.geometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISurfaceBySweptCurve
/// Returns false if the reader is not positioned on an element with name SurfaceBySweptCurve
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SurfaceBySweptCurve&gt;
///    &lt;BaseGeometry&gt;... &lt;/BaseGeometry&gt;
///    &lt;RailCurve&gt;... &lt;/RailCurve&gt;
///  &lt;/SurfaceBySweptCurve&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISurfaceBySweptCurve (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SurfaceBySweptCurve"))
        {
        CGSurfaceBySweptCurveDetail detail;
        CGSurfaceBySweptCurveFlags flags;

        flags.baseGeometry_defined = ReadTag_AnyCurve (value, "baseGeometry", detail.baseGeometry);
        flags.railCurve_defined = ReadTag_AnyCurve (value, "railCurve", detail.railCurve);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ISurfaceGroup
/// Returns false if the reader is not positioned on an element with name SurfaceGroup
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SurfaceGroup&gt;
///    &lt;ListOfSurface&gt;
///         &lt;Surface&gt;...&lt;/Surface&gt;
///    &lt;/ListOfSurface&gt;
///  &lt;/SurfaceGroup&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISurfaceGroup (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SurfaceGroup"))
        {
        CGSurfaceGroupDetail detail;
        CGSurfaceGroupFlags flags;

        flags.surfaceArray_defined = ReadListOfISurface (value, "ListOfSurface", "Surface", detail.surfaceArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IGeometryPtr
/// Returns false if the reader is not positioned on an element with name SurfacePatch
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;SurfacePatch&gt;
///    &lt;ExteriorLoop&gt;... &lt;/ExteriorLoop&gt;
///    &lt;ListOfHoleLoop&gt;
///         &lt;CurveChain&gt;...&lt;/CurveChain&gt;
///    &lt;/ListOfHoleLoop&gt;
///  &lt;/SurfacePatch&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISurfacePatch (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "SurfacePatch"))
        {
        CGSurfacePatchDetail detail;
        CGSurfacePatchFlags flags;

        flags.exteriorLoop_defined = ReadTag_AnyCurveChain (value, "exteriorLoop", detail.exteriorLoop);
        flags.holeLoopArray_defined = ReadListOfICurveChain (value, "ListOfHoleLoop", "HoleLoop", detail.holeLoopArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ITransformedGeometry
/// Returns false if the reader is not positioned on an element with name TransformedGeometry
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;TransformedGeometry&gt;
///    &lt;Transform&gt;... &lt;/Transform&gt;
///    &lt;Geometry&gt;... &lt;/Geometry&gt;
///  &lt;/TransformedGeometry&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadITransformedGeometry (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "TransformedGeometry"))
        {
        CGTransformedGeometryDetail detail;
        CGTransformedGeometryFlags flags;

        flags.transform_defined = ReadTagTransform (value, "transform", detail.transform);
        flags.geometry_defined = ReadTag_AnyGeometry (value, "geometry", detail.geometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IDgnExtrusion
/// Returns false if the reader is not positioned on an element with name DgnExtrusion
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnExtrusion&gt;
///    &lt;ExtrusionVector&gt;... &lt;/ExtrusionVector&gt;
///    &lt;Capped&gt;... &lt;/Capped&gt;
///    &lt;BaseGeometry&gt;... &lt;/BaseGeometry&gt;
///  &lt;/DgnExtrusion&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnExtrusion (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnExtrusion"))
        {
        CGDgnExtrusionDetail detail;
        CGDgnExtrusionFlags flags;

        flags.extrusionVector_defined = ReadTagDVector3d (value, "extrusionVector", detail.extrusionVector);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);
        flags.baseGeometry_defined = ReadTag_AnyCurveVector (value, "baseGeometry", detail.baseGeometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IDgnRotationalSweep
/// Returns false if the reader is not positioned on an element with name DgnRotationalSweep
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnRotationalSweep&gt;
///    &lt;Center&gt;... &lt;/Center&gt;
///    &lt;Axis&gt;... &lt;/Axis&gt;
///    &lt;SweepAngle&gt;... &lt;/SweepAngle&gt;
///    &lt;Capped&gt;... &lt;/Capped&gt;
///    &lt;BaseGeometry&gt;... &lt;/BaseGeometry&gt;
///  &lt;/DgnRotationalSweep&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnRotationalSweep (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnRotationalSweep"))
        {
        CGDgnRotationalSweepDetail detail;
        CGDgnRotationalSweepFlags flags;

        flags.center_defined = ReadTagDPoint3d (value, "center", detail.center);
        flags.axis_defined = ReadTagDVector3d (value, "axis", detail.axis);
        flags.sweepAngle_defined = ReadTagAngle (value, "sweepAngle", detail.sweepAngle);
        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);
        flags.baseGeometry_defined = ReadTag_AnyCurveVector (value, "baseGeometry", detail.baseGeometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IDgnRuledSweep
/// Returns false if the reader is not positioned on an element with name DgnRuledSweep
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnRuledSweep&gt;
///    &lt;Capped&gt;... &lt;/Capped&gt;
///    &lt;ListOfContour&gt;
///         &lt;Contour&gt;...&lt;/Contour&gt;
///    &lt;/ListOfContour&gt;
///  &lt;/DgnRuledSweep&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnRuledSweep (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnRuledSweep"))
        {
        CGDgnRuledSweepDetail detail;
        CGDgnRuledSweepFlags flags;

        flags.capped_defined = ReadTagbool (value, "capped", detail.capped);
        flags.contourArray_defined = ReadListOf_AnyCurveVector (value, "ListOfContour", "Contour", detail.contourArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an ITransitionSpiral
/// Returns false if the reader is not positioned on an element with name TransitionSpiral
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;TransitionSpiral&gt;
///    &lt;SpiralType&gt;... &lt;/SpiralType&gt;
///    &lt;Placement&gt;... &lt;/Placement&gt;
///    &lt;StartBearing&gt;... &lt;/StartBearing&gt;
///    &lt;StartRadius&gt;... &lt;/StartRadius&gt;
///    &lt;EndBearing&gt;... &lt;/EndBearing&gt;
///    &lt;Length&gt;... &lt;/Length&gt;
///    &lt;EndRadius&gt;... &lt;/EndRadius&gt;
///    &lt;ActiveStartFraction&gt;... &lt;/ActiveStartFraction&gt;
///    &lt;ActiveEndFraction&gt;... &lt;/ActiveEndFraction&gt;
///    &lt;Geometry&gt;... &lt;/Geometry&gt;
///  &lt;/TransitionSpiral&gt;
/// </code>
/// only one of (EndBearing, Length) should be provided.
/// </summary>
/// <returns></returns>
bool ReadITransitionSpiral (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "TransitionSpiral"))
        {
        CGTransitionSpiralDetail detail;
        CGTransitionSpiralFlags flags;

        flags.spiralType_defined = ReadTagString (value, "spiralType", detail.spiralType);
        flags.placement_defined = ReadTagPlacementOriginZX (value, "placement", detail.placement);
        flags.startBearing_defined = ReadTagAngle (value, "startBearing", detail.startBearing);
        flags.startRadius_defined = ReadTagdouble (value, "startRadius", detail.startRadius);
        flags.endBearing_defined = ReadTagAngle (value, "endBearing", detail.endBearing);   // optional !!!
        flags.length_defined= ReadTagdouble (value, "length", detail.length);        // optional !!!
        flags.endRadius_defined = ReadTagdouble (value, "endRadius", detail.endRadius);    
        flags.activeStartFraction_defined = ReadTagdouble (value, "activeStartFraction", detail.activeStartFraction);
        flags.activeEndFraction_defined = ReadTagdouble (value, "activeEndFraction", detail.activeEndFraction);
        flags.geometry_defined = ReadTag_AnyGeometry (value, "geometry", detail.geometry);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





/// <summary>
/// Read xml content and create an IDgnCurveVector
/// Returns false if the reader is not positioned on an element with name DgnCurveVector
///   or if it does not contain the following element sequence:
/// <code>
///  &lt;DgnCurveVector&gt;
///    &lt;BoundaryType&gt;... &lt;/BoundaryType&gt;
///    &lt;ListOfMember&gt;
///         &lt;Member&gt;...&lt;/Member&gt;
///    &lt;/ListOfMember&gt;
///  &lt;/DgnCurveVector&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIDgnCurveVector (TSource const &value, IGeometryPtr &result)
    {
    result = nullptr;
    if (IsCGType (value, "DgnCurveVector"))
        {
        CGDgnCurveVectorDetail detail;
        CGDgnCurveVectorFlags flags;

        flags.boundaryType_defined = ReadTagint (value, "boundaryType", detail.boundaryType);
        flags.memberArray_defined = ReadListOf_AnyICurvePrimitive (value, "ListOfMember", "Member", detail.memberArray);
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }





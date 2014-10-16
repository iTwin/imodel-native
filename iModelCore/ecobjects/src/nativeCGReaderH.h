



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
bool ReadILineSegment (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("LineSegment")
        && ReadToChildOrEnd ())
        {
        CGLineSegmentDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("startPoint")
                && ReadTagDPoint3d ("startPoint", detail.startPoint))
                continue;

            if (   CurrentElementNameMatch ("endPoint")
                && ReadTagDPoint3d ("endPoint", detail.endPoint))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadICircularArc (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularArc")
        && ReadToChildOrEnd ())
        {
        CGCircularArcDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", detail.radius))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", detail.startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", detail.sweepAngle))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnBox (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnBox")
        && ReadToChildOrEnd ())
        {
        CGDgnBoxDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("baseOrigin")
                && ReadTagDPoint3d ("baseOrigin", detail.baseOrigin))
                continue;

            if (   CurrentElementNameMatch ("topOrigin")
                && ReadTagDPoint3d ("topOrigin", detail.topOrigin))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", detail.vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", detail.vectorY))
                continue;

            if (   CurrentElementNameMatch ("baseX")
                && ReadTagdouble ("baseX", detail.baseX))
                continue;

            if (   CurrentElementNameMatch ("baseY")
                && ReadTagdouble ("baseY", detail.baseY))
                continue;

            if (   CurrentElementNameMatch ("topX")
                && ReadTagdouble ("topX", detail.topX))
                continue;

            if (   CurrentElementNameMatch ("topY")
                && ReadTagdouble ("topY", detail.topY))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnSphere (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnSphere")
        && ReadToChildOrEnd ())
        {
        CGDgnSphereDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("center")
                && ReadTagDPoint3d ("center", detail.center))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", detail.vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorZ")
                && ReadTagDVector3d ("vectorZ", detail.vectorZ))
                continue;

            if (   CurrentElementNameMatch ("radiusXY")
                && ReadTagdouble ("radiusXY", detail.radiusXY))
                continue;

            if (   CurrentElementNameMatch ("radiusZ")
                && ReadTagdouble ("radiusZ", detail.radiusZ))
                continue;

            if (   CurrentElementNameMatch ("startLatitude")
                && ReadTagAngle ("startLatitude", detail.startLatitude))
                continue;

            if (   CurrentElementNameMatch ("latitudeSweep")
                && ReadTagAngle ("latitudeSweep", detail.latitudeSweep))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnCone (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnCone")
        && ReadToChildOrEnd ())
        {
        CGDgnConeDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("centerA")
                && ReadTagDPoint3d ("centerA", detail.centerA))
                continue;

            if (   CurrentElementNameMatch ("centerB")
                && ReadTagDPoint3d ("centerB", detail.centerB))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", detail.vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", detail.vectorY))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnTorusPipe (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnTorusPipe")
        && ReadToChildOrEnd ())
        {
        CGDgnTorusPipeDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("center")
                && ReadTagDPoint3d ("center", detail.center))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", detail.vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", detail.vectorY))
                continue;

            if (   CurrentElementNameMatch ("majorRadius")
                && ReadTagdouble ("majorRadius", detail.majorRadius))
                continue;

            if (   CurrentElementNameMatch ("minorRadius")
                && ReadTagdouble ("minorRadius", detail.minorRadius))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", detail.sweepAngle))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/Block&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadIBlock (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Block")
        && ReadToChildOrEnd ())
        {
        CGBlockDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("cornerA")
                && ReadTagDPoint3d ("cornerA", detail.cornerA))
                continue;

            if (   CurrentElementNameMatch ("cornerB")
                && ReadTagDPoint3d ("cornerB", detail.cornerB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);//Block(detail.placement,detail.cornerA,detail.cornerB,detail.capped);
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
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/CircularCone&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularCone (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularCone")
        && ReadToChildOrEnd ())
        {
        CGCircularConeDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("height")
                && ReadTagdouble ("height", detail.height))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);//CircularCone(detail.placement,detail.height,detail.radiusA,detail.radiusB,detail.capped);
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
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/CircularCylinder&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadICircularCylinder (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularCylinder")
        && ReadToChildOrEnd ())
        {
        CGCircularCylinderDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("height")
                && ReadTagdouble ("height", detail.height))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", detail.radius))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);//CircularCylinder(detail.placement,detail.height,detail.radius,detail.capped);
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
bool ReadICircularDisk (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularDisk")
        && ReadToChildOrEnd ())
        {
        CGCircularDiskDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", detail.radius))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadICoordinate (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Coordinate")
        && ReadToChildOrEnd ())
        {
        CGCoordinateDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("xyz")
                && ReadTagDPoint3d ("xyz", detail.xyz))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIEllipticArc (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("EllipticArc")
        && ReadToChildOrEnd ())
        {
        CGEllipticArcDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", detail.startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", detail.sweepAngle))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIEllipticDisk (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("EllipticDisk")
        && ReadToChildOrEnd ())
        {
        CGEllipticDiskDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISingleLineText (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SingleLineText")
        && ReadToChildOrEnd ())
        {
        CGSingleLineTextDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("textString")
                && ReadTagString ("textString", detail.textString))
                continue;

            if (   CurrentElementNameMatch ("fontName")
                && ReadTagString ("fontName", detail.fontName))
                continue;

            if (   CurrentElementNameMatch ("characterXSize")
                && ReadTagdouble ("characterXSize", detail.characterXSize))
                continue;

            if (   CurrentElementNameMatch ("characterYSize")
                && ReadTagdouble ("characterYSize", detail.characterYSize))
                continue;

            if (   CurrentElementNameMatch ("justification")
                && ReadTagint ("justification", detail.justification))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/SkewedCone&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadISkewedCone (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SkewedCone")
        && ReadToChildOrEnd ())
        {
        CGSkewedConeDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("centerB")
                && ReadTagDPoint3d ("centerB", detail.centerB))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);//SkewedCone(detail.placement,detail.centerB,detail.radiusA,detail.radiusB,detail.capped);
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
bool ReadISphere (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Sphere")
        && ReadToChildOrEnd ())
        {
        CGSphereDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", detail.radius))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
///    &lt;capped&gt;... &lt;/capped&gt;
///  &lt;/TorusPipe&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadITorusPipe (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("TorusPipe")
        && ReadToChildOrEnd ())
        {
        CGTorusPipeDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", detail.radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", detail.radiusB))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", detail.startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", detail.sweepAngle))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", detail.capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);//TorusPipe(detail.placement,detail.radiusA,detail.radiusB,detail.startAngle,detail.sweepAngle,detail.capped);
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
bool ReadIVector (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Vector")
        && ReadToChildOrEnd ())
        {
        CGVectorDetail detail;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("xyz")
                && ReadTagDPoint3d ("xyz", detail.xyz))
                continue;

            if (   CurrentElementNameMatch ("vector")
                && ReadTagDVector3d ("vector", detail.vector))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIIndexedMesh (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("IndexedMesh")
        && ReadToChildOrEnd ())
        {
        CGIndexedMeshDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfCoord", "Coord", detail.xyzArray))
                continue;
            if (ReadListOfint ("ListOfCoordIndex", "CoordIndex", detail.coordIndexArray))
                continue;
            if (ReadListOfDPoint2d ("ListOfParam", "Param", detail.uvArray))
                continue;
            if (ReadListOfint ("ListOfParamIndex", "ParamIndex", detail.paramIndexArray))
                continue;
            if (ReadListOfDVector3d ("ListOfNormal", "Normal", detail.normalArray))
                continue;
            if (ReadListOfint ("ListOfNormalIndex", "NormalIndex", detail.normalIndexArray))
                continue;
            if (ReadListOfDVector3d ("ListOfColor", "Color", detail.colorArray))
                continue;
            if (ReadListOfint ("ListOfColorIndex", "ColorIndex", detail.colorIndexArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIAdjacentSurfacePatches (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("AdjacentSurfacePatches")
        && ReadToChildOrEnd ())
        {
        CGAdjacentSurfacePatchesDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISurfacePatch ("ListOfPatch", "Patch", detail.patchArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIBsplineCurve (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("BsplineCurve")
        && ReadToChildOrEnd ())
        {
        CGBsplineCurveDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("order", detail.order))
                continue;
            if (ReadTagbool ("closed", detail.closed))
                continue;
            if (ReadListOfDPoint3d ("ListOfControlPoint", "ControlPoint", detail.controlPointArray))
                continue;
            if (ReadListOfdouble ("ListOfWeight", "Weight", detail.weightArray))
                continue;
            if (ReadListOfdouble ("ListOfKnot", "Knot", detail.knotArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIBsplineSurface (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("BsplineSurface")
        && ReadToChildOrEnd ())
        {
        CGBsplineSurfaceDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("orderU", detail.orderU))
                continue;
            if (ReadTagbool ("closedU", detail.closedU))
                continue;
            if (ReadTagint ("numUControlPoint", detail.numUControlPoint))
                continue;
            if (ReadTagint ("orderV", detail.orderV))
                continue;
            if (ReadTagbool ("closedV", detail.closedV))
                continue;
            if (ReadTagint ("numVControlPoint", detail.numVControlPoint))
                continue;
            if (ReadListOfDPoint3d ("ListOfControlPoint", "ControlPoint", detail.controlPointArray))
                continue;
            if (ReadListOfdouble ("ListOfWeight", "Weight", detail.weightArray))
                continue;
            if (ReadListOfdouble ("ListOfKnotU", "KnotU", detail.knotUArray))
                continue;
            if (ReadListOfdouble ("ListOfKnotV", "KnotV", detail.knotVArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadICurveChain (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CurveChain")
        && ReadToChildOrEnd ())
        {
        CGCurveChainDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOf_AnyICurvePrimitive ("ListOfCurve", "Curve", detail.curveArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadICurveGroup (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CurveGroup")
        && ReadToChildOrEnd ())
        {
        CGCurveGroupDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfICurve ("ListOfCurve", "Curve", detail.curveArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadICurveReference (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CurveReference")
        && ReadToChildOrEnd ())
        {
        CGCurveReferenceDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("reversed", detail.reversed))
                continue;
            if (ReadTag_AnyCurve ("parentCurve", detail.parentCurve))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIGroup (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Group")
        && ReadToChildOrEnd ())
        {
        CGGroupDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfIGeometry ("ListOfMember", "Member", detail.memberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIInterpolatingCurve (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("InterpolatingCurve")
        && ReadToChildOrEnd ())
        {
        CGInterpolatingCurveDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("endConditionCode", detail.endConditionCode))
                continue;
            if (ReadTagint ("knotCode", detail.knotCode))
                continue;
            if (ReadTagDVector3d ("startVector", detail.startVector))
                continue;
            if (ReadTagDVector3d ("endVector", detail.endVector))
                continue;
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", detail.PointArray))
                continue;
            if (ReadListOfdouble ("ListOfKnot", "Knot", detail.KnotArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadILineString (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("LineString")
        && ReadToChildOrEnd ())
        {
        CGLineStringDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", detail.PointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIOperation (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Operation")
        && ReadToChildOrEnd ())
        {
        CGOperationDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("name", detail.name))
                continue;
            if (ReadListOfIGeometry ("ListOfMember", "Member", detail.memberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIParametricSurfacePatch (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("ParametricSurfacePatch")
        && ReadToChildOrEnd ())
        {
        CGParametricSurfacePatchDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagLoopType ("loopType", detail.loopType))
                continue;
            if (ReadTag_AnyParametricSurface ("surface", detail.surface))
                continue;

            if (ReadListOfICurveChain ("ListOfCurveChain", "CurveChain", detail.loopArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIPointChain (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("PointChain")
        && ReadToChildOrEnd ())
        {
        CGPointChainDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISinglePoint ("ListOfPoint", "Point", detail.PointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIPointGroup (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("PointGroup")
        && ReadToChildOrEnd ())
        {
        CGPointGroupDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfIPoint ("ListOfMember", "Member", detail.memberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIPolygon (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Polygon")
        && ReadToChildOrEnd ())
        {
        CGPolygonDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", detail.pointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIPrimitiveCurveReference (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("PrimitiveCurveReference")
        && ReadToChildOrEnd ())
        {
        CGPrimitiveCurveReferenceDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("reversed", detail.reversed))
                continue;
            if (ReadTag_AnyICurvePrimitive ("parentCurve", detail.parentCurve))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISharedGroupDef (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SharedGroupDef")
        && ReadToChildOrEnd ())
        {
        CGSharedGroupDefDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("name", detail.name))
                continue;
            if (ReadTag_AnyGeometry ("geometry", detail.geometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISharedGroupInstance (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SharedGroupInstance")
        && ReadToChildOrEnd ())
        {
        CGSharedGroupInstanceDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("sharedGroupName", detail.sharedGroupName))
                continue;
            if (ReadTagTransform ("transform", detail.transform))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIShelledSolid (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("ShelledSolid")
        && ReadToChildOrEnd ())
        {
        CGShelledSolidDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTag_AnySurface ("BoundingSurface", detail.BoundingSurface))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISolidBySweptSurface (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SolidBySweptSurface")
        && ReadToChildOrEnd ())
        {
        CGSolidBySweptSurfaceDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTag_AnySurface ("baseGeometry", detail.baseGeometry))
                continue;

            if (ReadTag_AnyCurve ("railCurve", detail.railCurve))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISolidByRuledSweep (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SolidByRuledSweep")
        && ReadToChildOrEnd ())
        {
        CGSolidByRuledSweepDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOf_AnyGeometry ("ListOfSection", "Section", detail.SectionArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISurfaceByRuledSweep (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SurfaceByRuledSweep")
        && ReadToChildOrEnd ())
        {
        CGSurfaceByRuledSweepDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOf_AnyGeometry ("ListOfSection", "Section", detail.SectionArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISolidGroup (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SolidGroup")
        && ReadToChildOrEnd ())
        {
        CGSolidGroupDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISolid ("ListOfSolid", "Solid", detail.solidArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISpiral (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Spiral")
        && ReadToChildOrEnd ())
        {
        CGSpiralDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("spiralType", detail.spiralType))
                continue;
            if (ReadTagDPoint3d ("startPoint", detail.startPoint))
                continue;
            if (ReadTagAngle ("startBearing", detail.startBearing))
                continue;
            if (ReadTagdouble ("startCurvature", detail.startCurvature))
                continue;
            if (ReadTagDPoint3d ("endPoint", detail.endPoint))
                continue;
            if (ReadTagAngle ("endBearing", detail.endBearing))
                continue;
            if (ReadTagdouble ("endCurvature", detail.endCurvature))
                continue;
            if (ReadTag_AnyGeometry ("geometry", detail.geometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISurfaceBySweptCurve (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SurfaceBySweptCurve")
        && ReadToChildOrEnd ())
        {
        CGSurfaceBySweptCurveDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTag_AnyCurve ("baseGeometry", detail.baseGeometry))
                continue;

            if (ReadTag_AnyCurve ("railCurve", detail.railCurve))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISurfaceGroup (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SurfaceGroup")
        && ReadToChildOrEnd ())
        {
        CGSurfaceGroupDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISurface ("ListOfSurface", "Surface", detail.surfaceArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadISurfacePatch (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SurfacePatch")
        && ReadToChildOrEnd ())
        {
        CGSurfacePatchDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTag_AnyCurveChain ("exteriorLoop", detail.exteriorLoop))
                continue;

            if (ReadListOfICurveChain ("ListOfHoleLoop", "HoleLoop", detail.holeLoopArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadITransformedGeometry (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("TransformedGeometry")
        && ReadToChildOrEnd ())
        {
        CGTransformedGeometryDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagTransform ("transform", detail.transform))
                continue;
            if (ReadTag_AnyGeometry ("geometry", detail.geometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnExtrusion (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnExtrusion")
        && ReadToChildOrEnd ())
        {
        CGDgnExtrusionDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagDVector3d ("extrusionVector", detail.extrusionVector))
                continue;
            if (ReadTagbool ("capped", detail.capped))
                continue;
            if (ReadTag_AnyCurveVector ("baseGeometry", detail.baseGeometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnRotationalSweep (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnRotationalSweep")
        && ReadToChildOrEnd ())
        {
        CGDgnRotationalSweepDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagDPoint3d ("center", detail.center))
                continue;
            if (ReadTagDVector3d ("axis", detail.axis))
                continue;
            if (ReadTagAngle ("sweepAngle", detail.sweepAngle))
                continue;
            if (ReadTagbool ("capped", detail.capped))
                continue;
            if (ReadTag_AnyCurveVector ("baseGeometry", detail.baseGeometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
bool ReadIDgnRuledSweep (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("DgnRuledSweep")
        && ReadToChildOrEnd ())
        {
        CGDgnRuledSweepDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("capped", detail.capped))
                continue;
            if (ReadListOf_AnyCurveVector ("ListOfContour", "Contour", detail.contourArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
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
///    &lt;EndRadius&gt;... &lt;/EndRadius&gt;
///    &lt;ActiveStartFraction&gt;... &lt;/ActiveStartFraction&gt;
///    &lt;ActiveEndFraction&gt;... &lt;/ActiveEndFraction&gt;
///    &lt;Geometry&gt;... &lt;/Geometry&gt;
///  &lt;/TransitionSpiral&gt;
/// </code>
/// </summary>
/// <returns></returns>
bool ReadITransitionSpiral (IGeometryPtr &result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("TransitionSpiral")
        && ReadToChildOrEnd ())
        {
        CGTransitionSpiralDetail detail;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("spiralType", detail.spiralType))
                continue;
            if (ReadTagPlacementOriginZX ("placement", detail.placement))
                continue;
            if (ReadTagAngle ("startBearing", detail.startBearing))
                continue;
            if (ReadTagdouble ("startRadius", detail.startRadius))
                continue;
            if (ReadTagAngle ("endBearing", detail.endBearing))
                continue;
            if (ReadTagdouble ("endRadius", detail.endRadius))
                continue;
            if (ReadTagdouble ("activeStartFraction", detail.activeStartFraction))
                continue;
            if (ReadTagdouble ("activeEndFraction", detail.activeEndFraction))
                continue;
            if (ReadTag_AnyGeometry ("geometry", detail.geometry))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.Create (detail);
        return true;
        }
    return false;
    }









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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d startPoint = s_default_DPoint3d;
        DPoint3d endPoint = s_default_DPoint3d;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("startPoint")
                && ReadTagDPoint3d ("startPoint", startPoint))
                continue;

            if (   CurrentElementNameMatch ("endPoint")
                && ReadTagDPoint3d ("endPoint", endPoint))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateLineSegment( startPoint, endPoint);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radius = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", radius))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", sweepAngle))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCircularArc( placement, radius, startAngle, sweepAngle);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d baseOrigin = s_default_DPoint3d;
        DPoint3d topOrigin = s_default_DPoint3d;
        DVector3d vectorX = s_default_DVector3d;
        DVector3d vectorY = s_default_DVector3d;
        double baseX = s_default_double;
        double baseY = s_default_double;
        double topX = s_default_double;
        double topY = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("baseOrigin")
                && ReadTagDPoint3d ("baseOrigin", baseOrigin))
                continue;

            if (   CurrentElementNameMatch ("topOrigin")
                && ReadTagDPoint3d ("topOrigin", topOrigin))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", vectorY))
                continue;

            if (   CurrentElementNameMatch ("baseX")
                && ReadTagdouble ("baseX", baseX))
                continue;

            if (   CurrentElementNameMatch ("baseY")
                && ReadTagdouble ("baseY", baseY))
                continue;

            if (   CurrentElementNameMatch ("topX")
                && ReadTagdouble ("topX", topX))
                continue;

            if (   CurrentElementNameMatch ("topY")
                && ReadTagdouble ("topY", topY))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnBox( baseOrigin, topOrigin, vectorX, vectorY, baseX, baseY, topX, topY, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d center = s_default_DPoint3d;
        DVector3d vectorX = s_default_DVector3d;
        DVector3d vectorZ = s_default_DVector3d;
        double radiusXY = s_default_double;
        double radiusZ = s_default_double;
        Angle startLatitude = s_default_Angle;
        Angle latitudeSweep = s_default_Angle;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("center")
                && ReadTagDPoint3d ("center", center))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorZ")
                && ReadTagDVector3d ("vectorZ", vectorZ))
                continue;

            if (   CurrentElementNameMatch ("radiusXY")
                && ReadTagdouble ("radiusXY", radiusXY))
                continue;

            if (   CurrentElementNameMatch ("radiusZ")
                && ReadTagdouble ("radiusZ", radiusZ))
                continue;

            if (   CurrentElementNameMatch ("startLatitude")
                && ReadTagAngle ("startLatitude", startLatitude))
                continue;

            if (   CurrentElementNameMatch ("latitudeSweep")
                && ReadTagAngle ("latitudeSweep", latitudeSweep))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnSphere( center, vectorX, vectorZ, radiusXY, radiusZ, startLatitude, latitudeSweep, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d centerA = s_default_DPoint3d;
        DPoint3d centerB = s_default_DPoint3d;
        DVector3d vectorX = s_default_DVector3d;
        DVector3d vectorY = s_default_DVector3d;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("centerA")
                && ReadTagDPoint3d ("centerA", centerA))
                continue;

            if (   CurrentElementNameMatch ("centerB")
                && ReadTagDPoint3d ("centerB", centerB))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", vectorY))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnCone( centerA, centerB, vectorX, vectorY, radiusA, radiusB, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d center = s_default_DPoint3d;
        DVector3d vectorX = s_default_DVector3d;
        DVector3d vectorY = s_default_DVector3d;
        double majorRadius = s_default_double;
        double minorRadius = s_default_double;
        Angle sweepAngle = s_default_Angle;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("center")
                && ReadTagDPoint3d ("center", center))
                continue;

            if (   CurrentElementNameMatch ("vectorX")
                && ReadTagDVector3d ("vectorX", vectorX))
                continue;

            if (   CurrentElementNameMatch ("vectorY")
                && ReadTagDVector3d ("vectorY", vectorY))
                continue;

            if (   CurrentElementNameMatch ("majorRadius")
                && ReadTagdouble ("majorRadius", majorRadius))
                continue;

            if (   CurrentElementNameMatch ("minorRadius")
                && ReadTagdouble ("minorRadius", minorRadius))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", sweepAngle))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnTorusPipe( center, vectorX, vectorY, majorRadius, minorRadius, sweepAngle, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        DPoint3d cornerA = s_default_DPoint3d;
        DPoint3d cornerB = s_default_DPoint3d;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("cornerA")
                && ReadTagDPoint3d ("cornerA", cornerA))
                continue;

            if (   CurrentElementNameMatch ("cornerB")
                && ReadTagDPoint3d ("cornerB", cornerB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateBlock( placement, cornerA, cornerB, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double height = s_default_double;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("height")
                && ReadTagdouble ("height", height))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCircularCone( placement, height, radiusA, radiusB, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double height = s_default_double;
        double radius = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("height")
                && ReadTagdouble ("height", height))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", radius))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCircularCylinder( placement, height, radius, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radius = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", radius))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCircularDisk( placement, radius);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d xyz = s_default_DPoint3d;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("xyz")
                && ReadTagDPoint3d ("xyz", xyz))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCoordinate( xyz);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", sweepAngle))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateEllipticArc( placement, radiusA, radiusB, startAngle, sweepAngle);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radiusA = s_default_double;
        double radiusB = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateEllipticDisk( placement, radiusA, radiusB);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        String textString = s_default_String;
        String fontName = s_default_String;
        double characterXSize = s_default_double;
        double characterYSize = s_default_double;
        int justification = s_default_int;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("textString")
                && ReadTagString ("textString", textString))
                continue;

            if (   CurrentElementNameMatch ("fontName")
                && ReadTagString ("fontName", fontName))
                continue;

            if (   CurrentElementNameMatch ("characterXSize")
                && ReadTagdouble ("characterXSize", characterXSize))
                continue;

            if (   CurrentElementNameMatch ("characterYSize")
                && ReadTagdouble ("characterYSize", characterYSize))
                continue;

            if (   CurrentElementNameMatch ("justification")
                && ReadTagint ("justification", justification))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSingleLineText( placement, textString, fontName, characterXSize, characterYSize, justification);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        DPoint3d centerB = s_default_DPoint3d;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("centerB")
                && ReadTagDPoint3d ("centerB", centerB))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSkewedCone( placement, centerB, radiusA, radiusB, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radius = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", radius))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSphere( placement, radius);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagPlacementOriginZX ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("radiusA")
                && ReadTagdouble ("radiusA", radiusA))
                continue;

            if (   CurrentElementNameMatch ("radiusB")
                && ReadTagdouble ("radiusB", radiusB))
                continue;

            if (   CurrentElementNameMatch ("startAngle")
                && ReadTagAngle ("startAngle", startAngle))
                continue;

            if (   CurrentElementNameMatch ("sweepAngle")
                && ReadTagAngle ("sweepAngle", sweepAngle))
                continue;

            if (   CurrentElementNameMatch ("capped", "bSolidFlag")
                && ReadTagbool ("capped", capped))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateTorusPipe( placement, radiusA, radiusB, startAngle, sweepAngle, capped);
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
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        DPoint3d xyz = s_default_DPoint3d;
        DVector3d vector = s_default_DVector3d;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("xyz")
                && ReadTagDPoint3d ("xyz", xyz))
                continue;

            if (   CurrentElementNameMatch ("vector")
                && ReadTagDVector3d ("vector", vector))
                continue;

            if (!SkipUnexpectedTag ())
                return false;
            }
        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateVector( xyz, vector);
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
        && ReadToChild ())
        {
        // Gather children
        bvector<DPoint3d> CoordArray;
        bvector<int> CoordIndexArray;
        bvector<DPoint2d> ParamArray;
        bvector<int> ParamIndexArray;
        bvector<DVector3d> NormalArray;
        bvector<int> NormalIndexArray;
        bvector<DVector3d> ColorArray;
        bvector<int> ColorIndexArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfCoord", "Coord", CoordArray))
                continue;
            if (ReadListOfint ("ListOfCoordIndex", "CoordIndex", CoordIndexArray))
                continue;
            if (ReadListOfDPoint2d ("ListOfParam", "Param", ParamArray))
                continue;
            if (ReadListOfint ("ListOfParamIndex", "ParamIndex", ParamIndexArray))
                continue;
            if (ReadListOfDVector3d ("ListOfNormal", "Normal", NormalArray))
                continue;
            if (ReadListOfint ("ListOfNormalIndex", "NormalIndex", NormalIndexArray))
                continue;
            if (ReadListOfDVector3d ("ListOfColor", "Color", ColorArray))
                continue;
            if (ReadListOfint ("ListOfColorIndex", "ColorIndex", ColorIndexArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateIndexedMesh (
            CoordArray,
            CoordIndexArray,
            ParamArray,
            ParamIndexArray,
            NormalArray,
            NormalIndexArray,
            ColorArray,
            ColorIndexArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> PatchArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISurfacePatch ("ListOfPatch", "Patch", PatchArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateAdjacentSurfacePatches (
            PatchArray);

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
        && ReadToChild ())
        {
        // Gather children
        int order = s_default_int;
        bool closed = s_default_bool;
        bvector<DPoint3d> ControlPointArray;
        bvector<double> WeightArray;
        bvector<double> KnotArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("order", order))
                continue;
            if (ReadTagbool ("closed", closed))
                continue;
            if (ReadListOfDPoint3d ("ListOfControlPoint", "ControlPoint", ControlPointArray))
                continue;
            if (ReadListOfdouble ("ListOfWeight", "Weight", WeightArray))
                continue;
            if (ReadListOfdouble ("ListOfKnot", "Knot", KnotArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateBsplineCurve (
            order,
            closed,
            ControlPointArray,
            WeightArray,
            KnotArray);

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
        && ReadToChild ())
        {
        // Gather children
        int orderU = s_default_int;
        bool closedU = s_default_bool;
        int numUControlPoint = s_default_int;
        int orderV = s_default_int;
        bool closedV = s_default_bool;
        int numVControlPoint = s_default_int;
        bvector<DPoint3d> ControlPointArray;
        bvector<double> WeightArray;
        bvector<double> KnotUArray;
        bvector<double> KnotVArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("orderU", orderU))
                continue;
            if (ReadTagbool ("closedU", closedU))
                continue;
            if (ReadTagint ("numUControlPoint", numUControlPoint))
                continue;
            if (ReadTagint ("orderV", orderV))
                continue;
            if (ReadTagbool ("closedV", closedV))
                continue;
            if (ReadTagint ("numVControlPoint", numVControlPoint))
                continue;
            if (ReadListOfDPoint3d ("ListOfControlPoint", "ControlPoint", ControlPointArray))
                continue;
            if (ReadListOfdouble ("ListOfWeight", "Weight", WeightArray))
                continue;
            if (ReadListOfdouble ("ListOfKnotU", "KnotU", KnotUArray))
                continue;
            if (ReadListOfdouble ("ListOfKnotV", "KnotV", KnotVArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateBsplineSurface (
            orderU,
            closedU,
            numUControlPoint,
            orderV,
            closedV,
            numVControlPoint,
            ControlPointArray,
            WeightArray,
            KnotUArray,
            KnotVArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<ICurvePrimitivePtr> CurveArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfIPrimitiveCurve ("ListOfCurve", "Curve", CurveArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCurveChain (
            CurveArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> CurveArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfICurve ("ListOfCurve", "Curve", CurveArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCurveGroup (
            CurveArray);

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
        && ReadToChild ())
        {
        // Gather children
        bool reversed = s_default_bool;
        ReaderTypeFor_ICurve parentCurve = s_default_ICurve;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("reversed", reversed))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateCurveReference (
            reversed,
            parentCurve);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> MemberArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfIGeometry ("ListOfMember", "Member", MemberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateGroup (
            MemberArray);

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
        && ReadToChild ())
        {
        // Gather children
        int endConditionCode = s_default_int;
        int knotCode = s_default_int;
        DVector3d startVector = s_default_DVector3d;
        DVector3d endVector = s_default_DVector3d;
        bvector<DPoint3d> PointArray;
        bvector<double> KnotArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagint ("endConditionCode", endConditionCode))
                continue;
            if (ReadTagint ("knotCode", knotCode))
                continue;
            if (ReadTagDVector3d ("startVector", startVector))
                continue;
            if (ReadTagDVector3d ("endVector", endVector))
                continue;
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", PointArray))
                continue;
            if (ReadListOfdouble ("ListOfKnot", "Knot", KnotArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateInterpolatingCurve (
            endConditionCode,
            knotCode,
            startVector,
            endVector,
            PointArray,
            KnotArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<DPoint3d> PointArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", PointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateLineString (
            PointArray);

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
        && ReadToChild ())
        {
        // Gather children
        String name = s_default_String;
        bvector<IGeometryPtr> MemberArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("name", name))
                continue;
            if (ReadListOfIGeometry ("ListOfMember", "Member", MemberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateOperation (
            name,
            MemberArray);

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
        && ReadToChild ())
        {
        // Gather children
        LoopType loopType = s_default_LoopType;
        ReaderTypeFor_IParametricSurface surface = s_default_IParametricSurface;
        bvector<IGeometryPtr> CurveChainArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagLoopType ("loopType", loopType))
                continue;

            if (ReadListOfICurveChain ("ListOfCurveChain", "CurveChain", CurveChainArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateParametricSurfacePatch (
            loopType,
            surface,
            CurveChainArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> PointArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISinglePoint ("ListOfPoint", "Point", PointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreatePointChain (
            PointArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> MemberArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfIPoint ("ListOfMember", "Member", MemberArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreatePointGroup (
            MemberArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<DPoint3d> PointArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfDPoint3d ("ListOfPoint", "Point", PointArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreatePolygon (
            PointArray);

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
        && ReadToChild ())
        {
        // Gather children
        bool reversed = s_default_bool;
        ReaderTypeFor_IPrimitiveCurve parentCurve = s_default_IPrimitiveCurve;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("reversed", reversed))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreatePrimitiveCurveReference (
            reversed,
            parentCurve);

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
        && ReadToChild ())
        {
        // Gather children
        String name = s_default_String;
        ReaderTypeFor_IGeometry geometry = s_default_IGeometry;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("name", name))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSharedGroupDef (
            name,
            geometry);

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
        && ReadToChild ())
        {
        // Gather children
        String sharedGroupName = s_default_String;
        Transform transform = s_default_Transform;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("sharedGroupName", sharedGroupName))
                continue;
            if (ReadTagTransform ("transform", transform))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSharedGroupInstance (
            sharedGroupName,
            transform);

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
        && ReadToChild ())
        {
        // Gather children
        ReaderTypeFor_ISurface BoundingSurface = s_default_ISurface;

        // Read children in any order ...
        for (;IsStartElement ();)
            {

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateShelledSolid (
            BoundingSurface);

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
        && ReadToChild ())
        {
        // Gather children
        ReaderTypeFor_ISurface baseGeometry = s_default_ISurface;
        ReaderTypeFor_ICurve railCurve = s_default_ICurve;

        // Read children in any order ...
        for (;IsStartElement ();)
            {


                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSolidBySweptSurface (
            baseGeometry,
            railCurve);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> SectionArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISurface ("ListOfSection", "Section", SectionArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSolidByRuledSweep (
            SectionArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> SectionArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfICurve ("ListOfSection", "Section", SectionArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSurfaceByRuledSweep (
            SectionArray);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> SolidArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISolid ("ListOfSolid", "Solid", SolidArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSolidGroup (
            SolidArray);

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
        && ReadToChild ())
        {
        // Gather children
        String spiralType = s_default_String;
        DPoint3d startPoint = s_default_DPoint3d;
        Angle startBearing = s_default_Angle;
        double startCurvature = s_default_double;
        DPoint3d endPoint = s_default_DPoint3d;
        Angle endBearing = s_default_Angle;
        double endCurvature = s_default_double;
        ReaderTypeFor_IGeometry geometry = s_default_IGeometry;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("spiralType", spiralType))
                continue;
            if (ReadTagDPoint3d ("startPoint", startPoint))
                continue;
            if (ReadTagAngle ("startBearing", startBearing))
                continue;
            if (ReadTagdouble ("startCurvature", startCurvature))
                continue;
            if (ReadTagDPoint3d ("endPoint", endPoint))
                continue;
            if (ReadTagAngle ("endBearing", endBearing))
                continue;
            if (ReadTagdouble ("endCurvature", endCurvature))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSpiral (
            spiralType,
            startPoint,
            startBearing,
            startCurvature,
            endPoint,
            endBearing,
            endCurvature,
            geometry);

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
        && ReadToChild ())
        {
        // Gather children
        ReaderTypeFor_ICurve baseGeometry = s_default_ICurve;
        ReaderTypeFor_ICurve railCurve = s_default_ICurve;

        // Read children in any order ...
        for (;IsStartElement ();)
            {


                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSurfaceBySweptCurve (
            baseGeometry,
            railCurve);

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
        && ReadToChild ())
        {
        // Gather children
        bvector<IGeometryPtr> SurfaceArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadListOfISurface ("ListOfSurface", "Surface", SurfaceArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSurfaceGroup (
            SurfaceArray);

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
        && ReadToChild ())
        {
        // Gather children
        ReaderTypeFor_ICurveChain exteriorLoop = s_default_ICurveChain;
        bvector<IGeometryPtr> HoleLoopArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {

            if (ReadListOfICurveChain ("ListOfHoleLoop", "HoleLoop", HoleLoopArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateSurfacePatch (
            exteriorLoop,
            HoleLoopArray);

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
        && ReadToChild ())
        {
        // Gather children
        Transform transform = s_default_Transform;
        ReaderTypeFor_IGeometry geometry = s_default_IGeometry;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagTransform ("transform", transform))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateTransformedGeometry (
            transform,
            geometry);

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
        && ReadToChild ())
        {
        // Gather children
        DVector3d extrusionVector = s_default_DVector3d;
        bool capped = s_default_bool;
        ReaderTypeFor_ISweepable baseGeometry = s_default_ISweepable;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagDVector3d ("extrusionVector", extrusionVector))
                continue;
            if (ReadTagbool ("capped", capped))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnExtrusion (
            extrusionVector,
            capped,
            baseGeometry);

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
        && ReadToChild ())
        {
        // Gather children
        DPoint3d center = s_default_DPoint3d;
        DVector3d axis = s_default_DVector3d;
        bool capped = s_default_bool;
        ReaderTypeFor_ISweepable baseGeometry = s_default_ISweepable;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagDPoint3d ("center", center))
                continue;
            if (ReadTagDVector3d ("axis", axis))
                continue;
            if (ReadTagbool ("capped", capped))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnRotationalSweep (
            center,
            axis,
            capped,
            baseGeometry);

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
        && ReadToChild ())
        {
        // Gather children
        bool capped = s_default_bool;
        bvector<IGeometryPtr> ContourArray;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagbool ("capped", capped))
                continue;
            if (ReadListOfISweepable ("ListOfContour", "Contour", ContourArray))
                continue;
                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateDgnRuledSweep (
            capped,
            ContourArray);

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
        && ReadToChild ())
        {
        // Gather children
        String spiralType = s_default_String;
        PlacementOriginZX placement = s_default_PlacementOriginZX;
        Angle startBearing = s_default_Angle;
        double startRadius = s_default_double;
        Angle endBearing = s_default_Angle;
        double endRadius = s_default_double;
        double activeStartFraction = s_default_double;
        double activeEndFraction = s_default_double;
        ReaderTypeFor_IGeometry geometry = s_default_IGeometry;

        // Read children in any order ...
        for (;IsStartElement ();)
            {
            if (ReadTagString ("spiralType", spiralType))
                continue;
            if (ReadTagPlacementOriginZX ("placement", placement))
                continue;
            if (ReadTagAngle ("startBearing", startBearing))
                continue;
            if (ReadTagdouble ("startRadius", startRadius))
                continue;
            if (ReadTagAngle ("endBearing", endBearing))
                continue;
            if (ReadTagdouble ("endRadius", endRadius))
                continue;
            if (ReadTagdouble ("activeStartFraction", activeStartFraction))
                continue;
            if (ReadTagdouble ("activeEndFraction", activeEndFraction))
                continue;

                
            if (!SkipUnexpectedTag ())
                return false;
            }

        // Get out of the primary element ..
        ReadEndElement ();
        result = m_factory.CreateTransitionSpiral (
            spiralType,
            placement,
            startBearing,
            startRadius,
            endBearing,
            endRadius,
            activeStartFraction,
            activeEndFraction,
            geometry);

        return true;
        }
    return false;
    }





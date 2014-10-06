



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
bool ReadILineSegment (IGeometryPtr & result)
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
bool ReadICircularArc (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularArc")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radius = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadIDgnBox (IGeometryPtr & result)
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadIDgnSphere (IGeometryPtr & result)
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadIDgnCone (IGeometryPtr & result)
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadIDgnTorusPipe (IGeometryPtr & result)
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadIBlock (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Block")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        DPoint3d cornerA = s_default_DPoint3d;
        DPoint3d cornerB = s_default_DPoint3d;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("cornerA")
                && ReadTagDPoint3d ("cornerA", cornerA))
                continue;

            if (   CurrentElementNameMatch ("cornerB")
                && ReadTagDPoint3d ("cornerB", cornerB))
                continue;

            if (   CurrentElementNameMatch ("capped")
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
bool ReadICircularCone (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularCone")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double height = s_default_double;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadICircularCylinder (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularCylinder")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double height = s_default_double;
        double radius = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
                continue;

            if (   CurrentElementNameMatch ("height")
                && ReadTagdouble ("height", height))
                continue;

            if (   CurrentElementNameMatch ("radius")
                && ReadTagdouble ("radius", radius))
                continue;

            if (   CurrentElementNameMatch ("capped")
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
bool ReadICircularDisk (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("CircularDisk")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radius = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadICoordinate (IGeometryPtr & result)
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
bool ReadIEllipticArc (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("EllipticArc")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadIEllipticDisk (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("EllipticDisk")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radiusA = s_default_double;
        double radiusB = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadISingleLineText (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SingleLineText")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        String textString = s_default_String;
        String fontName = s_default_String;
        double characterXSize = s_default_double;
        double characterYSize = s_default_double;
        int justification = s_default_int;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadISkewedCone (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("SkewedCone")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        DPoint3d centerB = s_default_DPoint3d;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadISphere (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("Sphere")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radius = s_default_double;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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
bool ReadITorusPipe (IGeometryPtr & result)
    {
    result = nullptr;
    if (CurrentElementNameMatch ("TorusPipe")
        && ReadToChild ())
        {
        // Start with the system default for each field ....
        IPlacement placement = s_default_IPlacement;
        double radiusA = s_default_double;
        double radiusB = s_default_double;
        Angle startAngle = s_default_Angle;
        Angle sweepAngle = s_default_Angle;
        bool capped = s_default_bool;

        for (;IsStartElement ();)
            {
            if (   CurrentElementNameMatch ("placement")
                && ReadTagIPlacement ("placement", placement))
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

            if (   CurrentElementNameMatch ("capped")
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
bool ReadIVector (IGeometryPtr & result)
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



//before expandAllNodeTypes
//<expandFileLater>in\expandAllNodeTypes.in</expandFileLater>
//after expandAllNodeTypes
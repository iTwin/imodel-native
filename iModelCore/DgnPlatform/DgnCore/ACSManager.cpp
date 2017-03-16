/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ACSManager.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

USING_NAMESPACE_BENTLEY_DGN

#define     TRIAD_SIZE_INCHES       0.6
#define     ARROW_BASE_START        0.3
#define     ARROW_BASE_WIDTH        0.2
#define     ARROW_TIP_END           1.25
#define     ARROW_TIP_START         0.85
#define     ARROW_TIP_FLANGE        0.75
#define     ARROW_TIP_WIDTH         0.4

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ACSElementHandler
{
    HANDLER_DEFINE_MEMBERS(CoordSys2d);
    HANDLER_DEFINE_MEMBERS(CoordSys3d);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (NOT_NOW)
/*=================================================================================**//**
* NOTE: Persistent data structure! See ACSData...
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ACSGrid
{
Point2d         m_repetitions;  // grid size for fixed number of repetitions (0,0 for infinite grid plane)...
Point2d         m_originOffset; // grid position relative to acs origin (0,0 for lower left)...
uint32_t        m_gridPerRef;   // grid dot per reference grid line...
uint32_t        m_unused;       // Unused pad bytes...
DPoint2d        m_spacing;      // grid x/y spacing in meters...

ACSGrid() {Init();};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Init()
    {
    m_gridPerRef = 10;
    m_unused = 0;
    m_spacing.x = m_spacing.y = 0.0;
    m_repetitions.x = m_repetitions.y = 0;
    m_originOffset.x = m_originOffset.y = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsDefaultSettings() const
    {
    ACSGrid     defaultGrid;

    return IsEqual(defaultGrid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEqual(ACSGrid const& otherData) const
    {
    if (m_repetitions.x != otherData.m_repetitions.x || m_repetitions.y != otherData.m_repetitions.y)
        return false;

    if (m_originOffset.x != otherData.m_originOffset.x || m_originOffset.y != otherData.m_originOffset.y)
        return false;

    if (m_spacing.x != otherData.m_spacing.x || m_spacing.y != otherData.m_spacing.y)
        return false;

    if (m_unused != otherData.m_unused)
        return false;

    if (m_gridPerRef != otherData.m_gridPerRef)
        return false;

    return true;
    }

}; // ACSGrid

/*=================================================================================**//**
* NOTE: Persistent data structure! Only add to new members to end!!!
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ACSData
{
ACSType         m_type;         // ACS_TYPE_RECT etc.
ACSFlags        m_flags;        // option flags
DPoint3d        m_origin;       // origin of acs
double          m_scale;        // scale of acs
RotMatrix       m_rotation;     // rotation of acs
ACSGrid         m_grid;         // ACS grid settings...

ACSData() {Init();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Init()
    {
    m_type      = ACSType::Rectangular;
    m_flags     = ACSFlags::Default;
    m_scale     = 1.0;

    m_origin.Zero();
    m_rotation.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEqual(ACSData const& otherData) const
    {
    if (m_type != otherData.m_type)
        return false;

    if (m_flags != otherData.m_flags)
        return false;

    if (!m_origin.IsEqual(otherData.m_origin))
        return false;

    if (m_scale != otherData.m_scale)
        return false;

    if (!m_rotation.IsEqual(otherData.m_rotation))
        return false;

    return m_grid.IsEqual(otherData.m_grid);
    }

}; // ACSData

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    /*AuxCoordSys::*/_Equals(IAuxCoordSysCP other) const override
    {
    if (NULL == other)
        return false;

    if (this == other)
        return true;

    AuxCoordSys const * otherACS;

    if (NULL == (otherACS = dynamic_cast <AuxCoordSys const *> (other)))
        return false;

    return m_acsData.IsEqual(otherACS->m_acsData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String /*AuxCoordSys::*/_GetTypeName() const override
    {
    L10N::StringId id;

    switch (m_acsData.m_type)
        {
        case ACSType::None:        id = DgnCoreL10N::ACS_TYPE_NONE(); break;
        case ACSType::Rectangular: id = DgnCoreL10N::ACS_TYPE_RECT();  break;
        case ACSType::Cylindrical: id = DgnCoreL10N::ACS_TYPE_CYL();   break;
        case ACSType::Spherical:   id = DgnCoreL10N::ACS_TYPE_SPHERE();  break;
        case ACSType::Extended:    id = DgnCoreL10N::ACS_TYPE_EXTEND(); break;
        };

    return DgnCoreL10N::GetString(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, DPoint2dCR spacing, uint32_t gridPerRef) override
    {
    m_acsData.m_grid.Init();

    if (0.0 == spacing.x || 0.0 == spacing.y || 0 == gridReps.x || 0 == gridReps.y)
        return ERROR;

    m_acsData.m_grid.m_repetitions  = gridReps;
    m_acsData.m_grid.m_originOffset = gridOffset;
    m_acsData.m_grid.m_spacing      = spacing;
    m_acsData.m_grid.m_gridPerRef   = gridPerRef;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, DPoint2dR spacing, uint32_t& gridPerRef) const override
    {
    gridReps   = m_acsData.m_grid.m_repetitions;
    gridOffset = m_acsData.m_grid.m_originOffset;
    spacing    = m_acsData.m_grid.m_spacing;
    gridPerRef = m_acsData.m_grid.m_gridPerRef;

    return (0.0 == spacing.x || 0.0 == spacing.y || 0 == gridReps.x || 0 == gridReps.y) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAuxCoordSys::Locate(DPoint3dR hitPt, DgnViewportR vp, DPoint3dCR borePt, double radius)
    {
    // Going to have to create CurveVectors and locate similiar to edit manipulator locate code...
    OutputP  output = vp.GetIViewOutput();

    if (NULL == output)
        return false;

    IAuxCoordSysP   currentACS = IACSManager::GetManager().GetActive(vp);
    bool            isCurrent = Equals(currentACS);
    DPoint3d        drawOrigin;

    GetOrigin(drawOrigin);

    QvElem*     qvElem = _CreateQvElems(&vp, &drawOrigin, isCurrent ? ACS_SIZE_ACTIVE : ACS_SIZE_INACTIVE, isCurrent ? ACSDisplayOptions::Active : ACSDisplayOptions::Inactive, true);

    if (NULL == qvElem)
        return false;

    DPoint3d    testPtView;

    vp.WorldToView(&testPtView, &borePt, 1);
    testPtView.z = 0.0;

    bool        hitFound = output->LocateQvElem(qvElem, *((DPoint2dCP) &testPtView), 1.0, hitPt, NULL, NULL, NULL);

    T_HOST.GetGraphicsAdmin()._DeleteQvElem(qvElem);

    return hitFound;
    }

//=======================================================================================
//! An IAuxCoordSys is an object that holds the data which
//! describes an auxiliary coordinate system
//=======================================================================================
struct IAuxCoordSys : RefCountedBase
{
public:
    // Only for ACS's of type ACS_TYPE_GeoCoordinate is the rotation matrix position dependent, don't publish this yet.
    RotMatrixR GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const {return _GetRotation(pRot, pPosition);}

    //! Boresite to ACS triad in the given view. The borePt and hitPt are in active coords...
    DGNPLATFORM_EXPORT bool Locate(DPoint3dR hitPt, DgnViewportR vp, DPoint3dCR borePt, double radius);
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IACSManager::IACSManager() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool IACSManager::IsPointAdjustmentRequired(DgnViewportR vp)
    {
    return vp.GetViewController()._IsPointAdjustmentRequired();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool IACSManager::IsSnapAdjustmentRequired(DgnViewportR vp)
    {
    return vp.GetViewController()._IsSnapAdjustmentRequired(DgnPlatformLib::GetHost().GetSessionSettingsAdmin()._GetACSPlaneSnapLock());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool IACSManager::IsContextRotationRequired(DgnViewportR vp)
    {
    return vp.GetViewController()._IsContextRotationRequired(DgnPlatformLib::GetHost().GetSessionSettingsAdmin()._GetACSContextLock());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
AuxCoordSystemCP IACSManager::GetActive(DgnViewportR vp)
    {
    SpatialViewControllerCP viewController = vp.GetSpatialViewControllerCP ();

    if (nullptr == viewController)
        return nullptr;

    return viewController->GetAuxCoordinateSystem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IACSManager::SetActive(AuxCoordSystemCP auxCoordSys, DgnViewportR vp)
    {
    SpatialViewControllerP viewController = vp.GetSpatialViewControllerP();

    if (nullptr == viewController)
        return ERROR;

    viewController->SetAuxCoordinateSystem(auxCoordSys);

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          ACSEventCaller
{
AuxCoordSystemPtr   m_acs;
ACSEventType        m_eventType;
DgnModelP           m_modelRef;

ACSEventCaller(AuxCoordSystemP acs, ACSEventType eventType, DgnModelP modelRef)
    {
    m_acs       = acs;
    m_eventType = eventType;
    m_modelRef  = modelRef;
    }

void CallHandler(IACSEvents& handler) {handler._OnACSEvent(m_acs.get(), m_eventType, m_modelRef);}

}; // ACSEventCaller

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::SendEvent(AuxCoordSystemP acs, ACSEventType eventType, DgnModelP modelRef)
    {
    if (NULL == m_listeners)
        return;

    ACSEventCaller eventCaller(acs, eventType, modelRef);
    m_listeners->CallAllHandlers(eventCaller);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::AddListener(IACSEvents* acsListener)
    {
    if (nullptr == m_listeners)
        m_listeners = new EventHandlerList<IACSEvents>;

    m_listeners->AddHandler(acsListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::DropListener(IACSEvents* acsListener)
    {
    if (nullptr != m_listeners)
        m_listeners->DropHandler(acsListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef AuxCoordSystem::_GetAdjustedColor(ColorDef inColor, bool isFill, DgnViewportCR viewport, ACSDisplayOptions options) const
    {
    ColorDef    color;

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Hilite))
        color = viewport.GetHiliteColor();
    else if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Active))
        color = ColorDef::White() == inColor ? viewport.GetContrastToBackgroundColor() : inColor;
    else
        color = ColorDef(150, 150, 150, 0);

    color = viewport.AdjustColorForContrast(color, viewport.GetBackgroundColor());

    if (isFill)
        color.SetAlpha(ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 225 : 200);
    else
        color.SetAlpha(ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 150 : 75);

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AuxCoordSystem::_GetAxisLabel(uint32_t axis) const
    {
    Utf8String axisLabel;

    switch (axis)
        {
        case 0:
            axisLabel.assign("X");
            break;
        case 1:
            axisLabel.assign("Y");
            break;
        case 2:
            axisLabel.assign("Z");
            break;
        }

    return axisLabel;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxCoordSystem::_AddAxisLabel(GraphicBuilderR graphic, uint32_t axis, ACSDisplayOptions options, DgnViewportCR vp) const
    {
    Utf8String  axisLabel = _GetAxisLabel(axis);

    if (Utf8String::IsNullOrEmpty(axisLabel.c_str()))
        return;

    double      angle = 0.0;
    DPoint2d    userOrigin = DPoint2d::FromZero();

    if (0 == axis)
        {
        angle = -msGeomConst_pi/2.0;
        userOrigin.x = 0.65;
        }
    else if (1 == axis)
        {
        angle = 0.0;
        userOrigin.y = 0.65;
        }

    DPoint3d    textPt = DPoint3d::From(userOrigin);
    RotMatrix   textMatrix;
    TextString  textStr;

    textMatrix.InitFromAxisAndRotationAngle(2, angle);

    textStr.SetText(axisLabel.c_str());
    textStr.SetOrientation(textMatrix);
    textStr.GetStyleR().SetFont(DgnFontManager::GetDecoratorFont());
    textStr.GetStyleR().SetSize(0.35);
    textStr.SetOriginFromJustificationOrigin(textPt, TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);

    ColorDef    color = _GetAdjustedColor(ColorDef::White(), false, vp, options);

    graphic.SetSymbology(color, color, 2);
    graphic.AddTextString(textStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxCoordSystem::_AddAxis(GraphicBuilderR builder, uint32_t axis, ACSDisplayOptions options, DgnViewportCR vp) const
    {
    if (2 == axis)
        {
        ColorDef    color = ColorDef::Blue();
        DPoint3d    linePts[2];

        memset(linePts, 0, sizeof (linePts));
        linePts[1].z = 0.65;

        ColorDef    lineColor = _GetAdjustedColor(color, false, vp, options);
        ColorDef    fillColor = _GetAdjustedColor(color, true, vp, options);

        builder.SetSymbology(lineColor, lineColor, 6);
        builder.AddPointString(2, linePts);

        builder.SetSymbology(lineColor, lineColor, 1);
        builder.AddLineString(2, linePts);

        double      start = 0.0, sweep = msGeomConst_2pi, scale = ARROW_TIP_WIDTH/2.0;
        DVec3d      xVec, yVec;
        DPoint3d    center;
        RotMatrix   viewRMatrix = vp.GetRotMatrix();

        memset(&center, 0, sizeof (center));

        viewRMatrix.GetRow(xVec, 0);
        viewRMatrix.GetRow(yVec, 1);

        builder.GetLocalToWorldTransform().MultiplyTransposeMatrixOnly(xVec);
        builder.GetLocalToWorldTransform().MultiplyTransposeMatrixOnly(yVec);

        xVec.Normalize();
        yVec.Normalize();

        DEllipse3d  ellipse;

        ellipse.InitFromDGNFields3d(center, xVec, yVec, scale, scale, start, sweep);
        builder.AddArc(ellipse, false, false);

        builder.SetBlankingFill(fillColor);
        builder.AddArc(ellipse, true, true);

        return;
        }

    DPoint3d    shapePts[8];

    memset(shapePts, 0, sizeof (shapePts));

    shapePts[0].x = ARROW_BASE_START;   shapePts[0].y = -ARROW_BASE_WIDTH;
    shapePts[1].x = ARROW_TIP_START;    shapePts[1].y = -ARROW_BASE_WIDTH;
    shapePts[2].x = ARROW_TIP_FLANGE;   shapePts[2].y = -ARROW_TIP_WIDTH;
    shapePts[3].x = ARROW_TIP_END;      shapePts[3].y = 0.0;
    shapePts[4].x = ARROW_TIP_FLANGE;   shapePts[4].y = ARROW_TIP_WIDTH;
    shapePts[5].x = ARROW_TIP_START;    shapePts[5].y = ARROW_BASE_WIDTH;
    shapePts[6].x = ARROW_BASE_START;   shapePts[6].y = ARROW_BASE_WIDTH;
    shapePts[7]   = shapePts[0];

    if (1 == axis)
        {
        for (int i=0; i < 8; i++)
            std::swap(shapePts[i].x, shapePts[i].y);
        }

    ColorDef    color = (0 == axis ? ColorDef::Red() : ColorDef::Green());
    ColorDef    lineColor = _GetAdjustedColor(color, false, vp, options);
    ColorDef    fillColor = _GetAdjustedColor(color, true, vp, options);

    builder.SetSymbology(lineColor, lineColor, 1);
    builder.AddLineString(8, shapePts);

    _AddAxisLabel(builder, axis, options, vp);

    builder.SetBlankingFill(fillColor);
    builder.AddShape(8, shapePts, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isOriginInView(DPoint3dR drawOrigin, DgnViewportCR viewport, bool adjustOrigin)
    {
    DPoint3d    testPtView, screenRange;
    Frustum     frustum = viewport.GetFrustum(DgnCoordSystem::Screen, false);

    viewport.WorldToView(&testPtView, &drawOrigin, 1);

    screenRange.x = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_100));
    screenRange.y = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_010));
    screenRange.z = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_001));

    // Check if current acs origin is outside view...
    bool inView = (!((testPtView.x < 0 || testPtView.x > screenRange.x) || (testPtView.y < 0 || testPtView.y > screenRange.y)));

    if (!adjustOrigin)
        return inView;

    if (!inView)
        {
        double offset = viewport.PixelsFromInches(TRIAD_SIZE_INCHES);

        LIMIT_RANGE(offset, screenRange.x-offset, testPtView.x);
        LIMIT_RANGE(offset, screenRange.y-offset, testPtView.y);
        }

    // Limit point to NPC box to prevent triad from being clipped from display...
    DPoint3d originPtNpc;

    viewport.ViewToNpc(&originPtNpc, &testPtView, 1);
    LIMIT_RANGE(0.0, 1.0, originPtNpc.x);
    LIMIT_RANGE(0.0, 1.0, originPtNpc.y);
    LIMIT_RANGE(0.0, 1.0, originPtNpc.z);
    viewport.NpcToView(&testPtView, &originPtNpc, 1);
    viewport.ViewToWorld(&drawOrigin, &testPtView, 1);

    return inView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr AuxCoordSystem::_CreateGraphic(DecorateContextR context, ACSDisplayOptions options) const
    {
    bool        checkOutOfView = (ACSDisplayOptions::None != (options & ACSDisplayOptions::CheckVisible));
    DPoint3d    drawOrigin = _GetOrigin();

    if (checkOutOfView && !isOriginInView(drawOrigin, *context.GetViewport(), true))
        options = options | ACSDisplayOptions::Deemphasized;

    double      pixelSize = context.GetViewport()->PixelsFromInches(TRIAD_SIZE_INCHES); // Active size...

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized))
        pixelSize *= 0.8;
    else if (ACSDisplayOptions::None == (options & ACSDisplayOptions::Active))
        pixelSize *= 0.9;

//    DrawingViewDefinitionCP drawingViewDef = context.GetViewport()->GetViewController().GetViewDefinition()._ToDrawingView();

    double      exagg = 1.0;//(nullptr == drawingViewDef ? 1.0 : drawingViewDef->GetAspectRatioSkew()); // NEEDSWORK_VIEWDEF
    double      scale = context.GetPixelSizeAtPoint(&drawOrigin) * pixelSize;
    RotMatrix   rMatrix = _GetRotation();
    Transform   transform;

    rMatrix.InverseOf(rMatrix);
    rMatrix.ScaleRows(rMatrix,  scale,  scale / exagg,  scale);
    transform.InitFrom(rMatrix, drawOrigin);

    Render::GraphicBuilderPtr graphic = context.CreateGraphic(GraphicBuilder::CreateParams(context.GetDgnDb(), transform));

    DgnViewportR vp = *context.GetViewport();
    _AddAxis(*graphic, 0, options, vp);
    _AddAxis(*graphic, 1, options, vp);
    _AddAxis(*graphic, 2, options, vp);

    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxCoordSystem::_Display(DecorateContextR context, ACSDisplayOptions options) const
    {
    Render::GraphicBuilderPtr graphic = _CreateGraphic(context, options);

    if (!graphic.IsValid())
        return;

    context.AddWorldOverlay(*graphic->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AuxCoordSystem::_PointFromString
(
DPoint3dR       outPoint,
Utf8StringR     errorMsg,
Utf8CP          inString,
bool            relative,
DPoint3dCP      inLastPoint,
DgnModelR       modelRef
) const
    {
    DPoint3d    lastPoint = DPoint3d::FromZero();
    DPoint3d    auxOrigin = _GetOrigin();
    RotMatrix   auxRMatrix = _GetRotation();
    ACSType     acsType = _GetType();

    // might need relative if '#' is used as a component.
    if (nullptr != inLastPoint)
        {
        lastPoint = *inLastPoint;
        lastPoint.Subtract(auxOrigin);
        auxRMatrix.Multiply(lastPoint);
        }
    else
        {
        relative = false;
        }

    GeometricModelP   model = modelRef.ToGeometricModelP();
    if (!model)
        {
        BeAssert(false);
        return ERROR;
        }

    // make a local copy of the input string that we can modify.
    Utf8String      tmpString(inString);
    StatusInt       status;

    switch (acsType)
        {
        case ACSType::Rectangular:
            {
            PointParserPtr  pointParser = PointParser::Create(*model);

            Point3d isRelative;
            if (SUCCESS != (status = pointParser->ToValue(outPoint, isRelative, tmpString.c_str())))
                return status;

            if (relative || isRelative.x)
                outPoint.x += lastPoint.x;
            if (relative || isRelative.y)
                outPoint.y += lastPoint.y;
            if (relative || isRelative.z)
                outPoint.z += lastPoint.z;

            break;
            }

        case ACSType::Cylindrical:
            {
            bvector<Utf8String> subStrings;
            BeStringUtilities::Split(inString, ",", subStrings);

            if (subStrings.size() < 2)
                return ERROR;

            DistanceParserPtr   distanceParser  = DistanceParser::Create(*model);
            DirectionParserPtr  directionParser = DirectionParser::Create(*model);

            double radius;
            if (SUCCESS != (status = distanceParser->ToValue(radius, subStrings[0].c_str())))
                return status;

            double theta;
            if (SUCCESS != (status = directionParser->ToValue(theta, subStrings[1].c_str())))
                return status;

            theta = Angle::DegreesToRadians(theta);

            if ( relative && ( (lastPoint.x != 0.0) || (lastPoint.y != 0.0)) )
                {
                theta += atan2(lastPoint.y, lastPoint.x);
                radius += sqrt(lastPoint.x*lastPoint.x + lastPoint.y*lastPoint.y);
                }

            outPoint.x = radius * cos(theta);
            outPoint.y = radius * sin(theta);

            if (modelRef.Is3d())
                {
                if (subStrings.size() < 3)
                    return ERROR;

                if (SUCCESS != (status = distanceParser->ToValue(outPoint.z, subStrings[2].c_str())))
                    return status;

                if (relative)
                    outPoint.z += lastPoint.z;
                }
            else
                {
                outPoint.z = 0.0;
                }
            break;
            }

        case ACSType::Spherical:
            {
            bvector<Utf8String> subStrings;
            BeStringUtilities::Split(inString, ",", subStrings);

            if (subStrings.size() < 3)
                return ERROR;

            DistanceParserPtr   distanceParser  = DistanceParser::Create(*model);
            DirectionParserPtr  directionParser = DirectionParser::Create(*model);

            double rho;
            if (SUCCESS != (status = distanceParser->ToValue(rho, subStrings[0].c_str())))
                return status;

            double theta;
            if (SUCCESS != (status = directionParser->ToValue(theta, subStrings[1].c_str())))
                return status;

            double phi;
            if (SUCCESS != (status = directionParser->ToValue(phi, subStrings[2].c_str())))
                return status;

            theta = Angle::DegreesToRadians(theta);
            phi   = Angle::DegreesToRadians(phi);

            if (relative)
                {
                double origRho;

                rho += (origRho =  ((DVec3dP)&lastPoint)->Magnitude());

                if (lastPoint.x != 0.0 || lastPoint.y != 0.0)
                    theta += atan2(lastPoint.y, lastPoint.x);

                if (origRho != 0.0)
                    phi += Angle::Acos(lastPoint.z/origRho);
                }

            double radius;
            outPoint.z  = rho * cos(phi);
            radius      = rho * sin(phi);
            outPoint.x  = radius * cos(theta);
            outPoint.y  = radius * sin(theta);
            break;
            }
        }

    auxRMatrix.MultiplyTranspose(outPoint);
    outPoint.Add(*( (DVec3d *)&auxOrigin));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt AuxCoordSystem::_StringFromPoint
(
Utf8StringR         outString,
Utf8StringR         errorMsg,
DPoint3dCR          inPoint,
bool                delta,
DPoint3dCP          deltaOrigin,
DgnModelR           modelRef,
DistanceFormatterR  distanceFormatter,
DirectionFormatterR directionFormatter 
) const
    {
    DPoint3d    origin;

    if (delta && (nullptr != deltaOrigin))
        origin = *deltaOrigin;
    else
        origin = _GetOrigin();

    DPoint3d    tPoint;
    RotMatrix   rotation;

    tPoint.DifferenceOf(inPoint, origin);
    rotation = _GetRotation();
    rotation.Multiply(tPoint);

    ACSType     acsType = _GetType();

    switch (acsType)
        {
        case ACSType::Rectangular:
            {
            PointFormatterPtr formatter = PointFormatter::Create(distanceFormatter);
            formatter->SetIs3d(modelRef.Is3d());
            outString = formatter->ToString(tPoint);
            break;
            }

        case ACSType::Cylindrical:
            {
            double      distance, angle;

            distance = sqrt(tPoint.x * tPoint.x + tPoint.y * tPoint.y);
            angle    = Angle::Atan2(tPoint.y, tPoint.x);

            outString = distanceFormatter.ToString(distance);

            Utf8String directionString = directionFormatter.ToStringFromRadians(angle);
            outString.append(",");
            outString.append(directionString);

            if (modelRef.Is3d())
                {
                Utf8String elevationString = distanceFormatter.ToString(tPoint.z);
                outString.append(",");
                outString.append(elevationString);
                }
            break;
            }

        case ACSType::Spherical:
            {
            double      radius, theta, phi;

            radius = ((DVec3d *) &tPoint)->Magnitude();
            theta = Angle::Atan2(tPoint.y, tPoint.x);

            if (LegacyMath::DEqual(radius, 0.0))
                phi = 0.0;
            else
                phi = Angle::Acos(tPoint.z/radius);

            outString = distanceFormatter.ToString(radius);
            outString.append(",");
            Utf8String thetaString = directionFormatter.ToStringFromRadians(theta);
            outString.append(thetaString);

            outString.append(",");
            // this seems wrong to me. I think it should use the AngleFormatter in the DirectionFormatter.
            Utf8String phiString = directionFormatter.ToStringFromRadians(phi);
            outString.append(phiString);
            break;
            }

        default:
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool AuxCoordSystem::GetGridSpacing(DPoint2dR spacing, uint32_t& gridPerRef, Point2dR gridReps, Point2dR gridOffset, DgnViewportR vp) const
    {
    if (SUCCESS == _GetStandardGridParams(gridReps, gridOffset, spacing, gridPerRef))
        return true;

    vp.GetViewController()._GetGridSpacing(spacing, gridPerRef); // Inherit view controller settings if not specified...
    gridReps.x = gridReps.y = 0;
    gridOffset.x = gridOffset.y = 0;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxCoordSystem::_DrawGrid(DecorateContextR context) const
    {
    // Called for active ACS when grid orientation is GridOrientationType::ACS.
    uint32_t    gridPerRef;
    Point2d     gridReps, gridOffset;
    DPoint2d    spacing;
    DPoint3d    origin = _GetOrigin();
    RotMatrix   rMatrix = _GetRotation();

    // Adjust origin for grid offset if we are displaying a fixed sized grid plane...
    if (GetGridSpacing(spacing, gridPerRef, gridReps, gridOffset, *context.GetViewport()) && (0 != gridOffset.x || 0 != gridOffset.y))
        {
        DVec3d  xVec, yVec;

        rMatrix.GetRow(xVec, 0);
        rMatrix.GetRow(yVec, 1);

        xVec.Scale(spacing.x);
        yVec.Scale(spacing.y);

        origin.SumOf(origin, xVec, -gridOffset.x, yVec, -gridOffset.y);
        }

    context.DrawStandardGrid(origin, rMatrix, spacing, gridPerRef, false, (0 == gridReps.x || 0 == gridReps.y) ? NULL : &gridReps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void AuxCoordSystem::_PointToGrid(DgnViewportR viewport, DPoint3dR point) const
    {
    uint32_t    gridPerRef;
    Point2d     gridReps, gridOffset;
    DPoint2d    roundingDistance;
    DPoint3d    origin = _GetOrigin();
    RotMatrix   rMatrix = _GetRotation();

    GetGridSpacing(roundingDistance, gridPerRef, gridReps, gridOffset, viewport);

    viewport.GetViewController().PointToStandardGrid(point, origin, rMatrix, roundingDistance);
    }

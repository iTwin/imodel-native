/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ACSManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    HANDLER_DEFINE_MEMBERS(CoordSysSpatial);
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
AuxCoordSystemPtr AuxCoordSystem::CreateNew(ViewDefinitionCR def, Utf8StringCR name)
    {
    DefinitionModelPtr model = def.GetDefinitionModel();
    if (!model.IsValid())
        return nullptr;

    if (def.IsSpatialView())
        return new AuxCoordSystemSpatial(*model, name);
    else if (def.IsView3d())
        return new AuxCoordSystem3d(*model, name);
    else
        return new AuxCoordSystem2d(*model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
AuxCoordSystemPtr AuxCoordSystem::CreateNew(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name)
    {
    DefinitionModelPtr scope(defnModel);
    if (!scope.IsValid())
        scope = &model.GetDgnDb().GetDictionaryModel();

    if (model.IsSpatialModel())
        return new AuxCoordSystemSpatial(*scope, name);
    else if (model.Is3dModel())
        return new AuxCoordSystem3d(*scope, name);
    else
        return new AuxCoordSystem2d(*scope, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode AuxCoordSystem::CreateCode(ViewDefinitionCR def, Utf8StringCR name)
    {
    DefinitionModelPtr model = def.GetDefinitionModel();
    if (!model.IsValid())
        return DgnCode();

    if (def.IsSpatialView())
        return AuxCoordSystemSpatial::CreateCode(*model, name);
    else if (def.IsView3d())
        return AuxCoordSystem3d::CreateCode(*model, name);
    else
        return AuxCoordSystem2d::CreateCode(*model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode AuxCoordSystem::CreateCode(DgnModelR model, DefinitionModelP defnModel, Utf8StringCR name)
    {
    DefinitionModelPtr scope(defnModel);
    if (!scope.IsValid())
        scope = &model.GetDgnDb().GetDictionaryModel();

    if (model.IsSpatialModel())
        return AuxCoordSystemSpatial::CreateCode(*scope, name);
    else if (model.Is3dModel())
        return AuxCoordSystem3d::CreateCode(*scope, name);
    else
        return AuxCoordSystem2d::CreateCode(*scope, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool AuxCoordSystem::_IsValidForView(ViewControllerCR viewController) const
    {
    if (viewController.IsSpatialView())
        return IsAuxCoordSystemSpatial();
        
    return (viewController.Is3d() == IsAuxCoordSystem3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AuxCoordSystem::GetTypeName() const
    {
    L10N::StringId id;

    switch (_GetType())
        {
        case ACSType::None:
            id = DgnCoreL10N::ACS_TYPE_NONE();
            break;

        case ACSType::Rectangular:
            id = DgnCoreL10N::ACS_TYPE_RECT();
            break;

        case ACSType::Cylindrical:
            id = DgnCoreL10N::ACS_TYPE_CYL();
            break;

        case ACSType::Spherical:
            id = DgnCoreL10N::ACS_TYPE_SPHERE();
            break;
        };

    return DgnCoreL10N::GetString(id);
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


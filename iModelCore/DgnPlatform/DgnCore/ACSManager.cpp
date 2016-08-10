/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ACSManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

USING_NAMESPACE_BENTLEY_DGN

#define     ARROW_BASE_START        0.3
#define     ARROW_BASE_WIDTH        0.2
#define     ARROW_TIP_END           1.25
#define     ARROW_TIP_START         0.85
#define     ARROW_TIP_FLANGE        0.75
#define     ARROW_TIP_WIDTH         0.4

#define     ACS_SIZE_ACTIVE         60.0
#define     ACS_SIZE_INACTIVE       55.0
#define     ACS_SIZE_OFFSCREEN      45.0
#define     ACS_XATTRIBUTE_ID       1
#define     ACS_LOCK_OFFSET         0.0
#define     ACS_LOCK_SIZE           1.5

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
double          m_uorPerGrid;   // grid x size spacing in uors...
double          m_ratio;        // grid x/y spacing ratio...

ACSGrid() {Init();};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Init()
    {
    m_gridPerRef = 10;
    m_uorPerGrid = 0.0;
    m_ratio      = 1.0;
    m_unused     = 0;

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

    if (m_uorPerGrid != otherData.m_uorPerGrid)
        return false;

    if (m_unused != otherData.m_unused)
        return false;

    if (m_gridPerRef != otherData.m_gridPerRef)
        return false;

    if (m_ratio != otherData.m_ratio)
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
ACSGrid         m_grid;         // New for Vancouver - ACS grid settings...

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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          AuxCoordSys : public IAuxCoordSys
{
private:

ACSData         m_acsData;
Utf8String      m_name;
Utf8String      m_description;
bool            m_attachedToView;
bool            m_viewInfoValidated;

protected:

virtual ACSType     /*AuxCoordSys::*/_GetType() const override {return m_acsData.m_type;}
virtual Utf8String  /*AuxCoordSys::*/_GetName() const override {return m_name;}
virtual Utf8String  /*AuxCoordSys::*/_GetDescription() const override {return m_description;}
virtual bool        /*AuxCoordSys::*/_GetIsReadOnly() const override {return false;}
virtual uint32_t    /*AuxCoordSys::*/_GetExtenderId() const override {return 0;}
virtual uint32_t    /*AuxCoordSys::*/_GetSerializedSize() const override {return sizeof (m_acsData);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetType(ACSType type) override
    {
    m_acsData.m_type = type;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetName(Utf8CP name) override
    {
    m_name.assign(name);
    TrimWhiteSpace(m_name); // Don't store leading/trailing spaces...

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetDescription(Utf8CP descr) override
    {
    m_description.assign(descr);
    TrimWhiteSpace(m_description); // Don't store leading/trailing spaces...

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetScale(double scale) override
    {
    m_acsData.m_scale = scale;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetOrigin(DPoint3dCR pOrigin) override
    {
    m_acsData.m_origin = pOrigin;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetRotation(RotMatrixCR pRot) override
    {
    m_acsData.m_rotation = pRot;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetFlags(ACSFlags flags) override
    {
    m_acsData.m_flags = flags;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_CompleteSetupFromViewController(SpatialViewControllerCP info)
    {
    m_attachedToView = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysPtr /*AuxCoordSys::*/_Clone() const override
    {
    return new AuxCoordSys(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    /*AuxCoordSys::*/_Equals(IAuxCoordSysCP other) const override
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
virtual Utf8String /*AuxCoordSys::*/_GetTypeName() const override
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
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double  /*AuxCoordSys::*/_GetScale() const override
    {
    return m_acsData.m_scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint3dR   /*AuxCoordSys::*/_GetOrigin(DPoint3dR pOrigin) const override
    {
    pOrigin = m_acsData.m_origin;

    return pOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR  /*AuxCoordSys::*/_GetRotation(RotMatrixR pRot) const override
    {
    pRot = m_acsData.m_rotation;

    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR  /*AuxCoordSys::*/_GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const override
    {
    return _GetRotation(pRot); // not position dependent.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSFlags    /*AuxCoordSys::*/_GetFlags() const override
    {
    return m_acsData.m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, uint32_t gridPerRef) override
    {
    m_acsData.m_grid.Init();

    if (0.0 == uorPerGrid || 0 == gridReps.x || 0 == gridReps.y)
        return ERROR;

    m_acsData.m_grid.m_repetitions  = gridReps;
    m_acsData.m_grid.m_originOffset = gridOffset;
    m_acsData.m_grid.m_uorPerGrid   = uorPerGrid;
    m_acsData.m_grid.m_gridPerRef   = gridPerRef;
    m_acsData.m_grid.m_ratio        = gridRatio;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, uint32_t& gridPerRef) const override
    {
    gridReps   = m_acsData.m_grid.m_repetitions;
    gridOffset = m_acsData.m_grid.m_originOffset;
    uorPerGrid = m_acsData.m_grid.m_uorPerGrid;
    gridPerRef = m_acsData.m_grid.m_gridPerRef;
    gridRatio  = m_acsData.m_grid.m_ratio;

    return (0.0 == uorPerGrid || 0 == gridReps.x || 0 == gridReps.y) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_Serialize(void *buffer, uint32_t maxSize) const override
    {
    if (maxSize < sizeof(m_acsData))
        return ERROR;

    memcpy(buffer, &m_acsData, sizeof(m_acsData));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_PointFromString
(
DPoint3dR       outPoint,
Utf8StringR     errorMsg,
Utf8CP          inString,
bool            relative,
DPoint3dCP      inLastPoint,
DgnModelR       modelRef
) override
    {
    DPoint3d    lastPoint;
    lastPoint.Zero();

    DPoint3d    auxOrigin;
    _GetOrigin(auxOrigin);

    RotMatrix   auxRMatrix;
    _GetRotation(auxRMatrix);

    ACSType acsType = _GetType();

    // might need relative if '#' is used as a component.
    if (NULL != inLastPoint)
        {
        lastPoint = *inLastPoint;
        lastPoint.Subtract(auxOrigin);
        auxRMatrix.Multiply(lastPoint);
        }
    else
        {
        relative    = false;
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
            PointParserPtr  pointParser = PointParser::Create(*model, *this);

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

            DistanceParserPtr   distanceParser  = DistanceParser::Create(*model, *this);
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

            DistanceParserPtr   distanceParser  = DistanceParser::Create(*model, *this);
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
virtual StatusInt       /*AuxCoordSys::*/_StringFromPoint
(
Utf8StringR         outString,
Utf8StringR         errorMsg,
DPoint3dCR          inPoint,
bool                delta,
DPoint3dCP          deltaOrigin,
DgnModelR           modelRef,
DistanceFormatterR  distanceFormatter,
DirectionFormatterR directionFormatter 
) override
    {
    DPoint3d    origin;

    if (delta && (NULL != deltaOrigin))
        origin = *deltaOrigin;
    else
        _GetOrigin(origin);

    DPoint3d    tPoint;
    RotMatrix   rotation;

    // NOTE: Don't apply ACS scale...will be handled by formatters...
    tPoint.DifferenceOf(inPoint, origin);
    _GetRotation(rotation);
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
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    /*AuxCoordSys::*/_DrawGrid(DecorateContextR context) const override
    {
    // Called for active ACS when tcb->gridOrientation is GridOrientationType::ACS.
    DPoint3d    origin;
    RotMatrix   rMatrix;

    GetOrigin(origin);
    GetRotation(rMatrix);

    Point2d     gridReps, gridOffset;
    DPoint2d    spacing;
    uint32_t    gridPerRef;

    // Adjust origin for grid offset if we are displaying a fixed sized grid plane...
    if (SUCCESS == GetGridSpacing(spacing, gridPerRef, gridReps, gridOffset, *context.GetViewport()) && (0 != gridOffset.x || 0 != gridOffset.y))
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
virtual void    /*AuxCoordSys::*/_PointToGrid(DgnViewportR viewport, DPoint3dR point) const override
    {
    DPoint3d    origin;
    RotMatrix   rMatrix;

    GetOrigin(origin);
    GetRotation(rMatrix);

    viewport.GetViewController().PointToStandardGrid(viewport, point, origin, rMatrix);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JohnFerguson    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     /*AuxCoordSys::*/ValidateData
(
DPoint3d&       origin,         /* => origin to test */
RotMatrix&      rMatrix,        /* => rotation to test */
bool            is3D            /* => indicates whether the model to be used with this ACS data is 3D */
)
    {
    /* NOTE: This routine is meant to enforce some sanity on the data:

    3D or 2D model:
        the rotation matrix must be orthogonal
        the origin must be in the design plane

    2D model:
        the rotation matrix must have a z-row of 0 0 1 and z-col of 0 0 1
        the origin->z = 0.0

    When complete, this rountine will have forced the given data into proper form
    if it did  not meet these criteria.

    If we fail to square and normalize we fall back to identity, to allow the system to proceed. */

    if (!is3D)
        {
        /* The test for isXYRotation should be sufficient, but that function will allow left handed matrices.
           LH Matrices are unlikely, but would cause big problems in 2D */
        if (!bsiRotMatrix_isXYRotation(&rMatrix, NULL) || 0.0 > rMatrix.Determinant())
            rMatrix.InitIdentity();
        }

    if (!rMatrix.IsOrthogonal())
        {
        rMatrix.TransposeOf(rMatrix);

        if (rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 2))
            rMatrix.TransposeOf(rMatrix);
        else
            rMatrix.InitIdentity();
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void     /*AuxCoordSys::*/TrimWhiteSpace(Utf8StringR inputStr)
    {
    // Don't store leading/trailing spaces...
    inputStr.erase(inputStr.find_last_not_of(' ')+1);
    inputStr.erase(0, inputStr.find_first_not_of(' '));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys()
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys(ACSData& acsData)
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;
    m_acsData           = acsData;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys(AuxCoordSys const* source)
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;

    m_acsData = source->m_acsData;
    m_name.assign(source->GetName());
    m_description.assign(source->GetDescription());
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static AuxCoordSys* /*AuxCoordSys::*/CreateNew()
    {
    return new AuxCoordSys();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static AuxCoordSys* /*AuxCoordSys::*/CreateNew(ACSData& acsData)
    {
    return new AuxCoordSys(acsData);
    }


}; // AuxCoordSys

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          IAuxCoordSys::GetGridScaleFactor(DgnViewportR vp) const
    {
    double      scaleFactor = 1.0;

#ifdef WIP_V10_MODEL_ACS
    // Apply ACS scale to grid if ACS Context Lock active...
    if (TO_BOOL (vp.GetTargetModel()->GetModelFlag(MODELFLAG_ACS_LOCK)))
        scaleFactor *= GetScale();
#endif

    return scaleFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IAuxCoordSys::GetGridSpacing(DPoint2dR spacing, uint32_t& gridPerRef, Point2dR gridReps, Point2dR gridOffset, DgnViewportR vp) const
    {
    double      uorPerGrid, gridRatio;

    if (SUCCESS != _GetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef))
        {
        vp.GetViewController()._GetGridSpacing(vp, spacing, gridPerRef); // Inherit view controller settings if not specified...

        return ERROR;
        }

    uorPerGrid *= GetGridScaleFactor(vp);

    spacing.x = uorPerGrid;
    spacing.y = spacing.x * gridRatio;

    spacing.Scale(spacing, (0 == gridPerRef) ? 1.0 : (double) gridPerRef);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAuxCoordSys::Locate(DPoint3dR hitPt, DgnViewportR vp, DPoint3dCR borePt, double radius)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef IAuxCoordSys::_GetColor(DgnViewportCR viewport, ColorDef inColor, uint32_t transparency, ACSDisplayOptions options) const
    {
    ColorDef    color;

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Hilite))
        color = viewport.GetHiliteColor();
    else if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Active))
        color = ColorDef::White() == inColor ? viewport.GetContrastToBackgroundColor() : inColor;
    else
        color = ColorDef(150, 150, 150, 0);

    color = viewport.AdjustColorForContrast(color, viewport.GetBackgroundColor());
    color = viewport.MakeColorTransparency(color, transparency);

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t IAuxCoordSys::_GetTransparency(bool isFill, ACSDisplayOptions options) const
    {
    uint32_t    transparency;

    if (isFill)
        transparency = (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 215 : 150);
    else
        transparency = (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 125 : 50);

    return transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IAuxCoordSys::_GetAxisLabel(uint32_t axis) const
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
void IAuxCoordSys::_AddAxisText(GraphicBuilderR graphic, Utf8CP labelStr, bool isAxisLabel, double userOrgX, double userOrgY, double scale, double angle, ACSDisplayOptions options) const
    {
    DPoint3d    textPt;
    RotMatrix   textMatrix;
    TextString  textStr;

    textPt.x = userOrgX; textPt.y = userOrgY; textPt.z = 0.0;
    textMatrix.InitFromAxisAndRotationAngle(2, angle);

    textStr.SetText(labelStr);
    textStr.SetOrientation(textMatrix);
    textStr.GetStyleR().SetFont(DgnFontManager::GetDecoratorFont());
    textStr.GetStyleR().SetSize(scale);
    textStr.SetOriginFromJustificationOrigin(textPt, TextString::HorizontalJustification::Center, TextString::VerticalJustification::Middle);

    // Draw background fill for hilited ACS for select ACS tool, in case multiple ACS share common origin...
    if (!isAxisLabel && ACSDisplayOptions::None != (options & ACSDisplayOptions::Hilite))
        {
        DPoint3d pts[5];

        textStr.ComputeBoundingShape(pts, (scale / 10.0));
        textStr.ComputeTransform().Multiply(pts, _countof(pts));

        graphic.SetBlankingFill(graphic.GetViewport()->GetBackgroundColor());
        graphic.AddShape(5, pts, true);
        }

    ColorDef    lineColor = _GetColor(*graphic.GetViewport(), ColorDef::White(), _GetTransparency(false, options), options);

    graphic.SetSymbology(lineColor, lineColor, isAxisLabel ? 2 : 1);
    graphic.AddTextString(textStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void IAuxCoordSys::_AddZAxis(GraphicBuilderR builder, ColorDef color, ACSDisplayOptions options) const
    {
    DPoint3d    linePts[2];

    memset(linePts, 0, sizeof (linePts));
    linePts[1].z = 0.65;

    ColorDef    lineColor = _GetColor(*builder.GetViewport(), color, _GetTransparency(false, options), options);
    ColorDef    fillColor = _GetColor(*builder.GetViewport(), color, _GetTransparency(true, options), options);

    builder.SetSymbology(lineColor, lineColor, 6);
    builder.AddPointString(2, linePts);

    builder.SetSymbology(lineColor, lineColor, 1);
    builder.AddLineString(2, linePts);

    double      start = 0.0, sweep = msGeomConst_2pi, scale = ARROW_TIP_WIDTH/2.0;
    DVec3d      xVec, yVec;
    DPoint3d    center;
    RotMatrix   viewRMatrix = builder.GetViewport()->GetRotMatrix();

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void IAuxCoordSys::_AddXYAxis(GraphicBuilderR builder, ColorDef color, Utf8CP labelStrP, bool swapAxis, ACSDisplayOptions options, ACSFlags flags) const
    {
    DPoint2d    userOrg;
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

    userOrg.x = 0.65; userOrg.y = 0.0;

    if (swapAxis)
        {
        for (int i=0; i < 8; i++)
            std::swap(shapePts[i].x, shapePts[i].y);

        std::swap(userOrg.x, userOrg.y);
        }

    ColorDef    lineColor = _GetColor(*builder.GetViewport(), color, _GetTransparency(false, options), options);
    ColorDef    fillColor = _GetColor(*builder.GetViewport(), color, _GetTransparency(true, options), options);

    builder.SetSymbology(lineColor, lineColor, 1);
    builder.AddLineString(8, shapePts);

    if (nullptr != labelStrP)
        _AddAxisText(builder, labelStrP, true, userOrg.x, userOrg.y, 0.35, swapAxis ? 0.0 : -msGeomConst_pi/2.0, options);

    builder.SetBlankingFill(fillColor);
    builder.AddShape(8, shapePts, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr IAuxCoordSys::_CreateGraphic(DecorateContextR context, DPoint3dCR drawOrigin, double acsSizePixels, ACSDisplayOptions options, bool drawName) const
    {
    double      scale = context.GetPixelSizeAtPoint(&drawOrigin) * acsSizePixels;
    double      exagg = context.GetViewport()->GetViewController().GetAspectRatioSkew();
    RotMatrix   rMatrix;
    Transform   transform;

    _GetRotation(rMatrix);

    rMatrix.InverseOf(rMatrix);
    rMatrix.ScaleRows(rMatrix,  scale,  scale / exagg,  scale);
    transform.InitFrom(rMatrix, drawOrigin);

    Render::GraphicBuilderPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport(), transform));

    ACSFlags    flags = _GetFlags();

    _AddZAxis(*graphic, ColorDef::Blue(), options);
    _AddXYAxis(*graphic, ColorDef::Red(), _GetAxisLabel(0).c_str(), false, options, flags);
    _AddXYAxis(*graphic, ColorDef::Green(), _GetAxisLabel(1).c_str(), true, options, flags);

    if (drawName)
        {
        rMatrix = context.GetViewport()->GetRotMatrix();
        rMatrix.InverseOf(rMatrix);
        rMatrix.ScaleRows(rMatrix, scale, scale, scale);

        Transform   invTransform;
        Transform   subToGraphic = Transform::From(rMatrix, drawOrigin);

        invTransform.InverseOf(transform);
        subToGraphic = Transform::FromProduct(subToGraphic, invTransform);

        Render::GraphicBuilderPtr labelGraphic = graphic->CreateSubGraphic(subToGraphic);
        GraphicParams graphicParams;

        _AddAxisText(*labelGraphic, _GetName().data(), false, 0.0, -0.5, 0.25, 0.0, options);
        graphic->AddSubGraphic(*labelGraphic, subToGraphic, graphicParams);
        }

    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool IAuxCoordSys::_IsOriginInView(DPoint3dR drawOrigin, DgnViewportCR viewport, bool adjustOrigin) const
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
        double offset = (ACS_SIZE_OFFSCREEN+15);

        LIMIT_RANGE (offset, screenRange.x-offset, testPtView.x);
        LIMIT_RANGE (offset, screenRange.y-offset, testPtView.y);
        }

    // Limit point to NPC box to prevent triad from being clipped from display...
    DPoint3d originPtNpc;

    viewport.ViewToNpc(&originPtNpc, &testPtView, 1);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.x);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.y);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.z);
    viewport.NpcToView(&testPtView, &originPtNpc, 1);
    viewport.ViewToWorld(&drawOrigin, &testPtView, 1);

    return inView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IAuxCoordSys::_DisplayInView(DecorateContextR context, ACSDisplayOptions options, bool drawName) const
    {
    DPoint3d    drawOrigin;
    bool        checkOutOfView = (ACSDisplayOptions::None != (options & ACSDisplayOptions::CheckVisible));

    _GetOrigin(drawOrigin);

    if (checkOutOfView && !_IsOriginInView(drawOrigin, *context.GetViewport(), true))
        options = options | ACSDisplayOptions::Deemphasized;

    double      screenSize;

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized))
        screenSize = ACS_SIZE_OFFSCREEN;
    else if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Active))
        screenSize = ACS_SIZE_ACTIVE;
    else
        screenSize = ACS_SIZE_INACTIVE;

    Render::GraphicBuilderPtr graphic = _CreateGraphic(context, drawOrigin, screenSize, options, drawName);

    if (!graphic.IsValid())
        return;

    context.AddWorldOverlay(*graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IACSManager::IACSManager()
    {
    m_inhibitCurrentACSDisplay  = false;
    m_extenders = nullptr;
    m_listeners = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP   IACSManager::GetActive(DgnViewportR vp)
    {
    SpatialViewControllerCP viewController = vp.GetSpatialViewControllerCP ();
    if (NULL == viewController)
        return NULL;

    //IAuxCoordSysP   acs;
    //
    //if (NULL != (acs = viewInfo->GetAuxCoordinateSystem ()))
    //    return acs;
    //
    //// there is currently no ACS on the viewInfo. We create a default ACS and put that on the viewInfo.
    //DgnModelP    targetDgnModel = viewInfo->GetTargetDgnModelP();
    //DgnModelP    rootDgnModel   = viewInfo->GetRootModelP();
    //ACSData         acsData;
    //
    //if (vp.IsTargetRoot ())
    //    {
    //    if (SUCCESS != dgnModel_getAuxCoordinateSystem (rootDgnModel ? rootDgnModel->GetDgnModelP () : NULL, &acsData.m_origin, &acsData.m_rotation, &acsData.m_scale, &acsData.m_type, &acsData.m_elementId) || 0 == static_cast<int>(acsData.m_type))
    //        dgnModel_getGlobalOrigin (rootDgnModel->GetDgnModelP (), &acsData.m_origin);
    //    }
    //else
    //    {
    //    IAuxCoordSysP   savedViewACS = setACSFromRefSavedView (targetDgnModel->AsDgnAttachmentP (), viewInfo);
    //
    //    if (savedViewACS)
    //        return savedViewACS;
    //
    //    dgnModel_getGlobalOrigin (targetDgnModel->GetDgnModelP (), &acsData.m_origin);
    //    }
    //
    //acs = AuxCoordSys::CreateNew (acsData);
    //viewInfo->SetAuxCoordinateSystem (acs);

    return viewController->GetAuxCoordinateSystem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IACSManager::SetActive(IAuxCoordSysP auxCoordSys, DgnViewportR vp)
    {
    SpatialViewControllerP viewController = vp.GetSpatialViewControllerP();
    if (NULL == viewController)
        return ERROR;

    // Avoid views getting "linked" by always cloning...
    // The elementId of any ACS set to be active should be zero, because we don't want changes to the active ACS to be
    //  changing the "original" stored in the file. Clone ensures this...
    IAuxCoordSysPtr copiedAcs = auxCoordSys->Clone();

    viewController->SetAuxCoordinateSystem(copiedAcs.get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::GetByName(Utf8CP name, DgnModelP modelRef, uint32_t options)
    {
    if (NULL == name)
        { BeAssert(false); return nullptr; }

#if defined (NEEDS_WORK_DGNITEM)
#endif
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IACSManager::Save(IAuxCoordSysP acs, DgnModelP modelRef, ACSSaveOptions saveOption, ACSEventType eventType)
    {
#if defined (NEEDS_WORK_DGNITEM)
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IACSManager::Delete(Utf8CP name, DgnModelP modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::DisplayCurrent(DecorateContextR context, bool isCursorView)
    {
    if (GetInhibitCurrentACSDisplay() || !context.GetViewport()->GetViewFlags().m_acsTriad)
        return;

    IAuxCoordSysP acs = GetActive(*context.GetViewport());

    if (!acs)
        return;

    acs->DisplayInView(context, (ACSDisplayOptions::CheckVisible | (isCursorView ? ACSDisplayOptions::Active : ACSDisplayOptions::Inactive)), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::CreateACS()
    {
    return AuxCoordSys::CreateNew();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::CreateACS(ACSType type, DPoint3dCR pOrigin, RotMatrixCR pRot, double scale, Utf8CP name, Utf8CP descr)
    {
    IAuxCoordSysPtr auxSys = AuxCoordSys::CreateNew();

    auxSys->SetType(type);
    auxSys->SetOrigin(pOrigin);
    auxSys->SetRotation(pRot);
    auxSys->SetScale(scale);

    auxSys->SetName(name);
    auxSys->SetDescription(descr);

    return auxSys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::AddExtender(IAuxCoordSystemExtender* extender)
    {
    if (NULL == m_extenders)
        m_extenders = new EventHandlerList<IAuxCoordSystemExtender>;

    m_extenders->AddHandler(extender);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::RemoveExtender(IAuxCoordSystemExtender* extender)
    {
    if (NULL != m_extenders)
        m_extenders->DropHandler(extender);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::AddListener(IACSEvents* acsListener)
    {
    if (NULL == m_listeners)
        m_listeners = new EventHandlerList<IACSEvents>;

    m_listeners->AddHandler(acsListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::DropListener(IACSEvents* acsListener)
    {
    if (NULL != m_listeners)
        m_listeners->DropHandler(acsListener);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          ACSEventCaller
{
IAuxCoordSysP   m_acs;
ACSEventType    m_eventType;
DgnModelP    m_modelRef;

ACSEventCaller(IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef)
    {
    m_acs       = acs;
    m_eventType = eventType;
    m_modelRef  = modelRef;
    }

void CallHandler(IACSEvents& handler) {handler._OnACSEvent(m_acs, m_eventType, m_modelRef);}

}; // ACSEventCaller

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::SendEvent(IAuxCoordSysP acsP, ACSEventType eventType, DgnModelP modelRef)
    {
    if (NULL == m_listeners)
        return;

    ACSEventCaller eventCaller(acsP, eventType, modelRef);
    m_listeners->CallAllHandlers(eventCaller);
    }

/*=================================================================================**//**
* Class used to define extended ACS data.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct          ACSPersistentData
{
uint32_t        extenderId;
int             extenderData[1];
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::ReadSettings(SpatialViewControllerP viewController)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    // NEEDSWORK...
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::SaveSettings(SpatialViewControllerCP viewController)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    XAttributeChangeSet* changeSet;

    if (NULL != (changeSet = eh.GetXAttributeChangeSet()))
        {
        XAttributeHandlerId acsHandlerId(XATTRIBUTEID_ViewInfo, VIEWINFO_XATTR_SUBID_Acs);

        // the original ACS implementation's xattribute ID was 0, make sure those are gone..
        changeSet->Schedule(acsHandlerId, 0, 0, NULL, XAttributeChange::CHANGETYPE_Delete, true);

        // schedule deletion of the current ACS XAttribute, we'll add it below.
        changeSet->Schedule(acsHandlerId, ACS_XATTRIBUTE_ID, 0, NULL, XAttributeChange::CHANGETYPE_Delete, true);

        IAuxCoordSysP   acsP;

        if (NULL != (acsP = viewController->GetAuxCoordinateSystem()))
            {
            int     extenderDataSize          = acsP->GetSerializedSize();
            int     dataSize                  = sizeof(ACSPersistentData) - sizeof(int) + extenderDataSize;
            ACSPersistentData *persistentData = (ACSPersistentData *) _alloca(dataSize);
            persistentData->extenderId        = acsP->GetExtenderId();

            // get the acs to serialize itself.
            acsP->Serialize(persistentData->extenderData, extenderDataSize);

            // schedule the addition of the XAttributes.
            changeSet->Schedule(acsHandlerId, ACS_XATTRIBUTE_ID, dataSize, persistentData, XAttributeChange::CHANGETYPE_Write, true);
            }
        }
#endif
    }

/*=================================================================================**//**
* Class used to find extended ACS data owner.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct          FindExtenderCaller
    {
    IAuxCoordSystemExtender*    m_foundExtender;
    uint32_t                    m_extenderId;

    FindExtenderCaller(uint32_t extenderId)  {m_extenderId = extenderId; m_foundExtender = NULL;}

    bool CallHandler(IAuxCoordSystemExtender& extender)
        {
        if (extender._GetExtenderId() == m_extenderId)
            {
            m_foundExtender = &extender;

            return true;
            }

        return false;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSystemExtender* IACSManager::FindExtender(uint32_t extenderId)
    {
    FindExtenderCaller caller(extenderId);

    if (NULL != m_extenders)
        m_extenders->CallAllHandlers(caller);

    return caller.m_foundExtender;
    }


/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ACSManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

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
UInt32          m_gridPerRef;   // grid dot per reference grid line...
UInt32          m_unused;       // Unused pad bytes...
double          m_uorPerGrid;   // grid x size spacing in uors...
double          m_ratio;        // grid x/y spacing ratio...

ACSGrid () {Init ();};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Init ()
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
bool IsDefaultSettings () const
    {
    ACSGrid     defaultGrid;

    return IsEqual (defaultGrid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEqual (ACSGrid const& otherData) const
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
ElementId       m_elementId;    // id of named acs element, 0 for unnamed
ACSGrid         m_grid;         // New for Vancouver - ACS grid settings...

ACSData () {Init ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Init ()
    {
    m_type      = ACSType::Rectangular;
    m_flags     = ACSFlags::Default;
    m_scale     = 1.0;

    m_origin.Zero ();
    m_rotation.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEqual (ACSData const& otherData) const
    {
    if (m_type != otherData.m_type)
        return false;

    if (m_flags != otherData.m_flags)
        return false;

    if (!m_origin.IsEqual (otherData.m_origin))
        return false;

    if (m_scale != otherData.m_scale)
        return false;

    if (!m_rotation.IsEqual (otherData.m_rotation))
        return false;

    return m_grid.IsEqual (otherData.m_grid);
    }

}; // ACSData

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          AuxCoordSys : public IAuxCoordSys
{
private:

ACSData         m_acsData;
WString         m_name;
WString         m_description;
bool            m_attachedToView;
bool            m_viewInfoValidated;

protected:

virtual ACSType     /*AuxCoordSys::*/_GetType () const override {return m_acsData.m_type;}
virtual WString     /*AuxCoordSys::*/_GetName () const override {return m_name;}
virtual WString     /*AuxCoordSys::*/_GetDescription () const override {return m_description;}
virtual bool        /*AuxCoordSys::*/_GetIsReadOnly () const override {return false;}
virtual ElementId   /*AuxCoordSys::*/_GetElementId() const override {return m_acsData.m_elementId ;}
virtual UInt32      /*AuxCoordSys::*/_GetExtenderId() const override {return 0;}
virtual UInt32      /*AuxCoordSys::*/_GetSerializedSize() const override {return sizeof (m_acsData);}

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetElementId (ElementId elementId, DgnModelP modelRef) override
    {
    if (m_attachedToView)
        {
        // If active ACS from named, these will be updated from named ACS element...
        m_name.clear ();
        m_description.clear ();
        }

    if (elementId.IsValid() && NULL != modelRef)
        {
        ElementRefP    elemRef = modelRef->FindElementById(elementId);
        ElementHandle  elHandle (elemRef);

        // Verify that new element id is valid...
        if (IACSManager::ElementIsACS (elHandle))
            {
            if (m_attachedToView)
                {
                // Update current ACS from named...
                if (SUCCESS != SetDataFromElement (elHandle))
                    elementId.Invalidate();
                }
            }
        else
            {
            elementId.Invalidate();
            }
        }

    m_acsData.m_elementId = elementId;

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    /*AuxCoordSys::*/_OnDataModified ()
    {
#if defined (NEEDS_WORK_DGNITEM)
    // Break active ACS connection to named ACS when data changed...
    if (!m_attachedToView)
        return;

    _SetElementId (ElementId(), NULL);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetType (ACSType type) override
    {
    m_acsData.m_type = type;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetName (WCharCP name) override
    {
    m_name.assign (name);
    TrimWhiteSpace (m_name); // Don't store leading/trailing spaces...

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetDescription (WCharCP descr) override
    {
    m_description.assign (descr);
    TrimWhiteSpace (m_description); // Don't store leading/trailing spaces...

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetScale (double scale) override
    {
    m_acsData.m_scale = scale;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetOrigin (DPoint3dCR pOrigin) override
    {
    m_acsData.m_origin = pOrigin;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SetRotation (RotMatrixCR pRot) override
    {
    // Also clear view independent flag when rotation being explicitly set...
    ACSFlags    flags = m_acsData.m_flags;

    flags = flags & ~ACSFlags::ViewIndependent;
    m_acsData.m_flags = flags;

    m_acsData.m_rotation = pRot;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_SetFlags (ACSFlags flags) override
    {
    m_acsData.m_flags = flags;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_CompleteSetupFromViewController (PhysicalViewControllerCP info)
    {
    m_attachedToView = true;

    // Make sure view independent ACS has CURRENT view rotation...
    if (ACSFlags::None != (ACSFlags::ViewIndependent & m_acsData.m_flags))
        {
        m_acsData.m_rotation = info->GetRotation ();
        }

    // Finish moderately expensive setup now that we have both acs data and view...
    if (!m_viewInfoValidated)
        {
#ifdef DGNV10FORMAT_CHANGES_WIP
        SetElementId (m_acsData.m_elementId, info->GetRootModelP());
        ValidateData (m_acsData.m_origin, m_acsData.m_rotation, (NULL != info->GetTargetDgnModelP ()) ? info->GetTargetDgnModelP()->Is3d () : info->TreatViewAs3D ());

        m_viewInfoValidated = true;
#endif
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IAuxCoordSysPtr /*AuxCoordSys::*/_Clone () const override
    {
    return new AuxCoordSys (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool    /*AuxCoordSys::*/_Equals (IAuxCoordSysCP other) const override
    {
    if (NULL == other)
        return false;

    if (this == other)
        return true;

    AuxCoordSys const * otherACS;

    if (NULL == (otherACS = dynamic_cast <AuxCoordSys const *> (other)))
        return false;

    return m_acsData.IsEqual (otherACS->m_acsData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WString /*AuxCoordSys::*/_GetTypeName () const override
    {
    return DgnCoreL10N::GetStringW((DgnCoreL10N::Number)(DgnCoreL10N::Number::ACS_TYPE_BASE_ + static_cast<int>(m_acsData.m_type)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double  /*AuxCoordSys::*/_GetScale () const override
    {
    return m_acsData.m_scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DPoint3dR   /*AuxCoordSys::*/_GetOrigin (DPoint3dR pOrigin) const override
    {
    pOrigin = m_acsData.m_origin;

    return pOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR  /*AuxCoordSys::*/_GetRotation (RotMatrixR pRot) const override
    {
    pRot = m_acsData.m_rotation;

    return pRot;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual RotMatrixR  /*AuxCoordSys::*/_GetRotation (RotMatrixR pRot, DPoint3dR pPosition) const override
    {
    return _GetRotation (pRot); // not position dependent.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ACSFlags    /*AuxCoordSys::*/_GetFlags () const override
    {
    return m_acsData.m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_SetStandardGridParams (Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, UInt32 gridPerRef) override
    {
    m_acsData.m_grid.Init ();

    if (0.0 == uorPerGrid || 0 == gridReps.x || 0 == gridReps.y)
        return ERROR;

    m_acsData.m_grid.m_repetitions  = gridReps;
    m_acsData.m_grid.m_originOffset = gridOffset;
    m_acsData.m_grid.m_uorPerGrid   = uorPerGrid;
    m_acsData.m_grid.m_gridPerRef   = gridPerRef;
    m_acsData.m_grid.m_ratio        = gridRatio;

    _OnDataModified ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt /*AuxCoordSys::*/_GetStandardGridParams (Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, UInt32& gridPerRef) const override
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
virtual StatusInt   /*AuxCoordSys::*/_Serialize (void *buffer, UInt32 maxSize) const override
    {
    if (maxSize < sizeof(m_acsData))
        return ERROR;

    memcpy (buffer, &m_acsData, sizeof(m_acsData));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_PointFromString
(
DPoint3dR       outPoint,
WStringR        errorMsg,
WCharCP         inString,
bool            relative,
DPoint3dCP      inLastPoint,
DgnModelR    modelRef
) override
    {
    DPoint3d    lastPoint;
    lastPoint.Zero();

    DPoint3d    auxOrigin;
    _GetOrigin   (auxOrigin);

    RotMatrix   auxRMatrix;
    _GetRotation (auxRMatrix);

    ACSType acsType = _GetType();

    // might need relative if '#' is used as a component.
    if (NULL != inLastPoint)
        {
        lastPoint = *inLastPoint;
        lastPoint.subtract (&auxOrigin);
        auxRMatrix.Multiply(lastPoint);
        }
    else
        {
        relative    = false;
        }

    DgnModelP   model = &modelRef;

    // make a local copy of the input string that we can modify.
    WString         tmpString (inString);
    StatusInt       status;

    switch (acsType)
        {
        case ACSType::Rectangular:
            {
            PointParserPtr  pointParser = PointParser::Create (*model, *this);

            Point3d isRelative;
            if (SUCCESS != (status = pointParser->ToValue (outPoint, isRelative, tmpString.c_str())))
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
            bvector<WString> subStrings;
            BeStringUtilities::Split (inString, L",", subStrings);

            if (subStrings.size() < 2)
                return ERROR;

            DistanceParserPtr   distanceParser  = DistanceParser::Create (*model, *this);
            DirectionParserPtr  directionParser = DirectionParser::Create (*model);

            double radius;
            if (SUCCESS != (status = distanceParser->ToValue (radius, subStrings[0].c_str())))
                return status;

            double theta;
            if (SUCCESS != (status = directionParser->ToValue (theta, subStrings[1].c_str())))
                return status;

            theta = bsiTrig_degreesToRadians (theta);

            if ( relative && ( (lastPoint.x != 0.0) || (lastPoint.y != 0.0)) )
                {
                theta += atan2 (lastPoint.y, lastPoint.x);
                radius += sqrt (lastPoint.x*lastPoint.x + lastPoint.y*lastPoint.y);
                }

            outPoint.x = radius * cos(theta);
            outPoint.y = radius * sin(theta);

            if (modelRef.Is3d ())
                {
                if (subStrings.size () < 3)
                    return ERROR;

                if (SUCCESS != (status = distanceParser->ToValue (outPoint.z, subStrings[2].c_str())))
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
            bvector<WString> subStrings;
            BeStringUtilities::Split (inString, L",", subStrings);

            if (subStrings.size() < 3)
                return ERROR;


            DistanceParserPtr   distanceParser  = DistanceParser::Create (*model, *this);
            DirectionParserPtr  directionParser = DirectionParser::Create (*model);

            double rho;
            if (SUCCESS != (status = distanceParser->ToValue (rho, subStrings[0].c_str())))
                return status;

            double theta;
            if (SUCCESS != (status = directionParser->ToValue (theta, subStrings[1].c_str())))
                return status;

            double phi;
            if (SUCCESS != (status = directionParser->ToValue (phi, subStrings[2].c_str())))
                return status;

            theta = bsiTrig_degreesToRadians (theta);
            phi   = bsiTrig_degreesToRadians (phi);

            if (relative)
                {
                double origRho;

                rho += (origRho =  ((DVec3dP)&lastPoint)->Magnitude ());

                if (lastPoint.x != 0.0 || lastPoint.y != 0.0)
                    theta += atan2 (lastPoint.y, lastPoint.x);

                if (origRho != 0.0)
                    phi += Angle::Acos (lastPoint.z/origRho);
                }

            double radius;
            outPoint.z  = rho * cos(phi);
            radius      = rho * sin(phi);
            outPoint.x  = radius * cos(theta);
            outPoint.y  = radius * sin(theta);
            break;
            }
        }

    auxRMatrix.MultiplyTranspose (outPoint);
    outPoint.Add (*( (DVec3d *)&auxOrigin));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt       /*AuxCoordSys::*/_StringFromPoint
(
WStringR            outString,
WStringR            errorMsg,
DPoint3dCR          inPoint,
bool                delta,
DPoint3dCP          deltaOrigin,
DgnModelR        modelRef,
DistanceFormatterR  distanceFormatter,
DirectionFormatterR directionFormatter 
) override
    {
    DPoint3d    origin;

    if (delta && (NULL != deltaOrigin))
        origin = *deltaOrigin;
    else
        _GetOrigin (origin);

    DPoint3d    tPoint;
    RotMatrix   rotation;

    // NOTE: Don't apply ACS scale...will be handled by formatters...
    tPoint.DifferenceOf (inPoint, origin);
    _GetRotation (rotation);
    rotation.Multiply (tPoint);

    ACSType     acsType = _GetType();

    switch (acsType)
        {
        case ACSType::Rectangular:
            {
            PointFormatterPtr formatter = PointFormatter::Create (distanceFormatter);
            formatter->SetIs3d (modelRef.Is3d());
            outString = formatter->ToString (tPoint);
            break;
            }

        case ACSType::Cylindrical:
            {
            double      distance, angle;

            distance = sqrt (tPoint.x * tPoint.x + tPoint.y * tPoint.y);
            angle    = Angle::Atan2 (tPoint.y, tPoint.x);

            outString = distanceFormatter.ToString (distance);

            WString directionString = directionFormatter.ToStringFromRadians (angle);
            outString.append (L",");
            outString.append (directionString);

            if (modelRef.Is3d ())
                {
                WString elevationString = distanceFormatter.ToString (tPoint.z);
                outString.append (L",");
                outString.append (elevationString);
                }
            break;
            }

        case ACSType::Spherical:
            {
            double      radius, theta, phi;

            radius = ((DVec3d *) &tPoint)->Magnitude ();
            theta = Angle::Atan2 (tPoint.y, tPoint.x);

            if (LegacyMath::DEqual(radius, 0.0))
                phi = 0.0;
            else
                phi = Angle::Acos (tPoint.z/radius);

            outString = distanceFormatter.ToString (radius);
            outString.append (L",");
            WString thetaString = directionFormatter.ToStringFromRadians (theta);
            outString.append (thetaString);

            outString.append (L",");
            // this seems wrong to me. I think it should use the AngleFormatter in the DirectionFormatter.
            WString phiString = directionFormatter.ToStringFromRadians (phi);
            outString.append (phiString);
            break;
            }

        default:
            return ERROR;
        }

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt    /*AuxCoordSys::*/_DeleteFromFile (DgnModelP modelRef) override
    {
    if (modelRef->IsReadOnly ())
        return DGNHANDLERS_STATUS_FileReadonly;

    ElementRefP  elemRef;

    if (!_GetElementId().IsValid() || NULL == (elemRef = modelRef->FindElementById (_GetElementId ())) || elemRef->IsDeleted ())
        return ERROR;

    EditElementHandle  elemHandle (elemRef);

    // make sure it's a named ACS element that we're going to replace...
    if (!IACSManager::ElementIsACS (elemHandle))
        return ERROR;

    return elemHandle.DeleteFromModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt   /*AuxCoordSys::*/_SaveToFile (DgnModelP modelRef, ACSSaveOptions option) override
    {
    if (modelRef->IsReadOnly ())
        return DGNHANDLERS_STATUS_FileReadonly;

    ElementRefP  elemRef;

    if (!_GetElementId().IsValid() || NULL == (elemRef = modelRef->FindElementById (_GetElementId ())) || elemRef->IsDeleted ())
        {
        // if we don't have a valid ElementId, we can't save unless ACSSaveOption is ACSSaveOptions::AllowNew.
        if (ACSSaveOptions::AllowNew != option)
            return DGNHANDLERS_STATUS_WriteInhibit;

        // if we already have an ACS with our name, can't save it.
        if (IACSManager::GetManager().GetByName (_GetName().data(), modelRef, 0).IsValid())
            return DGNHANDLERS_STATUS_AlreadyExists;

        StatusInt          status;
        EditElementHandle  eeh;

        if (SUCCESS != (status = ElementCreate (eeh, *modelRef)))
            return status;

        status = eeh.AddToModel ();

        // Update elementId in ACS object...
        if (SUCCESS == status)
            SetElementId (eeh.GetElementRef ()->GetElementId (), modelRef);

        return status;
        }

    // make sure it's a named ACS element that we're going to replace...
    if (!IACSManager::ElementIsACS (ElementHandle (elemRef)))
        return ERROR;

    StatusInt       status;
    EditElementHandle  eeh;

    if (SUCCESS != (status = ElementCreate (eeh, *modelRef)))
        return status;

    status = eeh.ReplaceInModel (elemRef);

    // Update elementId in ACS object...
    if (SUCCESS == status)
        SetElementId (eeh.GetElementRef ()->GetElementId (), modelRef);

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    /*AuxCoordSys::*/_DrawGrid (ViewportP viewport) const override
    {
    // Called for active ACS when tcb->gridOrientation is GridOrientationType::ACS.
    DPoint3d    origin;
    RotMatrix   rMatrix;

    GetOrigin (origin);
    GetRotation (rMatrix);

    Point2d     gridReps, gridOffset;
    DPoint2d    spacing;

    // Adjust origin for grid offset if we are displaying a fixed sized grid plane...
    if (SUCCESS == GetGridSpacing (spacing, gridReps, gridOffset, *viewport) && (0 != gridOffset.x || 0 != gridOffset.y))
        {
        DVec3d  xVec, yVec;

        rMatrix.GetRow (xVec, 0);
        rMatrix.GetRow (yVec, 1);

        xVec.Scale (spacing.x);
        yVec.Scale (spacing.y);

        origin.SumOf (origin, xVec, -gridOffset.x, yVec, -gridOffset.y);
        }

    viewport->DrawStandardGrid (origin, rMatrix, (0 == gridReps.x || 0 == gridReps.y) ? NULL : &gridReps);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    /*AuxCoordSys::*/_PointToGrid (ViewportP viewport, DPoint3dR point) const override
    {
    DPoint3d    origin;
    RotMatrix   rMatrix;

    GetOrigin (origin);
    GetRotation (rMatrix);

    viewport->PointToStandardGrid (point, origin, rMatrix);
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
        if (!bsiRotMatrix_isXYRotation (&rMatrix, NULL) || 0.0 > bsiRotMatrix_determinant (&rMatrix))
            bsiRotMatrix_initIdentity (&rMatrix);
        }

    if (!bsiRotMatrix_isOrthogonal (&rMatrix))
        {
        bsiRotMatrix_transpose (&rMatrix, &rMatrix);

        if (bsiRotMatrix_squareAndNormalizeColumns (&rMatrix, &rMatrix, 0, 2))
            bsiRotMatrix_transpose (&rMatrix, &rMatrix);
        else
            rMatrix.InitIdentity ();
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void     /*AuxCoordSys::*/TrimWhiteSpace (WStringR inputStr)
    {
    // Don't store leading/trailing spaces...
    inputStr.erase (inputStr.find_last_not_of (' ')+1);
    inputStr.erase (0, inputStr.find_first_not_of (' '));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys ()
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys (ACSData& acsData)
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;
    m_acsData           = acsData;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
/*AuxCoordSys::*/AuxCoordSys (AuxCoordSys const* source)
    {
    m_attachedToView    = false;
    m_viewInfoValidated = false;

    m_acsData = source->m_acsData;
    m_name.assign (source->GetName ());
    m_description.assign (source->GetDescription ());
    }

public:

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  SetDataFromElement (ElementHandleCR elHandle)
    {
    if (!IACSManager::ElementIsACS (elHandle))
        return DGNHANDLERS_STATUS_BadArg;

    DgnElementCP elmP = elHandle.GetElementCP ();

    m_acsData.m_origin      = elmP->ToAuxCoordinate().origin;
    m_acsData.m_rotation    = elmP->ToAuxCoordinate().rotation;
    m_acsData.m_type        = (ACSType) elmP->ToAuxCoordinate().flags.type;
    m_acsData.m_scale       = 1.0;
    if (elmP->GetElementId().GetValueUnchecked() == 0)
        m_acsData.m_elementId.Invalidate();
    else
        m_acsData.m_elementId = elmP->GetElementId();

    m_acsData.m_grid.Init (); // Set defaults for no grid settings...

    ElementGetName (elHandle, &m_name);
    ElementGetDescription (elHandle, &m_description);

    for (ConstElementLinkageIterator li = elHandle.BeginElementLinkages(); li != elHandle.EndElementLinkages(); ++li)
        {
        UInt16        linkageKey;
        UInt32        numEntries;
        double const* doubleData;
        byte const*   byteData;

        if (NULL != (doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries)))
            {
            if (DOUBLEARRAY_LINKAGE_KEY_AuxCoordScale == linkageKey && 1 == numEntries)
                m_acsData.m_scale = *doubleData;
            }
        else if (NULL != (byteData = ElementLinkageUtil::GetByteArrayDataCP (li, linkageKey, numEntries)))
            {
            if (BYTEARRAY_LINKAGE_KEY_AuxCoordGrid == linkageKey && sizeof (ACSGrid) >= numEntries)
                memcpy (&m_acsData.m_grid, byteData, numEntries);
            }
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    /*AuxCoordSys::*/ElementGetName (ElementHandleCR eh, WStringP pName)
    {
    WChar       acsName[MAX_ACS_NAME_LENGTH];
    StatusInt   status = DGNHANDLERS_STATUS_BadArg;

    if (!IACSManager::ElementIsACS (eh))
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = LinkageUtil::ExtractNamedStringLinkageByIndex (acsName, MAX_ACS_NAME_LENGTH, STRING_LINKAGE_KEY_Name, 0, eh.GetElementCP ())))
        acsName[0] = '\0';

    if (pName)
        {
        pName->assign (acsName);
        TrimWhiteSpace (*pName); // Don't return with leading/trailing spaces...
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    /*AuxCoordSys::*/ElementGetDescription (ElementHandleCR eh, WStringP pDesc)
    {
    WChar       acsDescr[MAX_ACS_NAME_LENGTH];
    StatusInt   status = DGNHANDLERS_STATUS_BadArg;

    if (!IACSManager::ElementIsACS (eh))
        return DGNHANDLERS_STATUS_BadArg;

    if (SUCCESS != (status = LinkageUtil::ExtractNamedStringLinkageByIndex (acsDescr, MAX_ACS_NAME_LENGTH, STRING_LINKAGE_KEY_Description, 0, eh.GetElementCP ())))
        {
        acsDescr[0] = '\0';

        // No description is not an error
        if (DGNHANDLERS_STATUS_LinkageNotFound == status)
            status = SUCCESS;
        }

    if (pDesc)
        {
        pDesc->assign (acsDescr);
        TrimWhiteSpace (*pDesc); // Don't return with leading/trailing spaces...
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   /*AuxCoordSys::*/ElementSetName (EditElementHandleR eeh, WStringCR name)
    {
    if (!IACSManager::ElementIsACS (eeh))
        return DGNHANDLERS_STATUS_BadArg;

    WString     wName = name;

    // Don't store leading/trailing spaces...
    TrimWhiteSpace (wName);

    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (SUCCESS != LinkageUtil::SetStringLinkage (&elm, STRING_LINKAGE_KEY_Name, wName.data ()))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   /*AuxCoordSys::*/ElementSetDescription (EditElementHandleR eeh, WStringCR name)
    {
    if (!IACSManager::ElementIsACS (eeh))
        return DGNHANDLERS_STATUS_BadArg;

    WString     wName = name;

    // Don't store leading/trailing spaces...
    TrimWhiteSpace (wName);

    DgnV8ElementBlank elm(*eeh.GetElementCP());

    if (0 == wName.length ())
        LinkageUtil::DeleteStringLinkage (&elm, STRING_LINKAGE_KEY_Description, 0);
    else if (SUCCESS != LinkageUtil::SetStringLinkage (&elm, STRING_LINKAGE_KEY_Description, wName.data ()))
        return ERROR;

    eeh.ReplaceElement (&elm);

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt /*AuxCoordSys::*/ElementCreate (EditElementHandleR eeh, DgnModelR model)
    {
    DgnV8ElementBlank elemBuf(DgnV8ElementBlank::Zeros::Full);

    elemBuf.SetLegacyType(GROUP_DATA_ELM);
    elemBuf.SetLevel(3);
    elemBuf.SetSizeWordsNoAttributes(sizeof (AuxCoordinate) / 2);
    elemBuf.SetElementId(m_acsData.m_elementId);

    eeh.SetElementDescr(new MSElementDescr(elemBuf, model), false);

    eeh.GetElementP ()->ToAuxCoordinateR().flags.type = static_cast <UInt16> (m_acsData.m_type);
    eeh.GetElementP ()->ToAuxCoordinateR().origin     = m_acsData.m_origin;
    eeh.GetElementP ()->ToAuxCoordinateR().rotation   = m_acsData.m_rotation;

    // Remove existing scale and grid linkages, will add new ones if needed...
    for (ElementLinkageIterator li = eeh.BeginElementLinkages(); li != eeh.EndElementLinkages(); ++li)
        {
        UInt16        linkageKey;
        UInt32        numEntries;
        double const* doubleData;
        byte const*   byteData;

        if (NULL != (doubleData = ElementLinkageUtil::GetDoubleArrayDataCP (li, linkageKey, numEntries)))
            {
            if (DOUBLEARRAY_LINKAGE_KEY_AuxCoordScale == linkageKey)
                eeh.RemoveElementLinkage (li);
            }
        else if (NULL != (byteData = ElementLinkageUtil::GetByteArrayDataCP (li, linkageKey, numEntries)))
            {
            if (BYTEARRAY_LINKAGE_KEY_AuxCoordGrid == linkageKey)
                eeh.RemoveElementLinkage (li);
            }
        }

    if (0.0 < m_acsData.m_scale && !LegacyMath::DEqual (m_acsData.m_scale, 1.0))
        ElementLinkageUtil::AppendDoubleArrayData (eeh, DOUBLEARRAY_LINKAGE_KEY_AuxCoordScale, 1, &m_acsData.m_scale);

    if (!m_acsData.m_grid.IsDefaultSettings ())
        ElementLinkageUtil::AppendByteArrayData (eeh, BYTEARRAY_LINKAGE_KEY_AuxCoordGrid, sizeof (m_acsData.m_grid), (byte *) &m_acsData.m_grid);

    if (SUCCESS != ElementSetName (eeh, _GetName ()))
        return ERROR;

    return ElementSetDescription (eeh, _GetDescription ());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static AuxCoordSys* /*AuxCoordSys::*/CreateNew ()
    {
    return new AuxCoordSys ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static AuxCoordSys* /*AuxCoordSys::*/CreateNew (ACSData& acsData)
    {
    return new AuxCoordSys (acsData);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static AuxCoordSys* /*AuxCoordSys::*/CreateNew (ElementHandleCR eh)
    {
    AuxCoordSys* acs = new AuxCoordSys ();

    if (SUCCESS != acs->SetDataFromElement (eh))
        {
        DELETE_AND_CLEAR (acs);

        return NULL;
        }

    return acs;
    }
#endif

}; // AuxCoordSys

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          IAuxCoordSys::GetGridScaleFactor (ViewportR vp) const
    {
    double      scaleFactor = 1.0;

#ifdef WIP_V10_MODEL_ACS
    // Apply ACS scale to grid if ACS Context Lock active...
    if (TO_BOOL (vp.GetTargetModel ()->GetModelFlag (MODELFLAG_ACS_LOCK)))
        scaleFactor *= GetScale ();
#endif

    return scaleFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IAuxCoordSys::GetGridSpacing (DPoint2dR spacing, Point2dR gridReps, Point2dR gridOffset, ViewportR vp) const
    {
    double      uorPerGrid, gridRatio;
    UInt32      gridPerRef;

    if (SUCCESS != _GetStandardGridParams (gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef))
        return ERROR;

    uorPerGrid *= GetGridScaleFactor (vp);

    spacing.x = uorPerGrid;
    spacing.y = spacing.x * gridRatio;

    spacing.scale (&spacing, (0 == gridPerRef) ? 1.0 : (double) gridPerRef);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAuxCoordSys::Locate (DPoint3dR hitPt, ViewportR vp, DPoint3dCR borePt, double radius)
    {
    IViewOutputP  output = vp.GetIViewOutput ();

    if (NULL == output)
        return false;

    IAuxCoordSysP   currentACS = IACSManager::GetManager ().GetActive (vp);
    bool            isCurrent = Equals (currentACS);
    DPoint3d        drawOrigin;

    GetOrigin (drawOrigin);

    QvElem*     qvElem = _CreateQvElems (&vp, &drawOrigin, isCurrent ? ACS_SIZE_ACTIVE : ACS_SIZE_INACTIVE, isCurrent ? ACSDisplayOptions::Active : ACSDisplayOptions::Inactive, true);

    if (NULL == qvElem)
        return false;

    DPoint3d    testPtView;

    vp.WorldToView (&testPtView, &borePt, 1);
    testPtView.z = 0.0;

    bool        hitFound = output->LocateQvElem (qvElem, *((DPoint2dCP) &testPtView), 1.0, hitPt, NULL, NULL, NULL);

    output->DeleteCacheElement (qvElem);

    return hitFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/03
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          IAuxCoordSys::_GetColor
(
ViewportP           viewport,
UInt32              menuColor,
UInt32              transparency,
ACSDisplayOptions   options
) const
    {
    UInt32      color;

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Hilite))
        color = viewport->GetHiliteColor ();
    else if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Active))
        color = WHITE_MENU_COLOR_INDEX == menuColor ? viewport->GetContrastToBackgroundColor () : viewport->GetMenuColor (menuColor);
    else
        color = viewport->MakeTrgbColor (150, 150, 150, 0);

    color = viewport->AdjustColorForContrast (color, viewport->GetBackgroundColor ());
    color = viewport->MakeColorTransparency (color, transparency);

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            IAuxCoordSys::_DrawAxisText
(
ViewportP           viewport,
ICachedDrawP        cached,
WCharCP             labelStr,
bool                isAxisLabel,
double              userOrgX,
double              userOrgY,
double              scale,
double              angle,
ACSDisplayOptions   options
) const
    {
    TextStringProperties tsProps;

    tsProps.SetJustification (TextElementJustification::CenterMiddle);

    DPoint2d    textScale;
    DPoint3d    textPt;
    RotMatrix   textMatrix;

    textPt.x = userOrgX; textPt.y = userOrgY; textPt.z = 0.0;
    textScale.x = textScale.y = scale;
    textMatrix.InitFromAxisAndRotationAngle(2,  angle);

    tsProps.SetFontSize (textScale);
    tsProps.SetIs3d (viewport->Is3dView ());

    TextString  textStr (labelStr, &textPt, &textMatrix, tsProps);
    textStr.SetOriginFromUserOrigin (textPt);

    ElemMatSymb elemMatSymb;

    // Draw background fill for hilited ACS for select ACS tool, in case multiple ACS share common origin...
    if (!isAxisLabel && ACSDisplayOptions::None != (options & ACSDisplayOptions::Hilite))
        {
        double          border = scale/10.0;
        DPoint3d        pts[5];
        DPoint2d        expand = {border, border};

        textStr.GenerateBoundingShape (pts, &expand);

        elemMatSymb.SetFillColorTBGR (viewport->GetBackgroundColor ());
        elemMatSymb.SetIsBlankingRegion (true);

        cached->ActivateMatSymb (&elemMatSymb);
        cached->DrawShape3d (5, pts, true, NULL);
        }

    elemMatSymb.SetLineColorTBGR (_GetColor (viewport, WHITE_MENU_COLOR_INDEX, _GetTransparency (false, options), options));
    elemMatSymb.SetFillColorTBGR (_GetColor (viewport, WHITE_MENU_COLOR_INDEX, _GetTransparency (false, options), options));
    elemMatSymb.SetWidth (isAxisLabel ? 2 : 1);
    elemMatSymb.SetIsBlankingRegion (false);

    cached->ActivateMatSymb (&elemMatSymb);
    cached->DrawTextString (textStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            IAuxCoordSys::_DrawZAxis
(
ViewportP           viewport,
ICachedDrawP        cached,
Transform*          transformP,
ACSDisplayOptions   options
) const
    {
    DPoint3d    linePts[2];

    memset (linePts, 0, sizeof (linePts));
    linePts[1].z = 0.65;

    ElemMatSymb elemMatSymb;

    elemMatSymb.SetLineColorTBGR (_GetColor (viewport, BLUE_MENU_COLOR_INDEX, _GetTransparency (false, options), options));
    elemMatSymb.SetFillColorTBGR (_GetColor (viewport, BLUE_MENU_COLOR_INDEX, _GetTransparency (true, options), options));
    elemMatSymb.SetWidth (2);
    cached->ActivateMatSymb (&elemMatSymb);

    cached->DrawLineString3d (2, linePts, NULL);

    elemMatSymb.SetWidth (6);
    cached->ActivateMatSymb (&elemMatSymb);

    cached->DrawPointString3d (2, linePts, NULL);

    double      start = 0.0, sweep = msGeomConst_2pi, scale = ARROW_TIP_WIDTH/2.0;
    DVec3d      xVec, yVec;
    DPoint3d    center;
    RotMatrix   viewRMatrix = viewport->GetRotMatrix ();

    memset (&center, 0, sizeof (center));

    viewRMatrix.getRow (&xVec, 0);
    viewRMatrix.getRow (&yVec, 1);

    transformP->multiplyTransposeMatrixOnly (&xVec);
    transformP->multiplyTransposeMatrixOnly (&yVec);

    xVec.normalize ();
    yVec.normalize ();

    elemMatSymb.SetWidth (1);
    cached->ActivateMatSymb (&elemMatSymb);

    DEllipse3d  ellipse;

    ellipse.InitFromDGNFields3d (center, xVec, yVec, scale, scale, start, sweep);

    cached->DrawArc3d (ellipse, true, true, NULL);
    cached->DrawArc3d (ellipse, false, false, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            IAuxCoordSys::_DrawAxisArrow
(
ViewportP           viewport,
ICachedDrawP        cached,
Transform*          transformP,
UInt32              menuColor,
WCharCP             labelStrP,
bool                swapAxis,
ACSDisplayOptions   options,
ACSFlags            flags
) const
    {
    double      scale = 0.35, angle = swapAxis ? 0.0 : -msGeomConst_pi/2.0;
    DPoint2d    userOrg;
    DPoint3d    shapePts[8];

    memset (shapePts, 0, sizeof (shapePts));

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
            std::swap (shapePts[i].x, shapePts[i].y);

        std::swap (userOrg.x, userOrg.y);
        }

    ElemMatSymb elemMatSymb;

    elemMatSymb.SetLineColorTBGR (_GetColor (viewport, menuColor, _GetTransparency (false, options), options));
    elemMatSymb.SetFillColorTBGR (_GetColor (viewport, menuColor, _GetTransparency (true, options), options));
    elemMatSymb.SetWidth (1);

    if (ACSFlags::None != (flags & ACSFlags::ViewIndependent))
        elemMatSymb.SetIndexedRasterPattern (2, viewport->GetIndexedLinePattern (2));

    if (NULL != labelStrP)
        {
        // Add text and arrow outline...
        cached->ActivateMatSymb (&elemMatSymb);
        cached->DrawLineString3d (8, shapePts, NULL);

        _DrawAxisText (viewport, cached, labelStrP, true, userOrg.x, userOrg.y, scale, angle, options);

        return;
        }

    // Draw arrow fill as blanking region...
    elemMatSymb.SetIsBlankingRegion (true);
    cached->ActivateMatSymb (&elemMatSymb);
    cached->DrawShape3d (8, shapePts, true, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem*         IAuxCoordSys::_CreateQvElems
(
ViewportP           viewport,
DPoint3dCP          drawOrigin,
double              acsSizePixels,
ACSDisplayOptions   options,
bool                drawName
) const
    {
    double      scale;
    RotMatrix   rMatrix;
    Transform   transform;

    _GetRotation (rMatrix);

    scale = viewport->GetPixelSizeAtPoint (drawOrigin, DgnCoordSystem::World) * acsSizePixels;

    double exagg = viewport->GetViewController().GetAspectRatioSkew();
    rMatrix.InverseOf(rMatrix);
    rMatrix.ScaleRows (rMatrix,  scale,  scale / exagg,  scale);
    transform.InitFrom(rMatrix, *drawOrigin);

    ICachedDrawP    cached = viewport->GetICachedDraw ();

    cached->BeginCacheElement (true, viewport->GetIViewOutput()->GetTempElementCache (), viewport->GetViewFlags ());

    cached->PushTransform (transform);

    ACSFlags    flags = _GetFlags();
    WChar     axisLabel[128];

    _DrawZAxis (viewport, cached, &transform, options);
    _DrawAxisArrow (viewport, cached, &transform, RED_MENU_COLOR_INDEX, NULL, false, options, flags);
    _DrawAxisArrow (viewport, cached, &transform, GREEN_MENU_COLOR_INDEX, NULL, true, options, flags);
    _DrawAxisArrow (viewport, cached, &transform, RED_MENU_COLOR_INDEX, _GetAxisLabel(0, axisLabel, 128), false, options, flags);
    _DrawAxisArrow (viewport, cached, &transform, GREEN_MENU_COLOR_INDEX, _GetAxisLabel(1, axisLabel, 128), true, options, flags);

    cached->PopTransform ();

    if (drawName)
        {
        rMatrix = viewport->GetRotMatrix ();
        rMatrix.InverseOf(rMatrix);
        rMatrix.ScaleRows (rMatrix,  scale,  scale,  scale);
        transform.InitFrom(rMatrix, *drawOrigin);

        cached->PushTransform (transform);

        _DrawAxisText (viewport, cached, _GetName().data(), false, 0.0, -0.5, 0.25, 0.0, options);

        cached->PopTransform ();
        }

    return cached->EndCacheElement ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAuxCoordSys::_IsOriginInView (DPoint3dR drawOrigin, ViewportP viewport, bool adjustOrigin) const
    {
    DPoint3d    testPtView, screenRange;
    viewport->WorldToView (&testPtView, &drawOrigin, 1);

    Frustum frustum = viewport->GetFrustum(DgnCoordSystem::Screen, false);

    screenRange.x = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_100));
    screenRange.y = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_010));
    screenRange.z = frustum.GetCorner(NPC_000).Distance(frustum.GetCorner(NPC_001));

    // Check if current acs origin is outside view...
    bool        inView = (!((testPtView.x < 0 || testPtView.x > screenRange.x) || (testPtView.y < 0 || testPtView.y > screenRange.y)));

    if (!adjustOrigin)
        return inView;

    if (!inView)
        {
        double      offset = (ACS_SIZE_OFFSCREEN+15);

        LIMIT_RANGE (offset, screenRange.x-offset, testPtView.x);
        LIMIT_RANGE (offset, screenRange.y-offset, testPtView.y);
        }

    // Limit point to NPC box to prevent triad from being clipped from display...
    DPoint3d    originPtNpc;

    viewport->ViewToNpc (&originPtNpc, &testPtView, 1);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.x);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.y);
    LIMIT_RANGE (0.0, 1.0, originPtNpc.z);
    viewport->NpcToView (&testPtView, &originPtNpc, 1);
    viewport->ViewToWorld (&drawOrigin, &testPtView, 1);

    return inView;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IAuxCoordSys::_DisplayInView (ViewportP viewport, ACSDisplayOptions options, bool drawName) const
    {
    DPoint3d    drawOrigin;
    bool        checkOutOfView = (ACSDisplayOptions::None != (options & ACSDisplayOptions::CheckVisible));

    _GetOrigin (drawOrigin);

    if (checkOutOfView && !_IsOriginInView (drawOrigin, viewport, true))
        options = options | ACSDisplayOptions::Deemphasized;

    double      screenSize;

    if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized))
        screenSize = ACS_SIZE_OFFSCREEN;
    else if (ACSDisplayOptions::None != (options & ACSDisplayOptions::Active))
        screenSize = ACS_SIZE_ACTIVE;
    else
        screenSize = ACS_SIZE_INACTIVE;

    QvElem*     qvElem;

    if (NULL == (qvElem = _CreateQvElems (viewport, &drawOrigin, screenSize, options, drawName)))
        return;

    IViewOutputP    output = viewport->GetIViewOutput ();

    output->DrawQvElem3d (qvElem, 0);
    output->DeleteCacheElement (qvElem);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          SavedACSIterator : std::iterator<std::forward_iterator_tag, const ElementRefP>
{
private:

PersistentElementRefListIterator    m_it;
ElementRefP                          m_elmRef;

SavedACSIterator (PersistentElementRefList* l) {m_elmRef = NULL; m_it = l->begin(); ToSavedACS();}
SavedACSIterator () {m_elmRef = NULL;}

void            ToSavedACS ()
    {
    for (ElementRefP ref = m_it.GetCurrentElementRef(); NULL != ref; ref = m_it.GetNextElementRef())
        {
        ElementHandle  eh (ref);

        if (IACSManager::ElementIsACS (eh))
            {
            m_elmRef = ref;

            return;
            }
        }

    m_elmRef = NULL;
    }

friend struct SavedACSCollection;

public:

SavedACSIterator&   operator++() {++m_it; ToSavedACS(); return *this;}
bool                operator==(SavedACSIterator const& rhs) const {return m_elmRef == rhs.m_elmRef;}
bool                operator!=(SavedACSIterator const& rhs) const {return !(*this == rhs);}

//! Access the saved acs element
ElementRefP const&  operator* () const {return m_elmRef;}

}; // SavedACSIterator

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          SavedACSCollection
{
private:

DgnModelR       m_model;

public:

typedef SavedACSIterator const_iterator;
typedef const_iterator iterator;    //!< only const iteration is possible

SavedACSCollection (DgnModelR dgnModel) : m_model (dgnModel) {;}

SavedACSIterator begin () const {return SavedACSIterator (m_model.GetControlElementsP ());}
SavedACSIterator end   () const {return SavedACSIterator ();}

}; // SavedACSCollection
#endif

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct          TraverseCaller
    {
    IACSTraversalHandler&   m_traverseHandler;
    DgnModelP            m_modelRef;

    TraverseCaller (IACSTraversalHandler& handler, DgnModelP modelRef) : m_traverseHandler(handler) {m_modelRef = modelRef;}
    bool CallHandler (IAuxCoordSystemExtender& extender) {return extender._TraverseExtendedACS (m_traverseHandler, m_modelRef);}

    }; // TraverseCaller

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct          GetByNameTraverser : IACSTraversalHandler
{
WCharCP         m_name;
IAuxCoordSysPtr m_foundACS;

GetByNameTraverser (WCharCP name)
    {
    if (NULL == name)
        BeAssert (false);

    m_name = name;
    }

virtual UInt32  _GetACSTraversalOptions () override {return 0;}

virtual bool    _HandleACSTraversal (IAuxCoordSysR acs) override
    {
    if ( ! acs.GetName().EqualsI (m_name))
        return false;

    m_foundACS = &acs;

    return true; // stop traversal
    }

}; // GetByNameTraverser

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IACSManager::IACSManager ()
    {
    m_inhibitCurrentACSDisplay  = false;
    m_extenders                 = NULL;
    m_listeners                 = NULL;
    }

#ifdef DGNV10FORMAT_CHANGES_WIP_VIEW
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/08
+---------------+---------------+---------------+---------------+---------------+------*/
static IAuxCoordSysP setACSFromRefSavedView (DgnAttachmentP refFile, ViewControllerP viewController)
    {
    if (!refFile)
        return NULL;

    EditElementHandle  eeh;

    if (!refFile->GetNamedViewElement (eeh))
        return NULL;

    ViewControllerPtr savedViewInfo = ViewController::Create (eeh);

    if (NULL != savedViewInfo.get())
        return NULL;

    IAuxCoordSysCP  savedACS = savedViewInfo->GetAuxCoordinateSystem ();

    if (!savedACS)
        return NULL;

    IAuxCoordSysPtr copiedAcs = savedACS->Clone ();

    viewController->SetAuxCoordinateSystem (copiedAcs.get());

    return copiedAcs.get();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP   IACSManager::GetActive (ViewportR vp)
    {
    PhysicalViewControllerCP viewController = vp.GetPhysicalViewControllerCP ();
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
StatusInt       IACSManager::SetActive (IAuxCoordSysP auxCoordSys, ViewportR vp)
    {
    PhysicalViewControllerP viewController = vp.GetPhysicalViewControllerP();
    if (NULL == viewController)
        return ERROR;

    // Avoid views getting "linked" by always cloning...
    // The elementId of any ACS set to be active should be zero, because we don't want changes to the active ACS to be
    //  changing the "original" stored in the file. Clone ensures this...
    IAuxCoordSysPtr copiedAcs = auxCoordSys->Clone ();

    viewController->SetAuxCoordinateSystem (copiedAcs.get());

    // send the acs asynch event.
#ifdef DGNV10FORMAT_CHANGES_WIP
    SendEvent (copiedAcs.get(), ACSEventType::GeometryChanged, vp.GetTargetModel ());
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IACSManager::Traverse (IACSTraversalHandler& handler, DgnModelP modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    for (ElementRefP elRef: SavedACSCollection (*modelRef))
        {
        IAuxCoordSysPtr auxCoordPtr = AuxCoordSys::CreateNew (ElementHandle (elRef));

        if (auxCoordPtr.IsNull ())
            continue;

        // if our traverser returns true, that means we want to stop the scan. Return ERROR to do so.
        if (handler._HandleACSTraversal (*auxCoordPtr))
            return true;
        }

    if (NULL != m_extenders)
        {
        TraverseCaller traverser (handler, modelRef);
        return m_extenders->CallAllHandlers (traverser, true);
        }
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::GetByName (WCharCP name, DgnModelP modelRef, UInt32 options)
    {
    if (NULL == name)
        { BeAssert (false); return NULL; }

    GetByNameTraverser getByName (name);

    Traverse (getByName, modelRef);

    return getByName.m_foundACS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IACSManager::Save (IAuxCoordSysP acs, DgnModelP modelRef, ACSSaveOptions saveOption, ACSEventType eventType)
    {

#if defined (NEEDS_WORK_DGNITEM)
    StatusInt   status;
    // Want Add, not replace...
    if (ACSEventType::None != (ACSEventType::NewACS & eventType))
        acs->SetElementId (ElementId(), NULL);

    // if we don't succeed in writing it to the file, send the event indicating a change, but an in-memory change only.
    if (SUCCESS == (status = acs->SaveToFile (modelRef, saveOption)))
        SendEvent (acs, (ACSEventType) (eventType | ACSEventType::ChangeWritten), modelRef);
    else
        SendEvent (acs, (ACSEventType) (eventType & ~ACSEventType::ChangeWritten), modelRef);

    return status;
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IACSManager::Delete (WCharCP name, DgnModelP modelRef)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (NULL == name)
        { BeAssert (false); return ERROR; }

    StatusInt       status = ERROR;
    IAuxCoordSysPtr deleteAcs;

    if ((deleteAcs = GetByName (name, modelRef, 0)) != NULL)
        {
        if (SUCCESS == (status = deleteAcs.get ()->DeleteFromFile (modelRef)))
            SendEvent (deleteAcs.get (), ACSEventType::Delete, modelRef);
        }

    return status;
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::DisplayCurrent (ViewportP viewport, bool isCursorView)
    {
    if (GetInhibitCurrentACSDisplay () || !viewport || !viewport->GetViewFlags()->auxDisplay)
        return;

    IAuxCoordSysP   acs = GetActive (*viewport);

    if (!acs)
        return;

    acs->DisplayInView (viewport, (ACSDisplayOptions::CheckVisible | (isCursorView ? ACSDisplayOptions::Active : ACSDisplayOptions::Inactive)), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::DisplayTransients (ViewContextR context, bool isPreUpdate)
    {
#if defined (NOT_NOW)
    if (isPreUpdate)
        return;

    ViewportP   viewport = context.GetViewport ();

    if (NULL == viewport)
        return;

    ViewFlagsCP viewFlags = context.GetViewFlags ();

    if (NULL == viewFlags || !viewFlags->auxDisplay)
        return;

    FOR_EACH (ElementRefP elRef, SavedACSCollection (*context.GetCurrentModel ()->GetDgnModelP ()))
        {
        IAuxCoordSysPtr auxCoordPtr = AuxCoordSys::CreateNew (ElementHandle (elRef));

        if (auxCoordPtr.IsNull ())
            continue;

        Point2d     gridReps, gridOffset;
        DPoint2d    spacing;

        if (SUCCESS != auxCoordPtr->GetGridSpacing (spacing, gridReps, gridOffset, *viewport))
            continue;

        DPoint3d    origin;
        RotMatrix   rMatrix;

        auxCoordPtr->GetOrigin (origin);
        auxCoordPtr->GetRotation (rMatrix);

        DVec3d      xVec, yVec;

        rMatrix.GetRow (xVec, 0);
        rMatrix.GetRow (yVec, 1);

        xVec.Scale (spacing.x);
        yVec.Scale (spacing.y);

        origin.SumOf (origin, xVec, -gridOffset.x, yVec, -gridOffset.y);

        DPoint3d    planePts[5];

        planePts[0] = planePts[4] = origin;
        planePts[1].SumOf (planePts[0], xVec, gridReps.x);
        planePts[2].SumOf (planePts[0], xVec, gridReps.x, yVec, gridReps.y);
        planePts[3].SumOf (planePts[0], yVec, gridReps.y);

        UInt32  color      = viewport->GetContrastToBackgroundColor ();
        UInt32  lineColor  = viewport->MakeColorTransparency (color, 190);
        UInt32  planeColor = viewport->MakeColorTransparency (color, 225);

        viewport->SetSymbologyRgb (lineColor, planeColor, 2, 0);

        context.PushPath (elRef);
        context.GetIDrawGeom ().DrawLineString3d (5, planePts, NULL);
        context.PopPath ();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::CreateACS ()
    {
    return AuxCoordSys::CreateNew ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::CreateACS
(
ACSType         type,
DPoint3dCR      pOrigin,
RotMatrixCR     pRot,
double          scale,
WCharCP       name,
WCharCP       descr
)
    {
    IAuxCoordSysPtr auxSys = AuxCoordSys::CreateNew ();

    auxSys->SetType (type);
    auxSys->SetOrigin (pOrigin);
    auxSys->SetRotation (pRot);
    auxSys->SetScale (scale);

    auxSys->SetName (name);
    auxSys->SetDescription (descr);

    return auxSys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       IACSManager::ElementFindByName
(
EditElementHandleR eeh,        // <=
WCharCP       inName,     //  =>
DgnModelP    modelRef    //  =>
)
    {
    WChar     nameBuf[MAX_ACS_NAME_LENGTH]; // Allow for some spaces around name

    wcsncpy (nameBuf, inName, MAX_ACS_NAME_LENGTH);
    nameBuf[MAX_ACS_NAME_LENGTH-1] = '\0';
    strutil_wstrpwspc (nameBuf);

    if (0 == wcslen (nameBuf))
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    for (ElementRefP elRef : SavedACSCollection (*modelRef))
        {
        ElementHandle  eh (elRef);
        WString     wName;

        if (SUCCESS != AuxCoordSys::ElementGetName (eh, &wName))
            continue;

        if ( ! wName.EqualsI (nameBuf))
            continue;

        eeh.SetElementRef (elRef);

        return SUCCESS;
        }
#endif

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::AddExtender (IAuxCoordSystemExtender* extender)
    {
    if (NULL == m_extenders)
        m_extenders = new EventHandlerList<IAuxCoordSystemExtender>;

    m_extenders->AddHandler (extender);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::RemoveExtender (IAuxCoordSystemExtender* extender)
    {
    if (NULL != m_extenders)
        m_extenders->DropHandler (extender);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::AddListener (IACSEvents* acsListener)
    {
    if (NULL == m_listeners)
        m_listeners = new EventHandlerList<IACSEvents>;

    m_listeners->AddHandler (acsListener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::DropListener (IACSEvents* acsListener)
    {
    if (NULL != m_listeners)
        m_listeners->DropHandler (acsListener);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/09
+===============+===============+===============+===============+===============+======*/
struct          ACSEventCaller
{
IAuxCoordSysP   m_acs;
ACSEventType    m_eventType;
DgnModelP    m_modelRef;

ACSEventCaller (IAuxCoordSysP acs, ACSEventType eventType, DgnModelP modelRef)
    {
    m_acs       = acs;
    m_eventType = eventType;
    m_modelRef  = modelRef;
    }

void CallHandler (IACSEvents& handler) {handler._OnACSEvent (m_acs, m_eventType, m_modelRef);}

}; // ACSEventCaller

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            IACSManager::SendEvent (IAuxCoordSysP acsP, ACSEventType eventType, DgnModelP modelRef)
    {
    if (NULL == m_listeners)
        return;

    ACSEventCaller eventCaller (acsP, eventType, modelRef);
    m_listeners->CallAllHandlers (eventCaller);
    }

/*=================================================================================**//**
* Class used to define extended ACS data.
* @bsiclass                                                     Barry.Bentley   01/07
+===============+===============+===============+===============+===============+======*/
struct          ACSPersistentData
{
UInt32          extenderId;
int             extenderData[1];
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::SaveSettings (PhysicalViewControllerCP viewController, ElementHandleCR eh)
    {
#ifdef DGN_IMPORTER_REORG_WIP
    XAttributeChangeSet* changeSet;

    if (NULL != (changeSet = eh.GetXAttributeChangeSet()))
        {
        XAttributeHandlerId acsHandlerId (XATTRIBUTEID_ViewInfo, VIEWINFO_XATTR_SUBID_Acs);

        // the original ACS implementation's xattribute ID was 0, make sure those are gone..
        changeSet->Schedule (acsHandlerId, 0, 0, NULL, XAttributeChange::CHANGETYPE_Delete, true);

        // schedule deletion of the current ACS XAttribute, we'll add it below.
        changeSet->Schedule (acsHandlerId, ACS_XATTRIBUTE_ID, 0, NULL, XAttributeChange::CHANGETYPE_Delete, true);

        IAuxCoordSysP   acsP;

        if (NULL != (acsP = viewController->GetAuxCoordinateSystem()))
            {
            int     extenderDataSize          = acsP->GetSerializedSize();
            int     dataSize                  = sizeof(ACSPersistentData) - sizeof(int) + extenderDataSize;
            ACSPersistentData *persistentData = (ACSPersistentData *) _alloca (dataSize);
            persistentData->extenderId        = acsP->GetExtenderId();

            // get the acs to serialize itself.
            acsP->Serialize (persistentData->extenderData, extenderDataSize);

            // schedule the addition of the XAttributes.
            changeSet->Schedule (acsHandlerId, ACS_XATTRIBUTE_ID, dataSize, persistentData, XAttributeChange::CHANGETYPE_Write, true);
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
    UInt32                      m_extenderId;

    FindExtenderCaller (UInt32 extenderId)  {m_extenderId = extenderId; m_foundExtender = NULL;}

    bool CallHandler (IAuxCoordSystemExtender& extender)
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
IAuxCoordSystemExtender* IACSManager::FindExtender (UInt32 extenderId)
    {
    FindExtenderCaller caller (extenderId);

    if (NULL != m_extenders)
        m_extenders->CallAllHandlers (caller);

    return caller.m_foundExtender;
    }

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysPtr IACSManager::ExtractACS (ElementRefP viewElementRef, DgnModelP modelRef)
    {
    XAttributeHandlerId handlerId (XATTRIBUTEID_ViewInfo, VIEWINFO_XATTR_SUBID_Acs);
    XAttributeHandle    xAttributeIter (viewElementRef, handlerId, ACS_XATTRIBUTE_ID);

    if (xAttributeIter.IsValid())
        {
        ACSPersistentData  *persistentData   = (ACSPersistentData*) xAttributeIter.PeekData();
        UInt32              dataSize         = xAttributeIter.GetSize();
        UInt32              extenderDataSize = dataSize - (sizeof(ACSPersistentData) - sizeof(int));
        UInt32              extenderId       = persistentData->extenderId;

        // if 0 is the extenderId, this is handled internally.
        if (0 == extenderId)
            {
            // NOTE: Additions to end of structure are allowed...
            UInt32      currSize = sizeof (ACSData);
            UInt32      copySize = (extenderDataSize <= currSize ? extenderDataSize : currSize);
            ACSData     acsData;

            memcpy (&acsData, persistentData->extenderData, copySize);

            return AuxCoordSys::CreateNew (acsData);
            }
        else
            {
            IAuxCoordSystemExtender* extender;

            if (NULL != (extender = FindExtender (extenderId)))
                return extender->_Deserialize ((void*)persistentData->extenderData, extenderDataSize, modelRef);
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void IACSManager::ReadSettings (PhysicalViewControllerP viewController, ElementRefP viewElementRef, DgnModelP modelRef)
    {
    IAuxCoordSysPtr acs = ExtractACS (viewElementRef, modelRef);

    if (acs.IsValid ())
        viewController->SetAuxCoordinateSystem (acs.get ());
    }
#endif


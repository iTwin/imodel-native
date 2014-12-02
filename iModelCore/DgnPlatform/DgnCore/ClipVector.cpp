/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ClipVector.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define TOLERANCE_ChordAngle            .1
#define TOLERANCE_ChordLen              1000


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVector::ClipVector (GPArrayCR gpa, double chordTolerance, double angleTolerance, double* zLow, double* zHigh, TransformCP pTransform)
    {
    Transform       transform, inverseTransform;

    if (NULL == pTransform)
        gpa.GraphicsPointArray::GetPlane (transform);
    else
        transform = *pTransform;

    inverseTransform.InverseOf (transform);

    size_t n = gpa.GetGraphicsPointCount ();
    for (size_t loopStart = 0, loopEnd = 0; loopStart < n; loopStart = loopEnd+1)
        {
        gpa.FindMajorBreakAfter (loopStart, loopEnd);

        GPArraySmartP    loopGpa;

        loopGpa->AppendFrom (gpa, loopStart, loopEnd);
        loopGpa->Transform (&inverseTransform);

        ClipPrimitivePtr    primitive = ClipPrimitive::CreateFromGPA (loopGpa, chordTolerance, angleTolerance, 0 != loopStart, zLow, zHigh, &transform);
    
        if (primitive.IsValid())
            push_back (primitive);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr  ClipVector::CreateFromCurveVector (CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double* zLow, double* zHigh)
    {
    Transform       localToWorld, worldToLocal;;
    CurveVectorPtr  localCurveVector;
    DRange3d        range;

    localCurveVector = curveVector.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtStart, localToWorld, worldToLocal, range);

    if (!localCurveVector.IsValid ())
        {
        BeAssert (false);
        return ClipVectorPtr();
        }

    switch (localCurveVector->GetBoundaryType())
        {
        case CurveVector::BOUNDARY_TYPE_Outer:
            {
            ClipPrimitivePtr    clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector (*localCurveVector, chordTolerance, angleTolerance, zLow, zHigh, &localToWorld);
            if (clipPrimitive.IsValid())
                return CreateFromPrimitive (clipPrimitive);
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            ClipVectorPtr   clipVector = ClipVector::Create ();
            for (ICurvePrimitivePtr const& loop: *localCurveVector)
                {
                ClipPrimitivePtr    clipPrimitive;
                CurveVectorCP       loopCurves;

                if (NULL == (loopCurves = loop->GetChildCurveVectorCP()) ||
                    ! (clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector (*loopCurves, chordTolerance, angleTolerance, zLow, zHigh, &localToWorld)).IsValid())
                    {
                    BeAssert (false);
                    continue;
                    }
                clipVector->push_back (clipPrimitive);
                }
            if (!clipVector->empty())
                return clipVector;

            break;
            }
        }
    BeAssert (false);
    return ClipVectorPtr(); 
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVector::ClipVector (ClipPrimitivePtr primitive)
    {
    push_back (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr   ClipVector::CreateCopy (ClipVectorCR inputVector)
    {
    ClipVectorP   clipVector = new ClipVector ();
    clipVector->AppendCopy (inputVector);
    return clipVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipVector::AppendCopy (ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back (ClipPrimitive::CreateCopy (*primitive));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipVector::Append (ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    ClipVector::AppendPlanes (ClipVectorPtr& clip, ClipPlaneSetCR planes, bool invisible)
    {
    if (!clip.IsValid())
        clip = new ClipVector ();

    clip->push_back (ClipPrimitive::CreateFromClipPlanes (planes, invisible));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    ClipVector::AppendShape (ClipVectorPtr& clip, DPoint2dCP points, size_t nPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    ClipPrimitivePtr    clipPrimitive = ClipPrimitive::CreateFromShape (points, nPoints, outside, zLow, zHigh, transform, invisible);

    if (!clip.IsValid())
        clip = new ClipVector ();

    clip->push_back (clipPrimitive);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClipVector::GetRange (DRange3dR range, TransformCP pTransform) const
    {
    range.Init ();

    for (ClipPrimitivePtr const& primitive: *this)
        {
        DRange3d        thisRange;

        if (primitive->GetRange (thisRange, pTransform))
            {
            if (range.IsEmpty())
                range = thisRange;
            else
                range.IntersectionOf (range, thisRange);
            }
        }
    return !range.IsEmpty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipVector::PointInside (DPoint3dCR point, double onTolerance) const
    {
    for (ClipPrimitivePtr const& primitive: *this)
        if (!primitive->PointInside (point, onTolerance))
            return false;

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    ClipVector::TransformInPlace (TransformCR transform)
    {
    for (ClipPrimitivePtr& primitive: *this)
        if (SUCCESS != primitive->TransformInPlace (transform))
            return ERROR;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClipVector::ExtractBoundaryLoops
(
int             *nLoops,        //!< [out] Number of loops 
int             nLoopPoints[],  //!< [out] Number of points in each loop 
DPoint2d        *loopPoints[],  //!< [out] Array of pointers to loop points 
ClipMask*       clipMaskP,      //!< [out] Clip mask (for front-back)
double*         zFrontP,        //!< [out] distance to front clip plane.
double*         zBackP,         //!< [out] distance to back clip plane.
TransformP      transformP,     //!< [out] transform (clip to world)
DPoint2dP       pointBuffer,    //!< [in] points buffer
size_t          nPoints         //!< [in] size of point buffer
) const
    {
    /* NOTE: This code probably isn't correct. Callers should really all be
             changed to work with the ClipDescr from ClipDescr::InitFromElement.
             Current callers of this function don't properly support curves in clips.

             processElementCut (viewHandler)
             DgnAttachment::ClipVoidOneRef
             extractAttachParamsFromNamedFence (refernce.cpp)
    */
    ClipMask    clipMask = ClipMask::None;
    size_t      pointCount = 0;
    double      zFront = 1.0e15, zBack = -1.0e15;

    *nLoops = 0;
    if (empty())
        return;


    for (ClipPrimitivePtr const& primitive: *this)
        {
        Transform   deltaTrans;

        if (primitive == front())
            {
            deltaTrans.InitIdentity ();
            }
        else
            {
            Transform   fwdTrans, invTrans;

            primitive->GetTransforms (&fwdTrans, NULL);
            front()->GetTransforms (NULL, &invTrans);
            deltaTrans.InitProduct (invTrans, fwdTrans);
            }

        loopPoints[*nLoops] = &pointBuffer[pointCount];

        ClipPolygonCP        clipPolygon;

        if (NULL != (clipPolygon = primitive->GetPolygon()))
            {
            clipMask = ClipMask::XAndY;

            if (primitive->ClipZHigh())
                {
                clipMask = clipMask | ClipMask::ZHigh;
                zFront = primitive->GetZHigh();
                }

            if (primitive->ClipZLow())
                {
                clipMask = clipMask | ClipMask::ZLow;
                zBack = primitive->GetZLow();
                }

            pointCount += clipPolygon->size();

            if (pointCount > nPoints)
                break;

            memcpy (loopPoints[*nLoops], &clipPolygon->front(), clipPolygon->size() * sizeof (DPoint2d));
            deltaTrans.Multiply (loopPoints[*nLoops], loopPoints[*nLoops], (int) clipPolygon->size());
            nLoopPoints[*nLoops] = (int) clipPolygon->size();
            (*nLoops) += 1;
            }
        }

    if (NULL != clipMaskP)
        *clipMaskP = clipMask;

    if (NULL != zFrontP)
        *zFrontP = zFront;

    if (NULL != zBackP)
        *zBackP = zBack;

    if (NULL != transformP)
        front()->GetTransforms (transformP, NULL);
    }

/*---------------------------------------------------------------------------------**//**                                                                                     
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipVector::SetInvisible (bool invisible)
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->SetInvisible (invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClipVector::ParseClipPlanes ()
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->ParseClipPlanes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ClipVector::ApplyCameraToPlanes (double focalLength)
    {
    for (ClipPrimitivePtr& primitive: *this)
        if (SUCCESS != primitive->ApplyCameraToPlanes(focalLength))
            return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::Create () { return new ClipVector(); }
ClipVectorPtr ClipVector::CreateFromPrimitive (ClipPrimitivePtr primitive) { return new ClipVector (primitive); }
ClipVectorPtr ClipVector::CreateFromGPA (GPArrayCR gpa, double chordTolerance, double angleTolerance,  double* zLow, double* zHigh, TransformCP transform)  { return new ClipVector (gpa, chordTolerance, angleTolerance, zLow, zHigh, transform);  }













                                                                         
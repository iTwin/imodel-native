/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define TOLERANCE_ChordAngle            .1
#define TOLERANCE_ChordLen              1000

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::CreateFromCurveVector(CurveVectorCR curveVector, double chordTolerance, double angleTolerance, double* zLow, double* zHigh)
    {
    Transform       localToWorld, worldToLocal;;
    CurveVectorPtr  localCurveVector;
    DRange3d        range;

    localCurveVector = curveVector.CloneInLocalCoordinates(LOCAL_COORDINATE_SCALE_UnitAxesAtStart, localToWorld, worldToLocal, range);

    if (!localCurveVector.IsValid())
        {
        BeAssert(false);
        return ClipVectorPtr();
        }

    switch (localCurveVector->GetBoundaryType())
        {
        case CurveVector::BOUNDARY_TYPE_Outer:
            {
            ClipPrimitivePtr clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector(*localCurveVector, chordTolerance, angleTolerance, zLow, zHigh, &localToWorld);
            if (clipPrimitive.IsValid())
                return new ClipVector(clipPrimitive.get());
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            ClipVectorPtr   clipVector = new ClipVector();
            for (ICurvePrimitivePtr const& loop: *localCurveVector)
                {
                ClipPrimitivePtr    clipPrimitive;
                CurveVectorCP       loopCurves;

                if (NULL == (loopCurves = loop->GetChildCurveVectorCP()) ||
                    ! (clipPrimitive = ClipPrimitive::CreateFromBoundaryCurveVector(*loopCurves, chordTolerance, angleTolerance, zLow, zHigh, &localToWorld)).IsValid())
                    {
                    BeAssert(false);
                    continue;
                    }
                clipVector->push_back(clipPrimitive);
                }
            if (!clipVector->empty())
                return clipVector;

            break;
            }
        }
    BeAssert(false);
    return nullptr; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::CreateCopy(ClipVectorCR inputVector)
    {
    ClipVectorP clipVector = new ClipVector();
    clipVector->m_boundingRange = inputVector.m_boundingRange;
    clipVector->AppendCopy(inputVector);
    return clipVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::Clone(TransformCP trans) const
    {
    ClipVectorPtr clone = CreateCopy(*this);
    if (trans)
        clone->TransformInPlace(*trans);
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::AppendCopy(ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back(ClipPrimitive::CreateCopy(*primitive));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::Append(ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back(primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClipVector::AppendPlanes(ClipVectorPtr& clip, ClipPlaneSetCR planes, bool invisible)
    {
    if (!clip.IsValid())
        clip = new ClipVector();

    clip->push_back(ClipPrimitive::CreateFromClipPlanes(planes, invisible));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClipVector::AppendShape(ClipVectorPtr& clip, DPoint2dCP points, size_t nPoints, bool outside, double const* zLow, double const* zHigh, TransformCP transform, bool invisible)
    {
    ClipPrimitivePtr    clipPrimitive = ClipPrimitive::CreateFromShape(points, nPoints, outside, zLow, zHigh, transform, invisible);

    if (!clip.IsValid())
        clip = new ClipVector();

    clip->push_back(clipPrimitive);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::GetRange(DRange3dR range, TransformCP pTransform) const
    {
    range.Init();

    for (ClipPrimitivePtr const& primitive: *this)
        {
        DRange3d        thisRange;

        if (primitive->GetRange(thisRange, pTransform))
            {
            if (range.IsEmpty())
                range = thisRange;
            else
                range.IntersectionOf(range, thisRange);
            }
        }

    if (!m_boundingRange.IsNull())
        range.IntersectionOf(m_boundingRange, range);

    return !range.IsEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::PointInside(DPoint3dCR point, double onTolerance) const
    {
    if (!m_boundingRange.IsNull() && !m_boundingRange.IsContained(point))
        return false;

    for (ClipPrimitivePtr const& primitive: *this)
        if (!primitive->PointInside(point, onTolerance))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClipVector::TransformInPlace(TransformCR transform)
    {
    for (ClipPrimitivePtr& primitive: *this)
        if (SUCCESS != primitive->TransformInPlace(transform))
            return ERROR;

    if (!m_boundingRange.IsNull())
        transform.Multiply(m_boundingRange, m_boundingRange);

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
            deltaTrans.InitIdentity();
            }
        else
            {
            Transform   fwdTrans, invTrans;

            primitive->GetTransforms(&fwdTrans, NULL);
            front()->GetTransforms(NULL, &invTrans);
            deltaTrans.InitProduct(invTrans, fwdTrans);
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

            memcpy(loopPoints[*nLoops], &clipPolygon->front(), clipPolygon->size() * sizeof (DPoint2d));
            deltaTrans.Multiply(loopPoints[*nLoops], loopPoints[*nLoops], (int) clipPolygon->size());
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
        front()->GetTransforms(transformP, NULL);
    }

/*---------------------------------------------------------------------------------**//**                                                                                     
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::SetInvisible(bool invisible)
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->SetInvisible(invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::ParseClipPlanes()
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->ParseClipPlanes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClipVector::MultiplyPlanesTimesMatrix(DMatrix4dCR matrix)
    {
    int numErrors = 0;
    for (ClipPrimitivePtr& primitive: *this)
        if (SUCCESS != primitive->MultiplyPlanesTimesMatrix(matrix))
            numErrors++;
    return numErrors == 0 ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipVector::ClassifyPointContainment(DPoint3dCP points, size_t nPoints, bool ignoreMasks) const
    {
    ClipPlaneContainment        currentContainment = ClipPlaneContainment_Ambiguous;

    for (ClipPrimitivePtr const& primitive: *this)
        {
        ClipPlaneContainment    thisContainment = primitive->ClassifyPointContainment(points, nPoints, ignoreMasks);

        if (ClipPlaneContainment_Ambiguous == thisContainment)
            return ClipPlaneContainment_Ambiguous;

        if (ClipPlaneContainment_Ambiguous == currentContainment)
            currentContainment = thisContainment;
        else if (currentContainment != thisContainment)
            return ClipPlaneContainment_Ambiguous;
        }
    return currentContainment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipVector::ClassifyRangeContainment(DRange3dCR range, bool ignoreMasks) const
    {
    DPoint3d    corners[8];
    range.Get8Corners(corners);
    return ClassifyPointContainment(corners, 8, ignoreMasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsAnyLineStringPointInside(DPoint3dCP points, size_t n, bool closed)
    {
    DSegment3d segment;
    for (ClipPrimitivePtr const& primitive : *this)
        {
        auto clipPlaneSet = primitive->GetClipPlanes();
        for (size_t i = 0; i + 1 < n; i++)
            {
            segment.Init(points[i], points[i+1]);
            if (clipPlaneSet->IsAnyPointInOrOn(segment))
                return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double SumSizes(bvector<DSegment1d> &intervals, size_t iBegin, size_t iEnd)
    {
    double s = 0.0;
    for (size_t i = iBegin; i < iEnd; i++)
        s += intervals[i].Delta();
    return s;
    }

#define TARGET_FRACTION_SUM (0.99999999)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsCompletelyContained(DPoint3dCP points, size_t n, bool closed)
    {
    DSegment3d segment;
    for (size_t i = 0; i + 1 < n; i++)
        {
        segment.Init(points[i], points[i + 1]);
        m_clipIntervals.clear();
        double fractionSum = 0.0;
        size_t index0 = 0;

        for (ClipPrimitivePtr const& primitive : *this)
            {
            auto clipPlaneSet = primitive->GetClipPlanes();
            clipPlaneSet->AppendIntervals(segment, m_clipIntervals);
            size_t index1 = m_clipIntervals.size();
            fractionSum += SumSizes(m_clipIntervals, index0, index1);
            index0 = index1;
            // ASSUME primitives are non-overlapping ...
            if (fractionSum >= TARGET_FRACTION_SUM)
                break;
            }
        if (fractionSum < TARGET_FRACTION_SUM)
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsAnyPointInside(DEllipse3dCR arc, bool closed)
    {
    for (ClipPrimitivePtr const& primitive : *this)
        {
        auto clipPlaneSet = primitive->GetClipPlanes();
        if (clipPlaneSet->IsAnyPointInOrOn(arc))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsCompletelyContained(DEllipse3dCR arc, bool closed)
    {
    m_clipIntervals.clear();
    double fractionSum = 0.0;
    size_t index0 = 0;

    for (ClipPrimitivePtr const& primitive : *this)
        {
        auto clipPlaneSet = primitive->GetClipPlanes();
        clipPlaneSet->AppendIntervals(arc, m_clipIntervals);
        size_t index1 = m_clipIntervals.size();
        fractionSum += SumSizes(m_clipIntervals, index0, index1);
        index0 = index1;
        // ASSUME primitives are non-overlapping ...
        if (fractionSum >= TARGET_FRACTION_SUM)
            break;
        }
    return fractionSum >= TARGET_FRACTION_SUM;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsAnyPointInside(MSBsplineCurveCR arc)
    {
    for (ClipPrimitivePtr const& primitive : *this)
        {
        auto clipPlaneSet = primitive->GetClipPlanes();
        if (clipPlaneSet->IsAnyPointInOrOn(arc))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipVector::IsCompletelyContained(MSBsplineCurveCR arc)
    {
    m_clipIntervals.clear();
    double fractionSum = 0.0;
    size_t index0 = 0;

    for (ClipPrimitivePtr const& primitive : *this)
        {
        auto clipPlaneSet = primitive->GetClipPlanes();
        clipPlaneSet->AppendIntervals(arc, m_clipIntervals);
        size_t index1 = m_clipIntervals.size();
        fractionSum += SumSizes(m_clipIntervals, index0, index1);
        index0 = index1;
        // ASSUME primitives are non-overlapping ...
        if (fractionSum >= TARGET_FRACTION_SUM)
            break;
        }
    return fractionSum >= TARGET_FRACTION_SUM;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ClipVector::ToJson() const
    {
    Json::Value val = Json::arrayValue;
    for (auto clipPrimitive : *this)
        val.append(clipPrimitive->_ToJson());

    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::FromJson(JsonValueCR json) 
    {
    if (json.isNull())
        return nullptr;

    ClipVectorPtr clip = new ClipVector();
    for (Json::ArrayIndex i = 0; i<json.size(); ++i)
        {
        auto primitive = ClipPrimitive::FromJson(json[i]);
        if (primitive.IsValid())
            clip->push_back(primitive);
        }

    return clip->empty() ? nullptr : clip;
    }

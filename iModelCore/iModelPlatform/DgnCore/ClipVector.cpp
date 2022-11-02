/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define TOLERANCE_ChordAngle            .1
#define TOLERANCE_ChordLen              1000

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::CreateCopy(ClipVectorCR inputVector)
    {
    ClipVectorP clipVector = new ClipVector();
    clipVector->m_boundingRange = inputVector.m_boundingRange;
    clipVector->AppendCopy(inputVector);
    return clipVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::Clone(TransformCP trans) const
    {
    ClipVectorPtr clone = CreateCopy(*this);
    if (trans)
        clone->TransformInPlace(*trans);
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::AppendCopy(ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back(ClipPrimitive::CreateCopy(*primitive));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::Append(ClipVectorCR clip)
    {
    for (ClipPrimitivePtr const& primitive: clip)
        push_back(primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ClipVector::AppendPlanes(ClipVectorPtr& clip, ClipPlaneSetCR planes, bool invisible)
    {
    if (!clip.IsValid())
        clip = new ClipVector();

    clip->push_back(ClipPrimitive::CreateFromClipPlanes(planes, invisible));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::SetInvisible(bool invisible)
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->SetInvisible(invisible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::ParseClipPlanes()
    {
    for (ClipPrimitivePtr& primitive: *this)
        primitive->ParseClipPlanes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipVector::ClassifyRangeContainment(DRange3dCR range, bool ignoreMasks) const
    {
    DPoint3d    corners[8];
    range.Get8Corners(corners);
    return ClassifyPointContainment(corners, 8, ignoreMasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipVector::ToJson(BeJsValue val) const
    {
    val.SetEmptyArray();
    for (auto clipPrimitive : *this)
        clipPrimitive->_ToJson(val.appendValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::FromJson(BeJsConst json)
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

BEGIN_UNNAMED_NAMESPACE

//=======================================================================================
// @bsistruct
//=======================================================================================
struct ClipVectorParser
{
private:
    ClipVectorPtr m_clip;
    Utf8CP m_cur;
    Utf8CP m_end;

    ClipVectorParser(Utf8StringCR input) : m_clip(ClipVector::Create()), m_cur(input.c_str()), m_end(m_cur + input.length())
        {
        if (!Parse())
            m_clip = nullptr;
        }

    bool IsPastTheEnd() const
        {
        return m_cur >= m_end;
        }

    Utf8Char CurChar(bool consume = false)
        {
        if (IsPastTheEnd())
            return 0;

        auto ch = *m_cur;
        if (consume)
            ++m_cur;

        return ch;
        }

    bool ParseDouble(double& output)
        {
        if (IsPastTheEnd())
            return false;

        if (1 != Utf8String::Sscanf_safe(m_cur, "%lf_", &output))
            return false;

        while (!IsPastTheEnd() && '_' != CurChar(true))
            {
            //
            }

        return !IsPastTheEnd();
        }

    bool EndArray()
        {
        return '_' == CurChar(true);
        }

    bool ParsePlane(ClipPlaneR plane)
        {
        auto flags = CurChar(true);
        if (flags < '0' || flags > '3')
            return false;

        flags = flags - '0';
        auto invisible = 0 != (flags & 1);
        auto interior = 0 != (flags & 2);

        DVec3d normal;
        double distance;
        if (!ParseDouble(normal.x) || !ParseDouble(normal.y) || !ParseDouble(normal.z) || !ParseDouble(distance))
            return false;

        plane = ClipPlane(normal, distance, invisible, interior);
        return true;
        }

    bool ParseConvexClipPlaneSet(ConvexClipPlaneSetR set)
        {
        while (!IsPastTheEnd() && '_' != CurChar())
            {
            ClipPlane plane;
            if (!ParsePlane(plane))
                return false;

            set.push_back(plane);
            }

        // An array - even if empty - is terminated by an underscore.
        if (set.empty() && !EndArray())
            return false;

        return EndArray();
        }

    ClipPrimitivePtr ParsePrimitive()
        {
        ClipPlaneSet planeSet;
        bool invisible = false;
        switch (CurChar(true))
            {
            case '1':
                invisible = true;
                break;
            case '0': break;
            default: return nullptr;
            }

        while (!IsPastTheEnd() && '_' != CurChar())
            {
            ConvexClipPlaneSet convexSet;
            if (!ParseConvexClipPlaneSet(convexSet))
                return nullptr;

            planeSet.push_back(convexSet);
            }

        // An array - even if empty - is terminated by an underscore.
        if (planeSet.empty() && !EndArray())
            return nullptr;

        return EndArray() ? ClipPrimitive::CreateFromClipPlanes(planeSet, invisible) : nullptr;
        }

    bool Parse()
        {
        while (!IsPastTheEnd() && '_' != CurChar())
            {
            auto primitive = ParsePrimitive();
            if (primitive.IsNull())
                return false;

            m_clip->push_back(primitive);
            }

        return EndArray() && IsPastTheEnd();
        }
public:
    static ClipVectorPtr Parse(Utf8StringCR input)
        {
        ClipVectorParser parser(input);
        return parser.m_clip;
        }
};

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ClipVector::FromCompactString(Utf8StringCR input)
    {
    return ClipVectorParser::Parse(input);
    }


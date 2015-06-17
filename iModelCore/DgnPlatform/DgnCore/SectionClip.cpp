/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SectionClip.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DgnPlatformInternal.h"
#include    <DgnPlatform/DgnCore/SectionClip.h>

namespace {

/*=================================================================================**//**
* Section View Clip Object (used by section and elevation views and clip volume by plane)
* @bsiclass                                                     SunandSandurkar  08/2007
+===============+===============+===============+===============+===============+======*/
struct          SectionClipObject : IViewClipObject
{
friend struct Dgn::SectionClipObjectFactory;
friend struct Dgn::SectionClipObjectLegacyData;

    DEFINE_BENTLEY_NEW_DELETE_OPERATORS 

private:

    bvector<DPoint3d> m_points;

    struct ClipData
        {
        struct Params
            {
            uint32_t cropMask:6;
            uint32_t preserveUp:1;
            uint32_t reserved:25;
            } params;

        double      topHeight;
        double      bottomHeight;
        double      frontDepth;
        double      backDepth;

        RotMatrix   rotMatrix;

        } m_clipData;

private: 
StatusInt    GetSidePlane 
(
DPlane3dR               cutPlane, 
DVec3dR                 xDirection,
DVec3dR                 yDirection,
ClipMask&               clipMask,
DRange2dR               clipRange,
size_t                  index,
ViewContextR            viewContext
) const;

void   SetDrawSymbology
(
ViewContextR    context,
ColorDef     color,
uint32_t        weight,
int32_t         style
);

bool IsDraw3D (ViewContextR context, DVec3dCR zVec);

StatusInt GetClipBoundaryByShape (ClipVectorPtr& clip, /*DgnModelP target, DgnViewportP vp*/DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const;
#if defined (NEEDS_WORK_DGNITEM)
virtual StatusInt _GetClipBoundary (ClipVectorPtr& clip, /*DgnModelP target, DgnViewportP vp*/DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const override; // added in graphite
virtual bool      _IsClipVolumePassValid (ClipVolumePass pass) const override;
virtual StatusInt _GetCuttingPlane (DPlane3dR cutPlane, DVec3dR xDirection, DVec3dR yDirection, ClipMask& clipMask, DRange2dR clipRange, bool& forward, int index, ViewContextR context) const override;
virtual bool      _GetAuxTransform (TransformR transform, ClipVolumePass pass) const override;
virtual StatusInt _GetTransform (TransformR trans) const override;
virtual size_t    _GetPrimaryCutPlaneCount() const override;
#endif
virtual StatusInt _ApplyTransform (TransformCR) override;

virtual void _Draw (ViewContextR) override;

virtual Utf8String _GetFactoryId() const override;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _FromJson (JsonValueCR json)
    {
    // WIP_SECTION_CLIP -- keep consistent with ForeignFormat::DgnV8::SectionSectionObject::ToJson
    m_clipData.params.cropMask = json["crop_mask"].asUInt();
    m_clipData.params.preserveUp = json["preserve_up"].asBool();

    m_clipData.topHeight = json["top_height"].asDouble();
    m_clipData.bottomHeight = json["bottom_height"].asDouble();
    m_clipData.frontDepth = json["front_depth"].asDouble();
    m_clipData.backDepth = json["back_depth"].asDouble();

    JsonUtils::RotMatrixFromJson (m_clipData.rotMatrix, json["rot_matrix"]);
    
    JsonUtils::DPoint3dVectorFromJson (m_points, json["points"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _ToJson (JsonValueR json) const
    {
    json["crop_mask"] = m_clipData.params.cropMask;
    json["preserve_up"] = TO_BOOL(m_clipData.params.preserveUp);

    json["top_height"] = m_clipData.topHeight;
    json["bottom_height"] = m_clipData.bottomHeight;
    json["front_depth"] = m_clipData.frontDepth;
    json["back_depth"] = m_clipData.backDepth;

    JsonUtils::RotMatrixToJson (json["rot_matrix"], m_clipData.rotMatrix);
    
    JsonUtils::DPoint3dVectorToJson (json["points"], m_points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void FromLegacyData (SectionClipObjectLegacyData const& data)
    {
    m_clipData.params.cropMask = data.m_clipData.params.cropMask; // WIP_SECTION_CLIP - assume cropmask values are same in Vancouver and DgnDb
    m_clipData.params.preserveUp = data.m_clipData.params.preserveUp;
    m_clipData.topHeight = data.m_clipData.topHeight;
    m_clipData.bottomHeight = data.m_clipData.bottomHeight;
    m_clipData.frontDepth = data.m_clipData.frontDepth;
    m_clipData.backDepth = data.m_clipData.backDepth;
    m_clipData.rotMatrix = data.m_clipData.rotMatrix;
    m_points = data.m_points;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
private: StatusInt        ValidateSectionClipObject ()  const
    {
    if (m_points.size() < 2)       // TR#  311329. Avoid Null pointer deref.
        return SUCCESS;

    // We store rotation matrix and clip points as data in clip element. Here we are validating if 
    // SectionclipObject is well formed. We compare the vector of two clip points with the xvec of rotation
    // matrix. If those are not parallel to each other, we assert.
    DVec3d  xVec, clipPointsVec;
    clipPointsVec.NormalizedDifference (m_points[0], m_points[1]);

    RotMatrix rMatrix = GetRotationMatrix ();
    rMatrix.GetColumn (xVec, 0);

    // For the geometry that is far from origin and small in size, IsParallelTo (*()) check was not sufficient.
    // Use angle between the vector and compare it with a tolerance of 1 e-8
    double  tolerance = 0.00000001; 
    if (clipPointsVec.SmallerUnorientedAngleTo(xVec) > tolerance)
        {
        BeAssert (false && "Section clip object is not well formed");
        return ERROR;
        }
    return SUCCESS;
    }
    
public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
SectionClipObject ()
    {
    memset (&m_clipData, 0, sizeof (m_clipData));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
~SectionClipObject ()
    {
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrixCR             _GetRotationMatrix () const
    {
    return m_clipData.rotMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _SetRotationMatrix (RotMatrixCR rMatrix)
    {
    m_clipData.rotMatrix = rMatrix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
double                  _GetSize (ClipVolumeSizeProp prop) const
    {
    switch (prop)
        {
        case CLIPVOLUME_SIZE_TopHeight:
            return m_clipData.topHeight;
        case CLIPVOLUME_SIZE_BottomHeight:
            return m_clipData.bottomHeight;
        case CLIPVOLUME_SIZE_FrontDepth:
            return m_clipData.frontDepth;
        case CLIPVOLUME_SIZE_BackDepth:
            return m_clipData.backDepth;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _SetSize (ClipVolumeSizeProp prop, double size)
    {
    switch (prop)
        {
        case CLIPVOLUME_SIZE_TopHeight:
            m_clipData.topHeight = size;
            break;
        case CLIPVOLUME_SIZE_BottomHeight:
            m_clipData.bottomHeight = size;
            break;
        case CLIPVOLUME_SIZE_FrontDepth:
            m_clipData.frontDepth = size;
            break;
        case CLIPVOLUME_SIZE_BackDepth:
            m_clipData.backDepth = size;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    _GetCrop (ClipVolumeCropProp prop) const
    {
    switch (prop)
        {
        case CLIPVOLUME_CROP_Top:
        case CLIPVOLUME_CROP_Bottom:
        case CLIPVOLUME_CROP_Front:
        case CLIPVOLUME_CROP_Back:
        case CLIPVOLUME_CROP_StartSide:
        case CLIPVOLUME_CROP_EndSide:
            return 0 != (m_clipData.params.cropMask & (0x0001 << prop));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _SetCrop (ClipVolumeCropProp prop, bool crop)
    {
    switch (prop)
        {
        case CLIPVOLUME_CROP_Top:
        case CLIPVOLUME_CROP_Bottom:
        case CLIPVOLUME_CROP_Front:
        case CLIPVOLUME_CROP_Back:
        case CLIPVOLUME_CROP_StartSide:
        case CLIPVOLUME_CROP_EndSide:
            {
            int cropBit = 1 << prop;
            if (crop)
                m_clipData.params.cropMask = (m_clipData.params.cropMask | cropBit);
            else
                m_clipData.params.cropMask = (m_clipData.params.cropMask & ~cropBit);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _CopyCrops (IViewClipObject const* from)
    {
    if (!from)
        return;

    SetCrop (CLIPVOLUME_CROP_Top,       from->GetCrop (CLIPVOLUME_CROP_Top));
    SetCrop (CLIPVOLUME_CROP_Bottom,    from->GetCrop (CLIPVOLUME_CROP_Bottom));
    SetCrop (CLIPVOLUME_CROP_Front,     from->GetCrop (CLIPVOLUME_CROP_Front));
    SetCrop (CLIPVOLUME_CROP_Back,      from->GetCrop (CLIPVOLUME_CROP_Back));
    SetCrop (CLIPVOLUME_CROP_StartSide, from->GetCrop (CLIPVOLUME_CROP_StartSide));
    SetCrop (CLIPVOLUME_CROP_EndSide,   from->GetCrop (CLIPVOLUME_CROP_EndSide));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                     _GetNumPoints () const
    {
    return m_points.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               _GetPoints (DPoint3dVector& points, size_t iFromPoint, size_t numPoints) const
    {
    if (static_cast <int> (iFromPoint + numPoints) > m_points.size())
        return ERROR;

    points.insert (points.begin (), m_points.begin() + iFromPoint, m_points.begin() + iFromPoint + numPoints);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _SetPoints (size_t numPoints, DPoint3dCP points)
    {
    // NEEDSWORK: Verify that the points meet the following requirements
    // Cannot be closed.
    // Should be convex.
    // Segments should be orthogonal.

    m_points.assign (points, points+numPoints);
    }

public: 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double  _GetWidth () const override
    {
    DVec3d startSegVector, StartEndScalar;
    startSegVector.NormalizedDifference (m_points[1], m_points[0]);
    StartEndScalar.DifferenceOf (m_points[m_points.size()-1], m_points[0]);
    return startSegVector.DotProduct (StartEndScalar);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mukesh.Pant                     05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _SetWidth (double newWidth) override
    {
    DVec3d startSegVector;
    startSegVector.NormalizedDifference (m_points[1], m_points[0]);

    double original = GetWidth ();
    double change = newWidth - original;

    startSegVector.Scale (change);

    DPoint3d        testPoint = m_points [m_points.size()-1];
    testPoint.Add (startSegVector);

    double lastSegDistance    = m_points[m_points.size()-2].Distance (m_points[m_points.size()-1]);
    double newLastSegDistance = testPoint.Distance (m_points[m_points.size()-1]);
    if (change < 0.0  && newLastSegDistance > lastSegDistance)
        return;

    // modify point 
    m_points [m_points.size()-1] = testPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void                    _SetPreserveUp (bool flag)
    {
    m_clipData.params.preserveUp = flag;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    _GetPreserveUp () const
    {
    return m_clipData.params.preserveUp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool             AreClipDataEqual (void const* data1, int numBytes1, void const* data2, int numBytes2, double distanceTolerance, double  directionTolerance)
    {
    if ((numBytes1 != numBytes2) || (numBytes1 != sizeof(ClipData)))
        return false;

    ClipData* clipData1 = (ClipData*) data1;
    ClipData* clipData2 = (ClipData*) data2;

    if (0 != memcmp (&clipData1->params, &clipData2->params, sizeof (ClipData::Params)))
        return false;

    if (fabs (clipData1->topHeight      - clipData2->topHeight)     > distanceTolerance ||
        fabs (clipData1->bottomHeight   - clipData2->bottomHeight)  > distanceTolerance ||
        fabs (clipData1->frontDepth     - clipData2->frontDepth)    > distanceTolerance ||
        fabs (clipData1->backDepth      - clipData2->backDepth)     > distanceTolerance)
        return false;

    if (!clipData1->rotMatrix.IsEqual (clipData2->rotMatrix, directionTolerance))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool             AreClipPointsEqual (void const* data1, int numBytes1, void const* data2, int numBytes2, double distanceTolerance, double  directionTolerance)
    {
    if (numBytes1 != numBytes2)
        return false;

    int         numPts  = numBytes1 / sizeof (DPoint3d);
    DPoint3dCP  pts1    = (DPoint3dCP) data1;
    DPoint3dCP  pts2    = (DPoint3dCP) data2;

    for (int ipt = 0; ipt < numPts; ipt++)
        {
        if (!pts1[ipt].IsEqual (pts2[ipt], distanceTolerance))
            return false;
        }

    return true;
    }
}; // SectionClipObject

} // anonymous namespace


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar  08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionClipObject::SetDrawSymbology (ViewContextR context, ColorDef color, uint32_t weight, int32_t style)
    {
    DgnViewportP      viewport    = context.GetViewport ();
    if (!viewport)
        return; // ex. RangeContext

    ElemMatSymbP    currElemSymb = context.GetElemMatSymb ();
    ColorDef    adjustedColor = viewport->AdjustColorForContrast(color, viewport->GetBackgroundColor ());

    currElemSymb->SetLineColor (adjustedColor);
    context.SetIndexedLineWidth (*currElemSymb, weight);
    context.SetIndexedLinePattern (*currElemSymb, style);
    context.GetIDrawGeom().ActivateMatSymb (currElemSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar  08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool SectionClipObject::IsDraw3D (ViewContextR context, DVec3dCR zVec)
    {
    DgnViewportP      viewport    = context.GetViewport ();
    if (!viewport)
        return true; // ex. RangeContext

    RotMatrix       viewportRot = viewport->GetRotMatrix ();
    DVec3d          viewportZVec;
    viewportRot.GetColumn (viewportZVec, 2);

    return !zVec.IsParallelTo (viewportZVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar  08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void   SectionClipObject::_Draw
(
ViewContextR        context
)
    {
    #ifdef WIP_SECTION_CLIP
    if (DrawPurpose::Plot == context.GetDrawPurpose () ||
        DrawPurpose::ExportVisibleEdges == context.GetDrawPurpose() ||
        (NULL != source.GetDgnModel() && NULL != source.GetDgnModel()->IsDgnAttachment()))   // TFS# 65435.  I'm not sure the reason for this test - but this is in SS3.
        return;
        */
    if (DrawPurpose::ChangedPre == context.GetDrawPurpose ())
        {
        // NOTE: xAttr aren't valid...only need to accumulate dirty region for erase, so just draw range...
        context.DrawElementRange (source.GetElementCP ());

        return;
        }
    #endif

    size_t             numPoints = this->GetNumPoints ();
    if (2 > numPoints)
        return;

    DPoint3dVector  points;
    this->GetPoints (points, 0, numPoints);

    DVec3d xVec, zVec;
    (this->GetRotationMatrix ()).GetColumn(xVec,  0);
    (this->GetRotationMatrix ()).GetColumn(zVec,  2);

    double topHeight = this->GetSize (CLIPVOLUME_SIZE_TopHeight);
    double bottomHeight = this->GetSize (CLIPVOLUME_SIZE_BottomHeight);

    double  pixelBasedLength = 20 * context.GetPixelSizeAtPoint (&points[0]);
    double  maxLength = 0.1 * points[0].Distance (points[1]);
    double  cornerEdgeLen = MIN (pixelBasedLength, maxLength);

    // Draw linestring
    IViewDrawR      output   = context.GetIViewDraw ();

    DVec3d   segVec;
    DPoint3d cornerPts[2];
    DPoint3d shape[5];
    for (size_t iseg = 0; iseg < numPoints-1; iseg++)
        {
        // Draw shape for every plane
        shape[0].SumOf (points[iseg],zVec, bottomHeight);
        shape[1].SumOf (points[iseg+1],zVec, bottomHeight);
        shape[2].SumOf (points[iseg+1],zVec, topHeight);
        shape[3].SumOf (points[iseg],zVec, topHeight);
        shape[4] = shape[0];

        SetDrawSymbology (context, ColorDef::White(), 0, 4);
        output.DrawLineString3d (5, shape, NULL);

        // Draw fat corners
        if (!IsDraw3D (context, zVec))
            {
            for (size_t iseg = 0; iseg < numPoints-1; iseg++)
                {
                segVec.NormalizedDifference (points[iseg+1], points[iseg]);
                SetDrawSymbology (context, ColorDef::White(), 2, 0);

                cornerPts[0] = points[iseg];
                cornerPts[1].SumOf (cornerPts[0],segVec, cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);

                cornerPts[0] = points[iseg+1];
                cornerPts[1].SumOf (cornerPts[0],segVec, -cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);
                }
            }
        else
            {
            segVec.NormalizedDifference (points[iseg+1], points[iseg]);
            SetDrawSymbology (context, ColorDef::White(), 2, 0);

            cornerPts[0] = shape[0];
            cornerPts[1].SumOf (cornerPts[0],segVec, cornerEdgeLen);
            output.DrawLineString3d (2, cornerPts, NULL);

            cornerPts[0] = shape[1];
            cornerPts[1].SumOf (cornerPts[0],segVec, -cornerEdgeLen);
            output.DrawLineString3d (2, cornerPts, NULL);

            cornerPts[0] = shape[2];
            cornerPts[1].SumOf (cornerPts[0],segVec, -cornerEdgeLen);
            output.DrawLineString3d (2, cornerPts, NULL);

            cornerPts[0] = shape[3];
            cornerPts[1].SumOf (cornerPts[0],segVec, cornerEdgeLen);
            output.DrawLineString3d (2, cornerPts, NULL);

            if (iseg == 0)
                {
                cornerPts[0] = shape[0];
                cornerPts[1].SumOf (cornerPts[0],zVec, cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);

                cornerPts[0] = shape[3];
                cornerPts[1].SumOf (cornerPts[0],zVec, -cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);
                }

            if (iseg == numPoints-2)
                {
                cornerPts[0] = shape[1];
                cornerPts[1].SumOf (cornerPts[0],zVec, cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);

                cornerPts[0] = shape[2];
                cornerPts[1].SumOf (cornerPts[0],zVec, -cornerEdgeLen);
                output.DrawLineString3d (2, cornerPts, NULL);
                }
            }
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    SunandSandurkar 07/07
//+---------------+---------------+---------------+---------------+---------------+------*/
void transformDistance (double& dist, TransformCR trans, RotMatrixCR rMatrix, int axis)
    {
    DVec3d vec;
    rMatrix.GetColumn(vec,  axis);

    trans.MultiplyMatrixOnly (vec);

    dist = dist * vec.Magnitude ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SectionClipObject::_ApplyTransform (TransformCR transform)
    {
    // Transform points
    DPoint3dVector points;
    if (SUCCESS != this->GetPoints (points, 0, this->GetNumPoints ()))
        return ERROR;

    bsiTransform_multiplyDPoint3dArrayInPlace (&transform, &points[0], (int) this->GetNumPoints ());
    this->SetPoints (this->GetNumPoints (), &points[0]);

    // Transform distances
    RotMatrix rMatrix = this->GetRotationMatrix ();
    double topHeight    = this->GetSize (CLIPVOLUME_SIZE_TopHeight);
    double bottomHeight = this->GetSize (CLIPVOLUME_SIZE_BottomHeight);
    double frontDepth   = this->GetSize (CLIPVOLUME_SIZE_FrontDepth);
    double backDepth    = this->GetSize (CLIPVOLUME_SIZE_BackDepth);

    bool   topHeightNeg = topHeight < 0.0, bottomHeightNeg = bottomHeight < 0.0, frontDepthNeg = frontDepth < 0.0, backDepthNeg = backDepth < 0.0;

    transformDistance (topHeight,    transform, rMatrix, 2);
    transformDistance (bottomHeight, transform, rMatrix, 2);
    transformDistance (frontDepth,   transform, rMatrix, 1);
    transformDistance (backDepth,    transform, rMatrix, 1);

    this->SetSize (CLIPVOLUME_SIZE_TopHeight, fabs (topHeight) * (topHeightNeg ? -1.0 : 1.0));
    this->SetSize (CLIPVOLUME_SIZE_BottomHeight,  fabs (bottomHeight) * (bottomHeightNeg ? -1.0 : 1.0));
    this->SetSize (CLIPVOLUME_SIZE_FrontDepth, fabs (frontDepth) * (frontDepthNeg ? -1.0 : 1.0));
    this->SetSize (CLIPVOLUME_SIZE_BackDepth, fabs (backDepth) * (backDepthNeg ? -1.0 : 1.0));

    // Transform rotMatrix
    bsiRotMatrix_multiplyTransformRotMatrix (&rMatrix, &transform, &rMatrix);
    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);
    this->SetRotationMatrix (rMatrix);

    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB                             6/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pushClipBoundaryLoops
(
ClipVectorPtr&      clip,
int                 nLoops,
int const*          nLoopPoints,
DPoint2dCP const*   loopPoints,
double              zLow,
double              zHigh,
TransformCR         transform,
ClipMask            clipMask,
bool                outside = false
)
    {
     for (int loop=nLoops-1; loop>=0; loop--)
        if (0 != nLoopPoints[loop])
            ClipVector::AppendShape (clip, loopPoints[loop], nLoopPoints[loop], 
                                    (outside == (0 == loop)), 
                                    (ClipMask::None != (clipMask & ClipMask::ZLow))  ? &zLow  : NULL, 
                                    (ClipMask::None != (clipMask & ClipMask::ZHigh)) ? &zHigh : NULL, 
                                     &transform);
    }

enum 
    {
    MAX_REFBOUNDS                   = 200,                         /* maximum bounds + voids */
    MAX_REFCLIPPNTS                 = 2500,                        /* maximum reference clip points */
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SectionClipObject::GetClipBoundaryByShape (ClipVectorPtr& clip, /*DgnModelP target, DgnViewportP vp*/DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const
    {
    int             nLoops;
    int             nLoopPoints[MAX_REFBOUNDS];
    DPoint2dP       loopPoints[MAX_REFBOUNDS];
    DPoint2d        pointBuffer[MAX_REFCLIPPNTS];
    ClipMask        clipMask;
    double          zFront, zBack;
    Transform       transform, inverse;
    DRange3d        transformedRange;

    DPoint3dVector  pts;
    this->GetPoints (pts, 0, 1);
    
    DPoint3d        origin = pts[0];

    DVec3d          scale;
    RotMatrix       rotMatrix;

    // A non-orthornormal matrix causes all kinds of problemos.
    rotMatrix.NormalizeColumnsOf (*(&this->GetRotationMatrix ()),scale);

    clipMask = ClipMask::All;

    if (!this->GetCrop (CLIPVOLUME_CROP_Bottom))
        clipMask = clipMask & ~ClipMask::ZLow;

    if (!this->GetCrop (CLIPVOLUME_CROP_Top))
        clipMask = clipMask & ~ClipMask::ZHigh;

    nLoops = 1;
    nLoopPoints[0] = 0;
    loopPoints[0] = pointBuffer;

    double          frontDepth          = this->GetSize (CLIPVOLUME_SIZE_FrontDepth);
    double          backDepth           = this->GetSize (CLIPVOLUME_SIZE_BackDepth);
    double          topHeight           = this->GetSize (CLIPVOLUME_SIZE_TopHeight);
    double          bottomHeight        = this->GetSize (CLIPVOLUME_SIZE_BottomHeight);
    bool            flipped             = frontDepth < 0.0;
    double          depthScale          = 1.0;
    bool            noBoundaryClipping  = (NULL != settings) && settings->ShouldIgnoreBoundaryClipping ();
    bool            noBoundaryClipping  = (NULL != settings) && settings->ShouldIgnoreBoundaryClipping ();

    if (flipped)
        {
        frontDepth = fabs (frontDepth);
        depthScale = -1.0;
        }

    if (pass == ClipVolumePass::Inside || pass == ClipVolumePass::Outside)
        {
        if (flipped)
            {
            zBack  = -1.0 * scale.z * frontDepth;
            zFront = scale.z * backDepth;
            }
        else
            {
            zBack  = -1.0 * scale.z * backDepth;
            zFront = scale.z * frontDepth;
            }

        if (!this->GetCrop (CLIPVOLUME_CROP_Back))
            clipMask = clipMask & ~ClipMask::ZLow;

        if (!this->GetCrop (CLIPVOLUME_CROP_Front))
            clipMask = clipMask & ~ClipMask::ZHigh;

        DVec3d xVec, zVec;
        rotMatrix.GetColumn (xVec, 0);
        rotMatrix.GetColumn (zVec, 2);
        zVec.Scale (-1.0);

        RotMatrix newMatrix;
        newMatrix.initFrom2Vectors (&xVec, &zVec);
        transform.InitFrom (newMatrix, origin);
        }
    else
        {
        zBack  = scale.z * bottomHeight;
        zFront = scale.z * topHeight;

        if (!this->GetCrop (CLIPVOLUME_CROP_Bottom))
            clipMask = clipMask & ~ClipMask::ZLow;

        if (!this->GetCrop (CLIPVOLUME_CROP_Top))
            clipMask = clipMask & ~ClipMask::ZHigh;

        transform.InitFrom (rotMatrix, origin);
        }

    //ClipUtil::GetMaxModelRange (maxRange, *target, vp, true, false); removed in graphite

    inverse.InverseOf(transform);
    inverse.Multiply(transformedRange, maxRange);
    
    double          rangeMargin = transformedRange.low.Distance (transformedRange.high) * .01;
    
    transformedRange.Extend (rangeMargin);
    double          xLow  = (this->GetCrop (CLIPVOLUME_CROP_StartSide) && !noBoundaryClipping) ? 0.0                                    : transformedRange.low.x;
    double          xHigh = (this->GetCrop (CLIPVOLUME_CROP_EndSide)   && !noBoundaryClipping) ? (scale.x * this->GetWidth ()) : transformedRange.high.x;

    if (pass == ClipVolumePass::Inside || pass == ClipVolumePass::Outside)
        {
        double          yLow  = (this->GetCrop (CLIPVOLUME_CROP_Bottom) && !noBoundaryClipping) ? (scale.z * fabs (bottomHeight)) : (transformedRange.low.y);
        double          yHigh = (this->GetCrop (CLIPVOLUME_CROP_Top)    && !noBoundaryClipping) ? (scale.z * -topHeight)          : (transformedRange.high.y);

        pointBuffer[nLoopPoints[0]++].SetComponents (xLow, yLow);
        pointBuffer[nLoopPoints[0]++].SetComponents (xHigh, yLow);
        pointBuffer[nLoopPoints[0]++].SetComponents (xHigh, yHigh);
        pointBuffer[nLoopPoints[0]++].SetComponents (xLow, yHigh);
        pointBuffer[nLoopPoints[0]++] = pointBuffer[0];

        pushClipBoundaryLoops (clip, nLoops, nLoopPoints, loopPoints, zBack, zFront, transform, clipMask, pass == ClipVolumePass::Outside);
        return SUCCESS;
        }

    pointBuffer[nLoopPoints[0]++].Zero ();
    size_t numPoints = this->GetNumPoints ();

    DPoint3dVector points;
    this->GetPoints (points, 0, numPoints);

    for (size_t index=0; index<numPoints; index++)
        {
        DPoint3d point = points[index];
        inverse.Multiply(point);
        pointBuffer[nLoopPoints[0]++].init (&point);
        }

    DPoint2d    firstPoint  = pointBuffer[0], lastPoint  = pointBuffer[nLoopPoints[0]-1];
    double      yLow        = (this->GetCrop (CLIPVOLUME_CROP_Back)      && !noBoundaryClipping) ? (-scale.y * backDepth * depthScale)    : (flipped ? transformedRange.high.y  : transformedRange.low.y);
    double      yHigh       = (this->GetCrop (CLIPVOLUME_CROP_Front)     && !noBoundaryClipping) ? ( scale.y * frontDepth * depthScale)   : (flipped ? transformedRange.low.y   : transformedRange.high.y);
    double      yLimit      = (ClipVolumePass::InsideForward == pass) ? yHigh : yLow;

    pointBuffer[nLoopPoints[0]++].SetComponents (xHigh, lastPoint.y);
    pointBuffer[nLoopPoints[0]++].SetComponents (xHigh, yLimit);
    pointBuffer[nLoopPoints[0]++].SetComponents (xLow,  yLimit);
    pointBuffer[nLoopPoints[0]++].SetComponents (xLow, firstPoint.x);
    pointBuffer[nLoopPoints[0]++] = firstPoint;

    pushClipBoundaryLoops (clip, nLoops, nLoopPoints, loopPoints, zBack, zFront, transform, clipMask, false);

    if (clip.IsValid() && !displayCutGeometry)
        clip->SetInvisible (true);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SectionClipObject::_GetClipBoundary (ClipVectorPtr& clip, /*DgnModelP target, DgnViewportP vp*/DRange3dR maxRange, ClipVolumePass pass, DynamicViewSettingsCP settings, bool displayCutGeometry) const
    {
    // Doglegs can't be defined with just planes (GetClipBoundaryByShape will be used.)
    if (2 != this->GetNumPoints () || pass == ClipVolumePass::Outside)
        return GetClipBoundaryByShape (clip, /*target, vp*/maxRange, pass, settings, displayCutGeometry);

    ConvexClipPlaneSet  convexSet;
    DMatrix3d           dMatrix;
    DVec3d              normal;

    DPoint3dVector      pts;
    this->GetPoints (pts, 0, 1);
    DPoint3d            origin = pts[0];

    dMatrix.initFromRotMatrix (&this->GetRotationMatrix ());

    double      frontDepth = this->GetSize (CLIPVOLUME_SIZE_FrontDepth);
    double      backDepth  = this->GetSize (CLIPVOLUME_SIZE_BackDepth);
    bool        noClipBoundary = (NULL != settings) && settings->ShouldIgnoreBoundaryClipping ();

    if (frontDepth < 0.0)
        {
        frontDepth = -frontDepth;
        dMatrix.column[1].Negate ();
        }

    switch (pass)
        {
        case ClipVolumePass::InsideBackward:
            normal.negate ((DVec3d *)&dMatrix.column[1]);
            convexSet.push_back (ClipPlane (normal, origin, !displayCutGeometry));
            break;

        case ClipVolumePass::InsideForward:
            convexSet.push_back (ClipPlane (dMatrix.column[1], origin, !displayCutGeometry));
            break;
        }

    // These are the front and back planes.   - We dont currently display cut geometry on these - so they are aways visible (dont test displayCutGeometry).
    if (this->GetCrop (CLIPVOLUME_CROP_Front) && pass != ClipVolumePass::InsideBackward)
        {
        normal.negate ((DVec3d *)&dMatrix.column[1]);

        convexSet.push_back  (ClipPlane (normal, normal.DotProduct (origin) - frontDepth, false /* Invisible*/));
        }

    if (this->GetCrop (CLIPVOLUME_CROP_Back) && pass != ClipVolumePass::InsideForward)
        {
        convexSet.push_back (ClipPlane (dMatrix.column[1], dMatrix.column[1].DotProduct (origin) - backDepth, false /* Invisible*/));
        }

    // These are the boundary clips.  
    bool        displayBoundaryCutPlanes = displayCutGeometry && (NULL == settings || !settings->ShouldIgnoreBoundaryCutPlanes());

    if (this->GetCrop (CLIPVOLUME_CROP_StartSide) && !noClipBoundary)
        {
        convexSet.push_back (ClipPlane (dMatrix.column[0], origin, !displayBoundaryCutPlanes));
        }

    if (this->GetCrop (CLIPVOLUME_CROP_EndSide) && !noClipBoundary)
        {
        normal.negate ((DVec3d *) &dMatrix.column[0]);

        convexSet.push_back (ClipPlane (normal, normal.DotProduct (origin) - this->GetWidth(), !displayBoundaryCutPlanes));
        }

    if (this->GetCrop (CLIPVOLUME_CROP_Bottom) && !noClipBoundary)
        {
        convexSet.push_back (ClipPlane (dMatrix.column[2], dMatrix.column[2].DotProduct (origin) + this->GetSize (CLIPVOLUME_SIZE_BottomHeight), !displayBoundaryCutPlanes));
        }

    if (this->GetCrop (CLIPVOLUME_CROP_Top) && !noClipBoundary)
        {
        normal.negate ((DVec3d *)&dMatrix.column[2]);
        convexSet.push_back (ClipPlane (normal, normal.DotProduct (origin) - this->GetSize (CLIPVOLUME_SIZE_TopHeight), !displayBoundaryCutPlanes));
        }
   
    clip = ClipVector::CreateFromPrimitive (ClipPrimitive::CreateFromClipPlanes (ClipPlaneSet (convexSet)));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley  09/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool        SectionClipObject::_GetAuxTransform (TransformR transform, ClipVolumePass pass, DynamicViewSettingsCR settings) const
    {
    switch (pass)
        {
        case ClipVolumePass::InsideForward:
            if (!settings.m_forward.m_flags.m_reflected)
                return false;
            break;

        case ClipVolumePass::InsideBackward:
            if (!settings.m_backward.m_flags.m_reflected)
                return false;
            break;

        default:
            return false;
        }

    DPoint3dVector  pts;
    this->GetPoints (pts, 0, 1);

    DVec3d          reflectDirection;
    this->GetRotationMatrix ().GetColumn (reflectDirection, 1);

    RotMatrix       reflectMatrix;
    reflectMatrix.InitFromDirectionAndScale (reflectDirection, -1.0);
    transform.InitFromMatrixAndFixedPoint (reflectMatrix, pts[0]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 01/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SectionClipObject::_GetTransform (TransformR transform) const
    {
    DVec3d clipX, clipZ;
    this->GetRotationMatrix ().GetColumn (clipX, 0);
    this->GetRotationMatrix ().GetColumn (clipZ, 2);

    if (this->GetSize (CLIPVOLUME_SIZE_FrontDepth) < 0)
        clipZ.Negate ();

    RotMatrix matrix;
    matrix.initFrom2Vectors (&clipX, &clipZ);

    DPoint3dVector pts;
    this->GetPoints (pts, 0, 1);

    transform.InitFrom (matrix, pts[0]);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          SectionClipObject::_GetPrimaryCutPlaneCount() const
    {
    return GetNumPoints() - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool    SectionClipObject::_IsClipVolumePassValid (ClipVolumePass region) const
    {
    if (region != ClipVolumePass::Outside)
        return true;

    // If no cropping then no outside...
    return this->GetCrop (CLIPVOLUME_CROP_Bottom) ||
           this->GetCrop (CLIPVOLUME_CROP_Top) ||
           this->GetCrop (CLIPVOLUME_CROP_StartSide) ||
           this->GetCrop (CLIPVOLUME_CROP_EndSide);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static double getNormalForwardComponent (DVec3dCR normal, DPoint3dCR origin, ViewContextR viewContext)
    {
    DPoint3d        normalTestPoints[2];
    DVec3d          testNormal;

    normalTestPoints[0] = origin;
    normalTestPoints[1].SumOf (normalTestPoints[0],normal);

    viewContext.LocalToView (normalTestPoints, normalTestPoints, 2);
    testNormal.NormalizedDifference (normalTestPoints[1], normalTestPoints[0]);

    return testNormal.z;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    SectionClipObject::GetSidePlane 
(
DPlane3dR               cutPlane, 
DVec3dR                 xDirection,
DVec3dR                 yDirection,
ClipMask&               clipMask,
DRange2dR               clipRange,
size_t                  index,
ViewContextR            viewContext,
DynamicViewSettingsCR   settings
) const
    {
    int                 sideIndex = 0;

    if (settings.ShouldIgnoreBoundaryCutPlanes() || settings.ShouldIgnoreBoundaryClipping() || (!settings.ShouldDisplayForward() && !settings.ShouldDisplayBackward()))
        return ERROR;

    RotMatrixCR         rMatrix = this->GetRotationMatrix();
    DVec3d              cutNormal;
    rMatrix.GetColumn (cutNormal, 1);
  
    // First test for case where the cut plane is parallel to the view (the common case in 2D) and don't bother generating cuts for this as they would
    // be perpendicular to the screen anyway and not worth the expense of generating them as they would appear as lines.
    // TR# 305353 - Make this optimization only when drawing to QVisoin - We can't have it otherwise as the associative point code
    // cannot be view dependent.
    //if (viewContext.GetIViewDraw().IsOutputQuickVision())
    //    {
    //    DPoint3d            testPoints[2];
    //    static double       s_skewTolerance = 1.0E-5;
    //
    //    testPoints[0].Zero ();
    //    testPoints[1] = cutNormal;
    //    viewContext.LocalToView (testPoints, testPoints, 2);
    //    if (testPoints[0].DistanceXY (testPoints[1]) < s_skewTolerance)
    //        return ERROR;
    //    }


    // Next Compute the cutting planes for the start and end sides (only if cropped).
    clipMask = ClipMask::None;
    clipRange.Init();

    rMatrix.GetColumn (yDirection, 2);
    if (this->GetCrop (CLIPVOLUME_CROP_Bottom) && !settings.ShouldIgnoreBoundaryClipping())
        {
        clipMask = clipMask | ClipMask::YLow;
        clipRange.low.y = this->GetSize (CLIPVOLUME_SIZE_BottomHeight);
        }

    if (this->GetCrop (CLIPVOLUME_CROP_Top) && !settings.ShouldIgnoreBoundaryClipping())
        {
        clipMask = clipMask | ClipMask::YHigh;
        clipRange.high.y = this->GetSize (CLIPVOLUME_SIZE_TopHeight);
        }

    double      frontDepth = this->GetSize (CLIPVOLUME_SIZE_FrontDepth);
    DVec3d      frontDirection;

    rMatrix.GetColumn (frontDirection, 1);

    if (frontDepth < 0.0)
        {
        frontDepth = -frontDepth;
        frontDirection.Negate ();
        }


    DPoint3dVector pts;
    this->GetPoints (pts, 0, this->GetNumPoints());

    DPoint3d    startPoint = pts[0], endPoint = pts[this->GetNumPoints()-1];;

    rMatrix.GetColumn (*(&cutPlane.normal), 0);
    cutPlane.origin = pts[this->GetNumPoints()-1];
    xDirection = frontDirection;

    if (!settings.ShouldDisplayForward() || this->GetCrop (CLIPVOLUME_CROP_Front))
        {
        clipMask = clipMask | ClipMask::XHigh;
        clipRange.high.x =  settings.ShouldDisplayForward() ? frontDepth : 0.0;
        }

    if (!settings.ShouldDisplayBackward () || this->GetCrop (CLIPVOLUME_CROP_Back))
        {
        clipMask = clipMask | ClipMask::XLow;
        clipRange.low.x =  settings.ShouldDisplayBackward() ? -this->GetSize (CLIPVOLUME_SIZE_BackDepth) : 0.0;
        }

    if (this->GetCrop (CLIPVOLUME_CROP_StartSide) && index == sideIndex++)
        {
        cutPlane.normal.Negate ();       
        cutPlane.origin = startPoint;
        return SUCCESS;
        }

    double fullRangeXLow = clipRange.low.x, fullRangeXHigh = clipRange.high.x;

    if (this->GetCrop (CLIPVOLUME_CROP_EndSide) && index == sideIndex++)
        {
        cutPlane.origin = endPoint;

        // If there are steps, shift range by offset between first-last step
        if (this->GetNumPoints() > 2)
            {
            double startToEndDepth = xDirection.DotProduct (endPoint) - xDirection.DotProduct (startPoint);
            if (settings.ShouldDisplayForward())
                clipRange.high.x -= startToEndDepth;
            if (settings.ShouldDisplayBackward ())
                clipRange.low.x  -= startToEndDepth;
            }

        return SUCCESS;
        } 

    // Now create the cutting planes for the top and bottom planes. - We reuse the X direction range (and limits) and recompute
    // the Y Direction. Loop over steps and create separate planesets for each step.
    rMatrix.GetColumn (yDirection, 0);

    size_t numPoints = this->GetNumPoints ();
    DPoint3dVector points;
    this->GetPoints (points, 0, numPoints);

    for (size_t plane = 0; plane < numPoints-1; plane+=2)
        {
        // Initialize settings for the current step
        clipMask = clipMask & ~(ClipMask::YLow | ClipMask::YHigh);
        startPoint = points[plane];
        endPoint   = points[plane + 1];
        rMatrix.GetColumn (*(&cutPlane.normal), 2);

        // For each step, shift range by step-depth
        if (0 < plane)
            {
            DPoint3d firstPoint = pts[0];

            double stepFromStart = xDirection.DotProduct (startPoint) - xDirection.DotProduct (firstPoint);
            if (settings.ShouldDisplayForward())
                clipRange.high.x = fullRangeXHigh - stepFromStart;
            if (settings.ShouldDisplayBackward ())
                clipRange.low.x  = fullRangeXLow - stepFromStart;
            }

        // Crop at start: For first plane, look at crop flag. For others, always crop.
        if (0 < plane || this->GetCrop (CLIPVOLUME_CROP_StartSide))
            {
            clipMask = clipMask | ClipMask::YLow;
            clipRange.low.y = 0.0;
            }
        
        // Crop at end: For last plane, look at crop flag. For others, always crop.
        if (plane != (numPoints-2) || this->GetCrop (CLIPVOLUME_CROP_EndSide))
            {
            clipMask = clipMask | ClipMask::YHigh;
            clipRange.high.y = yDirection.DotProduct (endPoint) - yDirection.DotProduct (startPoint);
            }

        if (this->GetCrop (CLIPVOLUME_CROP_Bottom) && index == sideIndex++)
            {
            cutPlane.origin.SumOf (startPoint,cutPlane.normal, this->GetSize (CLIPVOLUME_SIZE_BottomHeight));
            cutPlane.normal.Negate ();       
            return SUCCESS;
            }

        if (this->GetCrop (CLIPVOLUME_CROP_Top) && index == sideIndex++)
            {
            cutPlane.origin.SumOf (startPoint,cutPlane.normal, this->GetSize (CLIPVOLUME_SIZE_TopHeight));
            return SUCCESS;
            } 
        }
    
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   SectionClipObject::_GetCuttingPlane
(
DPlane3dR               cutPlane,
DVec3dR                 xDirection,
DVec3dR                 yDirection,
ClipMask&               clipMask,
DRange2dR               clipRange,
bool&                   isForwardFacing,
int                     index,
ViewContextR            viewContext,
DynamicViewSettingsCR   settings
) const
    {
    double          segmentLength;
    static double   s_forwardFacingNormalTolerance = 1.0E-5;

    size_t             nPoints = this->GetNumPoints();
    if (index >= static_cast <int> (nPoints - 1))
        {
        StatusInt       status;

        // TR# 332334 - forwardFacing was not set for side planes...
        if (SUCCESS == (status = GetSidePlane (cutPlane, xDirection, yDirection, clipMask, clipRange, index - nPoints + 1, viewContext, settings)))
            isForwardFacing = getNormalForwardComponent (cutPlane.normal, cutPlane.origin, viewContext) > s_forwardFacingNormalTolerance;       // Note - getSidePlane returns outward facing normals.

        return status;
        }

    DPoint3dVector pts;
    if (SUCCESS != this->GetPoints (pts, index, 2))
        return ERROR;

    DPoint3d        point0 = pts[0], point1 = pts[1];

    if (0.0 == (segmentLength = xDirection.NormalizedDifference (point1, point0)))
        return ERROR;

    this->GetRotationMatrix ().GetColumn (yDirection, 2);

    DVec3d      directedY = yDirection;
    if (this->GetSize (CLIPVOLUME_SIZE_FrontDepth) < 0)
        directedY.Negate ();

    isForwardFacing = true;
    if (settings.ShouldDisplayForward())
        {
        if (settings.ShouldDisplayBackward() && !settings.ShouldReflectBackward())
            isForwardFacing = false;
        }
    else if (settings.ShouldDisplayBackward() && !settings.ShouldReflectBackward())
        {
        directedY.Negate ();
        }

    cutPlane.normal.NormalizedCrossProduct (directedY, xDirection);
    cutPlane.origin = point0;

    // We'll set forward visible if all planes are front facing.  - Forward visibile
    // means nothing obscures the plane. - if all are front facing then nothing can be hidden
    DPoint3dVector        segPoints;
    for (int i=0; isForwardFacing && SUCCESS ==  this->GetPoints (segPoints, i, 2); i++)
        {
        DVec3d      testNormal, testXDirection;

        if (0.0 == testXDirection.NormalizedDifference (segPoints[1], segPoints[0]))      // TR# 330768 - xDirection was not being recalculated for each segment.
            continue;

        testNormal.NormalizedCrossProduct (directedY, testXDirection);

        if (getNormalForwardComponent (testNormal, segPoints[0], viewContext) > s_forwardFacingNormalTolerance) // These cutting planes are inward facing. (Unlike the side planes).
            {
            isForwardFacing = false;
            break;
            }
        }

    clipMask = ClipMask::None;
    clipRange.Init();

    if (this->GetCrop (CLIPVOLUME_CROP_Bottom) && !settings.ShouldIgnoreBoundaryClipping())
        {
        clipMask = clipMask | ClipMask::YLow;
        clipRange.low.y = this->GetSize (CLIPVOLUME_SIZE_BottomHeight);
        }

    if (this->GetCrop (CLIPVOLUME_CROP_Top) && !settings.ShouldIgnoreBoundaryClipping())
        {
        clipMask = clipMask | ClipMask::YHigh;
        clipRange.high.y = this->GetSize (CLIPVOLUME_SIZE_TopHeight);
        }

    bool        isFirstSegment = (0 == index), isLastSegment = (index == nPoints-2);
    if ((this->GetCrop (CLIPVOLUME_CROP_StartSide) || this->GetCrop (CLIPVOLUME_CROP_EndSide)) && !settings.ShouldIgnoreBoundaryClipping ())
        {
        DPoint3d        origin, minPoint, maxPoint;
        DVec3d          direction;

        this->GetRotationMatrix ().GetColumn (direction, 0);

        DPoint3dVector        pts;
        this->GetPoints (pts, 0, 1);

        origin = pts[0];

        minPoint = origin;
        maxPoint.SumOf (origin,direction, this->GetWidth ());

        double          param;
        DPoint3d        intersection;

        if (this->GetCrop (CLIPVOLUME_CROP_StartSide) &&
            bsiGeom_linePlaneIntersection (&param, &intersection, &point0, &point1, &minPoint, &direction) &&
            (isFirstSegment || param > 0.0) &&
            (isLastSegment  || param < 1.0))
            {
            clipMask = clipMask | ClipMask::XLow;
            clipRange.low.x = param * segmentLength;
            }
        
        if (this->GetCrop (CLIPVOLUME_CROP_EndSide) &&
            bsiGeom_linePlaneIntersection (&param, &intersection, &point0, &point1, &maxPoint, &direction) &&
            (0 == index || param > 0.0) &&
            (index == nPoints-2 || param < 1.0))
            {
            clipMask = clipMask | ClipMask::XHigh;
            clipRange.high.x = param * segmentLength;
            }
        }
    if (!isFirstSegment && (ClipMask::None == (clipMask & ClipMask::XLow)))
        {
        clipMask = clipMask | ClipMask::XLow;
        clipRange.low.x = 0.0;
        }
    if (!isLastSegment && (ClipMask::None== (clipMask & ClipMask::XHigh)))
        {
        clipMask = clipMask | ClipMask::XHigh;
        clipRange.high.x = segmentLength;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/07
+---------------+---------------+---------------+---------------+---------------+------*/
DynamicViewSettings::DynamicViewSettings () // graphite moved this here from ViewInfo.cpp
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DynamicViewSettings::Init() // graphite moved this here from ViewInfo.cpp
    {
    memset (this, 0, sizeof (*this));

    m_forward.m_styleIndex              = -1;
    m_backward.m_styleIndex             = -1;
    m_outside.m_styleIndex              = -1;
    m_cut.m_styleIndex                  = -1;

    m_forward.m_flags.m_display         = true;
    m_backward.m_flags.m_display        = true;

    m_outside.m_flags.m_disableLocate   = true;
    m_outside.m_flags.m_disableSnap     = true;

    //m_displayStyleIndex                 = -1;
    m_levelOfDetail                     = 1.0;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IViewClipObjectPtr SectionClipObjectFactory::_FromJson (JsonValueCR json)
    {
#if defined (NEEDS_WORK_DGNITEM)
    auto clip = new SectionClipObject;
    clip->_FromJson (json);
    return clip;
#endif
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SectionClipObject::_GetFactoryId() const {return SectionClipObjectFactory::GetFactoryId();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IViewClipObjectPtr SectionClipObjectLegacyData::CreateObject()
    {
#if defined (NEEDS_WORK_DGNITEM)
    auto clip = new SectionClipObject;
    clip->FromLegacyData (*this);
    return clip;
#endif
    return nullptr;
    }

#define CLIP_FACTORY_GUARDED    BeSystemMutexHolder guard;
static bmap<Utf8String,RefCountedPtr<IViewClipObject::Factory>> s_factories;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<IViewClipObject::Factory> IViewClipObject::Factory::RegisterFactory (Utf8StringCR factoryId, Factory& factory)
    {
    CLIP_FACTORY_GUARDED;
    RefCountedPtr<IViewClipObject::Factory> was (&factory);
    std::swap (s_factories[factoryId], was);
    return was;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<IViewClipObject::Factory> IViewClipObject::Factory::GetFactory (Utf8StringCR factoryId)
    {
    CLIP_FACTORY_GUARDED;
    auto i = s_factories.find (factoryId);
    return (i==s_factories.end())? NULL: i->second;
    }

/*---------------------------------------------------------------------------------**//**
* Store the state of this clip object as JSon. The format is:
*   json[factoryId] = object state
* Note that we uuse the factoryId as the member name.
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/void IViewClipObject::Factory::ToJson (JsonValueR json, IViewClipObject const& clip)
    {
    Json::Value clipJson (Json::objectValue);
    clip._ToJson (clipJson);
    json[clip._GetFactoryId().c_str()] = clipJson;  // factoryId is member name
    }

/*---------------------------------------------------------------------------------**//**
* A valid clip object JSon object is of the form:
*   json[factoryid] = object state
* That is, there is only one property, and it's name is the factoryId.
* @bsimethod                                    Sam.Wilson      06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/IViewClipObjectPtr IViewClipObject::Factory::FromJson (JsonValueCR json)
    {
    auto memberNames = json.getMemberNames ();
    if (memberNames.size() != 1)
        {
        BeAssert (false);
        return NULL;
        }

    auto& factoryId = memberNames[0];               // factoryId is member name
    RefCountedPtr<IViewClipObject::Factory> factory = GetFactory (factoryId);
    if (!factory.IsValid())
        {
        BeAssert (false);
        return NULL;
        }

    auto& value = json[factoryId.c_str()];
    if (!value.isObject())
        {
        BeAssert (false);
        return NULL;
        }

    return factory->_FromJson (value);
    }

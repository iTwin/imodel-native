/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/ElemRangeCalc.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/


BEGIN_BENTLEY_DGN_NAMESPACE

struct                     ElemRangeCalc;
struct                     RangeClipPlaneSet;
struct                     ClipStack;
typedef ClipStack const*   ClipStackCP;


/*=================================================================================**//**
* @bsimethod                                                    Ray.Bentley   09/06
+===============+===============+===============+===============+===============+======*/
struct          RangeClipPlanes 
{
private:
    ConvexClipPlaneSet      m_planes;

public:

    RangeClipPlanes(size_t  nPlanes, ClipPlaneCP  planes) : m_planes(planes, nPlanes) { }

    void ClipPoints(ElemRangeCalc*, ClipStackCP, int, DPoint3dCP,  size_t clipIndex, size_t planeIndex = 0) const;
    bool ClipRange(ElemRangeCalc*, ClipStackCP, DPoint3d corners[8], size_t clipIndex, bool fastClip) const;
};


/*=================================================================================**//**
* @bsimethod                                                    Ray.Bentley   09/06
+===============+===============+===============+===============+===============+======*/
struct          RangeClip
{
private:
    bvector <RangeClipPlanes>       m_planeSets;
    Transform                       m_transform;
    bool                            m_isCamera;
    DPoint3d                        m_camera;
    double                          m_focalLength;

public:
                   bool    IsEmpty() const { return 0 == m_planeSets.size(); }
                   bool    IsCamera() const { return m_isCamera; }
DGNPLATFORM_EXPORT     void    ClipPoints(ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, int numPoints, DPoint3dCP points, size_t clipIndex) const;
DGNPLATFORM_EXPORT     void    ClipRange(ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, DPoint3dCP, size_t clipIndex, bool fastClip) const;
                   void    ClipEllipse(ElemRangeCalc* rangeCalculator, ClipStackCP clipStack, DEllipse3dCR ellipse, size_t clipIndex) const;
                   void    ApplyTransform(DPoint3dP transformedPoints, DPoint3dCP points, int numPoints) const;

RangeClip(ClipPlaneSetCP pClip, TransformCP pTransform);
RangeClip(DPoint3dCR camera, double focalLength);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct          ClipStack
{
private:
    bvector <RangeClip> m_clips;

public:

            ClipStack()    { }
    void    Clear()        { m_clips.clear(); }
    bool    ContainsCamera() const;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void Pop()
    {
    if (m_clips.empty())
        {
        BeAssert(false && "attempt to pop empty clipstack");
        return;
        }
    m_clips.pop_back();
    while (!m_clips.empty() && m_clips.back().IsCamera())
        m_clips.pop_back();
    }

void Push(ClipPlaneSetCP pClip, TransformCP pTransform)
    {
    m_clips.push_back(RangeClip(pClip, pTransform));
    }

void Push(DPoint3dCR cameraPosition, double focalLength)
    {
    m_clips.push_back(RangeClip(cameraPosition, focalLength));
    }

DGNPLATFORM_EXPORT    void ClipPoints(ElemRangeCalc* rangeCalculator, int numPoints, DPoint3dCP pPoints, size_t clipIndex = 0) const;
DGNPLATFORM_EXPORT    void ClipRange(ElemRangeCalc* rangeCalculator, DPoint3dCP corners, size_t clipIndex = 0, bool fastClip = false) const;
                  void ClipEllipse(ElemRangeCalc* rangeCalculator, DEllipse3dCR ellipse, size_t clipIndex = 0) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ElemRangeCalc
{
private:
    DRange3d m_range;

public:
    bool IsValid() {return !m_range.IsNull();}
    void Invalidate() {m_range.Init();}

    ElemRangeCalc() {Invalidate();}
    DGNPLATFORM_EXPORT StatusInt GetRange(DRange3dR range);
    void SetRange(DRange3dCR range) {m_range = range;}

    DGNPLATFORM_EXPORT void Union(int numPoints, DPoint3dCP points, ClipStackCP currClip);
    DGNPLATFORM_EXPORT void Union(int numPoints, DPoint2d const* points, ClipStackCP currClip);
    DGNPLATFORM_EXPORT void Union(DRange3d const* in, ClipStackCP currClip);
    DGNPLATFORM_EXPORT void Union(DEllipse3d const* ellipse, ClipStackCP currClip);
    DGNPLATFORM_EXPORT StatusInt ToScanRange(AxisAlignedBox3dR range, bool is3d);
};

/*=================================================================================**//**
* Context to calculate the range of an element.
* @bsiclass                                                     KeithBentley    01/02
+===============+===============+===============+===============+===============+======*/
struct RangeGraphic : SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
private:
    ElemRangeCalc m_elRange;

public:
    RangeGraphic() {}

    ElemRangeCalc* GetElemRange() {return &m_elRange;}

    void Init(ViewContextP context);
    void      UpdateRange(int numPoints, DPoint3dCP points);
    void      UpdateRange(int numPoints, DPoint2dCP points);
    void      UpdateRange(DEllipse3dCP ellipse);
    StatusInt _ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled) override;
    StatusInt _ProcessSolidPrimitive(ISolidPrimitiveCR primitive) override;
    void      _DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override;
    StatusInt _DrawBody(ISolidKernelEntityCR entity, double) override;
    void      _DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) override;
    void      _DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) override;
    void      _DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool fill, double zDepth, DPoint2dCP range) override;
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    void      _DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range) override;
#endif
    void      _DrawTextString(TextStringCR text, double* zDepth) override;
    void      _DrawPolyface(PolyfaceQueryCR meshData, bool filled = false) override;
};

END_BENTLEY_DGN_NAMESPACE


/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/PickContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
// __BENTLEY_INTERNAL_ONLY__

#include    "Locate.h"
#include    <DgnPlatform/SimplifyGraphic.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum TestLStylePhase
    {
    TEST_LSTYLE_None        = 0,
    TEST_LSTYLE_Component   = 1,
    TEST_LSTYLE_BaseGeom    = 2,
    };

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct PickContext : ViewContext, IPickGeom, IGeometryProcessor
{
    DEFINE_T_SUPER(ViewContext)
private:
    bool              m_doneSearching;
    bool              m_unusableLStyleHit;
    bool              m_doLocateSilhouettes;
    bool              m_doLocateInteriors;
    TestLStylePhase   m_testingLStyle;
    GeomDetail        m_currGeomDetail;
    HitListP          m_hitList;
    HitPriority       m_hitPriorityOverride;
    DPoint3d          m_pickPointWorld;
    DPoint4d          m_pickPointView;
    double            m_pickAperture;
    double            m_pickApertureSquared;
    LocateOptions     m_options;
    StopLocateTest*   m_stopTester;
    GeometrySourceCP  m_currentGeomSource;

    virtual void _OutputGeometry(GeometrySourceCR) override;
    virtual bool _CheckStop() override;
    virtual StatusInt _VisitDgnModel(DgnModelP inDgnModel) override;
    virtual void _DrawAreaPattern(ViewContext::ClipStencil& boundary) override;
    virtual void _DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false) override;
    virtual void _DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed = false) override;
    virtual void _DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) override;
    virtual void _DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) override;
    virtual void _DrawStyledBSplineCurve3d(MSBsplineCurveCR) override;
    virtual void _DrawStyledBSplineCurve2d(MSBsplineCurveCR, double zDepth) override;
    virtual Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {_GetGeomDetail().Init(); SimplifyGraphic* graphic = new SimplifyGraphic(params, *this, *this); return graphic;}

    bool TestPoint(DPoint3dCR localPt, HitPriority priority, SimplifyGraphic const&);
    bool TestPointArray(size_t numPts, DPoint3dCP localPts, HitPriority priority, SimplifyGraphic const&);
    bool TestCurveVector(CurveVectorCR, HitPriority, SimplifyGraphic const&);
    bool TestCurveVectorInterior(CurveVectorCR, HitPriority priority, SimplifyGraphic const&);
    bool TestIndexedPolyEdge(DPoint3dCP verts, DPoint4dCP hVertsP, int closeVertexId, int segmentVertexId, DPoint3dR pickPt, HitPriority, SimplifyGraphic const&);

    void InitNpcSubRect(DPoint3dCR pickPointWorld, double pickAperture, DgnViewportR viewport);
    void InitSearch(DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP);
    bool PointWithinTolerance(DPoint4dCR testPt);
    DRay3d GetBoresite(SimplifyGraphic const&) const;

    void AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority, SimplifyGraphic const&);
    void AddSurfaceHit(DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority, SimplifyGraphic const&);

    virtual UnhandledPreference _GetUnhandledPreference(CurveVectorCR) const override {return UnhandledPreference::Curve;}
    virtual UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR) const override {return UnhandledPreference::Curve;}
    virtual UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR) const override {return UnhandledPreference::Curve;}

    virtual bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic const&) override;
    virtual bool _ProcessCurveVector(CurveVectorCR, bool filled, SimplifyGraphic const&) override;
    virtual bool _ProcessSolidPrimitive(ISolidPrimitiveCR, SimplifyGraphic const&) override;
    virtual bool _ProcessSurface(MSBsplineSurfaceCR, SimplifyGraphic const&) override;
    virtual bool _ProcessPolyface(PolyfaceQueryCR, bool filled, SimplifyGraphic const&) override;
    virtual bool _ProcessBody(ISolidKernelEntityCR, SimplifyGraphic const&) override;
    virtual bool _ProcessTextString(TextStringCR, SimplifyGraphic const&) override;

public:
    double GetPickAperture() {return m_pickAperture;}
    HitListP GetHitList() {return m_hitList;}
    bool GetDoneSearching() {return m_doneSearching;}
    void SetTestLStylePhase(TestLStylePhase phase) {m_testingLStyle = phase; if (TEST_LSTYLE_None == phase) m_unusableLStyleHit = false;}
    DPoint3dP GetProjectedPickPointView(DPoint3dR pPoint);
    void InitStrokeForCache() {m_doLocateSilhouettes = false;}
    bool GetLocateSilhouettes() {return m_doLocateSilhouettes;}
    bool* GetLocateInteriors() {return &m_doLocateInteriors;}

    virtual DPoint4dCR _GetPickPointView() const override {return m_pickPointView;}
    virtual DPoint3dCR _GetPickPointWorld() const override {return m_pickPointWorld;}
    virtual GeomDetailR _GetGeomDetail() override {return m_currGeomDetail;}
    virtual void _SetHitPriorityOverride(HitPriority priority) override {m_hitPriorityOverride = priority;}
    virtual bool _IsSnap() const override;
    virtual void _AddHit(HitDetailP) override;
    virtual DRay3d _GetBoresite(TransformCR localToWorld) const override;

    DGNPLATFORM_EXPORT PickContext(LocateOptions const& options, StopLocateTest* stopTester=NULL);
    DGNPLATFORM_EXPORT bool PickElements(DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureDevice, HitListP hitList);
    DGNPLATFORM_EXPORT TestHitStatus TestHit(HitDetailCR, DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList);
    DGNPLATFORM_EXPORT static void InitBoresite(DRay3dR boresite, DPoint3dCR spacePoint, DMatrix4dCR worldToLocal);
};

END_BENTLEY_DGN_NAMESPACE


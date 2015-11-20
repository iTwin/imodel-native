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
#include    <DgnPlatform/SimplifyViewDrawGeom.h>

BEGIN_BENTLEY_DGN_NAMESPACE

enum TestLStylePhase
    {
    TEST_LSTYLE_None        = 0,
    TEST_LSTYLE_Component   = 1,
    TEST_LSTYLE_BaseGeom    = 2,
    };

/*=================================================================================**//**
* Output to determine if element should be accepted for fence processing..
* @bsiclass                                                     Brien.Bastings  09/04
+===============+===============+===============+===============+===============+======*/
struct PickOutput : SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)

private:
    struct PickContext& m_pick;

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    static int LocateQvElemCheckStop(CallbackArgP);
#endif
    virtual StatusInt _ProcessCurvePrimitive(ICurvePrimitiveCR prim, bool closed, bool filled) override;
    virtual StatusInt _ProcessCurveVector(CurveVectorCR vector, bool isFilled) override;
    virtual StatusInt _ProcessSolidPrimitive(ISolidPrimitiveCR prim) override;
    virtual StatusInt _ProcessSurface(MSBsplineSurfaceCR surface) override;
    virtual StatusInt _ProcessFacetSet(PolyfaceQueryCR query, bool filled) override;
    virtual StatusInt _ProcessBody(ISolidKernelEntityCR entity) override;

protected:
    virtual void _AddTextString(TextStringCR, double* zDepth) override;
    virtual bool _DoClipping() const override {return m_inSymbolDraw;} // Only need clip for symbols...
    virtual bool _DoSymbolGeometry() const override {return true;}
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    virtual bool _DrawSprite(Render::ISpriteP sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency) override;
    virtual void _AddPointCloud(Render::IPointCloudDrawParams* drawParams) override;
#endif

public:
    PickOutput(struct PickContext& pick);
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct PickContext : ViewContext, IPickGeom
{
    DEFINE_T_SUPER(ViewContext)
private:
    friend struct PickOutput;

    PickOutput        m_graphic;
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

    virtual bool _CheckStop() override;
    virtual StatusInt _VisitDgnModel(DgnModelP inDgnModel) override;
    virtual void _OutputElement(GeometrySourceCR) override;
    virtual void _DrawAreaPattern(ViewContext::ClipStencil& boundary) override;
    virtual ILineStyleCP _GetCurrLineStyle(Render::LineStyleSymbP*) override;
    virtual void _DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false) override;
    virtual void _DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed = false) override;
    virtual void _DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) override;
    virtual void _DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) override;
    virtual void _DrawStyledBSplineCurve3d(MSBsplineCurveCR) override;
    virtual void _DrawStyledBSplineCurve2d(MSBsplineCurveCR, double zDepth) override;
    virtual void  _OnPreDrawTransient() override;
    virtual Render::GraphicPtr _BeginGraphic(Render::Graphic::CreateParams const& params) override {return &m_graphic;}

    bool TestPoint(DPoint3dCR localPt, HitPriority priority);
    bool TestPointArray(size_t numPts, DPoint3dCP localPts, HitPriority priority);
    bool TestCurveVector(CurveVectorCR, HitPriority);
    bool TestCurveVectorInterior(CurveVectorCR, HitPriority priority);
    bool TestIndexedPolyEdge(DPoint3dCP verts, DPoint4dCP hVertsP, int closeVertexId, int segmentVertexId, DPoint3dR pickPt, HitPriority);
    bool TestGraphics(Render::Graphic* qvElem, HitPriority);
    void InitNpcSubRect(DPoint3dCR pickPointWorld, double pickAperture, DgnViewportR viewport);
    void InitSearch(DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP);
    bool PointWithinTolerance(DPoint4dCR testPt);
    void AddSurfaceHit(DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority);

    StatusInt ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled);
    StatusInt ProcessCurveVector(CurveVectorCR, bool isFilled);
    StatusInt ProcessSolidPrimitive(ISolidPrimitiveCR);
    StatusInt ProcessSurface(MSBsplineSurfaceCR);
    StatusInt ProcessFacetSet(PolyfaceQueryCR, bool filled);
    StatusInt ProcessBody(ISolidKernelEntityCR);

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
    virtual bool _IsPointVisible(DPoint3dCP screenPt) override;
    virtual void _SetHitPriorityOverride(HitPriority priority) override {m_hitPriorityOverride = priority;}
    virtual void _AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority) override;
    virtual bool _IsSnap() const;
    virtual DRay3d _GetBoresite() const;

    DGNPLATFORM_EXPORT PickContext(LocateOptions const& options, StopLocateTest* stopTester=NULL);
    DGNPLATFORM_EXPORT bool PickElements(DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureDevice, HitListP hitList);
    DGNPLATFORM_EXPORT TestHitStatus TestHit(HitDetailCR, DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList);
    DGNPLATFORM_EXPORT static void InitBoresite(DRay3dR boresite, DPoint3dCR spacePoint, DMatrix4dCR worldToLocal);
};


END_BENTLEY_DGN_NAMESPACE


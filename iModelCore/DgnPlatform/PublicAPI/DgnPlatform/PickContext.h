/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/PickContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include  <DgnPlatform/SimplifyGraphic.h>

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
enum class TestHitStatus
{
    NotOn       = 0,
    IsOn        = 1,
    Aborted     = 2
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/10
//=======================================================================================
struct StopLocateTest
{
    virtual ~StopLocateTest(){}
    virtual bool _CheckStopLocate() = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  10/14
+===============+===============+===============+===============+===============+======*/
struct LocateOptions
{
private:
    bool m_disableDgnDbFilter = false;
    bool m_allowTransients = false;
    uint32_t m_maxHits;
    HitSource m_hitSource;
    LocateSurfacesPref m_locateSurface = LocateSurfacesPref::ByView;

public:
    LocateOptions()
        {
        m_maxHits = 20;
        m_hitSource = HitSource::DataPoint;
        }

    LocateOptions(HitSource hitSource, uint32_t maxHits)
        {
        m_hitSource = hitSource;
        m_maxHits = maxHits;
        }

    void SetDisableDgnDbFilter(bool disableDgnDbFilter) {m_disableDgnDbFilter = disableDgnDbFilter;}
    void SetAllowTransients(bool allowTransients) {m_allowTransients = allowTransients;}
    void SetMaxHits(uint32_t maxHits) {m_maxHits = maxHits;}
    void SetHitSource(HitSource hitSource) {m_hitSource = hitSource;}
    void SetLocateSurfaces(LocateSurfacesPref locateSurface) {m_locateSurface = locateSurface;}

    bool GetDisableDgnDbFilter() const {return m_disableDgnDbFilter;}
    bool GetAllowTransients() const {return m_allowTransients;}
    uint32_t GetMaxHits() const {return m_maxHits;}
    HitSource GetHitSource() const {return m_hitSource;}
    LocateSurfacesPref GetLocateSurfaces() const {return m_locateSurface;}
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct PickContext : ViewContext, IPickGeom, IGeometryProcessor
{
    DEFINE_T_SUPER(ViewContext)
    friend struct SheetAttachmentPicker;

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
    double            m_pixelScale = 1.0; // only changes for sheet attachments
    uint32_t          m_overflowCount;
    LocateOptions     m_options;
    StopLocateTest*   m_stopTester;
    DgnViewportP      m_sheetViewport = nullptr;
    GeometrySourceCP  m_currentGeomSource;
    IElemTopologyCPtr m_currElemTopo;

    void SetPickAperture(double val) {m_pickAperture=val; m_pickApertureSquared=val*val;}
    IPickGeomP _GetIPickGeom () override {return this;}
    StatusInt _OutputGeometry(GeometrySourceCR) override;
    bool _CheckStop() override;
    StatusInt _InitContextForView() override;
    StatusInt _VisitDgnModel(GeometricModelR inDgnModel) override;
    void _DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params) override;
    void _DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false) override;
    void _DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) override;
    void _DrawStyledBSplineCurve2d(MSBsplineCurveCR, double zDepth) override;
    Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {_GetGeomDetail().Init(); SimplifyGraphic* graphic = new SimplifyGraphic(params, *this, *this); return graphic;}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, TransformCP, ClipVectorCP) override {return new SimplifyGraphic(Render::Graphic::CreateParams(), *this, *this);}
    DPoint4d ConvertLocalToView(DPoint3dCR localPt, SimplifyGraphic const& graphic) const;
    DPoint3d ConvertViewToLocal(DPoint4dCR viewPt, SimplifyGraphic const& graphic) const;

    bool TestPoint(DPoint3dCR localPt, HitPriority priority, SimplifyGraphic const&);
    bool TestPointArray(size_t numPts, DPoint3dCP localPts, HitPriority priority, SimplifyGraphic const&);
    bool TestArcCenterAndPointString(ICurvePrimitiveCR primitive, SimplifyGraphic const&);
    bool TestCurveVector(CurveVectorCR, HitPriority, SimplifyGraphic const&);
    bool TestCurveVectorInterior(CurveVectorCR, HitPriority priority, SimplifyGraphic const&);

    void InitNpcSubRect(DPoint3dCR pickPointWorld, double pickAperture, DgnViewportCR viewport);
    void InitSearch(DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP);
    bool PointWithinTolerance(DPoint4dCR testPt);
    DRay3d GetBoresite(SimplifyGraphic const&) const;

    void AddHit(DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority, SimplifyGraphic const&);
    void AddSurfaceHit(DPoint3dCR hitPtLocal, DVec3dCR hitNormalLocal, HitPriority, SimplifyGraphic const&);

    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}
    UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Curve;}

    bool _ProcessCurvePrimitive(ICurvePrimitiveCR, bool closed, bool filled, SimplifyGraphic&) override;
    bool _ProcessCurveVector(CurveVectorCR, bool filled, SimplifyGraphic&) override;
    bool _ProcessSolidPrimitive(ISolidPrimitiveCR, SimplifyGraphic&) override;
    bool _ProcessSurface(MSBsplineSurfaceCR, SimplifyGraphic&) override;
    bool _ProcessPolyface(PolyfaceQueryCR, bool filled, SimplifyGraphic&) override;
    bool _ProcessBody(IBRepEntityCR, SimplifyGraphic&) override;
    bool _ProcessTextString(TextStringCR, SimplifyGraphic&) override;

public:
    double GetPickAperture() {return m_pickAperture;}
    HitListP GetHitList() {return m_hitList;}
    bool GetDoneSearching() {return m_doneSearching;}
    void SetTestLStylePhase(TestLStylePhase phase) {m_testingLStyle = phase; if (TEST_LSTYLE_None == phase) m_unusableLStyleHit = false;}
    DPoint3dP GetProjectedPickPointView(DPoint3dR pPoint);
    void InitStrokeForCache() {m_doLocateSilhouettes = false;}
    bool GetLocateSilhouettes() {return m_doLocateSilhouettes;}
    bool* GetLocateInteriors() {return &m_doLocateInteriors;}

    bool _IsSnap() const override;
    DPoint4dCR _GetPickPointView() const override {return m_pickPointView;}
    DPoint3dCR _GetPickPointWorld() const override {return m_pickPointWorld;}
    DRay3d _GetBoresite(TransformCR localToWorld) const override;
    void _SetHitPriorityOverride(HitPriority priority) override {m_hitPriorityOverride = priority;}
    GeomDetailR _GetGeomDetail() override {return m_currGeomDetail;}
    void _AddHit(HitDetailR) override;
    IElemTopologyCP _GetElemTopology() const override {return (m_currElemTopo.IsValid() ? m_currElemTopo.get() : nullptr);}
    void _SetElemTopology(IElemTopologyCP topo) override {m_currElemTopo = topo;}
    virtual bool _ProcessSheetAttachment(TileViewport&);

    DGNVIEW_EXPORT PickContext(LocateOptions const& options, StopLocateTest* stopTester=nullptr);
    DGNVIEW_EXPORT bool PickElements(DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureDevice, HitListP hitList);
    DGNVIEW_EXPORT TestHitStatus TestHit(HitDetailCR, DgnViewportR, DPoint3dCR pickPointWorld, double pickApertureScreen, HitListP hitList);
    static void InitBoresite(DRay3dR boresite, DPoint3dCR localPoint, DMatrix4dCR viewToLocal);
    static void InitBoresite(DRay3dR boresite, DPoint3dCR worldPoint, DgnViewportCR vp, TransformCP localToWorld=nullptr);
    static void InitBoresite(DRay3dR boresite, DgnButtonEventCR ev, TransformCP localToWorld=nullptr);
};

END_BENTLEY_DGN_NAMESPACE

